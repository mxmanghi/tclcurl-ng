# TclCurl Test Suite

The repository now has two distinct kinds of files under `tests/`:

- `*.test`: real `tcltest` test cases that are executed by `tests/all.tcl`
- `legacy/*.tcl`: legacy example scripts that are useful as references, but are not part of the automated suite

*Last update: 2026-03-31*

## Running the suite

`tests/all.tcl` now tries to load TclCurl from the build tree first and falls back to an installed package if needed.

By default, `tests/all.tcl` enables `tcltest`'s `start` verbosity, so each test prints a line when it begins running. You can still override that from the command line with `-verbose`.

To run only one test file directly through `tcltest`, use `tests/all.tcl` and the `-file` filter:

- `tclsh tests/all.tcl -file http.test`
- `make test TESTFLAGS='-file http.test'`

For the HTTP test integration, set these variables before running `make test` or `tclsh tests/all.tcl`:

- `TCLCURL_TEST_HTTP_BASE_URL`: base URL for the running curl HTTP server, default `http://127.0.0.1:8990/`
- `TCLCURL_TEST_BASE_URL`: generic fallback when the protocol-specific variable is not set
- `TCLCURL_TEST_HTTP_SERVER_SCRIPT`: path to the Tcl test server framework script, 
                                     used when no `-httpserver` CLI override is given; default `tests/testserver.tcl`

You can run the default Tcl test server framework directly with:

- `tclsh tests/testserver.tcl`
- `tclsh tests/testserver.tcl -host 127.0.0.1 -service http:8990`

When the Tcl HTTP test server is wired in, the server script path precedence is:

- `-httpserver /path/to/server.tcl`
- `TCLCURL_TEST_HTTP_SERVER_SCRIPT`
- `tests/testserver.tcl`

If the configured HTTP server is not reachable, the server-backed cases are skipped.

## Current server-backed coverage

`tests/http.test` focuses on the TclCurl behaviors that can already be asserted against curl's generic HTTP test server endpoint:

- basic GET transfers with `-bodyvar`
- HEAD requests with `-headervar`
- callback delivery through `-writeproc`
- `getinfo` transfer metadata
- easy handle lifecycle operations such as `reset` and `duphandle`
- HTTP error handling through `-failonerror` and `-errorbuffer`
- share handle wiring for DNS sharing
- redirect handling through `-followlocation`, `-maxredirs`, `-autoreferer`, and `-postredir`
- cookie state through `-cookiefile` and cookie sharing via `share cookies`

## Migration candidates from `curl/tests/http`

Based on the current TclCurl option surface in `generic/curl_setopts.h`, the strongest next targets for migration are:

- cookies: `-cookiesession`
- request bodies: `-post`, `-postfields`, `-httppost`, `-upload`, `-readproc`
- header handling: `-httpheader`, `-writeheader`, `-header`, `-http200aliases`
- transfer controls: `-range`, `-resumefrom`, `-timeout`, `-timeoutms`, `-connecttimeout`, `-buffersize`
- protocol negotiation: `-httpversion`, `-encoding`, `-transferencoding`, `-contentdecoding`, `-transferdecoding`
- name resolution and routing: `-resolve`, `-ipresolve`, `-interface`, `-port`, `-tcpnodelay`
- auth paths supported by the linked libcurl build: `-userpwd`, `-username`, `-password`, `-httpauth`, `-proxyauth`

## Gaps still worth planning

The curl Perl tests include scenarios that need more than a single HTTP base URL. The bigger remaining gaps are:

- tests that depend on proxy, HTTPS, FTP, SMTP, POP3, or authentication-specific server setups
- cases that need specific response scripts from `curl/tests/data`
- feature-conditional cases such as HTTP/2, HTTP/3, TLS auth, or compression variants that depend on the local libcurl build

Those are good candidates for a second support layer that can map named curl test assets from `CURL_ROOT/tests/http` or 
`tests/data` into Tcl fixtures instead of hard-coding endpoint paths in each test.
