# TclCurl Test Suite

The repository now has two distinct kinds of files under `tests/`:

- `*.test`: real `tcltest` test cases that are executed by `tests/all.tcl`
- `legacy/*.tcl`: legacy example scripts that are useful as references, but are not part of the automated suite

*Last update: 2026-04-08*

## Running the suite

`tests/all.tcl` tries to load TclCurl from the build tree first and falls back to an installed package if needed.

By default, `tests/all.tcl` enables `tcltest`'s `start` verbosity, so each test prints a line when it begins running. You can still override that from the command line with `-verbose`.

To run only one test file directly through `tcltest`, use `tests/all.tcl` and the `-file` filter:

- `tclsh tests/all.tcl -file http.test`
- `tclsh tests/all.tcl -file https.test`
- `tclsh tests/all.tcl -file ftp.test`
- `make test TESTFLAGS='-file http.test'`

The Tcl test support layer uses protocol-specific base URLs. You can override them before running `make test` or `tclsh tests/all.tcl`:

- `TCLCURL_TEST_HTTP_BASE_URL`: base URL for the running curl HTTP server, default `http://127.0.0.1:8990/`
- `TCLCURL_TEST_HTTPS_BASE_URL`: base URL for the running Tcl HTTPS test server, default `https://127.0.0.1:9443/`
- `TCLCURL_TEST_FTP_BASE_URL`: base URL for the running Tcl FTP test server, default `ftp://127.0.0.1:8991/`
- `TCLCURL_TEST_FTP_ROOT`: root directory used by the Tcl FTP test server, default `/tmp/ftp`
- `TCLCURL_TEST_HTTP_SERVER_SCRIPT`: path to the Tcl test server framework script, 
                                     used when no `-httpserver` CLI override is given; default `tests/testserver.tcl`

You can run the default Tcl test server framework directly with:

- `tclsh tests/testserver.tcl`
- `tclsh tests/testserver.tcl -host 127.0.0.1 -service http:8990`
- `tclsh tests/testserver.tcl -host 127.0.0.1 -service https:9443 -service ftp:8991`

By default, `tests/testserver.tcl` starts three services:

- HTTP on `127.0.0.1:8990`
- HTTPS on `127.0.0.1:9443`
- FTP on `127.0.0.1:8991`

When the Tcl HTTP test server is wired in, the server script path precedence is:

- `-httpserver /path/to/server.tcl`
- `TCLCURL_TEST_HTTP_SERVER_SCRIPT`
- `tests/testserver.tcl`

If the configured server for a protocol is not reachable, the corresponding server-backed cases are skipped.

## Running secure protocol tests

In order to test the https series of tests you have to create a self-signed pair of key/certificate to be stored in tests/certs

```
 mkdir -p tests/certs

  openssl req   -x509 -newkey rsa:2048 -sha256 -days 3650 -nodes \
                -keyout tests/certs/server.key \
                -out tests/certs/server.crt \
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

`tests/ftp.test` covers the local Tcl FTP server:

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

The HTTPS server requires:

- the Tcl `tls` package
- a local certificate and private key at:
  - `tests/certs/server.crt`
  - `tests/certs/server.key`

These files are intentionally not required to live in the repository. When they are missing, HTTPS-backed tests are skipped.

One simple way to generate them locally is:

```bash
mkdir -p tests/certs

openssl req -x509 -newkey rsa:2048 -sha256 -days 3650 -nodes \
  -keyout tests/certs/server.key \
  -out tests/certs/server.crt \
  -subj "/CN=localhost" \
  -addext "subjectAltName=DNS:localhost,IP:127.0.0.1"
```

## Migration candidates from `curl/tests/http`

Based on the current TclCurl option surface in `generic/curl_setopts.h`, the strongest next targets for migration are:

- transfer controls: `-connecttimeout`
- name resolution and routing: `-resolve`, `-ipresolve`, `-interface`, `-port`, `-tcpnodelay`
- auth paths supported by the linked libcurl build: `-userpwd`, `-username`, `-password`, `-httpauth`, `-proxyauth`

Useful follow-on areas not listed above but still worth covering are:

- callback diagnostics such as `-debugproc`
- advanced multipart forms through `-httppost` (`contentheader`, overridden `filename`, `filecontent`)
- broader metadata and utility APIs such as `getinfo`, `curl::curlConfig`, `easystrerror`, `multistrerror`, and `sharestrerror`

## Gaps still worth planning

The curl Perl tests include scenarios that need more than a single HTTP base URL. The bigger remaining gaps are:

- tests that depend on proxy, SMTP, POP3, or authentication-specific server setups
- cases that need specific response scripts from `curl/tests/data`
- feature-conditional cases such as HTTP/2, HTTP/3, TLS auth, or compression variants that depend on the
  local libcurl build

Those are good candidates for a second support layer that can map named curl test assets from `CURL_ROOT/tests/http` or 
`tests/data` into Tcl fixtures instead of hard-coding endpoint paths in each test.
