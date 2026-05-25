# -- logger.tcl
#
#

catch {package require syslog}
::oo::class create ::tclcurl::logger {
    if {[catch {package present syslog}]} {
        set log_command "puts"
    } else {
        if {[info exists ::tcl_interactive] && $::tcl_interactive} {
            set log_command [list syslog -perror -ident snig -facility user]
        } else {
            set log_command [list syslog -ident snig -facility user]
        }
    }

    method log {msg {severity info}} {
        eval $log_command $severity "[incr msg_num] - $msg"
    }

}
