package require sha256

proc ::tclcurl::test::captureWriteProc {chunk} {
    append ::tclcurl::test::writeBuffer $chunk
    incr ::tclcurl::test::writeProcCalls
    return
}

proc ::tclcurl::test::parse_server_response {body} {
    set parsed [dict create]
    foreach line [split [string trim $body] "\n"] {
        if {$line eq {}} {
            continue
        }
        set separator [string first "=" $line]
        if {$separator < 0} {
            continue
        }
        dict set parsed \
            [string range $line 0 [expr {$separator - 1}]] \
            [string range $line [expr {$separator + 1}] end]
    }
    return $parsed
}

proc ::tclcurl::test::readproc_reset {data} {
    set ::tclcurl::test::readproc(data) $data
    set ::tclcurl::test::readproc(offset) 0
    set ::tclcurl::test::readproc(calls) 0
}

proc ::tclcurl::test::readproc_chunk {size} {
    incr ::tclcurl::test::readproc(calls)
    set offset $::tclcurl::test::readproc(offset)
    set chunk [string range $::tclcurl::test::readproc(data) $offset [expr {$offset + $size - 1}]]
    incr ::tclcurl::test::readproc(offset) [string length $chunk]
    return $chunk
}

proc ::tclcurl::test::sha256_hex {data} {
    return [::sha2::sha256 -hex $data]
}

