/*
 * tclcurl.h --
 *
 * Header file for the TclCurl extension to enable Tcl interpreters
 * to access libcurl.
 *
 * Copyright (c) 2001-2011 Andres Garcia Garcia.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#ifndef __tclcurl_h__
#define __tclcurl_h__

#if (defined(WIN32) || defined(_WIN32))
#define CURL_STATICLIB 1
#endif

#include <curl/curl.h>
#include <curl/easy.h>

#include <stdio.h>
#include <string.h>

#define _MPRINTF_REPLACE
#include <curl/mprintf.h>

#ifdef  __cplusplus
extern "C" {
#endif

#include "config.h"
#include "tclcompat.h"

/*
 * Windows needs to know which symbols to export. Unix does not.
 * BUILD_tclcurl should be undefined for Unix.
 * Actually I don't use this, but it was in TEA so I keep in case
 * I ever understand what it is for.
 */

#ifdef BUILD_TclCurl
#undef  TCL_STORAGE_CLASS
#define TCL_STORAGE_CLASS DLLEXPORT
#endif

#define TclCurlVersion PACKAGE_VERSION

/*
 * This struct will contain the data of a transfer if the user wants
 * to put the body into a Tcl variable
 */
struct MemoryStruct {
    char   *memory;
    size_t size;
};

/* 
 * Struct that will be used for a linked list with all the
 * data for a post
 */
struct formArrayStruct {
    struct curl_forms       *formArray;
    struct curl_slist       *formHeaderList;
    struct formArrayStruct  *next;
};

struct curlObjData {
    CURL                   *curl;
    Tcl_Command             token;
    Tcl_Command             shareToken;
    Tcl_Interp             *interp;
    struct curl_slist      *headerList;
    struct curl_slist      *quote;
    struct curl_slist      *prequote;
    struct curl_slist      *postquote;
    struct curl_httppost   *postListFirst;
    struct curl_httppost   *postListLast;
    struct formArrayStruct *formArray;
    char                   *outFile;
    FILE                   *outHandle;
    int                     outFlag;
    char                   *inFile;
    FILE                   *inHandle;
    int                     inFlag;
    char                   *proxy;
    int                     transferText;
    char                   *errorBuffer;
    char                   *errorBufferName;
    char                   *errorBufferKey;
    char                   *headerFile;
    FILE                   *headerHandle;
    int                     headerFlag;
    char                   *stderrFile;
    FILE                   *stderrHandle;
    int                     stderrFlag;
    char                   *randomFile;
    char                   *headerVar;
    char                   *bodyVarName;
    struct MemoryStruct     bodyVar;
    char                   *progressProc;
    char                   *cancelTransVarName;
    int                     cancelTrans;
    char                   *writeProc;
    char                   *readProc;
    char                   *debugProc;
    struct curl_slist      *http200aliases;
    char                   *command;
    int                     anyAuthFlag;
    char                   *sshkeycallProc;
    struct curl_slist      *mailrcpt;
    char                   *chunkBgnProc;
    char                   *chunkBgnVar;
    char                   *chunkEndProc;
    char                   *fnmatchProc;
    struct curl_slist      *resolve;
    struct curl_slist      *telnetoptions;
};

struct shcurlObjData {
    Tcl_Command           token;
    CURLSH               *shandle;
};

#ifndef multi_h
enum curlFormIndices {
    NAME_HTTP_OPT,  CONTENTS_HTTP_OPT, FILE_HTTP_OPT, 
    CONTENTTYPE_HTTP_OPT, CONTENTHEADER_HTTP_OPT, FILENAME_HTTP_OPT,
    BUFFERNAME_HTTP_OPT, BUFFER_HTTP_OPT, FILECONTENT_HTTP_OPT
};

int curlseek(void *instream, curl_off_t offset, int origin);
int Tclcurl_MultiInit (Tcl_Interp *interp);
#endif

EXTERN int Tclcurl_Init(Tcl_Interp *interp);

Tcl_Obj* curlCreateObjCmd(Tcl_Interp *interp,struct curlObjData  *curlData);
int curlInitObjCmd(ClientData clientData, Tcl_Interp *interp, int objc,Tcl_Obj *const objv[]);
int curlObjCmd(ClientData clientData, Tcl_Interp *interp, int objc,Tcl_Obj *const objv[]);
int curlDeleteCmd(ClientData clientData);

int curlPerform(Tcl_Interp *interp,CURL *curlHandle,struct curlObjData *curlData);

int curlSetOptsTransfer(Tcl_Interp *interp, struct curlObjData *curlData,int objc,Tcl_Obj *const objv[]);
int curlConfigTransfer(Tcl_Interp *interp, struct curlObjData *curlData,int objc,Tcl_Obj *const objv[]);
int curlDupHandle(Tcl_Interp *interp, struct curlObjData *curlData,int objc,Tcl_Obj *const objv[]);
int curlResetHandle(Tcl_Interp *interp, struct curlObjData *curlData);

int curlSetOpts(Tcl_Interp *interp, struct curlObjData *curlData,Tcl_Obj *const objv,int tableIndex);

