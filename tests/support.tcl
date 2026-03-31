# --support.tcl
#
#

package require tcltest

namespace eval ::tclcurl::test {
}

namespace eval ::tclcurl::test::server {
    variable cached_availability
    variable cached_url
    variable configured_http_server_script
}

proc ::tclcurl::test::repo_root {} {
    return [file dirname [file dirname [file normalize [info script]]]]
}

proc ::tclcurl::test::build_library_candidates {} {
    set root [repo_root]
    lassign [split $::tcl_version "."] tcl_major tcl_minor

    if {$tcl_major == 8} {
        return [glob -nocomplain [file join $root lib*TclCurl*.so]]
    } elseif {$tcl_major == 9} {
        return [glob -nocomplain [file join $root libtcl9*TclCurl*.so]]
    } else {
        puts "for the moment no other OS supported"
    }

#    return [concat \
#        [glob -nocomplain [file join $root libtcl9*TclCurl*.so]] \
#        [glob -nocomplain [file join $root lib*TclCurl*.so]] \
#        [glob -nocomplain [file join $root lib*TclCurl*.dylib]] \
#        [glob -nocomplain [file join $root TclCurl*.dll]]]

    return ""
}

proc ::tclcurl::test::load_package {} {
    foreach libraryPath [build_library_candidates] {
        if {![catch {load $libraryPath Tclcurl}]} {
            set scriptPath [file join [repo_root] generic tclcurl.tcl]
            if {[file exists $scriptPath]} {
                source $scriptPath
            }
            package require TclCurl
            return
        }
    }

    if {![catch {package require TclCurl}]} { return }

    error "unable to load TclCurl from the installed packages or the build tree"
}

proc ::tclcurl::test::env_or_default {name defaultValue} {
    if {[info exists ::env($name)] && $::env($name) ne {}} {
        return $::env($name)
    }
    return $defaultValue
}

proc ::tclcurl::test::server::set_http_server_script {path} {
    variable configured_http_server_script
    set configured_http_server_script [file normalize $path]
    return $configured_http_server_script
}

proc ::tclcurl::test::server::http_server_script {} {
    variable configured_http_server_script

    if {[info exists configured_http_server_script] && $configured_http_server_script ne {}} {
        return $configured_http_server_script
    }

    set defaultScript [file join [::tclcurl::test::repo_root] tests testserver.tcl]
    set scriptPath [::tclcurl::test::env_or_default TCLCURL_TEST_HTTP_SERVER_SCRIPT $defaultScript]
    return [file normalize $scriptPath]
}

proc ::tclcurl::test::server::base_url {{path {}}} {
    set base [::tclcurl::test::env_or_default TCLCURL_TEST_HTTP_BASE_URL \
        [::tclcurl::test::env_or_default TCLCURL_TEST_BASE_URL http://127.0.0.1:8990]]
    set base [string trimright $base /]
    if {$path eq {}} {
        return "${base}/"
    }
    return "${base}/[string trimleft $path /]"
}

proc ::tclcurl::test::curl_root {} {
    return [env_or_default TCLCURL_CURL_ROOT {}]
}

proc ::tclcurl::test::curl_http_dir {} {
    set root [curl_root]
    if {$root eq {}} { return $root }
    return [file join $root tests http]
}

proc ::tclcurl::test::curl_server_built {} {
    set root [curl_root]
    if {$root eq {}} { return 0 }

    return [file isdirectory [file join $root tests server]]
}

proc ::tclcurl::test::server::url_endpoint_available {url} {
    if {![regexp {^https?://([^/:]+)(?::([0-9]+))?(/.*)?$} $url -> host port]} {
        return 0
    }
    if {$port eq {}} { set port 80 }
    if {[catch {set sock [socket $host $port]}]} { return 0 }
    close $sock

    return 1
}

proc ::tclcurl::test::server::http_server_available {} {
    variable cached_availability
    variable cached_url

    set url [base_url]
    if {[info exists cached_availability] && \
        [info exists cached_url] && \
         $cached_url eq $url} {
        return $cached_availability
    }

    set cached_url $url
    set cached_availability [url_endpoint_available $url]
    return $cached_availability
}

proc ::tclcurl::test::with_easy_handle {varName body} {
    upvar 1 $varName handle

    set handle [curl::init]
    catch {uplevel 1 $body} result options
    catch {$handle cleanup}
    unset handle
    return -options $options $result
}

::tcltest::testConstraint curl_http_dir [expr {[::tclcurl::test::curl_http_dir] ne {} && [file isdirectory [::tclcurl::test::curl_http_dir]]}]
::tcltest::testConstraint curl_server_built [::tclcurl::test::curl_server_built]
::tcltest::testConstraint http_server [::tclcurl::test::server::http_server_available]
