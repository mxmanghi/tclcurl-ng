/*
 *----------------------------------------------------------------------
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
 *----------------------------------------------------------------------
 */

#include <tcl.h>
#include <curl/curl.h>

const static char *getInfoTable[] = {
    "effectiveurl",   "httpcode",       "responsecode",
    "filetime",       "totaltime",      "namelookuptime",
    "connecttime",    "pretransfertime","sizeupload",
    "sizedownload",   "speeddownload",  "speedupload",
    "headersize",     "requestsize",    "sslverifyresult",
    "contentlengthdownload",            "contentlengthupload",
    "starttransfertime",                "contenttype",
    "redirecttime",   "redirectcount",  "httpauthavail",
    "proxyauthavail", "oserrno",        "numconnects",
    "sslengines",     "httpconnectcode","cookielist",
    "ftpentrypath",   "redirecturl",    "primaryip",
    "appconnecttime", "certinfo",       "conditionunmet",
    "primaryport",    "localip",        "localport",
    (char *)NULL
};

CURLcode
curlGetInfo(Tcl_Interp *interp,CURL *curlHandle,int tableIndex) {
    char                    *charPtr;
    long                     longNumber;
    double                   doubleNumber;
    struct curl_slist       *slistPtr;
    struct curl_certinfo    *certinfoPtr=NULL;
    int                      i;

    CURLcode    exitCode;

    Tcl_Obj    *resultObjPtr;

    switch(tableIndex) {
        case 0:
            exitCode=curl_easy_getinfo(curlHandle,CURLINFO_EFFECTIVE_URL,&charPtr);
            if (exitCode) {
                return exitCode;
            }
            resultObjPtr=Tcl_NewStringObj(charPtr,-1);
            Tcl_SetObjResult(interp,resultObjPtr);
            break;
        case 1:
        case 2:
            exitCode=curl_easy_getinfo(curlHandle,CURLINFO_RESPONSE_CODE,&longNumber);
            if (exitCode) {
                return exitCode;
            }
            resultObjPtr=Tcl_NewLongObj(longNumber);
            Tcl_SetObjResult(interp,resultObjPtr);
            break;
        case 3:
            exitCode=curl_easy_getinfo(curlHandle,CURLINFO_FILETIME,&longNumber);
            if (exitCode) {
                return exitCode;
            }
            resultObjPtr=Tcl_NewLongObj(longNumber);
            Tcl_SetObjResult(interp,resultObjPtr);
            break;
        case 4:
            exitCode=curl_easy_getinfo(curlHandle,CURLINFO_TOTAL_TIME,&doubleNumber);
            if (exitCode) {
                return exitCode;
            }
            resultObjPtr=Tcl_NewDoubleObj(doubleNumber);
            Tcl_SetObjResult(interp,resultObjPtr);
            break;
        case 5:
            exitCode=curl_easy_getinfo(curlHandle,CURLINFO_NAMELOOKUP_TIME,
                    &doubleNumber);
            if (exitCode) {
                return exitCode;
            }
            resultObjPtr=Tcl_NewDoubleObj(doubleNumber);
            Tcl_SetObjResult(interp,resultObjPtr);
            break;
        case 6:
            exitCode=curl_easy_getinfo(curlHandle,CURLINFO_CONNECT_TIME,
                    &doubleNumber);
            if (exitCode) {
                return exitCode;
            }
            resultObjPtr=Tcl_NewDoubleObj(doubleNumber);
            Tcl_SetObjResult(interp,resultObjPtr);
            break;
        case 7:
            exitCode=curl_easy_getinfo(curlHandle,CURLINFO_PRETRANSFER_TIME,
                    &doubleNumber);
            if (exitCode) {
                return exitCode;
            }
            resultObjPtr=Tcl_NewDoubleObj(doubleNumber);
            Tcl_SetObjResult(interp,resultObjPtr);
            break;
        case 8:
            exitCode = curl_easy_getinfo(curlHandle,CURLINFO_SIZE_UPLOAD_T,&doubleNumber);
            if (exitCode) {
                return exitCode;
            }
            resultObjPtr=Tcl_NewDoubleObj(doubleNumber);
            Tcl_SetObjResult(interp,resultObjPtr);
            break;
        case 9:
            exitCode = curl_easy_getinfo(curlHandle,CURLINFO_SIZE_DOWNLOAD_T,&doubleNumber);
            if (exitCode) {
                return exitCode;
            }
            resultObjPtr=Tcl_NewDoubleObj(doubleNumber);
            Tcl_SetObjResult(interp,resultObjPtr);
            break;
        case 10:
            exitCode=curl_easy_getinfo(curlHandle,CURLINFO_SPEED_DOWNLOAD_T,&doubleNumber);
            if (exitCode) {
                return exitCode;
            }
            resultObjPtr=Tcl_NewDoubleObj(doubleNumber);
            Tcl_SetObjResult(interp,resultObjPtr);
            break;
        case 11:
            exitCode=curl_easy_getinfo(curlHandle,CURLINFO_SPEED_UPLOAD_T,&doubleNumber);
            if (exitCode) {
                return exitCode;
            }
            resultObjPtr=Tcl_NewDoubleObj(doubleNumber);
            Tcl_SetObjResult(interp,resultObjPtr);
            break;
        case 12:
            exitCode=curl_easy_getinfo(curlHandle,CURLINFO_HEADER_SIZE,&longNumber);
            if (exitCode) {
                return exitCode;
            }
            resultObjPtr=Tcl_NewLongObj(longNumber);
            Tcl_SetObjResult(interp,resultObjPtr);
            break;
        case 13:
            exitCode=curl_easy_getinfo(curlHandle,CURLINFO_REQUEST_SIZE,&longNumber);
            if (exitCode) {
                return exitCode;
            }
            resultObjPtr=Tcl_NewLongObj(longNumber);
            Tcl_SetObjResult(interp,resultObjPtr);
            break;
        case 14:
            exitCode=curl_easy_getinfo(curlHandle,CURLINFO_SSL_VERIFYRESULT,
                    &longNumber);
            if (exitCode) {
                return exitCode;
            }
            resultObjPtr=Tcl_NewLongObj(longNumber);
            Tcl_SetObjResult(interp,resultObjPtr);
            break;
        case 15:
            exitCode = curl_easy_getinfo(curlHandle,CURLINFO_CONTENT_LENGTH_DOWNLOAD_T,&doubleNumber);
            if (exitCode) {
                return exitCode;
            }
            resultObjPtr = Tcl_NewDoubleObj(doubleNumber);
            Tcl_SetObjResult(interp,resultObjPtr);
            break;
        case 16:
            exitCode=curl_easy_getinfo(curlHandle,CURLINFO_CONTENT_LENGTH_UPLOAD_T,&doubleNumber);
            if (exitCode) {
                return exitCode;
            }
            resultObjPtr=Tcl_NewDoubleObj(doubleNumber);
            Tcl_SetObjResult(interp,resultObjPtr);
            break;
        case 17:
            exitCode=curl_easy_getinfo(curlHandle,CURLINFO_STARTTRANSFER_TIME,&doubleNumber);
            if (exitCode) {
                return exitCode;
            }
            resultObjPtr=Tcl_NewDoubleObj(doubleNumber);
            Tcl_SetObjResult(interp,resultObjPtr);
            break;
        case 18:
            exitCode=curl_easy_getinfo(curlHandle,CURLINFO_CONTENT_TYPE,&charPtr);
            if (exitCode) {
                return exitCode;
            }
            resultObjPtr=Tcl_NewStringObj(charPtr,-1);
            Tcl_SetObjResult(interp,resultObjPtr);
            break;
        case 19:
            exitCode=curl_easy_getinfo(curlHandle,CURLINFO_REDIRECT_TIME,&doubleNumber);
            if (exitCode) {
                return exitCode;
            }
            resultObjPtr=Tcl_NewDoubleObj(doubleNumber);
            Tcl_SetObjResult(interp,resultObjPtr);
            break;
        case 20:
            exitCode=curl_easy_getinfo(curlHandle,CURLINFO_REDIRECT_COUNT,&longNumber);
            if (exitCode) {
                return exitCode;
            }
            resultObjPtr=Tcl_NewLongObj(longNumber);
            Tcl_SetObjResult(interp,resultObjPtr);
            break;
        case 21:
        case 22:
            if (tableIndex==21) {
                exitCode=curl_easy_getinfo(curlHandle,CURLINFO_HTTPAUTH_AVAIL,&longNumber);
            } else {
                exitCode=curl_easy_getinfo(curlHandle,CURLINFO_PROXYAUTH_AVAIL,&longNumber);
            }
            if (exitCode) {
                return exitCode;
            }
            resultObjPtr=Tcl_NewListObj(0,(Tcl_Obj **)NULL);
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
            Tcl_SetObjResult(interp,resultObjPtr);
            break;
        case 23:
            exitCode=curl_easy_getinfo(curlHandle,CURLINFO_OS_ERRNO,&longNumber);
            if (exitCode) {
                return exitCode;
            }
            resultObjPtr=Tcl_NewLongObj(longNumber);
            Tcl_SetObjResult(interp,resultObjPtr);
            break;
        case 24:
            exitCode=curl_easy_getinfo(curlHandle,CURLINFO_NUM_CONNECTS,&longNumber);
            if (exitCode) {
                return exitCode;
            }
            resultObjPtr=Tcl_NewLongObj(longNumber);
            Tcl_SetObjResult(interp,resultObjPtr);
            break;
        case 25:
            exitCode=curl_easy_getinfo                                  \
                    (curlHandle,CURLINFO_SSL_ENGINES,&slistPtr);
            if (exitCode) {
                return exitCode;
            }
            resultObjPtr=Tcl_NewListObj(0,(Tcl_Obj **)NULL);
            while (slistPtr!=NULL) {
                Tcl_ListObjAppendElement(interp,resultObjPtr
                        ,Tcl_NewStringObj(slistPtr->data,-1));
                slistPtr=slistPtr->next;
            }
            curl_slist_free_all(slistPtr);
            Tcl_SetObjResult(interp,resultObjPtr);
            break;
        case 26:
            exitCode=curl_easy_getinfo                                  \
                    (curlHandle,CURLINFO_HTTP_CONNECTCODE,&longNumber);
            if (exitCode) {
                return exitCode;
            }
            resultObjPtr=Tcl_NewLongObj(longNumber);
            Tcl_SetObjResult(interp,resultObjPtr);
            break;
        case 27:
            exitCode=curl_easy_getinfo                                  \
                    (curlHandle,CURLINFO_COOKIELIST,&slistPtr);
            if (exitCode) {
                return exitCode;
            }
            resultObjPtr=Tcl_NewListObj(0,(Tcl_Obj **)NULL);
            while (slistPtr!=NULL) {
                Tcl_ListObjAppendElement(interp,resultObjPtr
                        ,Tcl_NewStringObj(slistPtr->data,-1));
                slistPtr=slistPtr->next;
            }
            curl_slist_free_all(slistPtr);
            Tcl_SetObjResult(interp,resultObjPtr);
            break;
        case 28:
            exitCode=curl_easy_getinfo(curlHandle,CURLINFO_FTP_ENTRY_PATH,&charPtr);
            if (exitCode) {
                return exitCode;
            }
            resultObjPtr=Tcl_NewStringObj(charPtr,-1);
            Tcl_SetObjResult(interp,resultObjPtr);
            break;
        case 29:
            exitCode=curl_easy_getinfo(curlHandle,CURLINFO_REDIRECT_URL,&charPtr);
            if (exitCode) {
                return exitCode;
            }
            resultObjPtr=Tcl_NewStringObj(charPtr,-1);
            Tcl_SetObjResult(interp,resultObjPtr);
            break;
        case 30:
            exitCode=curl_easy_getinfo(curlHandle,CURLINFO_PRIMARY_IP,&charPtr);
            if (exitCode) {
                return exitCode;
            }
            resultObjPtr=Tcl_NewStringObj(charPtr,-1);
            Tcl_SetObjResult(interp,resultObjPtr);
            break;
        case 31:
            exitCode=curl_easy_getinfo(curlHandle,CURLINFO_APPCONNECT_TIME,&doubleNumber);
            if (exitCode) {
                return exitCode;
            }
            resultObjPtr=Tcl_NewDoubleObj(doubleNumber);
            Tcl_SetObjResult(interp,resultObjPtr);
            break;
        case 32:
            exitCode=curl_easy_getinfo(curlHandle,CURLINFO_CERTINFO,&certinfoPtr);
            if (exitCode) {
                return exitCode;
            }
            resultObjPtr=Tcl_NewListObj(0,(Tcl_Obj **)NULL);
            Tcl_ListObjAppendElement(interp,resultObjPtr,Tcl_NewIntObj(certinfoPtr->num_of_certs));
            for (i=0; i < certinfoPtr->num_of_certs; i++) {
                for (slistPtr = certinfoPtr->certinfo[i]; slistPtr; slistPtr=slistPtr->next) {
                    Tcl_ListObjAppendElement(interp,resultObjPtr,Tcl_NewStringObj(slistPtr->data,-1));
                }
            }
            Tcl_SetObjResult(interp,resultObjPtr);
            break;
        case 33:
            exitCode = curl_easy_getinfo(curlHandle,CURLINFO_CONDITION_UNMET,&longNumber);
            if (exitCode) {
                return exitCode;
            }
            resultObjPtr = Tcl_NewLongObj(longNumber);
            Tcl_SetObjResult(interp,resultObjPtr);
            break;
        case 34:
            exitCode = curl_easy_getinfo(curlHandle,CURLINFO_PRIMARY_PORT,&longNumber);
            if (exitCode) {
                return exitCode;
            }
            resultObjPtr = Tcl_NewLongObj(longNumber);
            Tcl_SetObjResult(interp,resultObjPtr);
            break;
        case 35:
            exitCode=curl_easy_getinfo(curlHandle,CURLINFO_LOCAL_IP,&charPtr);
            if (exitCode) {
                return exitCode;
            }
            resultObjPtr=Tcl_NewStringObj(charPtr,-1);
            Tcl_SetObjResult(interp,resultObjPtr);
            break;
        case 36:
            exitCode=curl_easy_getinfo                                  \
                    (curlHandle,CURLINFO_LOCAL_PORT,&longNumber);
            if (exitCode) {
                return exitCode;
            }
            resultObjPtr=Tcl_NewLongObj(longNumber);
            Tcl_SetObjResult(interp,resultObjPtr);
            break;
    }
    return 0;
}


int TclCurl_GetInfo (Tcl_Interp* interp,Tcl_Obj* get_info_arg,CURL* curlHandle)
{
    int tableIndex;

    if (Tcl_GetIndexFromObj(interp,get_info_arg,getInfoTable,
                            "getinfo option",TCL_EXACT,&tableIndex) == TCL_ERROR) {
        return 1;
    }

    if (curlGetInfo(interp,curlHandle,tableIndex)) {
        return 1;
    }
    return 0;
}
