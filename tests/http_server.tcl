namespace eval ::tclcurl::testserver {}

package require sha256

proc ::tclcurl::testserver::http_header_lines {header_block} {
    return [regexp -all -inline {[^\r\n]+} $header_block]
}

proc ::tclcurl::testserver::escape_response_value {value} {
    return [string map [list "\\" "\\\\" "\n" "\\n" "\r" "\\r"] $value]
}

oo::class create ::tclcurl::testserver::http_service {
    superclass ::tclcurl::testserver::service

    variable request_data

    constructor args {
        array set request_data {}
        next {*}$args
    }

    destructor {
        foreach chan [array names request_data] {
            catch {close $chan}
        }
        next
    }

    method start {} {
        set listener [socket -server [list [self] accept] \
            -myaddr [my host] [my port]]
        my set_listener $listener
        my log "listening on [my endpoint]"
        return $listener
    }

    method accept {chan host port} {
        chan configure $chan -blocking 0 -buffering none -translation binary
        ::tclcurl::test::msgoutput "accept connection chan=$chan host=$host port=$port"
        chan event $chan readable [list [self] read_request $chan]
    }

    method read_request {chan} {
        if {[eof $chan]} {
            my close_client $chan
            return
        }

        set chunk [read $chan]
        if {$chunk eq {}} {
            return
        }

        append request_data($chan) $chunk
        set request [my complete_request $request_data($chan)]
        if {$request eq {}} {
            return
        }

        unset request_data($chan)
        chan event $chan readable {}
        my respond $chan $request
    }

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

    method respond {chan request} {
        set request_line [lindex [split $request "\r\n"] 0]
        if {![regexp {^([A-Z]+) ([^ ]+) HTTP/([0-9.]+)$} $request_line -> method target version]} {
            set response [my build_response_dict [dict create \
                status 400 \
                reason "Bad Request" \
                body "bad request\n" \
                head_only 0]]
            my send_response $chan $response
            return
        }

        set path [lindex [split $target ?] 0]
        ::tclcurl::test::msgoutput "route request method=$method path=$path"
        set headers [my parse_headers $request]
        set response [my route_request $method $path $target $version $headers $request]
        dict set response head_only [expr {$method eq "HEAD"}]
        set response [my build_response_dict $response]
        my send_response $chan $response
    }

    method parse_headers {request} {
        set header_end [string first "\r\n\r\n" $request]
        if {$header_end < 0} {
            return [dict create]
        }

        set header_block [string range $request 0 [expr {$header_end - 1}]]
        set header_lines [::tclcurl::testserver::http_header_lines $header_block]
        set headers [dict create]

        foreach header_line [lrange $header_lines 1 end] {
            if {![regexp {^([^:]+):\s*(.*)$} $header_line -> name value]} {
                continue
            }
            dict set headers [string tolower $name] $value
        }

        return $headers
    }

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

    method redirect_response {location {reason "Found"}} {
        set body "redirect=$location\n"
        return [dict create status 302      \
                            reason $reason  \
                            body $body      \
                            headers [list "Location: $location"]]
    }

    method route_request {method path target version headers request} {
        if {[regexp {^/redir_([0-9]+)$} $path -> redirect_step]} {
            if {$redirect_step < 5} {
                return [my redirect_response "/redir_[incr redirect_step]"]
            }

            set body "path=$path\nmethod=$method\n"
            return [dict create status 200 reason OK body $body headers {}]
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
            /request-inspect {
                set request_body [my request_body $request]
                set body [join [list \
                            "method=[::tclcurl::testserver::escape_response_value $method]" \
                            "path=[::tclcurl::testserver::escape_response_value $path]" \
                            "content-type=[::tclcurl::testserver::escape_response_value [my header_value $headers content-type]]" \
                            "content-length=[::tclcurl::testserver::escape_response_value [my header_value $headers content-length]]" \
                            "body-length=[string length $request_body]" \
                            "body-hex=[binary encode hex $request_body]" \
                            "body-sha256=[::sha2::sha256 -hex $request_body]"] "\n"]
                append body "\n"
                return [dict create status 200 reason OK body $body headers {}]
            }
            /shutdown {
                # Let's have also a method to have the server orderly stop operations
                set ::tclcurl::testserver::forever "no more"
                return [dict create status 200 reason OK body "server orderly shutdown\n" headers {}]
            }
            default {
                set body "path=$path\n"
                return [dict create status 200 reason OK body $body headers {}]
            }
        }
    }

    method build_response_dict {response} {
        set completed_response [dict create \
            status 200 \
            reason OK \
            body {} \
            head_only 0 \
            headers {} \
            status_line {}]

        dict for {key value} $response {
            dict set completed_response $key $value
        }

        return $completed_response
    }

    method send_response {chan response} {
        dict with response {}
        if {$status_line eq {}} {
            set status_line "HTTP/1.1 $status $reason"
        }

        ::tclcurl::test::msgoutput "send response chan=$chan status=$status reason=$reason body-length=[string length $body]"

        set response_headers [list \
            $status_line \
            "Content-Type: text/plain" \
            "Content-Length: [string length $body]" \
            "Connection: close"]
        set response_headers [concat $response_headers $headers]

        puts -nonewline $chan [join $response_headers "\r\n"]
        puts -nonewline $chan "\r\n\r\n"
        if {!$head_only} {
            puts -nonewline $chan $body
        }
        flush $chan
        my close_client $chan
    }

    method close_client {chan} {
        catch {unset request_data($chan)}
        catch {close $chan}
    }
}

::tclcurl::testserver register_service_class http ::tclcurl::testserver::http_service
