/*
 * curl_mime.h --
 *
 * mime formatted multipart form data management. 
 *
 * This file is partially derived from tclcurl-fa.
 *
 * Copyright (c) 2001-2011 Andres Garcia Garcia
 * Copyright (c) 2024-2026 Massimo Manghi
 *
 * SPDX-License-Identifier: TCL
 *
 * See the file "license.terms" at the top level of this distribution
 * for information on usage and redistribution of this file, and for the
 * complete disclaimer of warranties and limitation of liability.
 *
 */

#ifndef __curl_mime_h__
#define __curl_mime_h__

typedef struct TclCurlOptionDef TclCurlOptionDef;
typedef struct TclCurlOptsArgs TclCurlOptsArgs;

int     TclCurl_SetPostData(Tcl_Interp *interp,struct curlObjData *curlDataPtr);
void    TclCurl_ResetPostData(struct curlObjData *curlDataPtr);
/* void    TclCurl_ResetFormArray(struct curl_forms *formArray); */
/* Internal helper used by the setopts handler table. */
int     TclCurl_HandleHttpPost(TclCurlOptsArgs *args);

#endif
