package require TclCurl

set fileContent "This is what we will send if it the content is a file"

curl::transfer -url 127.0.0.1/cgi-bin/post1.tcl -verbose 1 -post 1                                                         \
        -httppost [list name "firstName" contents "Andres" contenttype "text/plain" contentheader [list "adios: goodbye"]] \
        -httppost [list name "lastName"  contents "Garcia"]                                                                \
        -httppost [list name "nombre"    bufferName noFile.txt buffer $fileContent contenttype "text/html"]                \
        -httppost [list name "submit"    contents "send"] -verbose 1




