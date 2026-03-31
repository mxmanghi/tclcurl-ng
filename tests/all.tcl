#!/usr/bin/env tclsh

package require tcltest

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

set tcltestArgv {}
for {set i 0} {$i < [llength $argv]} {incr i} {
        set arg [lindex $argv $i]
        if {$arg eq "-httpserver"} {
                incr i
                if {$i >= [llength $argv]} {
                        error "missing path after -httpserver"
                }
                ::tclcurl::test::server::set_http_server_script [lindex $argv $i]
                continue
        }
        lappend tcltestArgv $arg
}

# Allow command line arguments to be passed to the configure command
# This supports only running a single test or a single test file
if {[lsearch -exact $tcltestArgv -verbose] < 0} {
        ::tcltest::configure -verbose {start error}
}
::tcltest::configure {*}$tcltestArgv

::tcltest::runAllTests

if {$exitCode == 1} {
        puts "====== FAIL ====="
        exit $exitCode
} else {
        puts "====== SUCCESS ====="
}
