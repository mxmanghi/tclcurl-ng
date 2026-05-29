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

catch {::tclwire::ThreadMaster destroy }
source [file join [file dirname [file normalize [info script]]] threads_shared_db.tcl]
source [file join [file dirname [file normalize [info script]]] logger.tcl]

::oo::class create ::tclwire::ThreadMaster {
    variable max_threads_number
    variable accounting
    variable thread_script

    constructor {tscript {mtn 100}} {
        set max_threads_number $mtn
        set accounting ::tclwire::accounting
        set thread_script $tscript
    }

    destructor {
    }

    # -- start_worker_thread <thread-script>
    #
    # Central method for starting new threads executing the 
    # script stored in the procedure only argument
    # 
    # Returns: thread_id
    #

    method start_worker_thread {} {

        set thread_id [thread::create $thread_script]
        thread::preserve $thread_id

        # we allow worker->master thread communication through the ::thread::send
        ::thread::send -async $thread_id [list set ::master_thread_id [::thread::id]]
        return $thread_id

    }

    method get_available_thread {thread_id_v} {
        upvar 1 $thread_id_v thread_id

        set thread_id [$accounting get_idle_thread]
        if {$thread_id != ""} { return true }
        ::tsv::lock tclwire {
            if {[$accounting num_running_threads] < $max_threads_number} { 
                set thread_id [[self] start_worker_thread]
                $accounting add_new_thread $thread_id
                return true
            } 
        }
        return false
    }

    method ListThreads {} {
        return [concat {*}[$accounting per_status_list]]
    }

    method broadcast {cmd} {
        ::tsv::lock tclwire {
            foreach rt [my ListThreads] { thread::send -async $rt $cmd }
        }
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
package provide tclwire::threads 2.0

