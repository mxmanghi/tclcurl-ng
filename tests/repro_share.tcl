source [file join [file dirname [file normalize [info script]]] support.tcl]

::tclcurl::test::load_package

set share [curl::shareinit]
set first [curl::init]
set second [curl::init]
set firstBody {}
set secondBody {}

$share share dns
$first configure -url [::tclcurl::test::server::base_url] -share $share -bodyvar firstBody -noprogress 1
$second configure -url [::tclcurl::test::server::base_url] -share $share -bodyvar secondBody -noprogress 1

puts before1
$first perform
puts after1
$second perform
puts after2
puts [list [string length $firstBody] [string length $secondBody]]

catch {$second cleanup}
catch {$first cleanup}
catch {$share cleanup}
