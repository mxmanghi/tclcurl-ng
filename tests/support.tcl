# --support.tcl
#
#

package require tcltest

namespace eval ::tclcurl::test {
    variable debug 0
}

namespace eval ::tclcurl::test::server {
    variable cached_http_availability
    variable cached_http_url
    variable cached_https_availability
    variable cached_https_url
    variable cached_ftp_availability
    variable cached_ftp_url
    variable configured_http_server_script
}

proc ::tclcurl::test::repo_root {} {
    return [file dirname [file dirname [file normalize [info script]]]]
}

proc ::tclcurl::test::msgoutput_enabled {args} {
    puts stderr [join $args {}]
}

proc ::tclcurl::test::msgoutput_disabled {args} { }

proc ::tclcurl::test::configure_debug_output {{enabled 0}} {
    variable debug

    set debug [expr {$enabled ? 1 : 0}]
    if {$debug} {
        proc ::tclcurl::test::msgoutput args {
            ::tclcurl::test::msgoutput_enabled {*}$args
        }
    } else {
        proc ::tclcurl::test::msgoutput args {
            ::tclcurl::test::msgoutput_disabled {*}$args
        }
    }
    return $debug
}

proc ::tclcurl::test::build_library_candidates {} {
    set root [repo_root]
    lassign [split $::tcl_version "."] tcl_major tcl_minor

    if {$tcl_major == 8} {
        return [glob -nocomplain [file join $root lib*TclCurl*.so]]
    } elseif {$tcl_major == 9} {
        return [glob -nocomplain [file join $root libtcl9*TclCurl*.so]]
    } else {
        puts "no supported shared library library model found"
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
            ::tclcurl::test::msgoutput "TclCurl loaded from $libraryPath"
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

proc ::tclcurl::test::range_fixture {} {
    return [string repeat "0123456789abcdef" 8192]
}

proc ::tclcurl::test::normalizedBody {body} {
    set normalized [string map [list "\r\n" "\n" "\r" "\n"] $body]
    set lines {}
    foreach line [split $normalized "\n"] {
        if {$line eq {}} {
            continue
        }
        lappend lines $line
    }
    return [join $lines "\n"]
}

proc ::tclcurl::test::server::set_http_server_script {path} {
    variable configured_http_server_script
    set configured_http_server_script [file normalize $path]
    return $configured_http_server_script
}

proc ::tclcurl::test::server::http_server_script {} {
    variable configured_http_server_script

    if {[info exists configured_http_server_script] && \
        ($configured_http_server_script ne {})} {
        return $configured_http_server_script
    }

    set defaultScript [file join [::tclcurl::test::repo_root] tests testserver.tcl]
    set scriptPath [::tclcurl::test::env_or_default TCLCURL_TEST_HTTP_SERVER_SCRIPT $defaultScript]
    return [file normalize $scriptPath]
}

proc ::tclcurl::test::server::base_url {{path {}}} {
    set base [::tclcurl::test::env_or_default TCLCURL_TEST_HTTP_BASE_URL "http://127.0.0.1:8990"]
    set base [string trimright $base /]
    if {$path eq {}} { return "${base}/" }
    return "${base}/[string trimleft $path /]"
}

proc ::tclcurl::test::server::ftp_base_url {{path {}}} {
    set base [::tclcurl::test::env_or_default TCLCURL_TEST_FTP_BASE_URL "ftp://127.0.0.1:8991"]
    set base [string trimright $base /]
    if {$path eq {}} { return "${base}/" }
    return "${base}/[string trimleft $path /]"
}

proc ::tclcurl::test::server::https_base_url {{path {}}} {
    set base [::tclcurl::test::env_or_default TCLCURL_TEST_HTTPS_BASE_URL "https://127.0.0.1:9443"]
    set base [string trimright $base /]
    if {$path eq {}} { return "${base}/" }
    return "${base}/[string trimleft $path /]"
}

proc ::tclcurl::test::https_cert_file {} {
    return [file join [::tclcurl::test::repo_root] tests certs server.crt]
}

proc ::tclcurl::test::https_key_file {} {
    return [file join [::tclcurl::test::repo_root] tests certs server.key]
}

proc ::tclcurl::test::https_credentials_available {} {
    return [expr {[file exists [https_cert_file]] && [file exists [https_key_file]]}]
}

proc ::tclcurl::test::tls_package_available {} {
    return [expr {![catch {package require tls}]}]
}

proc ::tclcurl::test::ftp_root {} {
    return [env_or_default TCLCURL_TEST_FTP_ROOT "/tmp/ftp"]
}

proc ::tclcurl::test::ftp_reset_root {} {
    set root [ftp_root]
    catch {file delete -force $root}
    file mkdir $root
    return $root
}

proc ::tclcurl::test::curl_root {} {
    return [env_or_default CURL_ROOT {}]
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
    if {![regexp {^(?:https?|ftp)://([^/:]+)(?::([0-9]+))?(/.*)?$} $url -> host port]} {
        return 0
    }
    if {$port eq {}} {
        if {[string match "ftp://*" $url]} {
            set port 21
        } else {
            set port 80
        }
    }
    if {[catch {set sock [socket $host $port]}]} { return 0 }
    close $sock

    return 1
}

proc ::tclcurl::test::server::http_server_available {} {
    variable cached_http_availability
    variable cached_http_url

    set url [base_url]
    if {[info exists cached_http_availability] && \
        [info exists cached_http_url] && \
         $cached_http_url eq $url} {
        return $cached_http_availability
    }

    set cached_http_url $url
    set cached_http_availability [url_endpoint_available $url]
    return $cached_http_availability
}

proc ::tclcurl::test::server::ftp_server_available {} {
    variable cached_ftp_availability
    variable cached_ftp_url

    set url [ftp_base_url]
    if {[info exists cached_ftp_availability] && \
        [info exists cached_ftp_url] && \
        $cached_ftp_url eq $url} {
        return $cached_ftp_availability
    }

    set cached_ftp_url $url
    set cached_ftp_availability [url_endpoint_available $url]
    return $cached_ftp_availability
}

proc ::tclcurl::test::server::https_server_available {} {
    variable cached_https_availability
    variable cached_https_url

    set url [https_base_url]
    if {[info exists cached_https_availability] && \
        [info exists cached_https_url] && \
        $cached_https_url eq $url} {
        return $cached_https_availability
    }

    set cached_https_url $url
    set cached_https_availability [url_endpoint_available $url]
    return $cached_https_availability
}

proc ::tclcurl::test::with_easy_handle {varName body} {
    upvar 1 $varName handle

    set handle [curl::init]
    catch {uplevel 1 $body} result options
    catch {$handle cleanup}
    unset handle
    return -options $options $result
}

::tclcurl::test::configure_debug_output [::tclcurl::test::env_or_default TCLCURL_TEST_DEBUG 0]

::tcltest::testConstraint curl_http_dir [expr {[::tclcurl::test::curl_http_dir] ne {} && \
                                               [file isdirectory [::tclcurl::test::curl_http_dir]]}]
::tcltest::testConstraint curl_server_built [::tclcurl::test::curl_server_built]
::tcltest::testConstraint http_server [::tclcurl::test::server::http_server_available]
::tcltest::testConstraint tls_package [::tclcurl::test::tls_package_available]
::tcltest::testConstraint https_credentials [::tclcurl::test::https_credentials_available]
::tcltest::testConstraint https_server [expr {[::tclcurl::test::tls_package_available] && \
                                              [::tclcurl::test::https_credentials_available] && \
                                              [::tclcurl::test::server::https_server_available]}]
::tcltest::testConstraint ftp_server [::tclcurl::test::server::ftp_server_available]