proc ::tclcurl::test::transport_url {base_url_proc {path {}}} {
    return [uplevel #0 [list $base_url_proc $path]]
}

proc ::tclcurl::test::transport_basic_get {base_url_proc extra_opts} {
    set body {}
    ::tclcurl::test::with_easy_handle handle {
        $handle configure -url [::tclcurl::test::transport_url $base_url_proc] \
                          {*}$extra_opts \
                          -bodyvar body \
                          -noprogress 1
        set rc [$handle perform]
        list $rc [$handle getinfo responsecode] [expr {[string length $body] > 0}]
    }
}

proc ::tclcurl::test::transport_head {base_url_proc extra_opts} {
    catch {unset headers}
    ::tclcurl::test::with_easy_handle handle {
        $handle configure -url [::tclcurl::test::transport_url $base_url_proc] \
                          {*}$extra_opts \
                          -nobody 1 \
                          -header 1 \
                          -headervar headers \
                          -noprogress 1
        set rc [$handle perform]
        list $rc [$handle getinfo responsecode] [expr {[array size headers] > 0}]
    }
}

proc ::tclcurl::test::transport_writeproc {base_url_proc extra_opts} {
    set ::tclcurl::test::writeBuffer {}
    ::tclcurl::test::with_easy_handle handle {
        $handle configure -url [::tclcurl::test::transport_url $base_url_proc] \
                          {*}$extra_opts \
                          -writeproc ::tclcurl::test::captureWriteProc \
                          -noprogress 1
        set rc [$handle perform]
        list $rc [$handle getinfo responsecode] [expr {[string length $::tclcurl::test::writeBuffer] > 0}]
    }
}

proc ::tclcurl::test::transport_metadata {base_url_proc extra_opts} {
    set body {}
    set base [::tclcurl::test::transport_url $base_url_proc]
    ::tclcurl::test::with_easy_handle handle {
        $handle configure -url $base \
                          {*}$extra_opts \
                          -bodyvar body \
                          -noprogress 1
        $handle perform
        list [string match ${base}* [$handle getinfo effectiveurl]] \
             [expr {[$handle getinfo headersize] > 0}] \
             [expr {[$handle getinfo sizedownload] > 0}]
    }
}

proc ::tclcurl::test::transport_postfields {base_url_proc extra_opts payload} {
    set body {}
    ::tclcurl::test::with_easy_handle handle {
        $handle configure -url [::tclcurl::test::transport_url $base_url_proc request-inspect] \
                          {*}$extra_opts \
                          -post 1 \
                          -postfields $payload \
                          -bodyvar body \
                          -httpheader [list "Expect:"] \
                          -noprogress 1
        set rc [$handle perform]
        set response [::tclcurl::test::parse_server_response $body]
        list $rc \
             [$handle getinfo responsecode] \
             [dict get $response method] \
             [dict get $response content-type] \
             [expr {[dict get $response content-length] == [string length $payload]}] \
             [expr {[dict get $response body-sha256] eq [::tclcurl::test::sha256_hex $payload]}]
    }
}

proc ::tclcurl::test::transport_postfieldsize {base_url_proc extra_opts payload} {
    set body {}
    ::tclcurl::test::with_easy_handle handle {
        $handle configure -url [::tclcurl::test::transport_url $base_url_proc request-inspect] \
                          {*}$extra_opts \
                          -post 1 \
                          -postfields $payload \
                          -postfieldsize [string length $payload] \
                          -bodyvar body \
                          -httpheader [list "Expect:"] \
                          -noprogress 1
        set rc [$handle perform]
        set response [::tclcurl::test::parse_server_response $body]
        list $rc \
             [$handle getinfo responsecode] \
             [dict get $response method] \
             [dict get $response content-type] \
             [expr {[dict get $response content-length] == [string length $payload]}] \
             [expr {[dict get $response body-sha256] eq [::tclcurl::test::sha256_hex $payload]}]
    }
}

proc ::tclcurl::test::transport_postfieldsize_invalid {base_url_proc extra_opts payload} {
    set body {}
    ::tclcurl::test::with_easy_handle handle {
        list [catch {
                  $handle configure -url [::tclcurl::test::transport_url $base_url_proc request-inspect] \
                                    {*}$extra_opts \
                                    -post 1 \
                                    -postfields $payload \
                                    -postfieldsize bogus \
                                    -bodyvar body \
                                    -httpheader [list "Expect:"] \
                                    -noprogress 1
              } result] $result
    }
}

proc ::tclcurl::test::transport_post_readproc {base_url_proc extra_opts payload} {
    set body {}
    ::tclcurl::test::readproc_reset $payload
    ::tclcurl::test::with_easy_handle handle {
        $handle configure -url [::tclcurl::test::transport_url $base_url_proc request-inspect] \
                          {*}$extra_opts \
                          -post 1 \
                          -readproc ::tclcurl::test::readproc_chunk \
                          -infilesize [string length $payload] \
                          -bodyvar body \
                          -httpheader [list "Expect:"] \
                          -noprogress 1
        set rc [$handle perform]
        set response [::tclcurl::test::parse_server_response $body]
        list $rc \
             [$handle getinfo responsecode] \
             [dict get $response method] \
             [dict get $response content-type] \
             [expr {[dict get $response body-sha256] eq [::tclcurl::test::sha256_hex $payload]}] \
             [expr {$::tclcurl::test::readproc(calls) > 0}]
    }
}

proc ::tclcurl::test::transport_upload_infile {base_url_proc extra_opts fixture} {
    set body {}
    set fh [open $fixture rb]
    try {
        set payload [read $fh]
    } finally {
        close $fh
    }
    ::tclcurl::test::with_easy_handle handle {
        $handle configure -url [::tclcurl::test::transport_url $base_url_proc request-inspect] \
                          {*}$extra_opts \
                          -upload 1 \
                          -infile $fixture \
                          -infilesize [file size $fixture] \
                          -bodyvar body \
                          -httpheader [list "Expect:"] \
                          -noprogress 1
        set rc [$handle perform]
        set response [::tclcurl::test::parse_server_response $body]
        list $rc \
             [$handle getinfo responsecode] \
             [dict get $response method] \
             [dict get $response content-length] \
             [expr {[dict get $response body-sha256] eq [::tclcurl::test::sha256_hex $payload]}]
    }
}

proc ::tclcurl::test::transport_upload_readproc {base_url_proc extra_opts payload} {
    set body {}
    ::tclcurl::test::readproc_reset $payload
    ::tclcurl::test::with_easy_handle handle {
        $handle configure -url [::tclcurl::test::transport_url $base_url_proc request-inspect] \
                          {*}$extra_opts \
                          -upload 1 \
                          -readproc ::tclcurl::test::readproc_chunk \
                          -infilesize [string length $payload] \
                          -bodyvar body \
                          -httpheader [list "Expect:"] \
                          -noprogress 1
        set rc [$handle perform]
        set response [::tclcurl::test::parse_server_response $body]
        list $rc \
             [$handle getinfo responsecode] \
             [dict get $response method] \
             [dict get $response content-length] \
             [expr {[dict get $response body-sha256] eq [::tclcurl::test::sha256_hex $payload]}] \
             [expr {$::tclcurl::test::readproc(calls) > 0}]
    }
}

proc ::tclcurl::test::transport_followlocation_chain {base_url_proc extra_opts} {
    set body {}
    ::tclcurl::test::with_easy_handle handle {
        $handle configure -url [::tclcurl::test::transport_url $base_url_proc redir_0] \
                          {*}$extra_opts \
                          -followlocation 1 \
                          -bodyvar body \
                          -noprogress 1
        set rc [$handle perform]
        list $rc \
             [$handle getinfo responsecode] \
             [$handle getinfo redirectcount] \
             [string match [::tclcurl::test::transport_url $base_url_proc redir_5] [$handle getinfo effectiveurl]] \
             [expr {[::tclcurl::test::normalizedBody $body] eq "path=/redir_5\nmethod=GET"}]
    }
}

proc ::tclcurl::test::transport_maxredirs_error {base_url_proc extra_opts} {
    set errbuf {}
    ::tclcurl::test::with_easy_handle handle {
        $handle configure -url [::tclcurl::test::transport_url $base_url_proc redir_0] \
                          {*}$extra_opts \
                          -followlocation 1 \
                          -maxredirs 4 \
                          -errorbuffer errbuf \
                          -noprogress 1
        list [catch {$handle perform}] \
             [expr {[string length $errbuf] > 0}]
    }
}

proc ::tclcurl::test::transport_autoreferer {base_url_proc extra_opts} {
    set body {}
    set start_url [::tclcurl::test::transport_url $base_url_proc autoreferer-start]
    ::tclcurl::test::with_easy_handle handle {
        $handle configure -url $start_url \
                          {*}$extra_opts \
                          -followlocation 1 \
                          -autoreferer 1 \
                          -bodyvar body \
                          -noprogress 1
        set rc [$handle perform]
        list $rc [expr {[::tclcurl::test::normalizedBody $body] eq "method=GET\nreferer=$start_url"}]
    }
}

proc ::tclcurl::test::transport_postredir_as_get {base_url_proc extra_opts} {
    set body {}
    set start_url [::tclcurl::test::transport_url $base_url_proc postredir-301]
    ::tclcurl::test::with_easy_handle handle {
        $handle configure -url $start_url \
                          {*}$extra_opts \
                          -post 1 \
                          -followlocation 1 \
                          -bodyvar body \
                          -noprogress 1
        set rc [$handle perform]
        list $rc [expr {[::tclcurl::test::normalizedBody $body] eq "method=GET"}]
    }
}

proc ::tclcurl::test::transport_postredir_keep_post {base_url_proc extra_opts} {
    set body {}
    ::tclcurl::test::with_easy_handle handle {
        $handle configure -url [::tclcurl::test::transport_url $base_url_proc postredir-301] \
                          {*}$extra_opts \
                          -post 1 \
                          -followlocation 1 \
                          -postredir 301 \
                          -bodyvar body \
                          -noprogress 1
        set rc [$handle perform]
        list $rc [expr {[::tclcurl::test::normalizedBody $body] eq "method=POST"}]
    }
}

proc ::tclcurl::test::transport_range_single {base_url_proc extra_opts start end} {
    set ::tclcurl::test::writeBuffer {}
    set ::tclcurl::test::writeProcCalls 0
    catch {unset headers}
    set full_payload [::tclcurl::test::range_fixture]
    set expected [string range $full_payload $start $end]
    ::tclcurl::test::with_easy_handle handle {
        $handle configure -url [::tclcurl::test::transport_url $base_url_proc range-data] \
                          {*}$extra_opts \
                          -range "$start-$end" \
                          -writeproc ::tclcurl::test::captureWriteProc \
                          -headervar headers \
                          -noprogress 1
        set rc [$handle perform]
        list $rc \
             [$handle getinfo responsecode] \
             [expr {$::tclcurl::test::writeBuffer eq $expected}] \
             [expr {$::tclcurl::test::writeProcCalls > 1}] \
             [expr {[info exists headers(Content-Range)] && \
                    $headers(Content-Range) eq "bytes $start-$end/[string length $full_payload]"}]
    }
}

proc ::tclcurl::test::transport_range_multi {base_url_proc extra_opts ranges} {
    set body {}
    catch {unset headers}
    set full_payload [::tclcurl::test::range_fixture]
    ::tclcurl::test::with_easy_handle handle {
        $handle configure -url [::tclcurl::test::transport_url $base_url_proc range-data] \
                          {*}$extra_opts \
                          -range [join [lmap r $ranges { join $r "-" }] ","] \
                          -bodyvar body \
                          -headervar headers \
                          -noprogress 1
        set rc [$handle perform]
        set checks [list]
        foreach range $ranges {
            lassign $range start end
            lappend checks [expr {[string first "Content-Range: bytes $start-$end/[string length $full_payload]" $body] >= 0}]
            lappend checks [expr {[string first [string range $full_payload $start $end] $body] >= 0}]
        }
        list $rc \
             [$handle getinfo responsecode] \
             [string match "multipart/byteranges; boundary=*" $headers(Content-Type)] \
             [expr {[lsearch -exact $checks 0] < 0}]
    }
}

proc ::tclcurl::test::transport_timeout_success {base_url_proc extra_opts wait_path timeout_opt timeout_value expected_body} {
    set body {}
    set errbuf {}
    ::tclcurl::test::with_easy_handle handle {
        $handle configure -url [::tclcurl::test::transport_url $base_url_proc $wait_path] \
                          {*}$extra_opts \
                          $timeout_opt $timeout_value \
                          -bodyvar body \
                          -errorbuffer errbuf \
                          -noprogress 1
        set rc [$handle perform]
        list $rc \
             [$handle getinfo responsecode] \
             [expr {[::tclcurl::test::normalizedBody $body] eq $expected_body}] \
             [expr {$errbuf eq {}}]
    }
}

proc ::tclcurl::test::transport_timeout_failure {base_url_proc extra_opts wait_path timeout_opt timeout_value} {
    set body {}
    set errbuf {}
    ::tclcurl::test::with_easy_handle handle {
        $handle configure -url [::tclcurl::test::transport_url $base_url_proc $wait_path] \
                          {*}$extra_opts \
                          $timeout_opt $timeout_value \
                          -bodyvar body \
                          -errorbuffer errbuf \
                          -noprogress 1
        list [catch {$handle perform}] \
             [expr {[string length $errbuf] > 0}] \
             [expr {$body eq {}}]
    }
}

proc ::tclcurl::test::cookieEnginePath {name} {
    set token [format "%X" [clock clicks]]
    return [file join [pwd] "${name}-${token}.cookies"]
}

proc ::tclcurl::test::transport_host {base_url_proc} {
    if {![regexp {^[a-z]+://([^/:]+)} [::tclcurl::test::transport_url $base_url_proc] -> host]} {
        error "unable to determine transport host"
    }
    return $host
}

proc ::tclcurl::test::netscapeCookieRecordFor {base_url_proc name value} {
    return "[::tclcurl::test::transport_host $base_url_proc]\tFALSE\t/\tFALSE\t0\t${name}\t${value}"
}

proc ::tclcurl::test::netscapePersistentCookieRecordFor {base_url_proc name value} {
    set tomorrow [expr {[clock seconds] + 86400}]
    return "[::tclcurl::test::transport_host $base_url_proc]\tFALSE\t/\tFALSE\t${tomorrow}\t${name}\t${value}"
}

proc ::tclcurl::test::transport_cookies_isolated {base_url_proc extra_opts} {
    set primaryBody {}
    set secondaryBody {}
    set primary [curl::init]
    set secondary [curl::init]
    set primaryCookieFile [::tclcurl::test::cookieEnginePath primary]
    set secondaryCookieFile [::tclcurl::test::cookieEnginePath secondary]
    set code [catch {
        $primary configure -url [::tclcurl::test::transport_url $base_url_proc cookie-set/session/alpha] \
                           {*}$extra_opts \
                           -cookiefile $primaryCookieFile \
                           -bodyvar primaryBody \
                           -noprogress 1
        $primary perform

        set primaryBody {}
        $primary configure -url [::tclcurl::test::transport_url $base_url_proc cookie-echo] \
                           {*}$extra_opts \
                           -bodyvar primaryBody \
                           -noprogress 1
        $primary perform

        $secondary configure -url [::tclcurl::test::transport_url $base_url_proc cookie-echo] \
                             {*}$extra_opts \
                             -cookiefile $secondaryCookieFile \
                             -bodyvar secondaryBody \
                             -noprogress 1
        $secondary perform

        list [expr {[::tclcurl::test::normalizedBody $primaryBody] eq "cookie=session=alpha"}] \
             [expr {[::tclcurl::test::normalizedBody $secondaryBody] eq "cookie="}]
    } result options]
    catch {$secondary cleanup}
    catch {$primary cleanup}
    file delete -force $primaryCookieFile $secondaryCookieFile
    return -options $options $result
}

proc ::tclcurl::test::transport_cookies_shared {base_url_proc extra_opts} {
    set firstBody {}
    set secondBody {}
    set share [curl::shareinit]
    set first [curl::init]
    set second [curl::init]
    set firstCookieFile [::tclcurl::test::cookieEnginePath shared-first]
    set secondCookieFile [::tclcurl::test::cookieEnginePath shared-second]
    set code [catch {
        $share share cookies

        $first configure -url [::tclcurl::test::transport_url $base_url_proc cookie-set/session/bravo] \
                         {*}$extra_opts \
                         -share $share \
                         -cookiefile $firstCookieFile \
                         -bodyvar firstBody \
                         -noprogress 1
        $first perform

        $second configure -url [::tclcurl::test::transport_url $base_url_proc cookie-echo] \
                          {*}$extra_opts \
                          -share $share \
                          -cookiefile $secondCookieFile \
                          -bodyvar secondBody \
                          -noprogress 1
        $second perform

        list [expr {[::tclcurl::test::normalizedBody $firstBody] eq "set-cookie=session=bravo"}] \
             [expr {[::tclcurl::test::normalizedBody $secondBody] eq "cookie=session=bravo"}]
    } result options]
    catch {$second cleanup}
    catch {$first cleanup}
    catch {$share cleanup}
    file delete -force $firstCookieFile $secondCookieFile
    return -options $options $result
}

proc ::tclcurl::test::transport_cookielist_inject {base_url_proc extra_opts} {
    set body {}
    set cookieFile [::tclcurl::test::cookieEnginePath cookielist]
    set handle [curl::init]
    set code [catch {
        $handle configure -url [::tclcurl::test::transport_url $base_url_proc cookie-echo] \
                          {*}$extra_opts \
                          -cookiefile $cookieFile \
                          -cookielist [::tclcurl::test::netscapeCookieRecordFor $base_url_proc manual delta] \
                          -bodyvar body \
                          -noprogress 1
        set rc [$handle perform]
        list $rc \
             [expr {[::tclcurl::test::normalizedBody $body] eq "cookie=manual=delta"}] \
             [expr {[llength [$handle getinfo cookielist]] > 0}]
    } result options]
    catch {$handle cleanup}
    file delete -force $cookieFile
    return -options $options $result
}

proc ::tclcurl::test::transport_cookielist_flush {base_url_proc extra_opts} {
    set body {}
    set cookieJar [::tclcurl::test::cookieEnginePath cookiejar]
    set handle [curl::init]
    set jarContents {}
    set code [catch {
        $handle configure -url [::tclcurl::test::transport_url $base_url_proc cookie-echo] \
                          {*}$extra_opts \
                          -cookiejar $cookieJar \
                          -cookielist [::tclcurl::test::netscapeCookieRecordFor $base_url_proc flushme echo] \
                          -bodyvar body \
                          -noprogress 1
        $handle perform
        $handle configure -cookielist FLUSH
        set jarChan [open $cookieJar r]
        set jarContents [read $jarChan]
        close $jarChan
        list [file exists $cookieJar] \
             [expr {[string first "flushme" $jarContents] >= 0}]
    } result options]
    catch {$handle cleanup}
    file delete -force $cookieJar
    return -options $options $result
}

proc ::tclcurl::test::transport_cookiesession {base_url_proc extra_opts} {
    set body {}
    set cookieFile [::tclcurl::test::cookieEnginePath cookiesession]
    set handle [curl::init]
    set cookieChan [open $cookieFile w]
    puts $cookieChan "# Netscape HTTP Cookie File"
    puts $cookieChan [::tclcurl::test::netscapeCookieRecordFor $base_url_proc session stale]
    puts $cookieChan [::tclcurl::test::netscapePersistentCookieRecordFor $base_url_proc persistent alive]
    close $cookieChan
    set code [catch {
        $handle configure -url [::tclcurl::test::transport_url $base_url_proc cookie-echo] \
                          {*}$extra_opts \
                          -cookiefile $cookieFile \
                          -cookiesession 1 \
                          -bodyvar body \
                          -noprogress 1
        $handle perform
        list [expr {[string first "session=stale" $body] < 0}] \
             [expr {[string first "persistent=alive" $body] >= 0}]
    } result options]
    catch {$handle cleanup}
    file delete -force $cookieFile
    return -options $options $result
}
