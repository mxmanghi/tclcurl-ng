# TclCurl - get a URL with FTP, TELNET, LDAP, DICT, FILE, HTTP or HTTPS syntax.

This project was forked from the [flightaware/tclcurl-fa](https://github.com/flightaware/tclcurl-fa)
repository and the contitued as independed development effort since the upstream maintainers
apparently stopped accepting or reacting to PR

Some portions of this project were developed with AI-assisted drafting/editing
tools under human direction, supervision, review, and approval. All accepted changes
were selected, verified, and integrated by the human maintainer.

## Introduction

TclCurl gives the Tcl programmer access to the facilities of libcurl. For more information
about what libcurl is capable of check https://curl.se/

To make type:

      ./configure --with-tcl=<tclConfig.sh directory> ?--enable-threads?
      make
      make install

In order to highlight the continuity with tclcurl-fa version numbering will start with
version 8.0.0

 ### Configure options

The configure script will deduce `$PREFIX` from the tcl installation.
The generated `Makefile` uses the file `$PREFIX/lib/tclConfig.sh` that was left by
the make of tcl for most of its configuration parameters.

 ### Tclcurl specific configure options

 * --with-curlprefix: base directory for the cURL install. If left unspecified
 the path is determined by running `curl-config --prefix`

 * --with-curlinclude: overrrides the curl include files directory (default $curl_prefix/include)

 * --with-md-converter: this options controls which MD converter has to be invoked in order
   to generate the man page out of the markdown source. Accepted values are `lowdown` (default)
   and `pandoc`

 * --enable-deprecated-code: starting with tclcurl 8 options and features deprecated 
   by cURL have been removed. You can re-enable all deprecated or obsoleted options
   by passing the argument `--enable-deprecated-code:all`. You can also selectively
   enable deprecated code passing a comma separated list of the milestone versions which
   deprecated specific features. Accepted arguments
   are:
     + 7.56 - re-enable old style method to construct mime multipart/form-data messages)
     + 7.84 - re-enable deprecated options `-randomfile` and `-egdsocket`
     + 7.11.1 - re-enable `-dnsuseglobalcache`
     + 8.17.0 - re-enable `-krb4level`

 ### OS Support

On a properly Linux/Unix install the package shouldn't have problems to build and run.
To build tclcurl no more than the typical packages for building applications based on
libcurl and libtcl are needed.

*We retained the ReadMeW32.txt that explains how to compile tclcurl on Windows, but this fork
is currently tested only on Linux/Unix*

## Troubleshooting

## Usage

    package require TclCurl

    set curlHandle [curl::init]

    $curlHandle perform

    $curlHandle getinfo curlinfo_option

    $curlHandle duphandle

    $curlHandle cleanup

    curl::transfer

    curl::version

    curl::escape $url

    curl::unescape $url

    curl::curlConfig option

Check the man page for details.

## Authors

- Originally written by:  Andres Garcia (fandom@telefonica.net)
- Cloned from a repo by Steve Havelka once at:  https://bitbucket.org/smh377/tclcurl/ (now broken 2026-04-07)
- Code refactored and test suite developed by Massimo Manghi (massimo.manghi@rivetweb.org). Some parts were 
written with the help of the OpenAI/Codex assistant but every modification or AI generated code has been
evaluated by the author (2024-2026)

## License

BSD like license, check 'license.terms' for details.
