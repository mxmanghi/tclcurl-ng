# http_application.tcl --
#
# Application layer for the TclCurl HTTP test server.
#
# Copyright (c) 2024-2026 Massimo Manghi
#
# SPDX-License-Identifier: TCL
#
# See the file "license.terms" at the top level of this distribution
# for information on usage and redistribution of this file, and for the
# complete disclaimer of warranties and limitation of liability.

namespace eval ::tclcurl::testserver {}

package require sha256
package require zlib
package require base64
package require uri

if {[info commands ::tclcurl::testserver::CApplication] eq {}} {
    # Abstract application model for HTTP origin services. The service keeps
    # ownership of connection acceptance and low-level socket operations,
    # while application objects decide how a fully buffered request should be
    # interpreted and what response should be generated. The current
    # `service_request` entry point is synchronous, but it already matches the
    # future worker-thread handoff boundary.
    oo::class create ::tclcurl::testserver::CApplication {
        method escape_response_value {value} {
            return [string map [list "\\" "\\\\" "\n" "\\n" "\r" "\\r"] $value]
        }

        method decode_query_component {value} {
            set decoded [string map [list + " "] $value]
            while {[regexp -indices {%[0-9A-Fa-f][0-9A-Fa-f]} $decoded match]} {
                lassign $match first last
                set replacement [format %c 0x[string range $decoded [expr {$first + 1}] $last]]
                set decoded     [string replace $decoded $first $last $replacement]
            }
            return $decoded
        }

        method parse_query {target} {
            set uri_parts [::uri::split "http://localhost$target"]
            if {![dict exists $uri_parts query]} {
                return [dict create]
            }

            set query [dict get $uri_parts query]
            if {$query eq {}} {
                return [dict create]
            }

            set params [dict create]
            foreach pair [split $query &] {
                if {$pair eq {}} {
                    continue
                }
                set separator [string first = $pair]
                if {$separator < 0} {
                    dict set params [my decode_query_component $pair] {}
                    continue
                }
                dict set params \
                    [my decode_query_component [string range $pair 0 [expr {$separator - 1}]]] \
                    [my decode_query_component [string range $pair   [expr {$separator + 1}] end]]
            }

            return $params
        }

        method guess_content_type {path} {
            switch -exact -- [string tolower [file extension $path]] {
                .css  { return "text/css; charset=utf-8" }
                .gif  { return "image/gif" }
                .htm -
                .html { return "text/html; charset=utf-8" }
                .jpg -
                .jpeg { return "image/jpeg" }
                .js   { return "application/javascript; charset=utf-8" }
                .json { return "application/json; charset=utf-8" }
                .md   { return "text/markdown; charset=utf-8" }
                .png  { return "image/png" }
                .svg  { return "image/svg+xml; charset=utf-8" }
                .tcl  { return "text/plain; charset=utf-8" }
                .txt  { return "text/plain; charset=utf-8" }
                .xml  { return "application/xml; charset=utf-8" }
                default {
                    return "application/octet-stream"
                }
            }
        }

        # Mock worker entry point. The service passes the channel and the full
        # request here today; later the same method can move the channel and
        # buffered bytes to a worker thread.
        method service_request {service chan request} {
            set request_info [$service parse_request_line $request]
            if {$request_info eq {}} {
                $service log_request "method=? status=400 path=?"
                $service send_response $chan [$service bad_request_response]
                return
            }

            dict with request_info {}
            set path [lindex [split $target ?] 0]
            ::tclcurl::test::msgoutput "route request method=$method path=$path"
            set headers [$service parse_headers $request]
            set response [my route_request $service $method $path $target $version $headers $request]
            dict set response head_only [expr {$method eq "HEAD"}]
            set response [$service build_response_dict $response]
            $service log_request \
                "method=$method status=[dict get $response status] path=[::tclcurl::testserver::log_value $path]"
            if {[dict exists $response delay_ms]} {
                set callback [list $service send_response $chan $response]
                if {[dict exists $response close_only] && [dict get $response close_only]} {
                    set callback [list $service close_client $chan]
                }
                after [dict get $response delay_ms] $callback
                return
            }
            $service send_response $chan $response
        }

        method request_body {service request} {
            set header_end [string first "\r\n\r\n" $request]
            if {$header_end < 0} {
                return {}
            }

            set headers [$service parse_headers $request]
            if {[string tolower [$service header_value $headers transfer-encoding]] eq "chunked"} {
                return [$service decode_chunked_body [string range $request [expr {$header_end + 4}] end]]
            }

            return [string range $request [expr {$header_end + 4}] end]
        }

        method redirect_response {location {reason "Found"}} {
            set body "redirect=$location\n"
            return [dict create status 302      \
                                reason $reason  \
                                body $body      \
                                headers [list "Location: $location"]]
        }

        method static_file_response {path} {
            if {$path eq {}} {
                return {}
            }

            if {$path eq "/"} {
                set relative_segments [list index.html]
            } else {
                set relative_segments {}
                foreach segment [split [string trimleft $path /] /] {
                    if {$segment eq {} || $segment eq "." || $segment eq ".."} {
                        return {}
                    }
                    lappend relative_segments $segment
                }
            }

            set doc_root [::tclcurl::test::doc_root]
            set fs_path [file join $doc_root {*}$relative_segments]
            if {[file isdirectory $fs_path]} {
                set fs_path [file join $fs_path index.html]
            }
            if {![file exists $fs_path] || [file isdirectory $fs_path]} {
                return {}
            }

            set fh [open $fs_path rb]
            try {
                set body [read $fh]
            } finally {
                close $fh
            }

            return [dict create \
                status 200 \
                reason OK \
                body $body \
                headers [list "Content-Type: [my guess_content_type $fs_path]"]]
        }

        method byte_range_response {service headers full_body} {
            set total_length [string length $full_body]
            set range_header [$service header_value $headers range]
            if {$range_header eq {}} {
                return [dict create \
                    status 200 \
                    reason OK \
                    body $full_body \
                    headers [list "Accept-Ranges: bytes"]]
            }

            if {![regexp {^bytes=\s*([0-9]+-[0-9]+)(\s*,\s*([0-9]+-[0-9]+))*$} $range_header]} {
                return [dict create \
                    status 416 \
                    reason "Range Not Satisfiable" \
                    body {} \
                    headers [list "Content-Range: bytes */$total_length"]]
            }

            set ranges {}
            foreach range_spec [split [string range $range_header 6 end] ,] {
                set range_spec [string trim $range_spec]
                lassign [split $range_spec -] start end
                if {$start > $end || $end >= $total_length} {
                    return [dict create \
                        status 416 \
                        reason "Range Not Satisfiable" \
                        body {} \
                        headers [list "Content-Range: bytes */$total_length"]]
                }
                lappend ranges [list $start $end]
            }

            if {[llength $ranges] == 1} {
                lassign [lindex $ranges 0] start end
                set range_body [string range $full_body $start $end]
                return [dict create \
                    status 206 \
                    reason "Partial Content" \
                    body $range_body \
                    headers [list \
                        "Accept-Ranges: bytes" \
                        "Content-Range: bytes $start-$end/$total_length"]]
            }

            set boundary "tclcurl-boundary"
            set range_body {}
            foreach range $ranges {
                lassign $range start end
                append range_body "--$boundary\r\n"
                append range_body "Content-Type: text/plain\r\n"
                append range_body "Content-Range: bytes $start-$end/$total_length\r\n\r\n"
                append range_body [string range $full_body $start $end]
                append range_body "\r\n"
            }
            append range_body "--$boundary--\r\n"
            return [dict create \
                status 206 \
                reason "Partial Content" \
                body $range_body \
                headers [list \
                    "Accept-Ranges: bytes" \
                    "Content-Type: multipart/byteranges; boundary=$boundary"]]
        }

        method route_request {service method path target version headers request} {
            error "route_request must be implemented by subclasses"
        }
    }

    # Application that reproduces the current TclCurl test-suite routes. This
    # is the first concrete example of a general server application.
    oo::class create ::tclcurl::testserver::CTestApplication {
        superclass ::tclcurl::testserver::CApplication

        method route_request {service method path target version headers request} {
            if {[regexp {^/redir_([0-9]+)$} $path -> redirect_step]} {
                if {$redirect_step < 5} {
                    return [my redirect_response "/redir_[incr redirect_step]"]
                }

                set body "path=$path\nmethod=$method\n"
                return [dict create status 200 reason OK body $body headers {}]
            }

            if {[regexp {^/wait-([0-9]+)(ms|s)$} $path -> wait_amount wait_unit]} {
                set wait_time $wait_amount
                set close_only 0
                if {$wait_unit eq "s"} {
                    set wait_time [expr {$wait_time * 1000}]
                }
                set query_params [my parse_query $target]
                if {[dict exists $query_params reply] && [dict get $query_params reply] eq "none"} {
                    set close_only 1
                }

                return [dict create \
                    status 200 \
                    reason OK \
                    delay_ms $wait_time \
                    close_only $close_only \
                    body "waited=${wait_amount}${wait_unit}\n" \
                    headers {}]
            }

            if {[regexp {^/cookie-set/([^/]+)/([^/]+)$} $path -> cookie_name cookie_value]} {
                set body "set-cookie=$cookie_name=$cookie_value\n"
                return [dict create status     200     \
                                    reason     OK      \
                                    body       $body   \
                                    headers [list "Set-Cookie: $cookie_name=$cookie_value; Path=/"]]
            }

            switch -- $path {
                / {
                    set static_response [my static_file_response /]
                    if {$static_response ne {}} {
                        return $static_response
                    }
                    set index_path [file join [::tclcurl::test::repo_root] testservers index.html]
                    if {[file exists $index_path]} {
                        set fh [open $index_path rb]
                        try {
                            set body [read $fh]
                        } finally {
                            close $fh
                        }
                        return [dict create \
                            status 200 \
                            reason OK \
                            body $body \
                            headers [list "Content-Type: text/html; charset=utf-8"]]
                    }
                    set body "tclcurl test server\n"
                    return [dict create status 200 reason OK body $body headers {}]
                }
                /tclcurl-man {
                    set static_response [my static_file_response /tclcurl-man.html]
                    if {$static_response ne {}} {
                        return $static_response
                    }
                    set manual_path [::tclcurl::testserver::manual_html_source]
                    if {$manual_path ne {} && [file exists $manual_path]} {
                        set fh [open $manual_path rb]
                        try {
                            set body [read $fh]
                        } finally {
                            close $fh
                        }
                        return [dict create \
                            status 200 \
                            reason OK \
                            body $body \
                            headers [list "Content-Type: text/html; charset=utf-8"]]
                    }
                    set body "path=$path\n"
                    return [dict create status 404 reason "Not Found" body $body headers {}]
                }
                /tclcurl-http-root {
                    set body "tclcurl test server\n"
                    return [dict create status 200 reason OK body $body headers {}]
                }
                /tclcurl-http200alias {
                    set body "http200aliases=matched\n"
                    return [dict create \
                        status 200 \
                        reason OK \
                        body $body \
                        headers {} \
                        status_line "yummy/4.5 200 OK"]
                }
                /tclcurl-missing-resource {
                    set body "not found\n"
                    return [dict create status 404 reason "Not Found" body $body headers {}]
                }
                /autoreferer-start {
                    return [my redirect_response "/autoreferer-target"]
                }
                /autoreferer-target {
                    set referer {}
                    if {[dict exists $headers referer]} {
                        set referer [dict get $headers referer]
                    }
                    set body "method=$method\nreferer=$referer\n"
                    return [dict create status 200 reason OK body $body headers {}]
                }
                /postredir-301 {
                    if {$method eq "POST"} {
                        return [dict create status 301                  \
                                            reason "Moved Permanently"  \
                                            body   "redirect=/postredir-target\n" \
                                            headers [list "Location: /postredir-target"]]
                    }
                    set body "method=$method\n"
                    return [dict create status 200 reason OK body $body headers {}]
                }
                /postredir-target {
                    set body "method=$method\n"
                    return [dict create status 200 reason OK body $body headers {}]
                }
                /cookie-echo {
                    set cookie_header {}
                    if {[dict exists $headers cookie]} {
                        set cookie_header [dict get $headers cookie]
                    }
                    set body "cookie=$cookie_header\n"
                    return [dict create status 200 reason OK body $body headers {}]
                }
                /proxy-target {
                    return [dict create status 200 reason OK body "proxy=ok\n" headers {}]
                }
                /proxy-auth-target {
                    return [dict create status 200 reason OK body "proxy-auth=ok\n" headers {}]
                }
                /auth-basic {
                    set expected_user "testuser"
                    set expected_password "testpass"
                    set authorization [$service header_value $headers authorization]
                    if {![regexp {^Basic\s+(.+)$} $authorization -> auth_blob]} {
                        return [dict create \
                            status 401 \
                            reason "Unauthorized" \
                            body "auth=missing\n" \
                            headers [list "WWW-Authenticate: Basic realm=\"TclCurl Test\""]]
                    }

                    if {[catch {set decoded [::base64::decode $auth_blob]}]} {
                        return [dict create \
                            status 401 \
                            reason "Unauthorized" \
                            body "auth=invalid\n" \
                            headers [list "WWW-Authenticate: Basic realm=\"TclCurl Test\""]]
                    }

                    if {![regexp {^([^:]+):(.*)$} $decoded -> username password]} {
                        return [dict create \
                            status 401 \
                            reason "Unauthorized" \
                            body "auth=invalid\n" \
                            headers [list "WWW-Authenticate: Basic realm=\"TclCurl Test\""]]
                    }

                    if {$username ne $expected_user || $password ne $expected_password} {
                        return [dict create \
                            status 401 \
                            reason "Unauthorized" \
                            body "auth=denied\n" \
                            headers [list "WWW-Authenticate: Basic realm=\"TclCurl Test\""]]
                    }

                    set body "auth=ok\nuser=$username\n"
                    return [dict create status 200 reason OK body $body headers {}]
                }
                /request-inspect {
                    set request_body [my request_body $service $request]
                    set body [join [list \
                                "method=[my escape_response_value $method]" \
                                "path=[my escape_response_value $path]" \
                                "target=[my escape_response_value $target]" \
                                "request-version=[my escape_response_value $version]" \
                                "content-type=[my escape_response_value [$service header_value $headers content-type]]" \
                                "content-length=[my escape_response_value [$service header_value $headers content-length]]" \
                                "accept-encoding=[my escape_response_value [$service header_value $headers accept-encoding]]" \
                                "te=[my escape_response_value [$service header_value $headers te]]" \
                                "connection=[my escape_response_value [$service header_value $headers connection]]" \
                                "user-agent=[my escape_response_value [$service header_value $headers user-agent]]" \
                                "referer=[my escape_response_value [$service header_value $headers referer]]" \
                                "x-tclcurl-test=[my escape_response_value [$service header_value $headers x-tclcurl-test]]" \
                                "body-length=[string length $request_body]" \
                                "body-hex=[binary encode hex $request_body]" \
                                "body-sha256=[::sha2::sha256 -hex $request_body]"] "\n"]
                    append body "\n"
                    return [dict create status 200 reason OK body $body headers {}]
                }
                /deflated-data {
                    set body [::tclcurl::test::negotiation_payload]
                    return [dict create \
                        status 200 \
                        reason OK \
                        body [zlib compress $body] \
                        headers [list "Content-Encoding: deflate"]]
                }
                /chunked-data {
                    set body [::tclcurl::test::negotiation_payload]
                    return [dict create \
                        status 200 \
                        reason OK \
                        body $body \
                        headers {} \
                        transfer_encoding chunked]
                }
                /slow-chunked-data {
                    set body [::tclcurl::test::negotiation_payload]
                    return [dict create \
                        status 200 \
                        reason OK \
                        body {} \
                        headers {} \
                        transfer_encoding chunked \
                        stream_chunks [list \
                            [list 0 [string range $body 0 9]] \
                            [list 50 [string range $body 10 end]]]]
                }
                /slow-body-1 {
                    set body [::tclcurl::test::negotiation_payload]
                    return [dict create \
                        status 200 \
                        reason OK \
                        body {} \
                        headers {} \
                        stream_chunks [list \
                            [list 0 [string range $body 0 9]] \
                            [list 6000 [string range $body 10 end]]]]
                }
                /slow-body-2 {
                    set body [string range [::tclcurl::test::range_fixture] 0 191]
                    set stream_chunks {}
                    for {set offset 0} {$offset < [string length $body]} {incr offset 32} {
                        lappend stream_chunks [list 1000 [string range $body $offset [expr {$offset + 31}]]]
                    }
                    return [dict create \
                        status 200 \
                        reason OK \
                        body {} \
                        headers {} \
                        stream_chunks $stream_chunks]
                }
                /range-data {
                    return [my byte_range_response $service $headers [::tclcurl::test::range_fixture]]
                }
                /shutdown {
                    set ::tclcurl::testserver::forever "no more"
                    return [dict create status 200 reason OK body "server orderly shutdown\n" headers {}]
                }
                default {
                    if {$method eq "GET" || $method eq "HEAD"} {
                        set static_response [my static_file_response $path]
                        if {$static_response ne {}} {
                            return $static_response
                        }
                    }
                    set body "path=$path\n"
                    return [dict create status 200 reason OK body $body headers {}]
                }
            }
        }
    }
}
