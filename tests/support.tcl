# --support.tcl
#
#

package require tcltest

namespace eval ::tclcurl::test {
    variable cachedServerAvailability
    variable cachedServerUrl
    variable configuredHttpServerScript
}

proc ::tclcurl::test::repoRoot {} {
    return [file dirname [file dirname [file normalize [info script]]]]
}

proc ::tclcurl::test::buildLibraryCandidates {} {
    set root [repoRoot]
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

proc ::tclcurl::test::loadPackage {} {
    foreach libraryPath [buildLibraryCandidates] {
        if {![catch {load $libraryPath Tclcurl}]} {
            set scriptPath [file join [repoRoot] generic tclcurl.tcl]
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

proc ::tclcurl::test::envOrDefault {name defaultValue} {
    if {[info exists ::env($name)] && $::env($name) ne {}} {
        return $::env($name)
    }
    return $defaultValue
}

proc ::tclcurl::test::setHttpServerScript {path} {
    variable configuredHttpServerScript
    set configuredHttpServerScript [file normalize $path]
    return $configuredHttpServerScript
}

proc ::tclcurl::test::httpServerScript {} {
    variable configuredHttpServerScript

    if {[info exists configuredHttpServerScript] && $configuredHttpServerScript ne {}} {
        return $configuredHttpServerScript
    }

    set defaultScript [file join [repoRoot] tests http_server.tcl]
    set scriptPath [envOrDefault TCLCURL_TEST_HTTP_SERVER_SCRIPT $defaultScript]
    return [file normalize $scriptPath]
}

proc ::tclcurl::test::baseUrl {{path {}}} {
    set base [envOrDefault TCLCURL_TEST_HTTP_BASE_URL [envOrDefault TCLCURL_TEST_BASE_URL http://127.0.0.1:8990]]
    set base [string trimright $base /]
    if {$path eq {}} {
        return "${base}/"
    }
    return "${base}/[string trimleft $path /]"
}

proc ::tclcurl::test::curlRoot {} {
    return [envOrDefault TCLCURL_CURL_ROOT {}]
}

proc ::tclcurl::test::curlHttpDir {} {
    set root [curlRoot]
    if {$root eq {}} { return $root }
    return [file join $root tests http]
}

proc ::tclcurl::test::curlServerBuilt {} {
    set root [curlRoot]
    if {$root eq {}} { return 0 }

    return [file isdirectory [file join $root tests server]]
}

proc ::tclcurl::test::urlEndpointAvailable {url} {
    if {![regexp {^https?://([^/:]+)(?::([0-9]+))?(/.*)?$} $url -> host port]} {
        return 0
    }
    if {$port eq {}} { set port 80 }
    if {[catch {set sock [socket $host $port]}]} { return 0 }
    close $sock

    return 1
}

proc ::tclcurl::test::httpServerAvailable {} {
    variable cachedServerAvailability
    variable cachedServerUrl

    set url [baseUrl]
    if {[info exists cachedServerAvailability] && \
        [info exists cachedServerUrl] && \
         $cachedServerUrl eq $url} {
        return $cachedServerAvailability
    }

    set cachedServerUrl $url
    set cachedServerAvailability [urlEndpointAvailable $url]
    return $cachedServerAvailability
}

proc ::tclcurl::test::withEasyHandle {varName body} {
    upvar 1 $varName handle

    set handle [curl::init]
    catch {uplevel 1 $body} result options
    catch {$handle cleanup}
    unset handle
    return -options $options $result
}

::tcltest::testConstraint curl_http_dir [expr {[::tclcurl::test::curlHttpDir] ne {} && [file isdirectory [::tclcurl::test::curlHttpDir]]}]
::tcltest::testConstraint curl_server_built [::tclcurl::test::curlServerBuilt]
::tcltest::testConstraint http_server [::tclcurl::test::httpServerAvailable]
