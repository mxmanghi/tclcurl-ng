# -- thread_model.tcl
#
#

set thread_script {

    namespace eval :: {

        package require TclOO

        source [file join [pwd] testservers http_application.tcl]
        source [file join [pwd] testservers threads_shared_db.tcl]
        source [file join [pwd] testservers http_application.tcl]
        # source [file join [file dirname [file normalize [info script]]] http_application.tcl]
        source [file join [pwd] testservers logger.tcl]

        oo::class create ::tclcurl::testserver::CMockApplication {
            superclass ::tclcurl::testserver::CApplication

            method request_handling {args} {
                puts "args: $args"
            }

        }

        set logger [::tclcurl::logger new]
        #set app [::tclcurl::testserver::CTestApplication new]
        set app [::tclcurl::testserver::CMockApplication new]

        ::oo::class create ::tclcurl::ApplicationController {
            variable application
            variable accounting

            constructor {app} {
                set accounting ::tclwire::accounting
                set application $app
            }

            method exec_method {method args} {
                $accounting change_thread_status [::thread::id] running
                $app $method {*}$args
                $accounting change_thread_status [::thread::id] idle
            }

        }

        set app_controller [::tclcurl::ApplicationController new $app]

        set master_thread_id ""
        set ::auto_path [concat [file dirname [info script]] $::auto_path]

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


