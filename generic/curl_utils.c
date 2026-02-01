/*
 * tclcurl_utils.c
 */

#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <curl/curl.h>
#include <curl/easy.h>
#include <tcl.h>

#include "tclcurl.h"
#include "tclcompat.h"

enum ProtocolNamesIndices {
    TCLCURL_PROTO_HTTP,
    TCLCURL_PROTO_HTTPS,
    TCLCURL_PROTO_FTP,
    TCLCURL_PROTO_FTPS,
    TCLCURL_PROTO_SCP,
    TCLCURL_PROTO_SFTP,
    TCLCURL_PROTO_TELNET,
    TCLCURL_PROTO_LDAP,
    TCLCURL_PROTO_LDAPS,
    TCLCURL_PROTO_DICT,
    TCLCURL_PROTO_FILE,
    TCLCURL_PROTO_TFTP,
    TCLCURL_PROTO_IMAP,
    TCLCURL_PROTO_IMAPS,
    TCLCURL_PROTO_POP3,
    TCLCURL_PROTO_POP3S,
    TCLCURL_PROTO_SMTP,
    TCLCURL_PROTO_SMTPS,
    TCLCURL_PROTO_RTSP,
    TCLCURL_PROTO_RTMP,
    TCLCURL_PROTO_RTMPT,
    TCLCURL_PROTO_RTMPE,
    TCLCURL_PROTO_RTMPTE,
    TCLCURL_PROTO_RTMPS,
    TCLCURL_PROTO_RTMPTS,
    TCLCURL_PROTO_GOPHER,
    TCLCURL_PROTO_ALL
};

const static char *protocolNames[] = {
    "http",   "https",  "ftp",  "ftps", "scp",   "sftp",  "telnet", "ldap",
    "ldaps",  "dict",   "file", "tftp", "imap",  "imaps", "pop3",   "pop3s", 
    "smtp",   "smtps",  "rtsp", "rtmp", "rtmpt", "rtmpe", "rtmpte", "rtmps",
    "rtmpts", "gopher", "all",  (char*)NULL
};

unsigned long int 
TclCurl_BuildProtocolMask(Tcl_Interp* interp, Tcl_Obj** protocols,Tcl_Size protocols_c)
{
    unsigned long int protocolMask;
    int         curlTableIndex;
    Tcl_Size    i;

    for (i = 0,protocolMask = 0; i < protocols_c; i++) {
        if (Tcl_GetIndexFromObj(interp,protocols[i],protocolNames,
               "protocol",TCL_EXACT,&curlTableIndex)==TCL_ERROR) {
           return TCL_ERROR;
        }

        switch(curlTableIndex) {
            case TCLCURL_PROTO_HTTP:              /* http     1 */
                protocolMask|=CURLPROTO_HTTP;
                break;
            case TCLCURL_PROTO_HTTPS:             /* https    2 */
                protocolMask|=CURLPROTO_HTTPS;
                break;
            case TCLCURL_PROTO_FTP:               /* ftp      4 */
                protocolMask|=CURLPROTO_FTP;
                break;
            case TCLCURL_PROTO_FTPS:              /* ftps     8 */
                protocolMask|=CURLPROTO_FTPS;
                break;
            case TCLCURL_PROTO_SCP:               /* scp     16 */
                protocolMask|=CURLPROTO_SCP;
                break;
            case TCLCURL_PROTO_SFTP:              /* sftp    32 */
                protocolMask|=CURLPROTO_SFTP;
                break;
            case TCLCURL_PROTO_TELNET:            /* telnet  64 */
                protocolMask|=CURLPROTO_TELNET;
                break;
            case TCLCURL_PROTO_LDAP:              /* ldap   128 */
                protocolMask|=CURLPROTO_LDAP;
                break;
            case TCLCURL_PROTO_LDAPS:             /* ldaps  256 */
                protocolMask|=CURLPROTO_LDAPS;
                break;
            case TCLCURL_PROTO_DICT:              /* dict   512 */
                protocolMask|=CURLPROTO_DICT;
                break;
            case TCLCURL_PROTO_FILE:              /* file  1024 */
                protocolMask|=CURLPROTO_FILE;
                break;
            case TCLCURL_PROTO_TFTP:              /* tftp  2048 */
                protocolMask|=CURLPROTO_TFTP;
                break;
            case TCLCURL_PROTO_IMAP:              /* imap  4096 */
                protocolMask|=CURLPROTO_IMAP;
                break;
            case TCLCURL_PROTO_IMAPS:             /* imaps */
                protocolMask|=CURLPROTO_IMAPS;
                break;
            case TCLCURL_PROTO_POP3:              /* pop3 */
                protocolMask|=CURLPROTO_POP3;
                break;
            case TCLCURL_PROTO_POP3S:             /* pop3s */
                protocolMask|=CURLPROTO_POP3S;
                break;
            case TCLCURL_PROTO_SMTP:              /* smtp */
                protocolMask|=CURLPROTO_SMTP;
                break;
            case TCLCURL_PROTO_SMTPS:             /* smtps */
                protocolMask|=CURLPROTO_SMTPS;
                break;
            case TCLCURL_PROTO_RTSP:              /* rtsp */
                protocolMask|=CURLPROTO_RTSP;
                break;
            case TCLCURL_PROTO_RTMP:              /* rtmp */
                protocolMask|=CURLPROTO_RTMP;
                break;
            case TCLCURL_PROTO_RTMPT:             /* rtmpt */
                protocolMask|=CURLPROTO_RTMPT;
                break;
            case TCLCURL_PROTO_RTMPE:             /* rtmpe */
                protocolMask|=CURLPROTO_RTMPE;
                break;
            case TCLCURL_PROTO_RTMPTE:            /* rtmpte */
                protocolMask|=CURLPROTO_RTMPTE;
                break;
            case TCLCURL_PROTO_RTMPS:             /* rtmps */
                protocolMask|=CURLPROTO_RTMPS;
                break;
            case TCLCURL_PROTO_RTMPTS:            /* rtmpts */
                protocolMask|=CURLPROTO_RTMPTS;
                break;
            case TCLCURL_PROTO_GOPHER:            /* gopher */
                protocolMask|=CURLPROTO_GOPHER;
                break;
            case TCLCURL_PROTO_ALL:               /* all   FFFF */
                protocolMask|=CURLPROTO_ALL;
        }
    }

    return protocolMask;
}

