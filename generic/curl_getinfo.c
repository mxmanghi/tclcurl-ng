/*
 * curl_getinfo.c -- Functions related to the 'curl_easy_getinfo' function in libcurl.
 *
 * Implementation of the TclCurl extension that creates the curl namespace
 * so that Tcl interpreters can access libcurl.
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
 */    

#include <tcl.h>
#include <curl/curl.h>
#include <curl/easy.h>

#define TCLCURL_GETINFO_LIST(X) \
    X(TCLCURL_INFO_EFFECTIVEURL, "effectiveurl") \
    X(TCLCURL_INFO_HTTPCODE, "httpcode") \
    X(TCLCURL_INFO_RESPONSECODE, "responsecode") \
    X(TCLCURL_INFO_FILETIME, "filetime") \
    X(TCLCURL_INFO_TOTALTIME, "totaltime") \
    X(TCLCURL_INFO_NAMELOOKUPTIME, "namelookuptime") \
    X(TCLCURL_INFO_CONNECTTIME, "connecttime") \
    X(TCLCURL_INFO_PRETRANSFERTIME, "pretransfertime") \
    X(TCLCURL_INFO_SIZEUPLOAD, "sizeupload") \
    X(TCLCURL_INFO_SIZEDOWNLOAD, "sizedownload") \
    X(TCLCURL_INFO_SPEEDDOWNLOAD, "speeddownload") \
    X(TCLCURL_INFO_SPEEDUPLOAD, "speedupload") \
    X(TCLCURL_INFO_HEADERSIZE, "headersize") \
    X(TCLCURL_INFO_REQUESTSIZE, "requestsize") \
    X(TCLCURL_INFO_SSLVERIFYRESULT, "sslverifyresult") \
    X(TCLCURL_INFO_CONTENTLENGTHDOWNLOAD, "contentlengthdownload") \
    X(TCLCURL_INFO_CONTENTLENGTHUPLOAD, "contentlengthupload") \
    X(TCLCURL_INFO_STARTTRANSFERTIME, "starttransfertime") \
    X(TCLCURL_INFO_CONTENTTYPE, "contenttype") \
    X(TCLCURL_INFO_REDIRECTTIME, "redirecttime") \
    X(TCLCURL_INFO_REDIRECTCOUNT, "redirectcount") \
    X(TCLCURL_INFO_HTTPAUTHAVAIL, "httpauthavail") \
    X(TCLCURL_INFO_PROXYAUTHAVAIL, "proxyauthavail") \
    X(TCLCURL_INFO_OSERRNO, "oserrno") \
    X(TCLCURL_INFO_NUMCONNECTS, "numconnects") \
    X(TCLCURL_INFO_SSLENGINES, "sslengines") \
    X(TCLCURL_INFO_HTTPCONNECTCODE, "httpconnectcode") \
    X(TCLCURL_INFO_COOKIELIST, "cookielist") \
    X(TCLCURL_INFO_FTPENTRYPATH, "ftpentrypath") \
    X(TCLCURL_INFO_REDIRECTURL, "redirecturl") \
    X(TCLCURL_INFO_PRIMARYIP, "primaryip") \
    X(TCLCURL_INFO_APPCONNECTTIME, "appconnecttime") \
    X(TCLCURL_INFO_CERTINFO, "certinfo") \
    X(TCLCURL_INFO_CONDITIONUNMET, "conditionunmet") \
    X(TCLCURL_INFO_PRIMARYPORT, "primaryport") \
    X(TCLCURL_INFO_LOCALIP, "localip") \
    X(TCLCURL_INFO_LOCALPORT, "localport")

typedef enum {
#define TCLCURL_GETINFO_ENUM_ENTRY(name, label) name,
    TCLCURL_GETINFO_LIST(TCLCURL_GETINFO_ENUM_ENTRY)
#undef TCLCURL_GETINFO_ENUM_ENTRY
    TCLCURL_INFO_COUNT
} TclCurlGetInfoId;

static const char *const getInfoTable[TCLCURL_INFO_COUNT + 1] = {
#define TCLCURL_GETINFO_TABLE_ENTRY(name, label) label,
    TCLCURL_GETINFO_LIST(TCLCURL_GETINFO_TABLE_ENTRY)
#undef TCLCURL_GETINFO_TABLE_ENTRY
    NULL
};

/*----------------------------------------------------------------------
 *
 * curlGetInfo --
 *
 *  Invokes the 'curl_easy_getinfo' function in libcurl.
 *
 * Parameters:
 *
 * Results:
 *   0 if all went well.
 *   The CURLcode for the error.
 *
 *---------------------------------------------------------------------*/
