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

    ::tsv::lock tclwire {
        if {![::tsv::exists tclwire timestamp]} {
            ::tsv::set tclwire timestamp [clock format [clock seconds]]
            ::tsv::set tclwire accounting {}
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
                                                         created_on      [clock seconds] \
                                                         command         "" \
                                                         status  	     created]

            ::tsv::lpush tclwire idle_threads $tid end
        }
    }

    proc allocate_idle_thread {} {
        set idle_thread ""
        ::tsv::lock tclwire {
            foreach tid [::tsv::keylkeys tclwire accounting] {
                ::tsv::keylget tclwire accounting $tid thd_d
                if {[dict get $thd_d status] == "idle"} {
                    set idle_thread $tid
                    change_thread_status $tid "allocated"
                    break
                }
            }
        }
        return $idle_thread
    }

    proc change_thread_status {tid newstatus {tcl_command ""}} {
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
                        set command        $tcl_command
                        #::tsv::keylset tclwire running_threads $tid $last_run_start
                    }
                    idle {
                        set last_run_end [clock seconds]
                        if {$current_status == "running"} { incr nruns }
                    }
                    allocated {

                    }
                }
            }
            ::tsv::keylset tclwire accounting $tid $thread_d
        }
    }

    proc remove_thread {tid} {
        ::tsv::keyldel tclwire accounting $tid
    }

    proc get_thread_account {tid} {
        if {[::tsv::keylget tclwire accounting $tid th_d]} {
            return $th_d
        }
        return ""
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

    # -- per_status_list
    #
    # in case the caller needs to have atomic access to the database
    # the call must be within a '::tsv::lock tclwire' block

    proc per_status_lists {} {
        set per_status_db [dict create created {} idle {} running {} terminating {}]

        dict for {tid th_d} [get_threads_database] {
            dict with th_d {
                dict lappend per_status_db $status $tid
            }
        }
        return $per_status_db
    }

    # -- num_running_threads
    #
    # in case the caller needs to have atomic access to the database
    # the call must be within a '::tsv::lock tclwire' block

    proc num_running_threads {} {
        set threads_db [per_status_lists]
        if {[dict exists $threads_db running]} {
            return [llength [dict get $threads_db running]]
        }
        return 0
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
