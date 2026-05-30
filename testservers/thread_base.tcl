# -- thread_model.tcl
#
#

set thread_script {

    namespace eval :: {

        package require TclOO
        package require Thread

        source [file join [pwd] testservers http_application.tcl]
        source [file join [pwd] testservers threads_shared_db.tcl]
        source [file join [pwd] testservers http_application.tcl]
        # source [file join [file dirname [file normalize [info script]]] http_application.tcl]
        source [file join [pwd] testservers logger.tcl]

        namespace eval ::tclcurl::testserver {}

        oo::class create ::tclcurl::testserver::CMockApplication {
            superclass ::tclcurl::testserver::CApplication

            variable logger

            constructor {l} {
                set logger $l
            }

            destructor {
                $logger destroy
            }

            method request_handling {args} {
                $logger "args: $args"
            }

            method wait_for_nsecs {n} {
                $logger log "[::thread::id] starts sleeping"
                after [expr 1000*$n]
                $logger log "[::thread::id] awakes"
            }

        }

        set logger [::tclwire::logger new]
        #set app [::tclcurl::testserver::CTestApplication new]
        set app [::tclcurl::testserver::CMockApplication new $logger]

        $logger log "CMockApplication created as $app"

        ::oo::class create ::tclcurl::ApplicationController {
            variable application
            variable accounting

            constructor {app} {
                set accounting ::tclwire::accounting
                set application $app
            }

            method exec_method {method args} {
                $accounting change_thread_status [::thread::id] running
                $application $method {*}$args
                $accounting change_thread_status [::thread::id] idle
            }

        }

        set app_controller [::tclcurl::ApplicationController new $app]
        $logger log "CApplicationController created as $app_controller"

        set master_thread_id ""
        set ::auto_path [concat [file join [pwd] testservers] $::auto_path]

        proc stop_thread {} {
            ::thread::release [::thread::id]
        }
        $logger log "thread [::thread::id] created"

        ::thread::wait

        $logger log "thread [::thread::id] terminating"
        $logger destroy
    }

};#
#