int SetoptInt(Tcl_Interp *interp,CURL *curlHandle,CURLoption opt,int tableIndex,Tcl_Obj *tclObj);
int SetoptLong(Tcl_Interp *interp,CURL *curlHandle,CURLoption opt,int tableIndex,Tcl_Obj *tclObj);
int SetoptCurlOffT(Tcl_Interp *interp,CURL *curlHandle,CURLoption opt,int tableIndex,Tcl_Obj *tclObj);
int SetoptChar(Tcl_Interp *interp,CURL *curlHandle,CURLoption opt,int tableIndex,Tcl_Obj *tclObj);
int SetoptBlob(Tcl_Interp *interp,CURL *curlHandle,CURLoption opt,int tableIndex,Tcl_Obj *tclObj);
int SetoptSHandle(Tcl_Interp *interp,CURL *curlHandle,CURLoption opt,int tableIndex,Tcl_Obj *tclObj);
int SetoptsList(Tcl_Interp *interp,struct curl_slist **slistPtr,Tcl_Obj *const objv);

CURLcode curlGetInfo(Tcl_Interp *interp,CURL *curlHandle,int tableIndex);

void curlFreeSpace(struct curlObjData *curlData);

void curlErrorSetOpt(Tcl_Interp *interp,const char **configTable, int option,const char *parPtr);
size_t curlHeaderReader(void *ptr,size_t size,size_t nmemb,FILE *stream);
size_t curlBodyReader(void *ptr,size_t size,size_t nmemb,FILE *curlDataPtr);

int curlProgressCallback(void *clientp,double dltotal,double dlnow,double ultotal,double ulnow);

size_t curlWriteProcInvoke(void *ptr,size_t size,size_t nmemb,FILE *curlDataPtr);
size_t curlReadProcInvoke(void *ptr,size_t size,size_t nmemb,FILE *curlDataPtr);

long curlChunkBgnProcInvoke (const void *transfer_info, void *curlDataPtr, int remains);
long curlChunkEndProcInvoke (void *curlDataPtr);
int curlfnmatchProcInvoke(void *curlDataPtr, const char *pattern, const char *filename);

/* Puts a ssh key into a Tcl object */
Tcl_Obj *curlsshkeyextract(Tcl_Interp *interp,const struct curl_khkey *key);

/* Function that will be invoked by libcurl to see what the user wants to
   do about the new ssh host */
size_t curlsshkeycallback(CURL *easy,                        /* easy handle */
                          const struct curl_khkey *knownkey, /* known */
                          const struct curl_khkey *foundkey, /* found */
                          enum curl_khmatch,                 /* libcurl's view on the keys */
                          void *curlData);

int curlDebugProcInvoke(CURL *curlHandle, curl_infotype infoType,
        char * dataPtr, size_t size, void  *curlData);

int curlVersion (ClientData clientData, Tcl_Interp *interp,
    int objc,Tcl_Obj *const objv[]);

int curlEscape(ClientData clientData, Tcl_Interp *interp,
    int objc,Tcl_Obj *const objv[]);

int curlUnescape(ClientData clientData, Tcl_Interp *interp,
    int objc,Tcl_Obj *const objv[]);

int curlVersionInfo (ClientData clientData, Tcl_Interp *interp,
    int objc,Tcl_Obj *const objv[]);

int curlCopyCurlData (struct curlObjData *curlDataOld,
                      struct curlObjData *curlDataNew);

int curlOpenFile(Tcl_Interp *interp,char *fileName, FILE **handle, int writing, int text);

int  curlOpenFiles (Tcl_Interp *interp,struct curlObjData *curlData);
void curlCloseFiles(struct curlObjData *curlData);

int curlSetPostData(Tcl_Interp *interp,struct curlObjData *curlData);
void curlResetPostData(struct curlObjData *curlDataPtr);
void curlResetFormArray(struct curl_forms *formArray);

void curlSetBodyVarName(Tcl_Interp *interp,struct curlObjData *curlDataPtr);

char *curlstrdup (char *old);

Tcl_Obj* curlCreateShareObjCmd (Tcl_Interp *interp,struct shcurlObjData  *shcurlData);
int curlShareInitObjCmd (ClientData clientData, Tcl_Interp *interp,
        int objc,Tcl_Obj *const objv[]);
int curlShareObjCmd (ClientData clientData, Tcl_Interp *interp,
        int objc,Tcl_Obj *const objv[]);
int curlCleanUpShareCmd(ClientData clientData);


int curlErrorStrings (Tcl_Interp *interp, Tcl_Obj *const objv,int type);
int curlEasyStringError (ClientData clientData, Tcl_Interp *interp,
        int objc,Tcl_Obj *const objv[]);
int curlShareStringError (ClientData clientData, Tcl_Interp *interp,
        int objc,Tcl_Obj *const objv[]);
int curlMultiStringError (ClientData clientData, Tcl_Interp *interp,
        int objc,Tcl_Obj *const objv[]);

/* */

int               TclCurl_SetOpts(Tcl_Interp *interp, struct curlObjData *curlData,Tcl_Obj *const objv,int tableIndex);
unsigned long int TclCurl_BuildProtocolMask(Tcl_Interp* interp, Tcl_Obj** protocols,int protocols_c);
long              TclCurl_FTPClearCommandChannelOpt(Tcl_Interp* interp,Tcl_Obj* opt_o);
long              TclCurl_FTPSSLMethod(Tcl_Interp* interp,Tcl_Obj* ssl_method_o);
int               TclCurl_ErrorBuffer(Tcl_Interp *interp, struct curlObjData * const curlData,Tcl_Obj *const tcl_o);

#ifdef  __cplusplus
}
#endif

#endif
