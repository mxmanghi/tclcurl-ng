---
title: TclCurl Share
section: n
date: 19-05-2026
source: TclCurl 8.0.3
volume: TclCurl Easy Interface
---

# NAME

TclCurl - Share selected data between multiple easy handles.

# SYNOPSIS

```tcl
curl::shareinit
curl::sharestrerror errorCode
```

```tcl
shareHandle share ?data?
shareHandle unshare ?data?
shareHandle cleanup
```

# DESCRIPTION

The share API allows two or more easy handles to share selected kinds of data.
At present, TclCurl supports sharing cookie data and DNS cache data.

# curl::shareinit

This procedure must be called first. It returns a `shareHandle`, which you use
to share data between easy handles through the `-share` option of the
`configure` command. Each call to `shareinit` MUST have a corresponding call
to `cleanup` when the operation is complete.

**RETURN VALUE**

The `shareHandle` to use.

# shareHandle share ?data?

This parameter specifies the kind of data to be shared. It may be set to one
of the values described below:

`cookies`
:   Cookie data is shared between the easy handles associated with this shared
    object.

`dns`
:   Cached DNS entries are shared between the easy handles associated with
    this shared object.

    Note that when you use the multi interface, all easy handles added to the
    same multi handle share DNS cache by default, without requiring this
    setting.

# shareHandle unshare ?data?

This command does the opposite of `share`. The specified data type will no
longer be shared. Valid values are the same as for `share`.

# shareHandle cleanup

Deletes a shared object. After this command has been called, the share handle
can no longer be used.

# curl::sharestrerror errorCode

This procedure returns a string describing the error code passed in the
argument.

# SEE ALSO

curl, TclCurl
