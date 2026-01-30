/*
 * tcl_size.c --
 *
 * Checking and conversion of 
 *
 * Copyright (c) 2026 Massimo Manghi
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <tcl.h>
#include "tclcompat.h"

/* size_t -> Tcl_Size */
int TclCurl_SizeT2TclSize(size_t in, Tcl_Size *out) {
    if (in > (size_t)TCL_SIZE_MAX) return 0;  /* overflow for Tcl_Size */
    *out = (Tcl_Size)in;
    return 1;
}

/* Tcl_Size -> size_t */
int TclCurl_TclSize2SizeT(Tcl_Size in, size_t *out) {
    if (in < 0) return 0;  /* negative not representable as size_t */
    *out = (size_t)in;
    return 1;
}


