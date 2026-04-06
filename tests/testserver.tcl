#!/usr/bin/env tclsh
#
# testserver.tcl -- Start the server implementations needed by the test suite 
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

set ::argv_saved_for_testserver $argv
set argv {}
source [file join [file dirname [file normalize [info script]]] support.tcl]
set argv $::argv_saved_for_testserver
unset ::argv_saved_for_testserver

namespace eval ::tclcurl::testserver {
    variable next_service_id 0
    variable service_classes
    array set service_classes {}
}

oo::class create ::tclcurl::testserver::service {
    variable protocol host port quiet listener

    constructor args {
        array set options {
            -protocol {}
            -host 127.0.0.1
            -port {}
            -quiet 0
        }

        foreach {name value} $args {
            if {![info exists options($name)]} {
                error "unknown option: $name"
            }
            set options($name) $value
        }

        if {$options(-protocol) eq {}} {
            error "missing -protocol"
        }
        if {$options(-port) eq {}} {
            error "missing -port"
        }

        set protocol    $options(-protocol)
        set host        $options(-host)
        set port        $options(-port)
        set quiet       $options(-quiet)
        set listener    {}
    }

    destructor {
        my stop
    }

    method protocol {} {
        return $protocol
    }

    method host {} {
        return $host
    }

    method port {} {
        return $port
    }

    method endpoint {} {
        return "[my protocol]://[my host]:[my port]/"
    }

    method log {message} {
        if {!$quiet} {
            puts stderr $message
        }
    }

    method set_listener {chan} {
        set listener $chan
        return $listener
    }

    method stop {} {
        if {$listener ne {}} {
            catch {close $listener}
            set listener {}
        }
    }

    method start {} {
        error "start must be implemented by subclasses"
    }
}

proc ::tclcurl::testserver::usage {} {
    puts stderr "usage: tclsh tests/testserver.tcl ?-host 127.0.0.1? ?-service protocol:port? ... ?-quiet? ?-debug?"
}

proc ::tclcurl::testserver::register_service_class {protocol class_name} {

    variable service_classes
    set service_classes($protocol) $class_name
    namespace ensemble configure ::tclcurl::testserver -map [command_map]
    return $class_name
}

proc ::tclcurl::testserver::service_class {protocol} {
    variable service_classes

    if {![info exists service_classes($protocol)]} {
        error "unsupported protocol: $protocol"
    }
    return $service_classes($protocol)
}

proc ::tclcurl::testserver::parse_service_spec {spec} {
    if {![regexp {^([a-z0-9_+-]+):([0-9]+)$} $spec -> protocol port]} {
        error "invalid service spec: $spec"
    }
    if {$port < 1 || $port > 65535} {
        error "invalid port: $port"
    }

    return [dict create protocol $protocol port $port]
}

proc ::tclcurl::testserver::parse_args {argv} {
    set host 127.0.0.1
    set quiet 0
    set debug 0
    set services [list [dict create protocol http port 8990] \
                       [dict create protocol https port 9443] \
                       [dict create protocol ftp port 8991]]
    set custom_services 0

    for {set i 0} {$i < [llength $argv]} {incr i} {
        set arg [lindex $argv $i]
        switch -- $arg {
            -host {
                incr i
                if {$i >= [llength $argv]} {
                    error "missing value after -host"
                }
                set host [lindex $argv $i]
            }
            -service {
                incr i
                if {$i >= [llength $argv]} {
                    error "missing value after -service"
                }
                if {!$custom_services} {
                    set services {}
                    set custom_services 1
                }
                lappend services [parse_service_spec [lindex $argv $i]]
            }
            -quiet {
                set quiet 1
            }
            -debug {
                set debug 1
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

    return [dict create host $host quiet $quiet debug $debug services $services]
}

proc ::tclcurl::testserver::create_service {protocol host port quiet} {
    variable next_service_id

    set class_name [service_class $protocol]
    set object_name ::tclcurl::testserver::service[incr next_service_id]

    return [$class_name create  $object_name \
                                -protocol $protocol \
                                -host     $host \
                                -port     $port \
                                -quiet    $quiet]
}

proc ::tclcurl::testserver::start_services {config} {
    set host [dict get $config host]
    set quiet [dict get $config quiet]
    set instances {}

    foreach service_spec [dict get $config services] {
        set protocol [dict get $service_spec protocol]
        set port [dict get $service_spec port]
        set service [create_service $protocol $host $port $quiet]
        $service start
        lappend instances $service
    }

    return $instances
}

proc ::tclcurl::testserver::stop_services {services} {
    foreach service $services {
        catch {$service destroy}
    }
}

proc ::tclcurl::testserver::run {argv} {
    set config [parse_args $argv]
    ::tclcurl::test::configure_debug_output [dict get $config debug]
    set services [start_services $config]
    try {
        vwait ::tclcurl::testserver::forever
    } finally {
        stop_services $services
    }
}

proc ::tclcurl::testserver::command_map {} {
    return [dict create \
        create_service ::tclcurl::testserver::create_service \
        parse_args ::tclcurl::testserver::parse_args \
        parse_service_spec ::tclcurl::testserver::parse_service_spec \
        register_service_class ::tclcurl::testserver::register_service_class \
        run ::tclcurl::testserver::run \
        service_class ::tclcurl::testserver::service_class \
        start_services ::tclcurl::testserver::start_services \
        stop_services ::tclcurl::testserver::stop_services \
        usage ::tclcurl::testserver::usage]
}

namespace ensemble create -command ::tclcurl::testserver -map [::tclcurl::testserver::command_map]

source [file join [file dirname [file normalize [info script]]] http_server.tcl]
source [file join [file dirname [file normalize [info script]]] https_server.tcl]
source [file join [file dirname [file normalize [info script]]] ftp_server.tcl]

if {[file normalize $argv0] eq [file normalize [info script]]} {
    if {[catch {::tclcurl::testserver::run $argv} message]} {
        puts stderr $message
        ::tclcurl::testserver::usage
        exit 1
    }
    puts stderr "Server exits..."
}
