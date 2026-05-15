#!/usr/bin/env tclsh

package require TclCurl

set firstName [curl::escape "Massimo"]
set lastName  [curl::escape "Manghi"]
set city      [curl::escape "Parma Italy"]

set query "firstName=$firstName&lastName=$lastName&city=$city"
set url "http://127.0.0.1:8990/request-inspect?$query"

set body {}
set curlHandle [curl::init]

$curlHandle configure \
    -url $url \
    -bodyvar body \
    -noprogress 1

set rc [$curlHandle perform]
set httpCode [$curlHandle getinfo responsecode]

puts "perform rc: $rc"
puts "HTTP response code: $httpCode"
puts "Server inspection output:"
puts $body

$curlHandle cleanup
