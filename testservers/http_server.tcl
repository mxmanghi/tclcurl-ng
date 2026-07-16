# http_server.tcl -- Implementation of a simple HTTP server 
#
# Implementation of the HTTP server support used by the TclCurl extension.
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
if {[info commands zlib] eq {}} {
    package require zlib
}
package require base64
package require uri

if {[info commands ::tclcurl::testserver::http_endpoint_service] eq {}} {
    source [file join [file dirname [file normalize [info script]]] http_endpoint.tcl]
}

# HTTP origin service used by the tests. The shared base handles connection
# lifecycle and buffering; this class adds origin-specific request framing,
# routing and response generation.
oo::class create ::tclcurl::testserver::http_service {
    superclass ::tclcurl::testserver::http_endpoint_service

    method escape_response_value {value} {
        return [string map [list "\\" "\\\\" "\n" "\\n" "\r" "\\r"] $value]
    }

    method decode_query_component {value} {
        set decoded [string map [list + " "] $value]
        while {[regexp -indices {%[0-9A-Fa-f][0-9A-Fa-f]} $decoded match]} {
            lassign $match first last
            set replacement [format %c 0x[string range $decoded [expr {$first + 1}] $last]]
            set decoded [string replace $decoded $first $last $replacement]
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
                [my decode_query_component [string range $pair [expr {$separator + 1}] end]]
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

    method description {} {
        return "HTTP origin test server"
    }

    # Origin tests also exercise chunked uploads, so request completion needs
    # to understand both fixed-size and chunked request bodies.
    method complete_request {request_data} {
        set header_end [string first "\r\n\r\n" $request_data]
        if {$header_end < 0} { return {} }

        set headers [my parse_headers $request_data]
        if {[string tolower [my header_value $headers transfer-encoding]] eq "chunked"} {
            return [my complete_chunked_request $request_data $header_end]
        }

        set content_length 0
        if {[dict exists $headers content-length]} {
            set content_length [dict get $headers content-length]
        }
        set request_length [expr {$header_end + 4 + $content_length}]
        if {[string length $request_data] < $request_length} {
            return {}
        }

        return [string range $request_data 0 [expr {$request_length - 1}]]
    }

    method complete_chunked_request {request_data header_end} {
        set chunk_data [string range $request_data [expr {$header_end + 4}] end]
        set chunk_info [my parse_chunked_body $chunk_data]
        if {![dict get $chunk_info complete]} {
            return {}
        }

        set request_end [expr {$header_end + 4 + [dict get $chunk_info consumed_length] - 1}]
        return [string range $request_data 0 $request_end]
    }

    # Build the standard origin-server reply for malformed HTTP requests.
    method bad_request_response {} {
        return [my build_response_dict [dict create \
            status 400 \
            reason "Bad Request" \
            body "bad request\n" \
            head_only 0]]
    }

    # Dispatch a parsed origin request to the route table, then normalize and
    # emit the resulting response dictionary.
    method handle_request {chan request} {
        set request_info [my parse_request_line $request]
        if {$request_info eq {}} {
            my log_request "method=? status=400 path=?"
            my send_response $chan [my bad_request_response]
            return
        }

        dict with request_info {}
        set path [lindex [split $target ?] 0]
        ::tclcurl::test::msgoutput "route request method=$method path=$path"
        set headers [my parse_headers $request]
        set response [my route_request $method $path $target $version $headers $request]
        dict set response head_only [expr {$method eq "HEAD"}]
        set response [my build_response_dict $response]
        my log_request "method=$method status=[dict get $response status] path=[::tclcurl::testserver::log_value $path]"
        if {[dict exists $response delay_ms]} {
            set callback [list [self] send_response $chan $response]
            if {[dict exists $response close_only] && [dict get $response close_only]} {
                set callback [list [self] close_client $chan]
            }
            after [dict get $response delay_ms] $callback
            return
        }
        my send_response $chan $response
    }
    # Extract the request body after the header block and transparently decode
    # chunked uploads so route handlers can inspect the payload directly.
    method request_body {request} {
        set header_end [string first "\r\n\r\n" $request]
        if {$header_end < 0} {
            return {}
        }

        set headers [my parse_headers $request]
        if {[string tolower [my header_value $headers transfer-encoding]] eq "chunked"} {
            return [my decode_chunked_body [string range $request [expr {$header_end + 4}] end]]
        }

        return [string range $request [expr {$header_end + 4}] end]
    }

    method decode_chunked_body {body} {
        return [dict get [my parse_chunked_body $body] decoded_body]
    }

    method parse_chunked_body {body} {
        set decoded {}
        set cursor 0

        # libcurl can emit "Transfer-Encoding: chunked" for an empty POST
        # request without sending an explicit terminating zero-size chunk.
        if {$body eq {}} {
            return [dict create complete 1 decoded_body {} consumed_length 0]
        }

        while 1 {
            set line_end [string first "\r\n" $body $cursor]
            if {$line_end < 0} {
                return [dict create complete 0 decoded_body {} consumed_length 0]
            }

            set size_line [string trim [string range $body $cursor [expr {$line_end - 1}]]]
            set size_token [lindex [split $size_line ";"] 0]
            if {[scan $size_token %x chunk_size] != 1} {
                return [dict create complete 0 decoded_body {} consumed_length 0]
            }

            set data_start [expr {$line_end + 2}]
            if {$chunk_size == 0} {
                if {[string range $body $data_start [expr {$data_start + 1}]] eq "\r\n"} {
                    return [dict create \
                        complete 1 \
                        decoded_body $decoded \
                        consumed_length [expr {$data_start + 2}]]
                }

                set trailer_end [string first "\r\n\r\n" $body $data_start]
                if {$trailer_end < 0} {
                    return [dict create complete 0 decoded_body {} consumed_length 0]
                }

                return [dict create \
                    complete 1 \
                    decoded_body $decoded \
                    consumed_length [expr {$trailer_end + 4}]]
            }

            set data_end [expr {$data_start + $chunk_size}]
            if {[string length $body] < ($data_end + 2)} {
                return [dict create complete 0 decoded_body {} consumed_length 0]
            }
            if {[string range $body $data_end [expr {$data_end + 1}]] ne "\r\n"} {
                return [dict create complete 0 decoded_body {} consumed_length 0]
            }

            append decoded [string range $body $data_start [expr {$data_end - 1}]]
            set cursor [expr {$data_end + 2}]
        }
    }

    method header_value {headers name} {
        if {[dict exists $headers $name]} {
            return [dict get $headers $name]
        }
        return {}
    }

    method chunk_encode {body {chunk_size 16}} {
        set encoded {}
        set body_length [string length $body]
        for {set offset 0} {$offset < $body_length} {incr offset $chunk_size} {
            set chunk [string range $body $offset [expr {$offset + $chunk_size - 1}]]
            append encoded [my chunk_frame $chunk]
        }
        append encoded [my chunk_terminator]
        return $encoded
    }

    method chunk_frame {chunk_data} {
        return [format %X [string length $chunk_data]]\r\n$chunk_data\r\n
    }

    method chunk_terminator {} {
        return "0\r\n\r\n"
    }

    method streaming_payload_length {stream_chunks} {
        set total 0
        foreach stream_chunk $stream_chunks {
            lassign $stream_chunk delay_ms chunk_data
            incr total [string length $chunk_data]
        }
        return $total
    }

    method stream_response_body {chan response_headers head_only transfer_encoding stream_chunks} {
        if {[catch {
            puts -nonewline $chan [join $response_headers "\r\n"]
            puts -nonewline $chan "\r\n\r\n"
            flush $chan
        } write_error]} {
            ::tclcurl::test::msgoutput "response write failed chan=$chan error=$write_error"
            my close_client $chan
            return
        }

        if {$head_only} {
            my close_client $chan
            return
        }

        my send_stream_chunk $chan $transfer_encoding $stream_chunks 0
    }

    method send_stream_chunk {chan transfer_encoding stream_chunks index} {
        if {$index >= [llength $stream_chunks]} {
            if {$transfer_encoding eq "chunked"} {
                if {[catch {
                    puts -nonewline $chan [my chunk_terminator]
                    flush $chan
                } write_error]} {
                    ::tclcurl::test::msgoutput "stream write failed chan=$chan error=$write_error"
                }
            }
            my close_client $chan
            return
        }

        lassign [lindex $stream_chunks $index] delay_ms chunk_data
        after $delay_ms [list [self] write_stream_chunk $chan $transfer_encoding $stream_chunks $index $chunk_data]
    }

    method write_stream_chunk {chan transfer_encoding stream_chunks index chunk_data} {
        if {[catch {
            if {$transfer_encoding eq "chunked"} {
                puts -nonewline $chan [my chunk_frame $chunk_data]
            } else {
                puts -nonewline $chan $chunk_data
            }
            flush $chan
        } write_error]} {
            ::tclcurl::test::msgoutput "stream write failed chan=$chan error=$write_error"
            my close_client $chan
            return
        }

        my send_stream_chunk $chan $transfer_encoding $stream_chunks [expr {$index + 1}]
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

    method byte_range_response {headers full_body} {
        set total_length [string length $full_body]
        set range_header [my header_value $headers range]
        if {$range_header eq {}} {
            return [dict create \
                status 200 \
                reason OK \
                body $full_body \
                headers [list "Accept-Ranges: bytes"]]
        }

        # Example matches: "bytes=16-47" and "bytes=16-47,102-134"
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

    # Map origin-server paths to the behavior expected by the test suite.
    method route_request {method path target version headers request} {
        # Example matches: "/redir_1" and "/redir_5"
        if {[regexp {^/redir_([0-9]+)$} $path -> redirect_step]} {
            if {$redirect_step < 5} {
                return [my redirect_response "/redir_[incr redirect_step]"]
            }

            set body "path=$path\nmethod=$method\n"
            return [dict create status 200 reason OK body $body headers {}]
        }

        # Example matches: "/wait-200ms" and "/wait-2s"
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

        # Example match: "/cookie-set/session_id/abc123"
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
                set authorization [my header_value $headers authorization]
                # Example match: "Basic dGVzdHVzZXI6dGVzdHBhc3M="
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

                # Example decoded value: "testuser:testpass"
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
                set request_body [my request_body $request]
                set body [join [list \
                            "method=[my escape_response_value $method]" \
                            "path=[my escape_response_value $path]" \
                            "target=[my escape_response_value $target]" \
                            "request-version=[my escape_response_value $version]" \
                            "content-type=[my escape_response_value [my header_value $headers content-type]]" \
                            "content-length=[my escape_response_value [my header_value $headers content-length]]" \
                            "accept-encoding=[my escape_response_value [my header_value $headers accept-encoding]]" \
                            "te=[my escape_response_value [my header_value $headers te]]" \
                            "connection=[my escape_response_value [my header_value $headers connection]]" \
                            "user-agent=[my escape_response_value [my header_value $headers user-agent]]" \
                            "referer=[my escape_response_value [my header_value $headers referer]]" \
                            "x-tclcurl-test=[my escape_response_value [my header_value $headers x-tclcurl-test]]" \
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
                return [my byte_range_response $headers [::tclcurl::test::range_fixture]]
            }
            /shutdown {
                # Let's also have a method to orderly stop operations
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

    # Normalize partially specified route results into a complete response
    # dictionary so sending code can follow one path.
    method build_response_dict {response} {
        set completed_response [dict create \
            status 200 \
            reason OK \
            body {} \
            head_only 0 \
            headers {} \
            status_line {} \
            transfer_encoding {} \
            stream_chunks {}]

        dict for {key value} $response {
            dict set completed_response $key $value
        }

        return $completed_response
    }

    # Serialize a normalized response dictionary to the client socket, handling
    # HEAD requests, chunked transfers and delayed streaming responses.
    method send_response {chan response} {
        dict with response {}
        if {$status_line eq {}} {
            set status_line "HTTP/1.1 $status $reason"
        }

        ::tclcurl::test::msgoutput "send response chan=$chan status=$status reason=$reason body-length=[string length $body]"

        set response_headers [list $status_line "Connection: close"]
        if {![regexp -nocase {^Content-Type:} [join $headers "\n"]]} {
            lappend response_headers "Content-Type: text/plain"
        }
        if {$transfer_encoding eq "chunked"} {
            lappend response_headers "Transfer-Encoding: chunked"
        } elseif {[llength $stream_chunks] > 0} {
            lappend response_headers "Content-Length: [my streaming_payload_length $stream_chunks]"
        } else {
            lappend response_headers "Content-Length: [string length $body]"
        }
        set response_headers [concat $response_headers $headers]

        if {[llength $stream_chunks] > 0} {
            my stream_response_body $chan $response_headers $head_only $transfer_encoding $stream_chunks
            return
        }

        if {[catch {
            puts -nonewline $chan [join $response_headers "\r\n"]
            puts -nonewline $chan "\r\n\r\n"
            if {!$head_only} {
                if {$transfer_encoding eq "chunked"} {
                    puts -nonewline $chan [my chunk_encode $body]
                } else {
                    puts -nonewline $chan $body
                }
            }
            flush $chan
        } write_error]} {
            ::tclcurl::test::msgoutput "response write failed chan=$chan error=$write_error"
            my close_client $chan
            return
        }
        my close_client $chan
    }

}

::tclcurl::testserver register_service_class http ::tclcurl::testserver::http_service