int
TclCurl_ErrorBuffer(Tcl_Interp *interp, struct curlObjData* curlData,Tcl_Obj *const tcl_o)
{
    int             exitCode;
    const char     *startPtr;
    const char     *endPtr;
    char           *tmpStr = NULL;
    Tcl_RegExp      regExp;
    CURL           *curlHandle=curlData->curl;

    tmpStr   = curlstrdup(Tcl_GetString(tcl_o));
    regExp   = Tcl_RegExpCompile(interp,"(.*)(?:\\()(.*)(?:\\))");
    exitCode = Tcl_RegExpExec(interp,regExp,tmpStr,tmpStr);

    switch (exitCode) {
        case -1:
            Tcl_Free((char *)tmpStr);
            return TCL_ERROR;
        case 0:
            if (*tmpStr!=0) {
                curlData->errorBufferName=curlstrdup(tmpStr);
            } else {
                curlData->errorBuffer=NULL;
            }
            curlData->errorBufferKey=NULL;
            break;
        case 1:
        {
            int charLength;

            Tcl_RegExpRange(regExp,1,&startPtr,&endPtr);
            charLength = endPtr-startPtr;
            curlData->errorBufferName = Tcl_Alloc(charLength+1);
            strncpy(curlData->errorBufferName,startPtr,charLength);
            curlData->errorBufferName[charLength] = 0;
            Tcl_RegExpRange(regExp,2,&startPtr,&endPtr);
            charLength = endPtr-startPtr;
            curlData->errorBufferKey = Tcl_Alloc(charLength+1);
            strncpy(curlData->errorBufferKey,startPtr,charLength);
            curlData->errorBufferKey[charLength] = 0;
            break;
        }
    }

    Tcl_Free((char *)tmpStr);
    if (curlData->errorBufferName!=NULL) {
        curlData->errorBuffer=Tcl_Alloc(CURL_ERROR_SIZE);
        if (curl_easy_setopt(curlHandle,CURLOPT_ERRORBUFFER,
                curlData->errorBuffer)) {
            Tcl_Free((char *)curlData->errorBuffer);
            curlData->errorBuffer=NULL;
            return TCL_ERROR;
        }
    } else {
        Tcl_Free(curlData->errorBuffer);
    }

    return TCL_OK;
}

/*
 * Implementation of a very elementary function to strip spaces 
 * from string arguments
 *
 */


char* TclCurl_StripSpaces (char* str)
{
    char*  start = str;
    char*  end   = start + strlen(str) - 1;
    size_t len   = strlen(str);
    char** sstarts = (char **) calloc(len,sizeof(char *));
    int    segment = 0;
    int    s;
    bool   space_strand = true;
    char*  p = start;

    while (isspace(*p)) { p++; }
    while (p != end) {
        if (isspace(*p)) {
            *p = 0;
            space_strand = true;
        } else {
            if (space_strand) {
                sstarts[segment++] = p;
            }
            space_strand = false;
        }
        p++;
    }

    p = start;
    for (s = 0; s < segment; s++) {
        char*  s_p;
        size_t s_len;

        s_p   = sstarts[s];
        s_len = strlen(sstarts[s]);

        memmove(p,s_p,s_len);
        p += s_len;
        *p = 0;
    }

    free(sstarts);
    return start;
}

/*
 * C level reimplementation of Tcl command 'join' 
 *
 */

Tcl_Obj* TclCurl_JoinList (Tcl_Obj** objList,Tcl_Size obj_cnt,const char* join_c)
{
    Tcl_Obj* joined_string = Tcl_NewObj();
    Tcl_Obj* join_obj = Tcl_NewStringObj(join_c,-1);
    int i;

    Tcl_IncrRefCount(join_obj);
    for (i = 0;  i < obj_cnt;  i++) {
	    if (i > 0) {
            Tcl_AppendObjToObj(joined_string,join_obj);
        }
        Tcl_AppendObjToObj(joined_string,objList[i]);
    }
    Tcl_DecrRefCount(join_obj);
    return joined_string;
}

