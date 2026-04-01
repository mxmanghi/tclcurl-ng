namespace eval ::tclcurl::testserver {}

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

        if {[string first "\r\n\r\n" $request_data($chan)] < 0} {
            return
        }

        set request $request_data($chan)
        unset request_data($chan)
        chan event $chan readable {}
        my respond $chan $request
    }

    method respond {chan request} {
        set request_line [lindex [split $request "\r\n"] 0]
        if {![regexp {^([A-Z]+) ([^ ]+) HTTP/([0-9.]+)$} $request_line -> method target version]} {
            my send_response $chan 400 "Bad Request" "bad request\n" 0
            return
        }

        set path [lindex [split $target ?] 0]
        set response [my route_request $method $path $target $version $request]
        if {[dict exists $response status_line]} {
            my send_response $chan \
                [dict get $response status] \
                [dict get $response reason] \
                [dict get $response body] \
                [expr {$method eq "HEAD"}] \
                [dict get $response status_line]
            return
        }

        my send_response $chan \
            [dict get $response status] \
            [dict get $response reason] \
            [dict get $response body] \
            [expr {$method eq "HEAD"}]
    }

    method route_request {method path target version request} {
        switch -- $path {
            / {
                set body "tclcurl test server\n"
                return [dict create status 200 reason OK body $body]
            }
            /tclcurl-http200alias {
                set body "http200aliases=matched\n"
                return [dict create \
                    status 200 \
                    reason OK \
                    body $body \
                    status_line "yummy/4.5 200 OK"]
            }
            /tclcurl-missing-resource {
                set body "not found\n"
                return [dict create status 404 reason "Not Found" body $body]
            }
            default {
                set body "path=$path\n"
                return [dict create status 200 reason OK body $body]
            }
        }
    }

    method send_response {chan status reason body head_only {status_line {}}} {
        if {$status_line eq {}} {
            set status_line "HTTP/1.1 $status $reason"
        }

        set headers [list \
            $status_line \
            "Content-Type: text/plain" \
            "Content-Length: [string length $body]" \
            "Connection: close"]

        puts -nonewline $chan [join $headers "\r\n"]
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
