# -- thread_model.tcl
#
#

package require TclOO

set thread_script {

    namespace eval :: {
        source [file join [file dirname [file normalize [info script]]] threads_shared_db.tcl]
        # source [file join [file dirname [file normalize [info script]]] http_application.tcl]
        source [file join [file dirname [file normalize [info script]]] logger.tcl]

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

            constructor {app} {
                set application $app
            }

            method exec_method {method args} {
                
                ::

                $app $method {*}$args
            }

        }

        set app_controller [::tclcurl::ApplicationController $app]

        set master_thread_id ""
        set ::auto_path [concat [file dirname [info script]] $::auto_path]

        proc stop_thread {} { 
            ::thread::release [::thread::id]
        }
        log "thread [::thread::id] created"

        ::thread::wait

        $logger destroy
        log "thread [::thread::id] terminating"
    }

}
