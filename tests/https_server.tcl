namespace eval ::tclcurl::testserver {}

package require tls

oo::class create ::tclcurl::testserver::https_service {
    superclass ::tclcurl::testserver::http_service

    method start {} {
        if {![::tclcurl::test::https_credentials_available]} {
            error "https credentials not available"
        }

        set listener [::tls::socket -server [list [self] accept] \
            -myaddr [my host] \
            -certfile [::tclcurl::test::https_cert_file] \
            -keyfile [::tclcurl::test::https_key_file] \
            -ssl2 0 \
            -ssl3 0 \
            [my port]]
        my set_listener $listener
        my log "listening on [my endpoint]"
        return $listener
    }
}

::tclcurl::testserver register_service_class https ::tclcurl::testserver::https_service
