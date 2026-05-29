# -- threads_accounting_db.tcl
#
# if we want to venture into a accounting memory model of thread status management,
# procedures and accounting state must have code common to threads
#
# the accounting database is named `tclwire`
#
#  sections:
#
#   - timestamp: timestamp stored when accounting space is instantiated
#   - accounting <thread-id> <thread acconting>
#
#  thread accounting:
#
#   A dictionary having the following keys
#
#	- nruns: number of tasks carried out by the thread
#	- last_run_start: initial time of the last run performed
#	- last_run_end: ending time of the last run performed
#	- status: current status (created, idle, running)
#

package require Thread

namespace eval ::tclwire::accounting {
    variable running_threads

    ::tsv::lock tclwire {
        if {![::tsv::exists tclwire timestamp]} {
            ::tsv::set tclwire timestamp [clock format [clock seconds]]
        }
        if {![::tsv::exists tclwire idle_threads]} {
            ::tsv::set tclwire idle_threads {}
        }
        if {![array exists running_threads]} {
            array set running_threads {}
        }
    }

    proc add_new_thread {tid} {
        ::tsv::lock tclwire {
            #::tsv::keylget tclwire accounting $tid thread_d
            #if {[dict size $thread_d] > 0} {
            #    error "Thread $tid entry exists"
            #}
            ::tsv::keylset tclwire accounting $tid [list nruns           0 \
                                                         last_run_start  0 \
                                                         last_run_end    0 \
                                                         status  	     idle]

            ::tsv::lpush tclwire idle_threads $tid end
        }
    }

    proc SetThreadStatus {tid th_d} {
        ::tsv::keylset tclwire accounting $tid $th_d
    }

    proc get_idle_thread {} {
        return [::tsv::lpop tclwire idle_threads]
    }

    proc change_thread_status {tid newstatus} {
        variable running_threads

        ::tsv::lock tclwire {
            if {[::tsv::keylget tclwire accounting $tid thread_d] == ""} {
                error "Thread $tid account doesn't exists"
            }

            dict with thread_d {
                set current_status $status
                set status $newstatus
                switch $newstatus {
                    running {
                        set last_run_start [clock seconds]
                        set running_threads($tid) $last_run_start
                    }
                    idle {
                        if {[info exists running_threads($tid)]} {
                            unset running_threads($tid)
                        }
                        set last_run_end [clock seconds]
                        if {$current_status == "running"} { incr nruns }
                        ::tsv::lpush tclwire idle_threads $tid end
                    }
                }
            }
            SetThreadStatus $tid $thread_d
        }
    }

    proc RemoveThread {tid} {
        variable running_threads
        variable idle_threads

        ::tsv::keyldel tclwire accounting $tid
        ::tsv::lock tclwire {
            if {[info exists running_threads($tid)]} {
                unset running_thread($tid)
            }
        }
    }

    proc get_thread_account {tid} {
        if {[::tsv::keylget tclwire accounting $tid th_d]} {
            return $th_d
        }
        return ""
    }

    proc num_running_threads {} {
        ::tsv::lock tclwire {
            return [llength [concat {*}[[namespace current]::per_status_lists]]]
        }
    }

    proc per_status_lists {} {
        variable running_threads

        set running_threads_list [array names running_threads]
        set idle_threads [::tsv::get tclwire idle_threads]  

        return [list $running_threads_list $idle_threads]
    }

    proc release_stale_threads {} {
        set to_be_terminated {}
        
        ::tsv::lock tclwire {
            if {[::tsv::exists tclwire accounting]} {
                foreach tid [::tsv::keylkeys tclwire accounting] {
                    set thread_d [::tsv::keylget tclwire accounting $tid]
                    dict with thread_d {
                        if {($status == "idle") && \
                            (($nruns > 10) || (([clock seconds] - $last_run_end) > 60))} {
                            lappend to_be_terminated $tid
                            set status exiting
                        }
                    }
                    ::tsv::keylset tclwire accounting $tid $thread_d
                }
            }
            foreach thread_id $to_be_terminated {
                ::thread::send -async $thread_id { demand_thread_exit }
            }
        }
    }

    proc get_threads_database {} {
        
        ::tsv::lock tclwire {
            set threads_acc_d [dict create]
            foreach tid [::tsv::keylkeys tclwire accounting] {
                dict set threads_acc_d $tid [::tsv::keylget tclwire accounting $tid]
            }
        }
        return $threads_acc_d

    }

    set ns_commands [lmap c [info commands [namespace current]::*] {
        set c [namespace tail $c]
        if {[regexp {[A-Z].*} $c]} {
            continue
        } else {
            set c
        }}
    ]

    namespace export {*}$ns_commands
    namespace ensemble create

    unset ns_commands
    unset c
}
package provide tclwire::accounting 1.1
