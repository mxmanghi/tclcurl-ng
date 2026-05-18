#!/usr/bin/env tclsh

# all.tcl --
#
# Central hub for all tests 
#
# This file is partially derived from tclcurl-fa.
#
# Copyright (c) 2001-2011 Andres Garcia Garcia
# Copyright (c) 2024-2026 Massimo Manghi
#
# SPDX-License-Identifier: TCL
#
# See the file "license.terms" at the top level of this distribution
# for information on usage and redistribution of this file, and for the
# complete disclaimer of warranties and limitation of liability.

set ::argv_saved_for_all_tests $argv
set argv {}
package require tcltest
set argv $::argv_saved_for_all_tests
unset ::argv_saved_for_all_tests

namespace eval ::tclcurl::test {}

proc ::tclcurl::test::parse_port_value {name value} {
    if {![string is integer -strict $value] || $value < 1 || $value > 65535} {
        error "invalid value for $name: $value"
    }
    return $value
}

proc ::tclcurl::test::rewrite_base_url_port {url default_scheme default_host default_port} {
    set candidate $url
    if {$candidate eq {}} {
        set candidate "${default_scheme}://${default_host}:${default_port}"
    }

    if {![regexp {^([a-z]+)://([^/:]+)(?::([0-9]+))?(/.*)?$} $candidate -> scheme host existing_port path]} {
        error "cannot override port in malformed URL: $candidate"
    }
    if {$path eq {}} {
        set path {}
    }

    return "${scheme}://${host}:${default_port}${path}"
}

proc ::tclcurl::test::normalize_tcltest_arg {arg} {
    if {[string length $arg] >= 2 && [string range $arg 0 1] eq "--"} {
        return "-[string range $arg 2 end]"
    }
    return $arg
}

set tcltestArgv {}
set exitServer 0
set debug 0
set configuredHttpServer {}
array set root_overrides {}
array set https_credential_overrides {}
array set port_overrides {}
for {set i 0} {$i < [llength $argv]} {incr i} {
    set arg [lindex $argv $i]
    switch -- $arg {
        --httpserver {
            incr i
            if {$i >= [llength $argv]} {
                error "missing path after --httpserver"
            }
            set configuredHttpServer [lindex $argv $i]
        }
        --httpport -
        --httpsport -
        --ftpport -
        --proxyport {
            incr i
            if {$i >= [llength $argv]} {
                error "missing value after $arg"
            }
            set port_overrides($arg) [::tclcurl::test::parse_port_value $arg [lindex $argv $i]]
        }
        --docroot -
        --ftproot {
            incr i
            if {$i >= [llength $argv]} {
                error "missing value after $arg"
            }
            set root_overrides($arg) [file normalize [lindex $argv $i]]
        }
        --certfile -
        --keyfile {
            incr i
            if {$i >= [llength $argv]} {
                error "missing value after $arg"
            }
            set https_credential_overrides($arg) [file normalize [lindex $argv $i]]
        }
        --exitserver {
            set exitServer 1
        }
        --debug {
            set debug 1
        }
        default {
            lappend tcltestArgv [::tclcurl::test::normalize_tcltest_arg $arg]
        }
    }
}

if {[info exists port_overrides(--httpport)]} {
    set ::env(TCLCURL_TEST_HTTP_BASE_URL) \
        [::tclcurl::test::rewrite_base_url_port \
            [expr {[info exists ::env(TCLCURL_TEST_HTTP_BASE_URL)] ? $::env(TCLCURL_TEST_HTTP_BASE_URL) : {}}] \
            http 127.0.0.1 $port_overrides(--httpport)]
}
if {[info exists port_overrides(--httpsport)]} {
    set ::env(TCLCURL_TEST_HTTPS_BASE_URL) \
        [::tclcurl::test::rewrite_base_url_port \
            [expr {[info exists ::env(TCLCURL_TEST_HTTPS_BASE_URL)] ? $::env(TCLCURL_TEST_HTTPS_BASE_URL) : {}}] \
            https 127.0.0.1 $port_overrides(--httpsport)]
}
if {[info exists port_overrides(--ftpport)]} {
    set ::env(TCLCURL_TEST_FTP_BASE_URL) \
        [::tclcurl::test::rewrite_base_url_port \
            [expr {[info exists ::env(TCLCURL_TEST_FTP_BASE_URL)] ? $::env(TCLCURL_TEST_FTP_BASE_URL) : {}}] \
            ftp 127.0.0.1 $port_overrides(--ftpport)]
}
if {[info exists port_overrides(--proxyport)]} {
    set ::env(TCLCURL_TEST_PROXY_BASE_URL) \
        [::tclcurl::test::rewrite_base_url_port \
            [expr {[info exists ::env(TCLCURL_TEST_PROXY_BASE_URL)] ? $::env(TCLCURL_TEST_PROXY_BASE_URL) : {}}] \
            http 127.0.0.1 $port_overrides(--proxyport)]
}
if {[info exists root_overrides(--docroot)]} {
    set ::env(TCLCURL_TEST_DOC_ROOT) $root_overrides(--docroot)
}
if {[info exists root_overrides(--ftproot)]} {
    set ::env(TCLCURL_TEST_FTP_ROOT) $root_overrides(--ftproot)
}
if {[info exists https_credential_overrides(--certfile)]} {
    set ::env(TCLCURL_TEST_HTTPS_CERT_FILE) $https_credential_overrides(--certfile)
}
if {[info exists https_credential_overrides(--keyfile)]} {
    set ::env(TCLCURL_TEST_HTTPS_KEY_FILE) $https_credential_overrides(--keyfile)
}

::tcltest::configure -testdir [file dirname [file normalize [info script]]]
source [file join [file dirname [file normalize [info script]]] support.tcl]
::tclcurl::test::load_package

namespace import ::tcltest::*

# The following will be upleveled and run whenever a test calls
# ::tcltest::loadTestedCommands
::tcltest::configure -load {
    ::tclcurl::test::load_package
    namespace import ::tcltest::*
}

::tcltest::skipFiles [list]

# Hook to determine if any of the tests failed. Then we can exit with
# proper exit code: 0=all passed, 1=one or more failed

proc tcltest::cleanupTestsHook {} {
    variable numTests
    set ::exitCode [expr {$numTests(Failed) > 0}]
}

proc ::tclcurl::test::shutdown_http_server {} {
    set shutdownUrl [::tclcurl::test::server::base_url shutdown]
    if {![regexp {^https?://([^/:]+)(?::([0-9]+))?(/.*)$} $shutdownUrl -> host port path]} {
        return
    }
    if {$port eq {}} {
        set port 80
    }

    if {[catch {set sock [socket $host $port]}]} {
        return
    }

    try {
        chan configure $sock -translation binary -buffering none
        puts -nonewline $sock "GET $path HTTP/1.1\r\nHost: $host\r\nConnection: close\r\n\r\n"
        flush $sock
        catch {read $sock}
    } finally {
        catch {close $sock}
    }
}

if {$configuredHttpServer ne {}} {
    ::tclcurl::test::server::set_http_server_script $configuredHttpServer
}

::tclcurl::test::configure_debug_output $debug

# Allow command line arguments to be passed to the configure command
# This supports only running a single test or a single test file
if {[lsearch -exact $tcltestArgv -verbose] < 0} {
    ::tcltest::configure -verbose {start error}
}
::tcltest::configure {*}$tcltestArgv

try {
    ::tcltest::runAllTests
} finally {
    if {$exitServer} {
        ::tclcurl::test::shutdown_http_server
    }
}

if {$exitCode == 1} {
    puts "====== FAIL ====="
    exit $exitCode
} else {
    puts "====== SUCCESS ====="
}
