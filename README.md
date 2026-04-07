# TclCurl - get a URL with FTP, TELNET, LDAP, DICT, FILE, HTTP or HTTPS syntax.

This project was forked from the [flightaware/tclcurl-fa](https://github.com/flightaware/tclcurl-fa)
repository and the contitued as independed development effort since the upstream maintainers
apparently stopped accepting or reacting to PR

## Introduction

TclCurl gives the Tcl programmer access to the facilities of libcurl. For more information
about what libcurl is capable of check https://curl.se/

To make type:

      ./configure --with-tcl=<tclConfig.sh directory> ?--enable-threads?
      make
      make install

The configure script will deduce `$PREFIX` from the tcl installation.
The generated `Makefile` uses the file `$PREFIX/lib/tclConfig.sh` that was left by
the make of tcl for most of its configuration parameters.

*We retained the ReadMeW32.txt that explains how to compile in Windows, but this fork is currently tested only for Unix*

## Troubleshooting

If you are running Red Hat or Fedora and you are compiling from source,
make sure the directory `/usr/local/lib` is listed in `/etc/ld.so.conf`,
if it isn't add it before installing cURL.

If the configure script doesn't detect Tcl even though it is there, it
is probably because there is no `tclConfig.sh` file, maybe you need
to install a `tcl-devel` package or maybe it would be a good idea
to download the latest version, and install that.

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
written with the help of the OpenAI/Codex assistant but every modification and AI generated code has been
evaluated by the author (2024-2026)

## License

BSD like license, check 'license.terms' for details.
