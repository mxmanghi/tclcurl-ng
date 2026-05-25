# -- thread_master.tcl -- Implementation of thread pool manager
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
#
#

package require TclOO
package require Thread

catch {::tclcurl::ThreadMaster destroy }
source [file join [file dirname [file normalize [info script]]] logger.tcl]

::oo::class create ::tclcurl::ThreadMaster {
    variable max_threads_number
    variable running_threads
    variable idle_threads_list

    # to be moved into a logger class?
    variable log_command
    variable msg_num

    constructor {{mtn 100}} {
        set max_threads_number      $mtn
        array set running_threads   {}

        set msg_num 0
    }

    destructor {
    }

    method start_worker_thread {thread_script} {

        set thread_id [thread::create $thread_script]

        thread::preserve $thread_id

        # we allow worker->master thread communication through the ::thread::send

        ::thread::send $thread_id [list set ::master_thread_id [::thread::id]]
        lappend idle_threads_list $thread_id
        return $thread_id

    }

    method thread_is_available {} {
        if {[llength $idle_threads_list] > 0} { return true }
        if {[llength $running_threads_list] < $max_threads_number} { return true }
        return false
    }

    method get_available_thread {} {
        set running_threads_list [array names running_threads]
        my log "[llength $running_threads_list] running, [llength $idle_threads_list] idle threads" debug
        if {[llength $idle_threads_list] == 0} {
    
            if {[llength $running_threads_list] < $max_threads_number} {
                set thread_id [my start_worker_thread]
                my log "---> '$thread_id' <---" debug
            } else {
                set m "Internal server error: running threads number exceeds max_threads_number"

                my log $m
                return -code 1 -errorcode thread_not_available $m
            }

        } else {
            set thread_id [lindex $idle_threads_list 0]
        }

        return $thread_id
    }

    method running_threads {} {
        return [array names running_threads]
    }

    method idle_threads {} {
        return $idle_threads_list
    }

    method broadcast {cmd} {
        foreach rt [my running_threads] { thread::send -async $rt $cmd }
    }

    method stop_threads {} {
        set threads_list [my running_threads]
        foreach running_thread $threads_list {
            thread::send -async $running_thread stop_thread
        }

        return [llength $threads_list]
    }

    method terminate_idle_threads {} {
        my log "[llength $idle_threads_list] threads on the idle list" debug
        foreach thread_id $idle_threads_list {
            thread::release $thread_id
        }
    }

}
package provide tclcurl::threads 2.0

