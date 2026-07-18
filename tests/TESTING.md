---
title: TclCurl test suite
---

# TclCurl Test Suite

The repository now has two distinct kinds of files under `tests/`:

- `*.test`: real `tcltest` test cases that are executed by `tests/all.tcl`
- `legacy/*.tcl`: legacy example scripts that are useful as references, but are not part of the automated suite

*Last update: 2026-07-19*

## Running the suite

`tests/all.tcl` tries to load TclCurl from the build tree first and falls back to an installed package if needed.

By default, `tests/all.tcl` enables `tcltest`'s `start` verbosity, so each test prints a line when it begins running. You can still override that from the command line with `-verbose`.

The server-backed tests are run against
[TclWire](https://github.com/mxmanghi/tclwire).  The HTTP behavior expected by
the TclCurl suite is implemented as a concrete TclWire application in
`tests/tclcurl_test_server.tcl`, with the supplied runtime configuration in
`tests/tclwire.toml`.

To run the server-backed suite from a normal working tree, use this sequence:

1. Start the local test servers in one terminal:

```sh
tcl/tclwire.tcl --config tests/tclwire.toml \
  --logfile /tmp/tclwire.log \
  --dump-multipart-requests \
  --noftp-user-check \
  --certfile /tmp/certs/server.crt \
  --keyfile /tmp/certs/server.key
```

Run that command from a TclWire checkout, or adjust the `tcl/tclwire.tcl` path
to the location of your TclWire runtime.  For example, if the TclCurl
configuration lives in `~/Dropbox/tclcurl/tclwire.toml`:

```sh
tcl/tclwire.tcl --config ~/Dropbox/tclcurl/tclwire.toml \
  --logfile /tmp/tclwire.log \
  --dump-multipart-requests \
  --noftp-user-check \
  --certfile /tmp/certs/server.crt \
  --keyfile /tmp/certs/server.key
```

2. Run the suite in a second terminal from the repository root:

```sh
tclsh tests/all.tcl
```

3. To run only one test file, add `-file`:

```sh
tclsh tests/all.tcl -file http.test
```

If one of the configured protocol endpoints is not reachable, the
corresponding server-backed tests are skipped.

## Related Manual Pages

The generated HTML manuals for TclCurl are available at:

- [tclcurl.html](tclcurl.html)
- [tclcurl\_multi.html](tclcurl_multi.html)
- [tclcurl\_share.html](tclcurl_share.html)

## TclWire Test Application

`tests/tclcurl_test_server.tcl` is the test server implementation used by the
suite.  It is a TclWire application, not a standalone server process.  TclWire
loads it from the application entry in `tests/tclwire.toml`.

The application provides the dynamic routes used by the suite for redirects,
request inspection, authentication, cookies, range requests, content and
transfer decoding, streaming, and multipart upload inspection.  Static file
fallbacks are rooted at the configured document root, normally `/tmp/tclcurl`.

TclWire itself owns the process lifecycle, listener sockets, TLS setup, FTP and
FTPS services, proxy service, logging, request body spooling, and console
control.

## Reference Server

The older `testservers/` implementation remains in the repository as a
reference implementation of the protocol behavior expected by the tests.  It is
useful when comparing exact route behavior or debugging the TclWire
application, but it is no longer the primary server used by the test suite.

`testservers/testserver.tcl` accepts the following general command form:

```text
tclsh testservers/testserver.tcl \
                  ?--host host? \
                  ?--httpport port? ?--httpsport port? ?--ftpport port? ?--proxyport port? \
                  ?--certfile path? ?--keyfile path? \
                  ?--service protocol:port? ... \
                  ?--docroot path? ?--ftproot path? ?--keepdocroot? \
                  ?--logfile path? \
                  ?--startservers protocol[,protocol,...]|all? \
                  ?--quiet? ?--debug?
```

By default, `testservers/testserver.tcl` starts four reference services:

- HTTP on `127.0.0.1:8990`
- HTTPS on `127.0.0.1:9443`
- FTP on `127.0.0.1:8991`
- HTTP proxy on `127.0.0.1:8992`

The reference HTTP server keeps explicit dynamic routes for protocol behaviors
such as redirects, request inspection, and authentication. For `GET` and
`HEAD` requests that do not match one of those routes, it falls back to static
file serving rooted at `TCLCURL_TEST_DOC_ROOT` or `--docroot`.

Unless `--keepdocroot` is given, the server removes the configured document
root when it shuts down.

The server framework appends timestamped per-request log lines to
`/tmp/tclcurl.log` by default. Override that path with `--logfile` when you
want to keep the request log elsewhere.

## `tests/all.tcl`

`tests/all.tcl` accepts a small set of TclCurl-specific options together with
the usual `tcltest` command-line filters and output controls. The general form
is:

```text
tclsh tests/all.tcl \
                  ?--httpserver path/to/server.tcl? \
                  ?--httpport port? ?--httpsport port? ?--ftpport port? ?--proxyport port? \
                  ?--docroot path? ?--ftproot path? \
                  ?--certfile path? ?--keyfile path? \
                  ?--exitserver? ?--debug? \
                  ?--file pattern? ?--notfile pattern? ?--match pattern? ?--skip pattern? \
                  ?--verbose level...?
```

When the reference Tcl HTTP test server is wired in, the server script path precedence is:

- `--httpserver /path/to/server.tcl`
- `TCLCURL_TEST_HTTP_SERVER_SCRIPT`
- `testservers/testserver.tcl`

`testservers/testserver.tcl` accepts `--httpport`, `--httpsport`,
`--ftpport`, and `--proxyport` to override the default ports `8990`,
`9443`, `8991`, and `8992`.

`tests/all.tcl` accepts the same double-dash options `--httpport`,
`--httpsport`, `--ftpport`, and `--proxyport`. It also accepts `--docroot`,
`--ftproot`, `--certfile`, and `--keyfile`, and maps them to the corresponding
`TCLCURL_TEST_*` environment overrides used by the test support layer. When
you need both entry points to target the same services, pass matching values to
each script.

The Tcl test support layer uses protocol-specific base URLs. You can override them before running `make test` or `tclsh tests/all.tcl`:

- `TCLCURL_TEST_HTTP_BASE_URL`: base URL for the running Tcl HTTP test server, default `http://127.0.0.1:8990/`
- `TCLCURL_TEST_HTTPS_BASE_URL`: base URL for the running Tcl HTTPS test server, default `https://127.0.0.1:9443/`
- `TCLCURL_TEST_FTP_BASE_URL`: base URL for the running Tcl FTP test server, default `ftp://127.0.0.1:8991/`
- `TCLCURL_TEST_PROXY_BASE_URL`: base URL for the running Tcl HTTP proxy test server, default `http://127.0.0.1:8992/`
- `TCLCURL_TEST_DOC_ROOT`: shared document root used by the Tcl test servers, default `/tmp/tclcurl`
- `TCLCURL_TEST_FTP_ROOT`: root directory used by the Tcl FTP test server, default `TCLCURL_TEST_DOC_ROOT`
- `TCLCURL_TEST_HTTPS_CERT_FILE`: path to the certificate file used by the Tcl HTTPS test server, default `/tmp/certs/server.crt`
- `TCLCURL_TEST_HTTPS_KEY_FILE`: path to the private key file used by the Tcl HTTPS test server, default `/tmp/certs/server.key`
- `TCLCURL_TEST_HTTP_SERVER_SCRIPT`: path to the reference Tcl test server framework script,
  used when no `-httpserver` CLI override is given; default `testservers/testserver.tcl`

## Running secure protocol tests

In order to test HTTPS or FTPS, create a temporary self-signed certificate and
private key.  The examples below use `/tmp/certs`.

If you want to keep the credentials elsewhere, either:

- start TclWire with `--certfile /tmp/certs/server.crt --keyfile /tmp/certs/server.key`
- or run `tests/all.tcl --certfile /tmp/certs/server.crt --keyfile /tmp/certs/server.key`
- or export `TCLCURL_TEST_HTTPS_CERT_FILE` and `TCLCURL_TEST_HTTPS_KEY_FILE`

```
mkdir -p /tmp/certs

openssl req -x509 -newkey rsa:2048 -sha256 -days 3650 -nodes \
  -keyout /tmp/certs/server.key \
  -out /tmp/certs/server.crt \
  -subj "/CN=localhost" \
  -addext "subjectAltName=DNS:localhost,IP:127.0.0.1"
```

## Shared transport scenarios

`tests/http_common.tcl` contains transport-oriented scenario helpers that are shared by `http.test`, `https.test`, `redir.test`, and `cookies.test`.

The common layer currently covers:

- basic GET / HEAD requests
- callback delivery through `-writeproc`
- `getinfo` transfer metadata
- redirect handling through `-followlocation`, `-maxredirs`, `-autoreferer`, and `-postredir`
- request body handling through `-post`, `-postfields`, `-postfieldsize`, `-upload`, `-readproc`, and `-range`
- timeout handling through `-timeout` and `-timeoutms`
- cookie engine behaviors such as `-cookiefile`, `-cookielist`, `-cookiejar`, and `-cookiesession`

Tests that are primarily about easy-handle lifecycle or share-handle lifecycle stay in the protocol-specific files instead of being pulled into `http_common.tcl`.

## Current server-backed coverage

`tests/http.test` focuses on TclCurl behaviors exercised over plain HTTP:

- basic GET transfers with `-bodyvar`
- HEAD requests with `-headervar`
- callback delivery through `-writeproc`
- `getinfo` transfer metadata
- easy handle lifecycle operations such as `reset` and `duphandle`
- HTTP error handling through `-failonerror` and `-errorbuffer`
- share handle wiring for DNS sharing
- request body handling through `-post`, `-postfields`, `-postfieldsize`, `-upload`, `-readproc`, and `-range`
- timeout handling through `-timeout` and `-timeoutms`

`tests/output.test` covers output routing:

- `-file` and resetting it to the default destination
- `-writeheader` and resetting it to the default destination
- `-stderr` and resetting it to the default destination

`tests/progress.test` covers transfer callbacks and related controls:

- `-command`
- `-progressproc`
- pause / resume callback behavior
- `-buffersize`

`tests/negotiation.test` covers HTTP negotiation-oriented options:

- `-httpversion`
- `-encoding`
- `-transferencoding`
- `-contentdecoding`
- `-transferdecoding`

`tests/https.test` covers the TLS-specific aspects of the same HTTP behaviors:

- HTTPS GET with certificate verification disabled
- HTTPS GET with `-cainfo` pointing at the local self-signed certificate
- expected verification failure when the certificate is not trusted
- shared transport scenarios reused from `tests/http_common.tcl`

`tests/ftp.test` covers the local FTP service:

- upload and download of text and binary files
- directory listing via `LIST` / `NLST`
- `-dirlistonly`
- `-quote` / `-postquote`
- `-ftpcreatemissingdirs`
- `-resumefrom`

`tests/redir.test` and `tests/cookies.test` are thin wrappers around the shared transport scenarios, kept as separate files for readability and focused `tcltest` runs.

`tests/mime.test` currently covers multipart form submission through `-httppost`:

- mixed textual parts
- buffer-backed parts
- file-backed parts

## HTTPS setup

The HTTPS service requires:

- the Tcl `tls` package
- a local certificate and private key

You can override those paths:

- for direct TclWire runs, with `--certfile /tmp/certs/server.crt --keyfile /tmp/certs/server.key`
- for the test suite, with `TCLCURL_TEST_HTTPS_CERT_FILE` and `TCLCURL_TEST_HTTPS_KEY_FILE`

These files are intentionally not required to live in the repository. When they are missing, HTTPS-backed tests are skipped.

One simple way to generate them locally is:

```bash
mkdir -p /tmp/certs

openssl req -x509 -newkey rsa:2048 -sha256 -days 3650 -nodes \
  -keyout /tmp/certs/server.key \
  -out /tmp/certs/server.crt \
  -subj "/CN=localhost" \
  -addext "subjectAltName=DNS:localhost,IP:127.0.0.1"
```

## Areas of Active Coverage Growth

The current suite already covers the core HTTP, HTTPS, and FTP behaviors used
by TclCurl, but some areas are still being expanded. If you are looking for
the parts of the option surface that are most likely to gain broader automated
coverage next, the main groups are:

- transfer controls such as `-connecttimeout`
- name resolution and routing options such as `-resolve`, `-ipresolve`, `-interface`, `-port`, and `-tcpnodelay`
- authentication options supported by the linked libcurl build, such as `-userpwd`, `-username`, `-password`, `-httpauth`, and `-proxyauth`

Other areas that are also expected to grow over time include:

- callback diagnostics such as `-debugproc`
- advanced multipart form handling through `-httppost`, including `contentheader`, overridden `filename`, and `filecontent`
- broader metadata and utility APIs such as `getinfo`, `curl::curlConfig`, `easystrerror`, `multistrerror`, and `sharestrerror`

## Current Limitations

Although the local test framework now covers a substantial part of TclCurl's
single-transfer behavior, some categories still have more limited automated
coverage.

The main remaining limitations are:

- scenarios that depend on more specialized server setups, such as proxy, SMTP, POP3, or authentication-specific environments
- cases that require response scripts or fixtures derived from `curl/tests/data`
- feature-conditional behaviors such as HTTP/2, HTTP/3, TLS auth, or compression variants that depend on how the local libcurl build was configured

In practice, this means that the absence of a test in one of those areas does
not necessarily indicate lack of support in TclCurl. It more often means that
the local test framework has not yet been extended to exercise that behavior
in a reproducible way.
