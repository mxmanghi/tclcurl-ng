# proxy_server.tcl --
#
# Implementation of a minimal HTTP proxy server for testing TclCurl.
#
# Copyright (c) 2024-2026 Massimo Manghi
#
# SPDX-License-Identifier: TCL
#
# See the file "license.terms" at the top level of this distribution
# for information on usage and redistribution of this file, and for the
# complete disclaimer of warranties and limitation of liability.

namespace eval ::tclcurl::testserver {}

package require base64

oo::class create ::tclcurl::testserver::proxy_service {
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
        my log [my listening_message]
        return $listener
    }

    method description {} {
        return "HTTP proxy test server"
    }

    method accept {chan host port} {
        chan configure $chan -blocking 0 -buffering none -translation binary
        ::tclcurl::test::msgoutput "proxy accept chan=$chan host=$host port=$port"
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

        set content_length 0
        # Example match: "Content-Length: 123"
        if {[regexp -nocase {\mContent-Length:\s*([0-9]+)} [string range $request_data 0 [expr {$header_end - 1}]] -> content_length]} {
            set request_length [expr {$header_end + 4 + $content_length}]
            if {[string length $request_data] < $request_length} {
                return {}
            }
            return [string range $request_data 0 [expr {$request_length - 1}]]
        }

        return [string range $request_data 0 [expr {$header_end + 3}]]
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
            # Example match: "Host: 127.0.0.1:8990"
            if {![regexp {^([^:]+):\s*(.*)$} $header_line -> name value]} {
                continue
            }
            dict set headers [string tolower $name] $value
        }

        return $headers
    }

    method parse_target {target headers} {
        # Example match: "http://127.0.0.1:8990/proxy-target"
        if {[regexp {^http://([^/:]+)(?::([0-9]+))?(/.*)?$} $target -> host port path]} {
            if {$port eq {}} {
                set port 80
            }
            if {$path eq {}} {
                set path /
            }
            return [dict create host $host port $port path $path]
        }

        set host_header [dict get $headers host]
        # Example matches: "127.0.0.1" and "127.0.0.1:8990"
        if {![regexp {^([^:]+)(?::([0-9]+))?$} $host_header -> host port]} {
            error "invalid Host header"
        }
        if {$port eq {}} {
            set port 80
        }
        return [dict create host $host port $port path $target]
    }

    method auth_required {path} {
        return [expr {$path eq "/proxy-auth-target"}]
    }

    method validate_proxy_auth {headers} {
        set authorization {}
        if {[dict exists $headers proxy-authorization]} {
            set authorization [dict get $headers proxy-authorization]
        }
        # Example match: "Basic cHJveHl1c2VyOnByb3h5cGFzcw=="
        if {![regexp {^Basic\s+(.+)$} $authorization -> auth_blob]} {
            return missing
        }
        if {[catch {set decoded [::base64::decode $auth_blob]}]} {
            return invalid
        }
        if {$decoded ne "proxyuser:proxypass"} {
            return denied
        }
        return ok
    }

    method proxy_response {chan status reason body headers} {
        set response_headers [concat [list \
            "HTTP/1.1 $status $reason" \
            "Content-Type: text/plain" \
            "Content-Length: [string length $body]" \
            "Connection: close"] $headers]
        catch {
            puts -nonewline $chan [join $response_headers "\r\n"]
            puts -nonewline $chan "\r\n\r\n"
            puts -nonewline $chan $body
            flush $chan
        }
        my close_client $chan
    }

    method forwarded_header_name {name} {
        set parts {}
        foreach part [split $name -] {
            lappend parts [string totitle $part]
        }
        return [join $parts -]
    }

    method respond {chan request} {
        set request_line [lindex [split $request "\r\n"] 0]
        # Example match: "GET http://127.0.0.1:8990/proxy-target HTTP/1.1"
        if {![regexp {^([A-Z]+) ([^ ]+) HTTP/([0-9.]+)$} $request_line -> method target version]} {
            my proxy_response $chan 400 "Bad Request" "bad proxy request\n" {}
            return
        }

        set headers [my parse_headers $request]
        set target_info [my parse_target $target $headers]
        set path [dict get $target_info path]

        if {[my auth_required $path]} {
            set auth_status [my validate_proxy_auth $headers]
            if {$auth_status ne "ok"} {
                my proxy_response $chan 407 \
                    "Proxy Authentication Required" "proxy-auth=$auth_status\n" \
                    [list "Proxy-Authenticate: Basic realm=\"TclCurl Proxy Test\""]
                return
            }
        }

        set host [dict get $target_info host]
        set port [dict get $target_info port]
        set origin_path [dict get $target_info path]
        if {[catch {set upstream [socket $host $port]} socket_error]} {
            my proxy_response $chan 502 "Bad Gateway" "proxy-error=$socket_error\n" {}
            return
        }

        chan configure $upstream -blocking 1 -buffering none -translation binary
        set upstream_headers {}
        dict for {name value} $headers {
            if {$name in {proxy-authorization proxy-connection}} {
                continue
            }
            lappend upstream_headers "[my forwarded_header_name $name]: $value"
        }

        set header_end [string first "\r\n\r\n" $request]
        set request_body [string range $request [expr {$header_end + 4}] end]

        try {
            puts -nonewline $upstream "$method $origin_path HTTP/$version\r\n"
            puts -nonewline $upstream "[join $upstream_headers "\r\n"]"
            puts -nonewline $upstream "\r\n\r\n"
            if {$request_body ne {}} {
                puts -nonewline $upstream $request_body
            }
            flush $upstream
            set response [read $upstream]
        } finally {
            catch {close $upstream}
        }

        catch {
            puts -nonewline $chan $response
            flush $chan
        }
        my close_client $chan
    }

    method close_client {chan} {
        catch {unset request_data($chan)}
        catch {close $chan}
    }
}

::tclcurl::testserver register_service_class proxy ::tclcurl::testserver::proxy_service