static Tcl_Obj*
curlGetInfo(Tcl_Interp *interp,CURL *curlHandle,int tableIndex) {
    char*                    charPtr;
    long                     longNumber;
    double                   doubleNumber;
    struct curl_slist*       slistPtr;
    struct curl_slist*       slistHead;
    struct curl_certinfo*    certinfoPtr=NULL;
    int                      i;

    Tcl_Obj*    resultObjPtr = NULL;

    switch(tableIndex) {
        case TCLCURL_INFO_EFFECTIVEURL:
            if (!curl_easy_getinfo(curlHandle,CURLINFO_EFFECTIVE_URL,&charPtr)) {
                resultObjPtr = Tcl_NewStringObj(charPtr,-1);
            }
            break;
        case TCLCURL_INFO_HTTPCODE:
        case TCLCURL_INFO_RESPONSECODE:
            if (!curl_easy_getinfo(curlHandle,CURLINFO_RESPONSE_CODE,&longNumber)) {
                resultObjPtr = Tcl_NewLongObj(longNumber);
            }
            break;
        case TCLCURL_INFO_FILETIME:
            if (!curl_easy_getinfo(curlHandle,CURLINFO_FILETIME,&longNumber)) {
                resultObjPtr = Tcl_NewLongObj(longNumber);
            }
            break;
        case TCLCURL_INFO_TOTALTIME:
            if (!curl_easy_getinfo(curlHandle,CURLINFO_TOTAL_TIME,&doubleNumber)) {
                resultObjPtr = Tcl_NewDoubleObj(doubleNumber);
            }
            break;
        case TCLCURL_INFO_NAMELOOKUPTIME:
            if (!curl_easy_getinfo(curlHandle,CURLINFO_NAMELOOKUP_TIME,&doubleNumber)) {
                resultObjPtr = Tcl_NewDoubleObj(doubleNumber);
            }
            break;
        case TCLCURL_INFO_CONNECTTIME:
            if (!curl_easy_getinfo(curlHandle,CURLINFO_CONNECT_TIME,&doubleNumber)) {
                resultObjPtr = Tcl_NewDoubleObj(doubleNumber);
            }
            break;
        case TCLCURL_INFO_PRETRANSFERTIME:
            if (!curl_easy_getinfo(curlHandle,CURLINFO_PRETRANSFER_TIME,&doubleNumber)) {
                resultObjPtr = Tcl_NewDoubleObj(doubleNumber);
            }
            break;
        case TCLCURL_INFO_SIZEUPLOAD:
        {
            curl_off_t longCurlInt;

            if (!curl_easy_getinfo(curlHandle,CURLINFO_SIZE_UPLOAD_T,&longCurlInt)) {
                resultObjPtr = Tcl_NewWideIntObj((Tcl_WideInt)longCurlInt);
            }
            break;
        }
        case TCLCURL_INFO_SIZEDOWNLOAD:
        {
            curl_off_t longCurlInt;

            if (!curl_easy_getinfo(curlHandle,CURLINFO_SIZE_DOWNLOAD_T,&longCurlInt)) {
                resultObjPtr = Tcl_NewWideIntObj((Tcl_WideInt)longCurlInt);
            }
            break;
        }
        case TCLCURL_INFO_SPEEDDOWNLOAD:
        {
            curl_off_t longCurlInt;

            if (!curl_easy_getinfo(curlHandle,CURLINFO_SPEED_DOWNLOAD_T,&longCurlInt)) {
                resultObjPtr = Tcl_NewWideIntObj((Tcl_WideInt)longCurlInt);
            }
            break;
        }
        case TCLCURL_INFO_SPEEDUPLOAD:
        {
            curl_off_t longCurlInt;

            if (!curl_easy_getinfo(curlHandle,CURLINFO_SPEED_UPLOAD_T,&longCurlInt)) {
                resultObjPtr = Tcl_NewWideIntObj((Tcl_WideInt)longCurlInt);
            }
            break;
        }
        case TCLCURL_INFO_HEADERSIZE:
            if (!curl_easy_getinfo(curlHandle,CURLINFO_HEADER_SIZE,&longNumber)) {
                resultObjPtr = Tcl_NewLongObj(longNumber);
            }
            break;
        case TCLCURL_INFO_REQUESTSIZE:
            if (!curl_easy_getinfo(curlHandle,CURLINFO_REQUEST_SIZE,&longNumber)) {
                resultObjPtr = Tcl_NewLongObj(longNumber);
            }
            break;
        case TCLCURL_INFO_SSLVERIFYRESULT:
            if (!curl_easy_getinfo(curlHandle,CURLINFO_SSL_VERIFYRESULT,&longNumber)) {
                resultObjPtr = Tcl_NewLongObj(longNumber);
            }
            break;
        case TCLCURL_INFO_CONTENTLENGTHDOWNLOAD:
        {
            curl_off_t longCurlInt;

            if (!curl_easy_getinfo(curlHandle,CURLINFO_CONTENT_LENGTH_DOWNLOAD_T,&longCurlInt)) {
                resultObjPtr = Tcl_NewWideIntObj((Tcl_WideInt)longCurlInt);
            }
            break;
        }
        case TCLCURL_INFO_CONTENTLENGTHUPLOAD:
        {
            curl_off_t longCurlInt;

            if (!curl_easy_getinfo(curlHandle,CURLINFO_CONTENT_LENGTH_UPLOAD_T,&longCurlInt)) {
                resultObjPtr = Tcl_NewWideIntObj((Tcl_WideInt)longCurlInt);
            }
            break;
        }
        case TCLCURL_INFO_STARTTRANSFERTIME:
            if (!curl_easy_getinfo(curlHandle,CURLINFO_STARTTRANSFER_TIME,&doubleNumber)) {
                resultObjPtr = Tcl_NewDoubleObj(doubleNumber);
            }
            break;
        case TCLCURL_INFO_CONTENTTYPE:
            if (!curl_easy_getinfo(curlHandle,CURLINFO_CONTENT_TYPE,&charPtr)) {
                resultObjPtr = Tcl_NewStringObj(charPtr,-1);
            }
            break;
        case TCLCURL_INFO_REDIRECTTIME:
            if (!curl_easy_getinfo(curlHandle,CURLINFO_REDIRECT_TIME,&doubleNumber)) {
                resultObjPtr = Tcl_NewDoubleObj(doubleNumber);
            }
            break;
        case TCLCURL_INFO_REDIRECTCOUNT:
            if (!curl_easy_getinfo(curlHandle,CURLINFO_REDIRECT_COUNT,&longNumber)) {
                resultObjPtr = Tcl_NewLongObj(longNumber);
            }
            break;
        case TCLCURL_INFO_HTTPAUTHAVAIL:
        case TCLCURL_INFO_PROXYAUTHAVAIL:
            if (tableIndex == TCLCURL_INFO_HTTPAUTHAVAIL) {
                if (!curl_easy_getinfo(curlHandle,CURLINFO_HTTPAUTH_AVAIL,&longNumber)) {
                    resultObjPtr = Tcl_NewListObj(0,(Tcl_Obj **)NULL);
                }
            } else {
                if (!curl_easy_getinfo(curlHandle,CURLINFO_PROXYAUTH_AVAIL,&longNumber)) {
                    resultObjPtr = Tcl_NewListObj(0,(Tcl_Obj **)NULL);
                }
            }
            if (resultObjPtr != NULL) {
                if (longNumber&CURLAUTH_BASIC) {
                    Tcl_ListObjAppendElement(interp,resultObjPtr,Tcl_NewStringObj("basic",-1));
                }
                if (longNumber&CURLAUTH_DIGEST) {
                    Tcl_ListObjAppendElement(interp,resultObjPtr,Tcl_NewStringObj("digest",-1));
                }
                if (longNumber&CURLAUTH_GSSNEGOTIATE) {
                    Tcl_ListObjAppendElement(interp,resultObjPtr,Tcl_NewStringObj("gssnegotiate",-1));
                }
                if (longNumber&CURLAUTH_NTLM) {
                    Tcl_ListObjAppendElement(interp,resultObjPtr,Tcl_NewStringObj("NTLM",-1));
                }
            }
            break;
        case TCLCURL_INFO_OSERRNO:
            if (!curl_easy_getinfo(curlHandle,CURLINFO_OS_ERRNO,&longNumber)) {
                resultObjPtr = Tcl_NewLongObj(longNumber);
            }
            break;
        case TCLCURL_INFO_NUMCONNECTS:
            if (!curl_easy_getinfo(curlHandle,CURLINFO_NUM_CONNECTS,&longNumber)) {
                resultObjPtr = Tcl_NewLongObj(longNumber);
            }
            break;
        case TCLCURL_INFO_SSLENGINES:
            if (!curl_easy_getinfo(curlHandle,CURLINFO_SSL_ENGINES,&slistPtr)) {
                resultObjPtr = Tcl_NewListObj(0,(Tcl_Obj **)NULL);
                slistHead = slistPtr;
                while (slistPtr != NULL) {
                    Tcl_ListObjAppendElement(interp,resultObjPtr,Tcl_NewStringObj(slistPtr->data,-1));
                    slistPtr = slistPtr->next;
                }
                curl_slist_free_all(slistHead);
            }
            break;
        case TCLCURL_INFO_HTTPCONNECTCODE:
            if (!curl_easy_getinfo(curlHandle,CURLINFO_HTTP_CONNECTCODE,&longNumber)) {
                resultObjPtr = Tcl_NewLongObj(longNumber);
            }
            break;
        case TCLCURL_INFO_COOKIELIST:
            if (!curl_easy_getinfo(curlHandle,CURLINFO_COOKIELIST,&slistPtr)) {
                resultObjPtr = Tcl_NewListObj(0,(Tcl_Obj **)NULL);
                slistHead = slistPtr;
                while (slistPtr!=NULL) {
                    Tcl_ListObjAppendElement(interp,resultObjPtr,Tcl_NewStringObj(slistPtr->data,-1));
                    slistPtr = slistPtr->next;
                }
                curl_slist_free_all(slistHead);
            }
            break;
        case TCLCURL_INFO_FTPENTRYPATH:
            if (!curl_easy_getinfo(curlHandle,CURLINFO_FTP_ENTRY_PATH,&charPtr)) {
                resultObjPtr = Tcl_NewStringObj(charPtr,-1);
            }
            break;
        case TCLCURL_INFO_REDIRECTURL:
            if (!curl_easy_getinfo(curlHandle,CURLINFO_REDIRECT_URL,&charPtr)) {
                resultObjPtr = Tcl_NewStringObj(charPtr,-1);
            }
            break;
        case TCLCURL_INFO_PRIMARYIP:
            if (!curl_easy_getinfo(curlHandle,CURLINFO_PRIMARY_IP,&charPtr)) {
                resultObjPtr = Tcl_NewStringObj(charPtr,-1);
            }
            break;
        case TCLCURL_INFO_APPCONNECTTIME:
            if (!curl_easy_getinfo(curlHandle,CURLINFO_APPCONNECT_TIME,&doubleNumber)) {
                resultObjPtr = Tcl_NewDoubleObj(doubleNumber);
            }
            break;
        case TCLCURL_INFO_CERTINFO:
            if (!curl_easy_getinfo(curlHandle,CURLINFO_CERTINFO,&certinfoPtr)) {
                resultObjPtr = Tcl_NewListObj(0,(Tcl_Obj **)NULL);
                Tcl_ListObjAppendElement(interp,resultObjPtr,Tcl_NewIntObj(certinfoPtr->num_of_certs));
                for (i=0; i < certinfoPtr->num_of_certs; i++) {
                    for (slistPtr = certinfoPtr->certinfo[i]; slistPtr; slistPtr=slistPtr->next) {
                        Tcl_ListObjAppendElement(interp,resultObjPtr,Tcl_NewStringObj(slistPtr->data,-1));
                    }
                }
            }
            break;
        case TCLCURL_INFO_CONDITIONUNMET:
            if (!curl_easy_getinfo(curlHandle,CURLINFO_CONDITION_UNMET,&longNumber)) {
                resultObjPtr = Tcl_NewLongObj(longNumber);
            }
            break;
        case TCLCURL_INFO_PRIMARYPORT:
            if (!curl_easy_getinfo(curlHandle,CURLINFO_PRIMARY_PORT,&longNumber)) {
                resultObjPtr = Tcl_NewLongObj(longNumber);
            }
            break;
        case TCLCURL_INFO_LOCALIP:
            if (!curl_easy_getinfo(curlHandle,CURLINFO_LOCAL_IP,&charPtr)) {
                resultObjPtr = Tcl_NewStringObj(charPtr,-1);
            }
            break;
        case TCLCURL_INFO_LOCALPORT:
            if (!curl_easy_getinfo(curlHandle,CURLINFO_LOCAL_PORT,&longNumber)) {
                resultObjPtr = Tcl_NewLongObj(longNumber);
            }
            break;
    }
    return resultObjPtr;
}


int TclCurl_GetInfo (Tcl_Interp* interp,Tcl_Obj* get_info_arg,CURL* curlHandle,Tcl_Obj** result_p)
{
    int tableIndex;
    Tcl_Obj* resultObj;

    if (Tcl_GetIndexFromObj(interp,get_info_arg,getInfoTable,
                            "getinfo option",TCL_EXACT,&tableIndex) == TCL_ERROR) {
        return 1;
    }

    resultObj = curlGetInfo(interp,curlHandle,tableIndex);
    if (resultObj == NULL) {
        return 1;
    }
    *result_p = resultObj;

    return 0;
}
