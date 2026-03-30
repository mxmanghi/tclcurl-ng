#!/usr/bin/env tclsh

namespace eval ::tclcurl::httpd {
    variable config
    array set config {
        host 127.0.0.1
        port 8990
        quiet 0
    }
}

proc ::tclcurl::httpd::usage {} {
    puts stderr "usage: tclsh tests/http_server.tcl ?-host 127.0.0.1? ?-port 8990? ?-quiet?"
}

proc ::tclcurl::httpd::parseArgs {argv} {
    variable config

    for {set i 0} {$i < [llength $argv]} {incr i} {
        set arg [lindex $argv $i]
        switch -- $arg {
            -host {
                incr i
                if {$i >= [llength $argv]} {
                    error "missing value after -host"
                }
                set config(host) [lindex $argv $i]
            }
            -port {
                incr i
                if {$i >= [llength $argv]} {
                    error "missing value after -port"
                }
                set config(port) [lindex $argv $i]
                if {![string is integer -strict $config(port)] || $config(port) < 1 || $config(port) > 65535} {
                    error "invalid port: $config(port)"
                }
            }
            -quiet {
                set config(quiet) 1
            }
            -h -
            -help -
            --help {
                usage
                exit 0
            }
            default {
                error "unknown argument: $arg"
            }
        }
    }
}

proc ::tclcurl::httpd::log {message} {
    variable config
    if {!$config(quiet)} {
        puts stderr $message
    }
}

proc ::tclcurl::httpd::accept {chan host port} {
    fconfigure $chan -blocking 0 -buffering none -translation binary
    fileevent $chan readable [list ::tclcurl::httpd::readRequest $chan]
}

proc ::tclcurl::httpd::readRequest {chan} {
    if {[eof $chan]} {
        catch {close $chan}
        return
    }

    set chunk [read $chan]
    if {$chunk eq {}} {
        return
    }

    append ::tclcurl::httpd::request($chan) $chunk

    if {[string first "\r\n\r\n" $::tclcurl::httpd::request($chan)] < 0} {
        return
    }

    set request $::tclcurl::httpd::request($chan)
    unset ::tclcurl::httpd::request($chan)
    fileevent $chan readable {}
    respond $chan $request
}

proc ::tclcurl::httpd::respond {chan request} {
    set requestLine [lindex [split $request "\r\n"] 0]
    if {![regexp {^([A-Z]+) ([^ ]+) HTTP/([0-9.]+)$} $requestLine -> method target version]} {
        sendResponse $chan 400 "Bad Request" "bad request\n" 0
        return
    }

    set path [lindex [split $target ?] 0]
    switch -- $path {
        / {
            set body "tclcurl test server\n"
            sendResponse $chan 200 "OK" $body [expr {$method eq "HEAD"}]
        }
        /tclcurl-missing-resource {
            set body "not found\n"
            sendResponse $chan 404 "Not Found" $body [expr {$method eq "HEAD"}]
        }
        default {
            set body "path=$path\n"
            sendResponse $chan 200 "OK" $body [expr {$method eq "HEAD"}]
        }
    }
}

proc ::tclcurl::httpd::sendResponse {chan status reason body headOnly} {
    set headers [list \
        "HTTP/1.1 $status $reason" \
        "Content-Type: text/plain" \
        "Content-Length: [string length $body]" \
        "Connection: close"]

    puts -nonewline $chan [join $headers "\r\n"]
    puts -nonewline $chan "\r\n\r\n"
    if {!$headOnly} {
        puts -nonewline $chan $body
    }
    flush $chan
    catch {close $chan}
}

if {[catch {::tclcurl::httpd::parseArgs $argv} message]} {
    puts stderr $message
    ::tclcurl::httpd::usage
    exit 1
}

set listener [socket -server ::tclcurl::httpd::accept -myaddr $::tclcurl::httpd::config(host) $::tclcurl::httpd::config(port)]
::tclcurl::httpd::log "listening on http://$::tclcurl::httpd::config(host):$::tclcurl::httpd::config(port)/"
vwait ::tclcurl::httpd::forever
