---
title: TclCurl Multi Interface
section: n
date: 19-05-2026
source: TclCurl 8.0.2
volume: TclCurl Easy Interface
---

# NAME

TclCurl - Perform multiple simultaneous URL transfers with the TclCurl multi interface.

# SYNOPSIS

```tcl
curl::multiinit
curl::multistrerror errorCode
```

```tcl
multiHandle addhandle
multiHandle removehandle
multiHandle configure
multiHandle perform
multiHandle active
multiHandle getinfo
multiHandle cleanup
multiHandle auto ?-command command?
```

# DESCRIPTION

TclCurl's multi interface provides capabilities that are distinct from those
of the easy interface. In particular, it is designed to:

- Provide a "pull" interface, in which the application decides when and how
  data transfers are driven.
- Support multiple simultaneous transfers within the same thread.
- Allow Tk applications to remain responsive while transfers are in progress.

# BLOCKING

Some operations still use blocking code even when the multi interface is used.
You should be aware of the following current limitations:

- Name resolution on non-Windows systems unless c-ares is used.
- GnuTLS SSL connections.
- Active FTP connections.
- HTTP proxy CONNECT operations.
- SOCKS proxy handshakes.
- `file://` transfers.
- TELNET transfers.

# curl::multiinit

This procedure must be called first. It returns a `multiHandle`, which you use
to invoke the TclCurl multi-interface procedures. Each call to `multiinit`
MUST have a corresponding call to `cleanup` when the operation is complete.

**RETURN VALUE**

The `multiHandle` to use.

# multiHandle addhandle ?easyHandle?

Each individual transfer is represented by an easy handle. You must create the
easy handle, configure it with the appropriate options, and then add it to the
multi stack with the `addhandle` command.

If the easy handle is not configured to use a shared or global DNS cache, it
will use the DNS cache shared by all easy handles associated with the same
multi handle.

After an easy handle has been added to a multi stack, you must not call
`perform` on that easy handle directly.

`multiHandle` is the value returned by the `curl::multiinit` call.

**RETURN VALUE**

Possible return values are:

`-1`
:   The handle was added to the multi stack. Call `perform` soon.

`0`
:   The handle was added successfully.

`1`
:   Invalid multi handle.

`2`
:   Invalid easy handle. This may mean that it is not an easy handle, or that
    it is already in use by this or by another multi handle.

`3`
:   Out of memory.

`4`
:   You found a bug in TclCurl.

# multiHandle removehandle ?easyHandle?

When a transfer is complete, or when you want to stop a transfer before it is
complete, you can use the `removehandle` command. Once removed from the multi
handle, the easy handle can again be used with other easy-interface
operations.

Note that when a transfer completes, the easy handle remains associated with
the multi stack. You must remove it explicitly before cleaning it up or
reconfiguring it and adding it again to the multi handle for another
transfer.

**RETURN VALUE**

Possible return values are:

`0`
:   The handle was removed successfully.

`1`
:   Invalid multi handle.

`2`
:   Invalid easy handle.

`3`
:   Out of memory.

`4`
:   You found a bug in TclCurl.

# multiHandle configure

So far the only options are:

**-pipelining**
:   Pass `1` to enable this option or `0` to disable it. When enabled,
    TclCurl will attempt to use HTTP pipelining where possible for transfers
    associated with this multi handle.

    In practice, if you add a second request that can reuse an existing
    connection, that request may be sent on the same connection instead of
    being executed in parallel on a separate one.

**-maxconnects**
:   Pass a number which will be used as the maximum amount of simultaneously
    open connections that TclCurl may cache. The default is `10`. TclCurl may
    enlarge the cache so that it can hold up to four times the number of easy
    handles added to the multi handle.

    By setting this option, you can prevent the cache size from growing beyond
    the limit set by you. When the cache is full, curl closes the oldest one
    in the cache to prevent the number of open connections from increasing.

    This option applies only to multi handles. When using the easy interface,
    use the easy handle's own `maxconnects` option instead.

# multiHandle perform

Adding easy handles to the multi stack does not start any transfer. One of the
main purposes of the multi interface is to let the application drive the
transfers explicitly. You do this by invoking `perform`.

When `perform` is called, TclCurl advances the transfers for the easy handles
in the multi stack that are currently ready for I/O. As data is sent or
received, TclCurl may invoke the callbacks configured on the individual easy
handles, such as read, write, progress, header, or debug callbacks, according
to the state of each transfer. Depending on the state of the transfers, this
may involve all handles, some of them, or none.

If, after a call to `perform`, the number of running handles has changed from
the previous call, or is lower than the number of easy handles added to the
multi handle, one or more transfers are no longer running. In that case, you
can call `getinfo` to obtain information about each completed transfer. If an
added handle fails very quickly, it may never be counted as running.

**RETURN VALUE**

If everything goes well, this command returns the number of running handles,
or `0` if all transfers are complete. In case of error, it returns the error
code.

This command reports only errors related to the multi stack as a whole.
Individual transfers may still have failed even when `perform` itself returns
successfully.

# multiHandle active

Use the `active` command to determine whether any of the easy handles are
currently ready to transfer data before invoking `perform`. It returns the
number of transfers that are currently active.

**RETURN VALUE**

The number of active transfers, or `-1` in case of error.

# multiHandle getinfo

This procedure returns a small amount of information about transfers in the
multi stack. For more detailed information, use the
`getinfo` command on the individual easy handles.

**RETURN VALUE**

A list with the following elements:

- the `easyHandle` to which the information applies
- the state of the transfer, `1` if it is complete
- the transfer exit code, `0` if no error occurred
- the number of messages still in the info queue

If there are no messages in the queue, this command returns `{"" 0 0 0}`.

# multiHandle cleanup

This procedure must be the last one called for a multi stack. It is the
counterpart of `curl::multiinit` and must be called with the same
`multiHandle` that `curl::multiinit` returned.

# multiHandle auto ?-command command?

With this command, Tcl's event loop takes care of periodically invoking
`perform` for you. Before using it, you must already have added at least one
easy handle to the multi handle.

The `-command` option lets you specify a command to invoke after all easy
handles have finished their transfers. In practice, you should use this
command to perform the necessary cleanup of the handles. Otherwise, the
transferred files may not be complete.

This support is still experimental and may change without warning.

You can find a couple of examples at `tests/multi`.

# curl::multistrerror errorCode

This procedure returns a string describing the error code passed in the
argument.

# SEE ALSO

TclCurl, curl
