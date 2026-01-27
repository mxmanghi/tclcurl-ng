package require TclCurl
package require fileutil

# This is one contrived example, but it works.

proc writeToFile {readData} {
    incr ::i
    switch $::i {
        1 { set card "first" }
        2 { set card "second" }
        3 { set card "third" }
        default { set card "${::i}th" }
    } 

    puts "$card call to writeToFile"
    puts -nonewline $::inFile $readData

    return
}

set i 0
set inFile [open "/tmp/cosa.tar" w+]
fconfigure $inFile -translation binary

curl::transfer -url         "https://github.com/flightaware/tclcurl-fa/archive/refs/tags/v7.22.1.tar.gz" \
               -writeproc   writeToFile \
               -buffersize  250

close $inFile
