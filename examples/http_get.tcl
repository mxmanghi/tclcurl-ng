#!/usr/bin/env tclsh

package require TclCurl

set body {}
set curlHandle [curl::init]

$curlHandle configure \
    -url http://127.0.0.1:8990/tclcurl-man \
    -bodyvar body \
    -noprogress 1

set rc [$curlHandle perform]
set httpCode [$curlHandle getinfo responsecode]

puts "perform rc: $rc"
puts "HTTP response code: $httpCode"
puts "Downloaded bytes: [string length $body]"

$curlHandle cleanup
