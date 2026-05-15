# Examples

This directory contains runnable TclCurl examples extracted from the manual
page and aligned with the local test infrastructure used by the project.

# Starting the Local HTTP Server

Some examples use the HTTP server implemented for the test suite. To start it,
run the following command from the root of the TclCurl source tree:

```sh
tclsh tests/testserver.tcl
```

By default, this starts several local services, including an HTTP server on:

```text
http://127.0.0.1:8990/
```

The HTTP example in this directory uses the path:

```text
http://127.0.0.1:8990/tclcurl-man
```

That path is provided by `tests/http_server.tcl` and returns the HTML version
of the TclCurl manual from `doc/tclcurl.html`.

# Running the Basic HTTP GET Example

After the test server is running, open a second terminal in the root of the
source tree and run:

```sh
tclsh examples/http_get.tcl
```

The script performs a basic HTTP `GET` request, stores the returned document
in a Tcl variable, and prints:

- the TclCurl return code from `perform`
- the HTTP response code
- the number of downloaded bytes

If the request succeeds, the TclCurl return code is `0`, the HTTP response
code is `200`, and the downloaded body contains the HTML manual page served by
the local test server.

# Running the URL-Encoded GET Example

The file `examples/http_urlencoded.tcl` shows how to build a query string for
an HTTP `GET` request using `curl::escape` to encode form-style values safely.

Run it from the root of the source tree with:

```sh
tclsh examples/http_urlencoded.tcl
```

This script sends a request to the local test path:

```text
http://127.0.0.1:8990/request-inspect
```

The server responds with an inspection report that includes the HTTP method,
the request path, and the full request target, including the encoded query
string. This makes it easy to verify that the parameters were encoded and sent
as intended.

# Notes

- The examples assume that TclCurl is available to `package require TclCurl`.
- The commands above should be run from the repository root so that the test
  server can find the expected project files.
