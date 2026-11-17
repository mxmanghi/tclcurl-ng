# TclCurl

This project was forked from the [flightaware/tclcurl-fa](https://github.com/flightaware/tclcurl-fa)
repository and then continued as an independent development effort since the upstream maintainers
apparently stopped accepting or reacting to PRs.

TclCurl gives Tcl programs access to libcurl. It can use HTTP, HTTPS, FTP,
FTPS, SFTP, SCP, SMTP, IMAP, POP3, LDAP and the other protocols enabled in
the linked libcurl build.

The current project supports most options from legacy TclCurl. Some deprecated
options can be included at build time for compatibility, although they may have
no effect when the linked libcurl no longer implements the corresponding
functionality.

Some portions of this project were developed with AI-assisted drafting/editing
tools under human direction, supervision, review, and approval. All accepted changes
were selected, verified, and integrated by the human maintainer, who instructed the
AI assistant on how the code had to be reorganized.

Version 8.0.0 began this fork's version series and marks possible
incompatibilities caused by removing functionality that modern libcurl has
deprecated or made obsolete.

For more information about libcurl's capabilities, see <https://curl.se/>.

## Building on Linux and other Unix-like systems

The required build dependencies are a C compiler, Tcl development files, and
libcurl development files (including `curl-config`). The generated manual and
HTML files are included in the repository; `lowdown` or `pandoc` is needed only
to regenerate them.

Configure with the directory containing `tclConfig.sh`, then build, test, and
install:

```sh
./configure --with-tcl=/path/to/tclConfig.sh-directory
make
make test
make install
```

The configure script normally derives the installation prefix and build
settings from Tcl's `tclConfig.sh`. Use `--prefix` to select a different
installation prefix. Thread support is detected from Tcl automatically; it can
also be controlled explicitly with `--enable-threads` or `--disable-threads`.

### TclCurl-specific configure options

- `--with-curlprefix=DIR`: base directory of the libcurl installation. If it
  is omitted, the path is obtained from `curl-config --prefix`.

- `--with-curlinclude=DIR`: directory containing the public libcurl headers.

- `--with-libcurl=DIR`: directory containing the libcurl library.

- `--with-md-converter=auto|lowdown|pandoc`: select the Markdown converter
  used to regenerate the manual and HTML documentation. The default, `auto`,
  prefers `lowdown`, then `pandoc`, and skips regeneration if neither is found.

- `--enable-deprecated-code=LIST`: restore selected compatibility code removed
  from the default TclCurl 8 build. Use `all` to enable everything, or provide a
  comma-separated list containing one or more of:

  - `7.11.1` for `-dnsuseglobalcache`;
  - `7.56` for the old multipart/form-data API;
  - `7.84` for `-randomfile` and `-egdsocket`;
  - `8.17.0` for `-krb4level`;
  - `8.22.0` for the TLS authentication options removed by that libcurl release.

An optional leading `v` is accepted in each version. For example:

```sh
./configure --with-tcl=/path/to/tclConfig.sh-directory \
    --enable-deprecated-code=7.56,7.84
```

## Building on Windows

The supported Windows build uses Microsoft Visual C++ and `nmake`, rather than
the historical MinGW/self-extracting-package process. It requires:

- a Tcl installation containing Tcl's `nmake` support files;
- a static libcurl installation with headers and libraries;
- the headers and libraries for libcurl's dependencies, currently including
  zlib.

The CI build obtains libcurl and zlib with vcpkg using the
`x64-windows-static` triplet. From a Visual Studio Developer Command Prompt,
change to the repository's `win` directory and run:

```bat
nmake /f makefile.vc INSTALLDIR=C:\tcl ^
    CURLDIR=C:\vcpkg\packages\curl_x64-windows-static ^
    CURLDEPSDIR=C:\vcpkg\packages\zlib_x64-windows-static

nmake /f makefile.vc INSTALLDIR=C:\tcl ^
    CURLDIR=C:\vcpkg\packages\curl_x64-windows-static ^
    CURLDEPSDIR=C:\vcpkg\packages\zlib_x64-windows-static install

nmake /f makefile.vc INSTALLDIR=C:\tcl ^
    CURLDIR=C:\vcpkg\packages\curl_x64-windows-static ^
    CURLDEPSDIR=C:\vcpkg\packages\zlib_x64-windows-static test
```

Adjust the three paths for the local installations. TclCurl uses the protocols,
TLS backend, certificate handling, and other features provided by that libcurl
build; it does not install OpenSSL DLLs or a CA bundle into the Windows system
directories.

## Usage

    package require TclCurl
    curl::transfer
    curl::version
    curl::escape $url
    curl::unescape $url
    curl::curlConfig option
    set curlHandle [curl::init]
    
### Curl Handle methods

    $curlHandle configure
    $curlHandle perform
    $curlHandle getinfo curlinfo_option
    $curlHandle duphandle
    $curlHandle cleanup

Check the man page for details.

## Authors

- Originally written by:  Andres Garcia (fandom@telefonica.net)
- Cloned from a repo by Steve Havelka once at:  https://bitbucket.org/smh377/tclcurl/ (now broken 2026-04-07)
- Code refactored and test suite developed by Massimo Manghi (massimo.manghi@rivetweb.org). Some parts were
written with the help of the OpenAI/Codex assistant but every modification or AI generated code has been
evaluated by the author (2024-2026), who directed step-by-step how the code had to be reorganized.

## License

BSD-like license; see `license.terms` for details.
