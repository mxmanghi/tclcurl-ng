/*
 * tclcurl.c --
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stddef.h>
#include <tcl.h>
#include "tclcurl.h"

#ifdef _WIN32
#define UNICODE
#define _UNICODE
#include <tchar.h>
#endif

#include "tclcurl.h"
#include "curl_mime.h"

#ifndef _WIN32
#include <unistd.h>
#endif

#include <stddef.h>  /* for size_t */

#ifndef multi_h
#ifdef TCL_THREADS
    TCL_DECLARE_MUTEX(cookieLock)
    TCL_DECLARE_MUTEX(dnsLock)
    TCL_DECLARE_MUTEX(sslLock)
    TCL_DECLARE_MUTEX(connectLock)

    void curlShareLockFunc  (CURL *handle, curl_lock_data data, curl_lock_access access, void *userptr);
    void curlShareUnLockFunc(CURL *handle, curl_lock_data data, void *userptr);
#endif
#endif

const static char *lockData[] = {
    "cookies", "dns", (char *)NULL
};

const static char *shareCmd[] = {
    "share", "unshare", "cleanup", (char *)NULL
};

const static char *versionInfoTable[] = {
    "-version",    "-versionnum",    "-host",         "-features",
    "-sslversion", "-sslversionnum", "-libzversion",
    "-protocols",  (char *)NULL
};

#define TCLCURL_COMMANDS(X) \
    X(TCLCURL_CMD_SETOPT,"setopt")       \
    X(TCLCURL_CMD_PERFORM,"perform")     \
    X(TCLCURL_CMD_GETINFO,"getinfo")     \
    X(TCLCURL_CMD_CLEANUP,"cleanup")     \
    X(TCLCURL_CMD_CONFIGURE,"configure") \
    X(TCLCURL_CMD_DUPHANDLE,"duphandle") \
    X(TCLCURL_CMD_RESET,"reset")         \
    X(TCLCURL_CMD_PAUSE,"pause")         \
    X(TCLCURL_CMD_RESUME,"resume")

typedef enum {
#define X(sym,cmd) sym,
    TCLCURL_COMMANDS(X)
#undef X
    TCLCURL_CMD_COUNT
} tclcurl_cmd_id;

static const char* const commandTable[TCLCURL_CMD_COUNT] = {
#define X(sym, cmd) [sym] = cmd,
    TCLCURL_COMMANDS(X)
#undef X
};

static struct shcurlObjData *
curlGetShareDataFromToken(Tcl_Command token)
{
    Tcl_CmdInfo info;

    if ((token == NULL) || !Tcl_GetCommandInfoFromToken(token, &info)) {
        return NULL;
    }

    return (struct shcurlObjData *) info.objClientData;
}

static void
curlLinkEasyToShare(struct curlObjData *curlData, struct shcurlObjData *shcurlData)
{
    curlData->shareToken = shcurlData->token;
    curlData->nextSharedHandle = shcurlData->users;
    shcurlData->users = curlData;
}

void
curlDetachShareHandle(struct curlObjData *curlData)
{
    struct shcurlObjData *shcurlData;
    struct curlObjData **linkPtr;

    if (curlData->shareToken == NULL) {
        curlData->nextSharedHandle = NULL;
        return;
    }

    if (curlData->curl != NULL) {
        curl_easy_setopt(curlData->curl, CURLOPT_SHARE, NULL);
    }

    shcurlData = curlGetShareDataFromToken(curlData->shareToken);
    if (shcurlData != NULL) {
        for (linkPtr = &shcurlData->users; *linkPtr != NULL; linkPtr = &(*linkPtr)->nextSharedHandle) {
            if (*linkPtr == curlData) {
                *linkPtr = curlData->nextSharedHandle;
                break;
            }
        }
    }

    curlData->shareToken = NULL;
    curlData->nextSharedHandle = NULL;
}


/*
 *----------------------------------------------------------------------
 *
 * Tclcurl_Init --
 *
 *  This procedure initializes the package
 *
 * Results:
 *  A standard Tcl result.
 *
 *----------------------------------------------------------------------
 */

EXTERN int
Tclcurl_Init (Tcl_Interp *interp) {

#ifdef USE_TCL_STUBS
    if (Tcl_InitStubs(interp,"8.6-10",0)==NULL) {
        return TCL_ERROR;
    }
#else
    if (Tcl_PkgRequire(interp,"Tcl","8.6-10",0)==NULL) {
        return TCL_ERROR;
    }
#endif

    Tcl_CreateObjCommand (interp,"::curl::init",curlInitObjCmd,
            (ClientData)NULL,(Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateObjCommand (interp,"::curl::version",curlVersion,
            (ClientData)NULL,(Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateObjCommand (interp,"::curl::escape",curlEscape,
            (ClientData)NULL,(Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateObjCommand (interp,"::curl::unescape",curlUnescape,
            (ClientData)NULL,(Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateObjCommand (interp,"::curl::versioninfo",curlVersionInfo,
            (ClientData)NULL,(Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateObjCommand (interp,"::curl::shareinit",curlShareInitObjCmd,
            (ClientData)NULL,(Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateObjCommand (interp,"::curl::easystrerror",curlEasyStringError,
            (ClientData)NULL,(Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateObjCommand (interp,"::curl::sharestrerror",curlShareStringError,
            (ClientData)NULL,(Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateObjCommand (interp,"::curl::multistrerror",curlMultiStringError,
            (ClientData)NULL,(Tcl_CmdDeleteProc *)NULL);

    Tclcurl_MultiInit(interp);

    Tcl_PkgProvide(interp,"TclCurl",PACKAGE_VERSION);

    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * curlCreateObjCmd --
 *
 *  Looks for the first free handle (curl1, curl2,...) and creates a
 *  Tcl command for it.
 *
 * Results:
 *  A string with the name of the handle, don't forget to free it.
 *
 * Side effects:
 *  See the user documentation.
 *
 *----------------------------------------------------------------------
 */

Tcl_Obj *
curlCreateObjCmd (Tcl_Interp *interp,struct curlObjData  *curlData) {
    char                handleName[32];
    int                 i;
    Tcl_CmdInfo         info;
    Tcl_Command         cmdToken;

    /* We try with curl1, if it already exists with curl2...*/
    for (i=1;;i++) {
        snprintf(handleName,sizeof(handleName),"curl%d",i);
        if (!Tcl_GetCommandInfo(interp,handleName,&info)) {
            cmdToken=Tcl_CreateObjCommand(interp,handleName,curlObjCmd,
                                (ClientData)curlData,
                                (Tcl_CmdDeleteProc *)curlDeleteCmd);
            break;
        }
    }
    curlData->token=cmdToken;

    return Tcl_NewStringObj(handleName,-1);
}

/*
 *----------------------------------------------------------------------
 *
 * curlInitObjCmd --
 *
 *  This procedure is invoked to process the "curl::init" Tcl command.
 *  See the user documentation for details on what it does.
 *
 * Results:
 *  A standard Tcl result.
 *
 * Side effects:
 *  See the user documentation.
 *
 *----------------------------------------------------------------------
 */

int
curlInitObjCmd (ClientData clientData,Tcl_Interp *interp,
                int objc,Tcl_Obj *const objv[]) {

    Tcl_Obj             *resultPtr;
    CURL                *curlHandle;
    struct curlObjData  *curlData;
    Tcl_Obj             *handleObj;

    curlData = (struct curlObjData *)Tcl_Alloc(sizeof(struct curlObjData));
    if (curlData == NULL) {
        resultPtr=Tcl_NewStringObj("Couldn't allocate memory",-1);
        Tcl_SetObjResult(interp,resultPtr);
        return TCL_ERROR;
    }

    memset(curlData, 0, sizeof(struct curlObjData));
    curlData->interp = interp;
    curlData->postFieldSize = -1;

    /* This is required to be done when the package is loaded
     * and before any thread is created. It's not recommended to
     * assume curllib will call it implicitly */

    curl_global_init(CURL_GLOBAL_DEFAULT); 

    /* And then we call curl_easy_init() */

    curlHandle = curl_easy_init();
    if (curlHandle == NULL) {
        resultPtr=Tcl_NewStringObj("Couldn't open curl handle",-1);
        Tcl_SetObjResult(interp,resultPtr);
        return TCL_ERROR;
    }

    handleObj = curlCreateObjCmd(interp,curlData);

    /* This is the curl central data structure. In meany examples
     * it referred to as 'easy' because returned by the curl_easy_init() 
     * call */

    curlData->curl = curlHandle;

    Tcl_SetObjResult(interp,handleObj);

    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * curlObjCmd --
 *
 *  This procedure is invoked to process the "curl" commands.
 *  See the user documentation for details on what it does.
 *
 * Results:
 *  A standard Tcl result.
 *
 * Side effects:
 *  See the user documentation.
 *
 *----------------------------------------------------------------------
 */
int
curlObjCmd (ClientData clientData, Tcl_Interp *interp,
    int objc,Tcl_Obj *const objv[]) {

    struct curlObjData     *curlData = (struct curlObjData *)clientData;
    CURL                   *curlHandle=curlData->curl;
    int                    tableIndex;

    if (objc<2) {
        Tcl_WrongNumArgs(interp,1,objv,"option arg ?arg?");
        return TCL_ERROR;
    }
    if (Tcl_GetIndexFromObj(interp, objv[1], commandTable, "option",
            TCL_EXACT,&tableIndex)==TCL_ERROR) {
        return TCL_ERROR;
    }

    switch(tableIndex) {
        case TCLCURL_CMD_SETOPT:
            if (objc != 4) {
                Tcl_WrongNumArgs(interp,2,objv,"option value");
                return TCL_ERROR;
            }
            if (curlSetOptsTransfer(interp,curlData,objc,objv) == TCL_ERROR) {
                return TCL_ERROR;
            }
            break;
        case TCLCURL_CMD_PERFORM:
            if (objc != 2) {
                Tcl_WrongNumArgs(interp,2,objv,"");
                return TCL_ERROR;
            }
            if (curlPerform(interp,curlHandle,curlData)) {
                if (curlData->errorBuffer != NULL) {
                    if (curlData->errorBufferKey == NULL) {
                        Tcl_SetVar(interp,curlData->errorBufferName,
                                          curlData->errorBuffer,0);
                    } else {
                        Tcl_SetVar2(interp,curlData->errorBufferName,
                                           curlData->errorBufferKey,
                                           curlData->errorBuffer,0);
                    }
                }
                return TCL_ERROR;
            }
            break;
        case TCLCURL_CMD_GETINFO:
        {
            Tcl_Obj* resultObjPtr;

            if (objc != 3) {
                Tcl_WrongNumArgs(interp,2,objv,"option");
                return TCL_ERROR;
            }
            if (TclCurl_GetInfo(interp,objv[2],curlHandle,&resultObjPtr)) {
                return TCL_ERROR;
            }
            Tcl_SetObjResult(interp,resultObjPtr);
            break;
        }
        case TCLCURL_CMD_CLEANUP:
            if (objc != 2) {
                Tcl_WrongNumArgs(interp,2,objv,"");
                return TCL_ERROR;
            }
            Tcl_DeleteCommandFromToken (interp,curlData->token);
            break;
        case TCLCURL_CMD_CONFIGURE:
            if (objc < 4 || objc % 2) {
                Tcl_WrongNumArgs(interp,2,objv,"option value ?option value ...?");
                return TCL_ERROR;
            }
            if (curlConfigTransfer(interp,curlData,objc,objv) == TCL_ERROR) {
                return TCL_ERROR;
            }
            break;
        case TCLCURL_CMD_DUPHANDLE:
            if (objc != 2) {
                Tcl_WrongNumArgs(interp,2,objv,"");
                return TCL_ERROR;
            }
            if (curlDupHandle(interp,curlData,objc,objv) == TCL_ERROR) {
                return TCL_ERROR;
            }
            break;
        case TCLCURL_CMD_RESET:
            if (objc != 2) {
                Tcl_WrongNumArgs(interp,2,objv,"");
                return TCL_ERROR;
            }
            if (curlResetHandle(interp,curlData) == TCL_ERROR) {
                return TCL_ERROR;
            }
            break;
        case TCLCURL_CMD_PAUSE:
            if (objc != 2) {
                Tcl_WrongNumArgs(interp,2,objv,"");
                return TCL_ERROR;
            }
            if (curl_easy_pause(curlData->curl,CURLPAUSE_ALL) == TCL_ERROR) {
                return TCL_ERROR;
            }
            break;
        case TCLCURL_CMD_RESUME:
            if (objc != 2) {
                Tcl_WrongNumArgs(interp,2,objv,"");
                return TCL_ERROR;
            }
            if (curl_easy_pause(curlData->curl,CURLPAUSE_CONT) == TCL_ERROR) {
                return TCL_ERROR;
            }
            break;
    }
    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * curlDeleteCmd --
 *
 *  This procedure is invoked when curl handle is deleted.
 *
 * Results:
 *  A standard Tcl result.
 *
 * Side effects:
 *  Cleans the curl handle and frees the memory.
 *
 *----------------------------------------------------------------------
 */
int
curlDeleteCmd(ClientData clientData) {
    struct curlObjData     *curlData=(struct curlObjData *)clientData;
    CURL                   *curlHandle=curlData->curl;

    curlDetachShareHandle(curlData);
    curl_easy_cleanup(curlHandle);
    curlFreeSpace(curlData);

    Tcl_Free((char *)curlData);

    return TCL_OK;
}

/*----------------------------------------------------------------------
 *
 * curlseek --
 *
 *  When the user requests the 'any' auth, libcurl may need
 *  to send the PUT/POST data more than once and thus may need to ask
 *  the app to "rewind" the read data stream to start.
 *
 *----------------------------------------------------------------------
 */

static int
curlseek(void *instream, curl_off_t offset, int origin)
{
    if (-1 == fseek((FILE *)instream, 0, origin)) {
          return CURLIOE_FAILRESTART;
    }
    return CURLIOE_OK;
}


/*
 *----------------------------------------------------------------------
 *
 * curlPerform --
 *
 *  Invokes the libcurl function 'curl_easy_perform'
 *
 * Parameter:
 *  interp: Pointer to the interpreter we are using.
 *  curlHandle: the curl handle for which the option is set.
 *  objc and objv: The usual in Tcl.
 *
 * Results:
 *  Standard Tcl return codes.
 *----------------------------------------------------------------------
 */
int
curlPerform(Tcl_Interp *interp,CURL *curlHandle,struct curlObjData *curlData) {
    int         exitCode;
    Tcl_Obj     *resultPtr;

    if (curlOpenFiles(interp,curlData)) {
        return TCL_ERROR;
    }
    if (TclCurl_SetPostData(interp,curlData)) {
        return TCL_ERROR;
    }
    exitCode = curl_easy_perform(curlHandle);
    resultPtr = Tcl_NewIntObj(exitCode);
    Tcl_SetObjResult(interp,resultPtr);
    curlCloseFiles(curlData);
    TclCurl_ResetPostData(curlData);
    if (curlData->bodyVarName) {
        curlSetBodyVarName(interp,curlData);
    }
    if (curlData->command) {
        Tcl_GlobalEval(interp,curlData->command);
    }
    return exitCode;
}


/*
 *----------------------------------------------------------------------
 *
 * SetoptsList --
 *
 *  Prepares a slist for future use.
 *
 * Parameters:
 *  slistPtr: Pointer to the slist to prepare.
 *  objv: Tcl object with a list of the data.
 *
 * Results:
 *  0 if all went well.
 *  1 in case of error.
 *
 *----------------------------------------------------------------------
 */
int
SetoptsList(Tcl_Interp *interp,struct curl_slist **slistPtr,Tcl_Obj *CONST objv) {
    Tcl_Size      i,headerNumber;
    Tcl_Obj**     headers;

    if (slistPtr != NULL) {
        curl_slist_free_all(*slistPtr);
        *slistPtr = NULL;
    }

    if (Tcl_ListObjGetElements(interp,objv,&headerNumber,&headers) == TCL_ERROR) {
        return 1;
    }

    for (i=0;i<headerNumber;i++) {
       *slistPtr = curl_slist_append(*slistPtr,Tcl_GetString(headers[i]));
        if (slistPtr == NULL) {
            return 1;
        }
    }
    return 0;
}

/*
 *----------------------------------------------------------------------
 *
 * curlErrorSetOpt --
 *
 *	When an error happens when setting an option, this function
 *	takes cares of reporting it
 *
 * Parameters:
 *	interp: Pointer to the interpreter we are using.
 *	option: The index of the option in 'optionTable'
 *	parPtr: String with the parameter we wanted to set the option to.
 *
 *----------------------------------------------------------------------
 */

void
curlErrorSetOpt(Tcl_Interp *interp,const char **configTable, int option,
        const char *parPtr) {
    Tcl_Obj     *resultPtr;

    resultPtr=Tcl_ObjPrintf("setting option %s: %s",configTable[option],parPtr);
    Tcl_SetObjResult(interp,resultPtr);
}

/*
 *----------------------------------------------------------------------
 *
 * curlHeaderReader --
 *
 *  This is the function that will be invoked if the user wants to put
 *  the headers into a variable
 *
 * Parameters:
 *  header: string with the header line.
 *  size and nmemb: if so happens size * nmemb is the size of the
 *  header string.
 *  curlData: A pointer to the curlData structure for the transfer.
 *
 * Results:
 *  The number of bytes actually written or -1 in case of error, in
 *  which case 'libcurl' will abort the transfer.
 *
 *-----------------------------------------------------------------------
 */
size_t
curlHeaderReader(void *ptr,size_t size,size_t nmemb,FILE *curlDataPtr) {

    char                *header=ptr;
    struct curlObjData  *curlData=(struct curlObjData *)curlDataPtr;
    Tcl_RegExp           regExp;

    const char          *startPtr;
    const char          *endPtr;

    char                *headerName;
    char                *headerContent;
    char                *httpStatus;

    int                  match,charLength;

    regExp=Tcl_RegExpCompile(curlData->interp,"(.*?)(?::\\s*)(.*?)(\\r*)(\\n)");
    match=Tcl_RegExpExec(curlData->interp,regExp,header,header);

    if (match) {
        Tcl_RegExpRange(regExp,1,&startPtr,&endPtr);
        charLength=endPtr-startPtr;
        headerName=Tcl_Alloc(charLength+1);
        strncpy(headerName,startPtr,charLength);
        headerName[charLength]=0;

        Tcl_RegExpRange(regExp,2,&startPtr,&endPtr);
        charLength=endPtr-startPtr;
        headerContent=Tcl_Alloc(charLength+1);
        strncpy(headerContent,startPtr,charLength);
        headerContent[charLength]=0;
        /* There may be multiple 'Set-Cookie' headers, so we use a list */
        if (Tcl_StringCaseMatch(headerName,"Set-Cookie",1)) {
            Tcl_SetVar2 (curlData->interp,
                         curlData->headerVar,headerName,
                         headerContent,TCL_LIST_ELEMENT|TCL_APPEND_VALUE);
        } else {
            Tcl_SetVar2(curlData->interp,
                        curlData->headerVar,headerName,
                        headerContent,0);
        }
        Tcl_Free(headerContent);
        Tcl_Free(headerName);
    }
    regExp=Tcl_RegExpCompile(curlData->interp,"(^(HTTP|http)[^\r]+)(\r*)(\n)");
    match=Tcl_RegExpExec(curlData->interp,regExp,header,header);
    if (match) {
        Tcl_RegExpRange(regExp,1,&startPtr,&endPtr);
        charLength=endPtr-startPtr;
        httpStatus=Tcl_Alloc(charLength+1);
        strncpy(httpStatus,startPtr,charLength);
        httpStatus[charLength]=0;

        Tcl_SetVar2(curlData->interp,curlData->headerVar,"http",
                httpStatus,0);
        Tcl_Free(httpStatus);
    }
    return size*nmemb;
}

/*
 *----------------------------------------------------------------------
 *
 * curlBodyReader --
 *
 *  This is the function that will be invoked as a callback while 
 *  transferring the body of a request into a Tcl variable.
 *
 *  This function has been adapted from an example in libcurl's FAQ.
 *
 * Parameters:
 *  header: string with the header line.
 *  size and nmemb: it so happens size * nmemb if the size of the
 *  header string.
 *  curlData: A pointer to the curlData structure for the transfer.
 *
 * Results:
 *  The number of bytes actually written or -1 in case of error, in
 *  which case 'libcurl' will abort the transfer.
 *
 *-----------------------------------------------------------------------
 */
size_t
curlBodyReader(void *ptr,size_t size,size_t nmemb,FILE *curlDataPtr) {

    register int realsize = size * nmemb;
    struct MemoryStruct *mem=&(((struct curlObjData *)curlDataPtr)->bodyVar);

    mem->memory = (char *)Tcl_Realloc(mem->memory,mem->size + realsize);
    if (mem->memory) {
        memcpy(&(mem->memory[mem->size]), ptr, realsize);
        mem->size += realsize;
    }
    return realsize;
}

/*
 *----------------------------------------------------------------------
 *
 * curlProgressCallback --
 *
 *  This is the function that will be invoked as a callback during a  
 *  transfer.
 *
 *  This function has been adapted from an example in libcurl's FAQ.
 *
 * Parameters:
 *  clientData: The curlData struct for the transfer.
 *  dltotal: Total amount of bytes to download.
 *  dlnow: Bytes downloaded so far.
 *  ultotal: Total amount of bytes to upload.
 *  ulnow: Bytes uploaded so far.
 *
 * Results:
 *  Returning a non-zero value will make 'libcurl' abort the transfer
 *  and return 'CURLE_ABORTED_BY_CALLBACK'.
 *
 *-----------------------------------------------------------------------
 */
int
curlProgressCallback(void *clientData,curl_off_t dltotal,curl_off_t dlnow,
        curl_off_t ultotal,curl_off_t ulnow) {

    struct curlObjData    *curlData=(struct curlObjData *)clientData;
    Tcl_Obj               *tclProcPtr;

    if (curlData->cancelTransVarName) {
        if (curlData->cancelTrans) {
            curlData->cancelTrans=0;
            return -1;
        }
    }

    tclProcPtr = Tcl_NewListObj(0, 0);
    Tcl_ListObjAppendElement(curlData->interp, tclProcPtr, Tcl_NewStringObj(curlData->progressProc, -1));
    Tcl_ListObjAppendElement(curlData->interp, tclProcPtr, Tcl_NewWideIntObj((Tcl_WideInt)dltotal));
    Tcl_ListObjAppendElement(curlData->interp, tclProcPtr, Tcl_NewWideIntObj((Tcl_WideInt)dlnow));
    Tcl_ListObjAppendElement(curlData->interp, tclProcPtr, Tcl_NewWideIntObj((Tcl_WideInt)ultotal));
    Tcl_ListObjAppendElement(curlData->interp, tclProcPtr, Tcl_NewWideIntObj((Tcl_WideInt)ulnow));
    Tcl_IncrRefCount(tclProcPtr);

    if (Tcl_EvalObjEx(curlData->interp,tclProcPtr,TCL_EVAL_GLOBAL)!=TCL_OK) {
        Tcl_DecrRefCount(tclProcPtr);
        return -1;
    }
    Tcl_DecrRefCount(tclProcPtr);
    return 0;
}

/*
 *----------------------------------------------------------------------
 *
 * curlWriteProcInvoke --
 *
 *  This is the function that will be invoked as a callback when the user
 *  wants to invoke a Tcl procedure to write the recieved data.
 *
 *  This function has been adapted from an example in libcurl's FAQ.
 *
 * Parameters:
 *  ptr: A pointer to the data.
 *  size and nmemb: it so happens size * nmemb if the size of the
 *  data read.
 *  curlData: A pointer to the curlData structure for the transfer.
 *
 * Results:
 *  The number of bytes actually written or -1 in case of error, in
 *  which case 'libcurl' will abort the transfer.
 *
 *-----------------------------------------------------------------------
 */
size_t
curlWriteProcInvoke(void *ptr,size_t size,size_t nmemb,FILE *curlDataPtr) {
    register int realsize = size * nmemb;
    struct curlObjData  *curlData = (struct curlObjData *)curlDataPtr;
    register int        curl_retcode;
    int                 code;
    int                 cmd_list_size;
    const char        **argvPtr;
    Tcl_Size            argcPtr;
    Tcl_Obj**           objList;
    int                 i;

    if (curlData->cancelTransVarName) {
        if (curlData->cancelTrans) {
            curlData->cancelTrans = 0;
            return -1;
        }
    }

    curl_retcode = realsize;
    if (Tcl_SplitList(curlData->interp,curlData->writeProc,&argcPtr,&argvPtr) != TCL_OK) {
        return -1;
    }

    cmd_list_size = argcPtr;

    objList = (Tcl_Obj **) Tcl_Alloc((cmd_list_size+1)*sizeof(Tcl_Obj*));
    for (i = 0; i < cmd_list_size; i++) {
        objList[i] = Tcl_NewStringObj(argvPtr[i],-1);
        Tcl_IncrRefCount(objList[i]);
    }
    objList[cmd_list_size] = Tcl_NewByteArrayObj(ptr,realsize);
    Tcl_IncrRefCount(objList[cmd_list_size]);

    code = Tcl_EvalObjv(curlData->interp,cmd_list_size+1,objList,TCL_EVAL_GLOBAL);
    if (code != TCL_OK) {
        curl_retcode = -1;
    }
    for (i = 0; i <= cmd_list_size; i++) { Tcl_DecrRefCount(objList[i]); }
    Tcl_Free((char *)objList);
    Tcl_Free((char *)argvPtr);
    return curl_retcode;
}

/*
 *----------------------------------------------------------------------
 *
 * curlReadProcInvoke --
 *
 *  This is the function that will be invoked as a callback when the user
 *  wants to invoke a Tcl procedure to read the data to send.
 *
 * Parameters:
 *  header: string with the header line.
 *  size and nmemb: it so happens size * nmemb if the size of the
 *  header string.
 *  curlData: A pointer to the curlData structure for the transfer.
 *
 * Results:
 *  The number of bytes actually read or CURL_READFUNC_ABORT in case
 *  of error, in which case 'libcurl' will abort the transfer.
 *
 *-----------------------------------------------------------------------
 */
size_t
curlReadProcInvoke(void *ptr,size_t size,size_t nmemb,FILE *curlDataPtr) {
    register int realsize = size * nmemb;
    struct curlObjData  *curlData=(struct curlObjData *)curlDataPtr;
    Tcl_Obj             *tclProcPtr;
    Tcl_Obj             *readDataPtr;
    unsigned char       *readBytes;
    Tcl_Size             sizeRead;

    if (curlData->cancelTransVarName) {
        if (curlData->cancelTrans) {
            curlData->cancelTrans=0;
            return CURL_READFUNC_ABORT;
        }
    }
    tclProcPtr=Tcl_ObjPrintf("%s %d",curlData->readProc,realsize);
    Tcl_IncrRefCount(tclProcPtr);
    if (Tcl_EvalObjEx(curlData->interp,tclProcPtr,TCL_EVAL_GLOBAL)!=TCL_OK) {
        Tcl_DecrRefCount(tclProcPtr);
        return CURL_READFUNC_ABORT;
    }
    Tcl_DecrRefCount(tclProcPtr);
    readDataPtr=Tcl_GetObjResult(curlData->interp);
    readBytes=Tcl_GetByteArrayFromObj(readDataPtr,&sizeRead);
    memcpy(ptr,readBytes,sizeRead);

    return sizeRead;
}

/*
 *----------------------------------------------------------------------
 *
 * curlChunkBgnProcInvoke --
 *
 *  This is the function that will be invoked as a callback when the user
 *  wants to invoke a Tcl procedure to process every wildcard matching file
 *  on a ftp transfer.
 *
 * Parameters:
 *  transfer_info: a curl_fileinfo structure about the file.
 *  curlData: A pointer to the curlData structure for the transfer.
 *  remains: number of chunks remaining.
 *
 *-----------------------------------------------------------------------
 */
long
curlChunkBgnProcInvoke (const void *transfer_info, void *curlDataPtr, int remains) {
    struct curlObjData             *curlData=(struct curlObjData *)curlDataPtr;
    Tcl_Obj                        *tclProcPtr;
    int                             i;
    const struct curl_fileinfo     *fileinfoPtr=(const struct curl_fileinfo *)transfer_info;

    if (curlData->chunkBgnVar==NULL) {
        curlData->chunkBgnVar=curlstrdup("fileData");
    }

    Tcl_SetVar2(curlData->interp,curlData->chunkBgnVar,"filename",
            fileinfoPtr->filename,0);

    switch(fileinfoPtr->filetype) {
        case 0:
            Tcl_SetVar2(curlData->interp,curlData->chunkBgnVar,"filetype","file",0);
            break;
        case 1:
            Tcl_SetVar2(curlData->interp,curlData->chunkBgnVar,"filetype","directory",0);
            break;
        case 2:
            Tcl_SetVar2(curlData->interp,curlData->chunkBgnVar,"filetype","symlink",0);
            break;
        case 3:
            Tcl_SetVar2(curlData->interp,curlData->chunkBgnVar,"filetype","device block",0);
            break;
        case 4:
            Tcl_SetVar2(curlData->interp,curlData->chunkBgnVar,"filetype","device char",0);
            break;
        case 5:
            Tcl_SetVar2(curlData->interp,curlData->chunkBgnVar,"filetype","named pipe",0);
            break;
        case 6:
            Tcl_SetVar2(curlData->interp,curlData->chunkBgnVar,"filetype","socket",0);
            break;
        case 7:
            Tcl_SetVar2(curlData->interp,curlData->chunkBgnVar,"filetype","door",0);
            break;
        case 8:
            Tcl_SetVar2(curlData->interp,curlData->chunkBgnVar,"filetype","error",0);
            break;
    }

    Tcl_SetVar2Ex(curlData->interp,curlData->chunkBgnVar,"time",
            Tcl_NewLongObj(fileinfoPtr->time),0);

    Tcl_SetVar2Ex(curlData->interp,curlData->chunkBgnVar,"perm",
            Tcl_NewIntObj(fileinfoPtr->perm),0);

    Tcl_SetVar2Ex(curlData->interp,curlData->chunkBgnVar,"uid",
            Tcl_NewIntObj(fileinfoPtr->uid),0);
    Tcl_SetVar2Ex(curlData->interp,curlData->chunkBgnVar,"gid",
            Tcl_NewIntObj(fileinfoPtr->gid),0);
    Tcl_SetVar2Ex(curlData->interp,curlData->chunkBgnVar,"size",
            Tcl_NewLongObj(fileinfoPtr->size),0);
    Tcl_SetVar2Ex(curlData->interp,curlData->chunkBgnVar,"hardlinks",
            Tcl_NewIntObj(fileinfoPtr->hardlinks),0);
    Tcl_SetVar2Ex(curlData->interp,curlData->chunkBgnVar,"flags",
            Tcl_NewIntObj(fileinfoPtr->flags),0);

    tclProcPtr=Tcl_ObjPrintf("%s %d",curlData->chunkBgnProc,remains);
    Tcl_IncrRefCount(tclProcPtr);
    if (Tcl_EvalObjEx(curlData->interp,tclProcPtr,TCL_EVAL_GLOBAL)!=TCL_OK) {
        Tcl_DecrRefCount(tclProcPtr);
        return CURL_CHUNK_BGN_FUNC_FAIL;
    }
    Tcl_DecrRefCount(tclProcPtr);

    if (Tcl_GetIntFromObj(curlData->interp,Tcl_GetObjResult(curlData->interp),&i)!=TCL_OK) {
        return CURL_CHUNK_BGN_FUNC_FAIL;
    }
    switch(i) {
        case 0:
            return CURL_CHUNK_BGN_FUNC_OK;
        case 1:
            return CURL_CHUNK_BGN_FUNC_SKIP;
    }
    return CURL_CHUNK_BGN_FUNC_FAIL;
}

/*
 *----------------------------------------------------------------------
 *
 * curlChunkEndProcInvoke --
 *
 *  This is the function that will be invoked every time a file has
 *  been downloaded or skipped, it does little more than called the
 *  given proc.
 *
 * Parameters:
 *  curlData: A pointer to the curlData structure for the transfer.
 *
 * Results:
 *
 *-----------------------------------------------------------------------
 */
long
curlChunkEndProcInvoke (void *curlDataPtr) {

    struct curlObjData      *curlData=(struct curlObjData *)curlDataPtr;
    Tcl_Obj                 *tclProcPtr;
    int                      i;

    tclProcPtr=Tcl_NewStringObj(curlData->chunkEndProc,-1);
    Tcl_IncrRefCount(tclProcPtr);
    if (Tcl_EvalObjEx(curlData->interp,tclProcPtr,TCL_EVAL_GLOBAL)!=TCL_OK) {
        Tcl_DecrRefCount(tclProcPtr);
        return CURL_CHUNK_END_FUNC_FAIL;
    }
    Tcl_DecrRefCount(tclProcPtr);

    if (Tcl_GetIntFromObj(curlData->interp,Tcl_GetObjResult(curlData->interp),&i)!=TCL_OK) {
        return CURL_CHUNK_END_FUNC_FAIL;
    }
    if (i==1) {
        return CURL_CHUNK_BGN_FUNC_FAIL;
    }
    return CURL_CHUNK_END_FUNC_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * curlfnmatchProcInvoke --
 *
 *  This is the function that will be invoked to tell whether a filename
 *  matches a pattern when doing a 'wildcard' download. It invokes a Tcl
 *  proc to do the actual work.
 *
 * Parameters:
 *  curlData: A pointer to the curlData structure for the transfer.
 *  pattern: The pattern to match.
 *  filename: The file name to be matched.
 *
 *-----------------------------------------------------------------------
 */
int
curlfnmatchProcInvoke(void *curlDataPtr, const char *pattern, const char *filename) {

    struct curlObjData      *curlData=(struct curlObjData *)curlDataPtr;
    Tcl_Obj                 *tclProcPtr;
    int                      i;

    tclProcPtr=Tcl_ObjPrintf("%s %s %s",curlData->fnmatchProc,pattern,filename);
    Tcl_IncrRefCount(tclProcPtr);
    if (Tcl_EvalObjEx(curlData->interp,tclProcPtr,TCL_EVAL_GLOBAL)!=TCL_OK) {
        Tcl_DecrRefCount(tclProcPtr);
        return CURL_FNMATCHFUNC_FAIL;
    }
    Tcl_DecrRefCount(tclProcPtr);

    if (Tcl_GetIntFromObj(curlData->interp,Tcl_GetObjResult(curlData->interp),&i)!=TCL_OK) {
        return CURL_FNMATCHFUNC_FAIL;
    }
    switch(i) {
        case 0:
            return CURL_FNMATCHFUNC_MATCH;
        case 1:
            return CURL_FNMATCHFUNC_NOMATCH;
    }
    return CURL_FNMATCHFUNC_FAIL;
}

/*
 *----------------------------------------------------------------------
 *
 * curlshkeyextract --
 *
 *  Out of one of libcurl's ssh key struct, this function will return a 
 *  Tcl_Obj with a list, the first element is the type ok key, the second
 *  the key itself.
 *
 * Parameters:
 *  interp: The interp need to deal with the objects.
 *  key: a curl_khkey struct with the key.
 *
 * Results:
 *  The object with the list.
 *
 *-----------------------------------------------------------------------
 */
Tcl_Obj *
curlsshkeyextract(Tcl_Interp *interp,const struct curl_khkey *key) {

    Tcl_Obj         *keyObjPtr;

    keyObjPtr=Tcl_NewListObj(0,(Tcl_Obj **)NULL);

    switch(key->keytype) {
        case CURLKHTYPE_RSA1:
            Tcl_ListObjAppendElement(interp,keyObjPtr,Tcl_NewStringObj("rsa1",-1));
            break;
        case CURLKHTYPE_RSA:
            Tcl_ListObjAppendElement(interp,keyObjPtr,Tcl_NewStringObj("rsa",-1));
            break;
        case CURLKHTYPE_DSS:
            Tcl_ListObjAppendElement(interp,keyObjPtr,Tcl_NewStringObj("dss",-1));
            break;
        default:
            Tcl_ListObjAppendElement(interp,keyObjPtr,Tcl_NewStringObj("unknnown",-1));
            break;
    }
    Tcl_ListObjAppendElement(interp,keyObjPtr,Tcl_NewStringObj(key->key,-1));

    return keyObjPtr;
}

/*
 *----------------------------------------------------------------------
 *
 * curlshkeycallback --
 *
 *  This is the function that will be invoked as a callback when the user
 *  wants to invoke a Tcl procedure to decide about this new ssh host
 *
 * Parameters:
 *  curl: curl's easy handle for the connection.
 *  knownkey:    The key from the hosts_file.
 *  foundkey:    The key from the remote site.
 *  match:       What libcurl thinks about how they match
 *  curlDataPtr: Points to the structure with all the TclCurl data
 *               for the connection.
 *
 * Results:
 *  A libcurl return code so that libcurl knows what to do.
 *
 *-----------------------------------------------------------------------
 */
size_t
curlsshkeycallback(CURL *curl ,const struct curl_khkey *knownkey,
        const struct curl_khkey *foundkey, enum curl_khmatch match,void *curlDataPtr) {

    struct curlObjData  *tclcurlDataPtr=(struct curlObjData *)curlDataPtr;
    Tcl_Interp          *interp;

    Tcl_Obj             *objv[4];
    Tcl_Obj             *returnObjPtr;

    int                  action;
    int                  i, result;

    interp=tclcurlDataPtr->interp;

    objv[0]=Tcl_NewStringObj(tclcurlDataPtr->sshkeycallProc,-1);
    objv[1]=curlsshkeyextract(interp,knownkey);
    objv[2]=curlsshkeyextract(interp,foundkey);

    switch(match) {
        case CURLKHMATCH_OK:
            objv[3]=Tcl_NewStringObj("match",-1);
            break;
        case CURLKHMATCH_MISMATCH:
            objv[3]=Tcl_NewStringObj("mismatch",-1);
            break;
        case CURLKHMATCH_MISSING:
            objv[3]=Tcl_NewStringObj("missing",-1);
            break;
        case CURLKHMATCH_LAST:
            objv[3]=Tcl_NewStringObj("error",-1);
    }

    for (i=0;i<4;i++) Tcl_IncrRefCount(objv[i]);
    result=Tcl_EvalObjv(interp,4,objv,TCL_EVAL_GLOBAL);
    for (i=0;i<4;i++) Tcl_DecrRefCount(objv[i]);
    if (result!=TCL_OK)      {return CURLKHSTAT_REJECT;}

    returnObjPtr=Tcl_GetObjResult(interp);

    if (Tcl_GetIntFromObj(interp,returnObjPtr,&action)!=TCL_OK)   {return CURLKHSTAT_REJECT;}

    switch(action) {
        case 0:
            return CURLKHSTAT_FINE_ADD_TO_FILE;
        case 1:
            return CURLKHSTAT_FINE;
        case 2:
            return CURLKHSTAT_REJECT;
        case 3:
            return CURLKHSTAT_DEFER;
    }
    return CURLKHSTAT_REJECT;
}

/*
 *----------------------------------------------------------------------
 *
 * curlDebugProcInvoke --
 *
 *  This is the function that will be invoked as a callback when the user
 *  wants to invoke a Tcl procedure to write the debug data produce by
 *  the verbose option.
 *
 * Parameters:
 *   curlHandle: A pointer to the handle for the transfer.
 *   infoType: Integer with the type of data.
 *   dataPtr: the data passed to the procedure.
 *   curlDataPtr: ointer to the curlData structure for the transfer.
 *
 * Results:
 *   The number of bytes actually written or -1 in case of error, in
 *   which case 'libcurl' will abort the transfer.
 *
 *-----------------------------------------------------------------------
 */
int
curlDebugProcInvoke(CURL *curlHandle, curl_infotype infoType,
        char * dataPtr, size_t size, void  *curlDataPtr) {
    struct curlObjData  *curlData=(struct curlObjData *)curlDataPtr;
    Tcl_Obj             *objv[3];
    int                  i;

    if (curlData->cancelTransVarName) {
        if (curlData->cancelTrans) {
            curlData->cancelTrans=0;
            return -1;
        }
    }

    objv[0]=Tcl_NewStringObj(curlData->debugProc,-1);
    objv[1]=Tcl_NewIntObj(infoType);
    objv[2]=Tcl_NewByteArrayObj((const unsigned char *)dataPtr,size);
    for (i=0;i<3;i++) Tcl_IncrRefCount(objv[i]);
    Tcl_EvalObjv(curlData->interp,3,objv,TCL_EVAL_GLOBAL);
    for (i=0;i<3;i++) Tcl_DecrRefCount(objv[i]);

    return 0;
}

/*
 *----------------------------------------------------------------------
 *
 * curlFreeSpace --
 *
 *    Frees the space taken by a curlObjData struct either because we are
 *    deleting the handle or reseting it.
 *
 *  Parameter:
 *    interp: Pointer to the interpreter we are using.
 *    curlHandle: the curl handle for which the option is set.
 *    objc and objv: The usual in Tcl.
 *
 * Results:
 *	A standard Tcl result.
 *
 *----------------------------------------------------------------------
 */
void
curlFreeSpace(struct curlObjData *curlData) {

    curl_slist_free_all(curlData->headerList);
    curl_slist_free_all(curlData->quote);
    curl_slist_free_all(curlData->prequote);
    curl_slist_free_all(curlData->postquote);

    Tcl_Free(curlData->outFile);
    Tcl_Free(curlData->inFile);
    Tcl_Free(curlData->postFields);
    Tcl_Free(curlData->proxy);
    Tcl_Free(curlData->errorBuffer);
    Tcl_Free(curlData->errorBufferName);
    Tcl_Free(curlData->errorBufferKey);
    Tcl_Free(curlData->stderrFile);
    Tcl_Free(curlData->randomFile);
    Tcl_Free(curlData->headerVar);
    Tcl_Free(curlData->bodyVarName);
    if (curlData->bodyVar.memory) {
        Tcl_Free(curlData->bodyVar.memory);
    }
    Tcl_Free(curlData->progressProc);
    if (curlData->cancelTransVarName) {
        Tcl_UnlinkVar(curlData->interp,curlData->cancelTransVarName);
        Tcl_Free(curlData->cancelTransVarName);
    }
    Tcl_Free(curlData->writeProc);
    Tcl_Free(curlData->readProc);
    Tcl_Free(curlData->debugProc);
    curl_slist_free_all(curlData->http200aliases);
    Tcl_Free(curlData->sshkeycallProc);
    curl_slist_free_all(curlData->mailrcpt);
    Tcl_Free(curlData->chunkBgnProc);
    Tcl_Free(curlData->chunkBgnVar);
    Tcl_Free(curlData->chunkEndProc);
    Tcl_Free(curlData->fnmatchProc);
    curl_slist_free_all(curlData->resolve);
    curl_slist_free_all(curlData->telnetoptions);
#ifndef CURL_PRE_7_56_DEPR
    if (curlData->mime != NULL) {
        curl_mime_free(curlData->mime);
    }
#endif

    Tcl_Free(curlData->command);
}

/*
 *----------------------------------------------------------------------
 *
 * curlDupHandle --
 *
 *  This function is invoked by the 'duphandle' command, it will 
 *  create a duplicate of the given handle.
 *
 * Parameters:
 *  The stantard parameters for Tcl commands
 *
 * Results:
 *  A standard Tcl result.
 *
 * Side effects:
 *  See the user documentation.
 *
 *----------------------------------------------------------------------
 */
int
curlDupHandle(Tcl_Interp *interp, struct curlObjData *curlData,
        int objc, Tcl_Obj *const objv[]) {

    CURL                *newCurlHandle;
    Tcl_Obj             *result;
    struct curlObjData  *newCurlData;
    Tcl_Obj             *handleObj;

    newCurlHandle=curl_easy_duphandle(curlData->curl);
    if (newCurlHandle==NULL) {
        result=Tcl_NewStringObj("Couldn't create new handle.",-1);
        Tcl_SetObjResult(interp,result);
        return TCL_ERROR;
    }

    newCurlData=(struct curlObjData *)Tcl_Alloc(sizeof(struct curlObjData));

    curlCopyCurlData(curlData,newCurlData);
    newCurlData->curl=newCurlHandle;
    if (newCurlData->shareToken != NULL) {
        struct shcurlObjData *shcurlData = curlGetShareDataFromToken(newCurlData->shareToken);

        if (shcurlData == NULL) {
            curl_easy_cleanup(newCurlHandle);
            curlFreeSpace(newCurlData);
            Tcl_Free((char *) newCurlData);
            result=Tcl_NewStringObj("Couldn't duplicate handle with deleted share handle.",-1);
            Tcl_SetObjResult(interp,result);
            return TCL_ERROR;
        }
        curlLinkEasyToShare(newCurlData, shcurlData);
    }
    handleObj=curlCreateObjCmd(interp,newCurlData);

    Tcl_SetObjResult(interp,handleObj);

    return TCL_OK;
}


/*
 *----------------------------------------------------------------------
 *
 * curlResetHandle --
 *
 *  This function is invoked by the 'reset' command, it reset all the
 *  options in the handle to the state it had when 'init' was invoked.
 *
 * Parameters:
 *  The stantard parameters for Tcl commands
 *
 * Results:
 *  A standard Tcl result.
 *
 * Side effects:
 *	See the user documentation.
 *
 *----------------------------------------------------------------------
 */
int
curlResetHandle(Tcl_Interp *interp, struct curlObjData *curlData)  {
    struct curlObjData   *tmpPtr=
                    (struct curlObjData *)Tcl_Alloc(sizeof(struct curlObjData));

    tmpPtr->curl       = curlData->curl;
    tmpPtr->token      = curlData->token;
    tmpPtr->interp     = curlData->interp;
    tmpPtr->shareToken = NULL;
    tmpPtr->nextSharedHandle = NULL;

    curlDetachShareHandle(curlData);

    curlFreeSpace(curlData);
    memset(curlData, 0, sizeof(struct curlObjData));

    curlData->curl       = tmpPtr->curl;
    curlData->token      = tmpPtr->token;
    curlData->interp     = tmpPtr->interp;
    curlData->postFieldSize = -1;

    curl_easy_reset(curlData->curl);

    Tcl_Free((char *)tmpPtr);

    return TCL_OK;

}

/*
 *----------------------------------------------------------------------
 *
 * curlVersion --
 *
 *	This procedure is invoked to process the "curl::init" Tcl command.
 *	See the user documentation for details on what it does.
 *
 * Parameters:
 *  The stantard parameters for Tcl commands
 *
 * Results:
 *	A standard Tcl result.
 *
 * Side effects:
 *	See the user documentation.
 *
 *----------------------------------------------------------------------
 */
int
curlVersion (ClientData clientData, Tcl_Interp *interp,
    int objc,Tcl_Obj *const objv[]) {

    Tcl_Obj     *versionPtr;

    versionPtr=Tcl_ObjPrintf("TclCurl Version %s (%s)",TclCurlVersion,
                                                 curl_version());
    Tcl_SetObjResult(interp,versionPtr);

    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * curlEscape --
 *
 *  This function is invoked to process the "curl::escape" Tcl command.
 *  See the user documentation for details on what it does.
 *
 *
 * Parameters:
 *  The stantard parameters for Tcl commands
 *
 * Results:
 *  A standard Tcl result.
 *
 * Side effects:
 *  See the user documentation.
 *
 *----------------------------------------------------------------------
 */
int
curlEscape(ClientData clientData, Tcl_Interp *interp,
    int objc,Tcl_Obj *const objv[]) {

    Tcl_Obj        *resultObj;
    char           *escapedStr;

    escapedStr=curl_easy_escape(NULL,Tcl_GetString(objv[1]),0);

    if (!escapedStr) {
        resultObj=Tcl_NewStringObj("curl::escape bad parameter",-1);
        Tcl_SetObjResult(interp,resultObj);
        return TCL_ERROR;
    }
    resultObj=Tcl_NewStringObj(escapedStr,-1);
    Tcl_SetObjResult(interp,resultObj);
    curl_free(escapedStr);

    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * curlUnescape --
 *
 *  This function is invoked to process the "curl::Unescape" Tcl command.
 *  See the user documentation for details on what it does.
 *
 *
 * Parameters:
 *  The stantard parameters for Tcl commands
 *
 * Results:
 *  A standard Tcl result.
 *
 * Side effects:
 *  See the user documentation.
 *
 *----------------------------------------------------------------------
 */
int
curlUnescape(ClientData clientData, Tcl_Interp *interp,
    int objc,Tcl_Obj *const objv[]) {

    Tcl_Obj        *resultObj;
    char           *unescapedStr;

    unescapedStr=curl_easy_unescape(NULL,Tcl_GetString(objv[1]),0,NULL);
    if (!unescapedStr) {
        resultObj=Tcl_NewStringObj("curl::unescape bad parameter",-1);
        Tcl_SetObjResult(interp,resultObj);
        return TCL_ERROR;
    }
    resultObj=Tcl_NewStringObj(unescapedStr,-1);
    Tcl_SetObjResult(interp,resultObj);
    curl_free(unescapedStr);

    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * curlVersionInfo --
 *
 *  This function invokes 'curl_version_info' to query how 'libcurl' was
 *  compiled.
 *
 * Parameters:
 *  The standard parameters for Tcl commands, but nothing is used.
 *
 * Results:
 *  A standard Tcl result.
 *
 * Side effects:
 *  See the user documentation.
 *
 *----------------------------------------------------------------------
 */
int
curlVersionInfo (ClientData clientData, Tcl_Interp *interp,
    int objc,Tcl_Obj *const objv[]) {

    int                            tableIndex;
    int                            i;
    curl_version_info_data        *infoPtr;
    Tcl_Obj                       *resultObjPtr=NULL;

    if (objc!=2) {
        resultObjPtr=Tcl_NewStringObj("usage: curl::versioninfo -option",-1);
        Tcl_SetObjResult(interp,resultObjPtr);
        return TCL_ERROR;
    }

    if (Tcl_GetIndexFromObj(interp, objv[1], versionInfoTable, "option",
            TCL_EXACT,&tableIndex)==TCL_ERROR) {
        return TCL_ERROR;
    }

    infoPtr=curl_version_info(CURLVERSION_NOW);

    switch(tableIndex) {
        case 0:
            resultObjPtr=Tcl_NewStringObj(infoPtr->version,-1);
            break;
        case 1:
            resultObjPtr=Tcl_ObjPrintf("%X",infoPtr->version_num);
            break;
        case 2:
            resultObjPtr=Tcl_NewStringObj(infoPtr->host,-1);
            break;
        case 3:
            resultObjPtr=Tcl_NewListObj(0,(Tcl_Obj **)NULL);
            if (infoPtr->features&CURL_VERSION_IPV6) {
                Tcl_ListObjAppendElement(interp,resultObjPtr,
                        Tcl_NewStringObj("IPV6",-1));
            }
            if (infoPtr->features&CURL_VERSION_KERBEROS4) {
                Tcl_ListObjAppendElement(interp,resultObjPtr,
                        Tcl_NewStringObj("KERBEROS4",-1));
            }
            if (infoPtr->features&CURL_VERSION_SSL) {
                Tcl_ListObjAppendElement(interp,resultObjPtr,
                        Tcl_NewStringObj("SSL",-1));
            }
            if (infoPtr->features&CURL_VERSION_LIBZ) {
                Tcl_ListObjAppendElement(interp,resultObjPtr,
                        Tcl_NewStringObj("LIBZ",-1));
            }
            if (infoPtr->features&CURL_VERSION_NTLM) {
                Tcl_ListObjAppendElement(interp,resultObjPtr,
                        Tcl_NewStringObj("NTLM",-1));
            }
            if (infoPtr->features&CURL_VERSION_GSSNEGOTIATE) {
                Tcl_ListObjAppendElement(interp,resultObjPtr,
                        Tcl_NewStringObj("GSSNEGOTIATE",-1));
            }
            if (infoPtr->features&CURL_VERSION_DEBUG) {
                Tcl_ListObjAppendElement(interp,resultObjPtr,
                        Tcl_NewStringObj("DEBUG",-1));
            }
            if (infoPtr->features&CURL_VERSION_ASYNCHDNS) {
                Tcl_ListObjAppendElement(interp,resultObjPtr,
                        Tcl_NewStringObj("ASYNCHDNS",-1));
            }
            if (infoPtr->features&CURL_VERSION_SPNEGO) {
                Tcl_ListObjAppendElement(interp,resultObjPtr,
                        Tcl_NewStringObj("SPNEGO",-1));
            }
            if (infoPtr->features&CURL_VERSION_LARGEFILE) {
                Tcl_ListObjAppendElement(interp,resultObjPtr,
                        Tcl_NewStringObj("LARGEFILE",-1));
            }
            if (infoPtr->features&CURL_VERSION_IDN) {
                Tcl_ListObjAppendElement(interp,resultObjPtr,
                        Tcl_NewStringObj("IDN",-1));
            }
            if (infoPtr->features&CURL_VERSION_SSPI) {
                Tcl_ListObjAppendElement(interp,resultObjPtr,
                        Tcl_NewStringObj("SSPI",-1));
            }
            break;
            if (infoPtr->features&CURL_VERSION_CONV) {
                Tcl_ListObjAppendElement(interp,resultObjPtr,
                                         Tcl_NewStringObj("CONV",-1));
            }
        case 4:
            resultObjPtr=Tcl_NewStringObj(infoPtr->ssl_version,-1);
            break;
        case 5:
            resultObjPtr=Tcl_NewLongObj(infoPtr->ssl_version_num);
            break;
        case 6:
            resultObjPtr=Tcl_NewStringObj(infoPtr->libz_version,-1);
            break;
        case 7:
        {
            resultObjPtr=Tcl_NewListObj(0,(Tcl_Obj **)NULL);
            for (i=0;;i++) {
                if (infoPtr->protocols[i]!=NULL) {
                    Tcl_ListObjAppendElement(interp,resultObjPtr,
                                             Tcl_NewStringObj(infoPtr->protocols[i],-1));
                } else {
                    break;
                }
            }
        }
    }

    Tcl_SetObjResult(interp,resultObjPtr);

    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * curlCopyCurlData --
 *
 *  This function copies the contents of a curlData struct into another.
 *
 * Parameters:
 *  curlDataOld: The original one.
 *  curlDataNew: The new one
 *
 * Results:
 *  A standard Tcl result.
 *
 * Side effects:
 *  See the user documentation.
 *
 *----------------------------------------------------------------------
 */
int
curlCopyCurlData (struct curlObjData *curlDataOld,
                      struct curlObjData *curlDataNew) {

    /* This takes care of the int and long values */
    memcpy(curlDataNew, curlDataOld, sizeof(struct curlObjData));

    /* Some of the data doesn't get copied */

    curlDataNew->headerList=NULL;
    curlDataNew->quote=NULL;
    curlDataNew->prequote=NULL;
    curlDataNew->postquote=NULL;
    curlDataNew->postListFirst=NULL;
    curlDataNew->postListLast=NULL;
    curlDataNew->outHandle=NULL;
    curlDataNew->outFlag=0;
    curlDataNew->inHandle=NULL;
    curlDataNew->inFlag=0;
    curlDataNew->headerHandle=NULL;
    curlDataNew->headerFlag=0;
    curlDataNew->stderrHandle=NULL;
    curlDataNew->stderrFlag=0;
    curlDataNew->http200aliases=NULL;
    curlDataNew->mailrcpt=NULL;
    curlDataNew->resolve=NULL;
    curlDataNew->telnetoptions=NULL;
#ifdef CURL_PRE_7_56_DEPR
    curlDataNew->formArray=NULL;
#else
    curlDataNew->mime=NULL;
#endif

    /* The strings need a special treatment. */

    curlDataNew->outFile=curlstrdup(curlDataOld->outFile);
    curlDataNew->inFile=curlstrdup(curlDataOld->inFile);
    curlDataNew->postFields=curlstrdup(curlDataOld->postFields);
    curlDataNew->proxy=curlstrdup(curlDataOld->proxy);
    curlDataNew->errorBuffer=curlstrdup(curlDataOld->errorBuffer);
    curlDataNew->errorBufferName=curlstrdup(curlDataOld->errorBufferName);
    curlDataNew->errorBufferKey=curlstrdup(curlDataOld->errorBufferKey);
    curlDataNew->headerFile=curlstrdup(curlDataOld->headerFile);
    curlDataNew->stderrFile=curlstrdup(curlDataOld->stderrFile);
    curlDataNew->randomFile=curlstrdup(curlDataOld->randomFile);
    curlDataNew->headerVar=curlstrdup(curlDataOld->headerVar);
    curlDataNew->bodyVarName=curlstrdup(curlDataOld->bodyVarName);
    curlDataNew->progressProc=curlstrdup(curlDataOld->progressProc);
    curlDataNew->cancelTransVarName=curlstrdup(curlDataOld->cancelTransVarName);
    curlDataNew->writeProc=curlstrdup(curlDataOld->writeProc);
    curlDataNew->readProc=curlstrdup(curlDataOld->readProc);
    curlDataNew->debugProc=curlstrdup(curlDataOld->debugProc);
    curlDataNew->command=curlstrdup(curlDataOld->command);
    curlDataNew->sshkeycallProc=curlstrdup(curlDataOld->sshkeycallProc);
    curlDataNew->chunkBgnProc=curlstrdup(curlDataOld->chunkBgnProc);
    curlDataNew->chunkBgnVar=curlstrdup(curlDataOld->chunkBgnVar);
    curlDataNew->chunkEndProc=curlstrdup(curlDataOld->chunkEndProc);
    curlDataNew->fnmatchProc=curlstrdup(curlDataOld->fnmatchProc);

    curlDataNew->bodyVar.memory=(char *)Tcl_Alloc(curlDataOld->bodyVar.size);
    memcpy(curlDataNew->bodyVar.memory,curlDataOld->bodyVar.memory
            ,curlDataOld->bodyVar.size);
    curlDataNew->bodyVar.size=curlDataOld->bodyVar.size;

    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * curlOpenFiles --
 *
 *	Before doing a transfer with the easy interface or adding an easy
 *	handle to a multi one, this function takes care of opening all
 *	necessary files for the transfer.
 *
 * Parameters:
 *	curlData: The pointer to the struct with the transfer data.
 *
 * Results:
 *	'0' all went well, '1' in case of error.
 *
 *----------------------------------------------------------------------
 */
int
curlOpenFiles(Tcl_Interp *interp,struct curlObjData *curlData) {

    if (curlData->outFlag) {
        if (curlOpenFile(interp,curlData->outFile,&(curlData->outHandle),1,
                curlData->transferText)) {
            return 1;
        }
        curl_easy_setopt(curlData->curl,CURLOPT_WRITEDATA,curlData->outHandle);
    }
    if (curlData->inFlag) {
        if (curlOpenFile(interp,curlData->inFile,&(curlData->inHandle),0,
                curlData->transferText)) {
            return 1;
        }
        curl_easy_setopt(curlData->curl,CURLOPT_READDATA,curlData->inHandle);
        if (curlData->anyAuthFlag) {
            curl_easy_setopt(curlData->curl, CURLOPT_SEEKFUNCTION, (curl_seek_callback)curlseek);
            curl_easy_setopt(curlData->curl, CURLOPT_SEEKDATA, curlData->inHandle);
        }
    }
    if (curlData->headerFlag) {
        if (curlOpenFile(interp,curlData->headerFile,&(curlData->headerHandle),1,1)) {
            return 1;
        }
        curl_easy_setopt(curlData->curl,CURLOPT_HEADERDATA,curlData->headerHandle);
    }
    if (curlData->stderrFlag) {
        if (curlOpenFile(interp,curlData->stderrFile,&(curlData->stderrHandle),1,1)) {
            return 1;
        }
        curl_easy_setopt(curlData->curl,CURLOPT_STDERR,curlData->stderrHandle);
    }
    return 0;
}

/*----------------------------------------------------------------------
 *
 * curlCloseFiles --
 *
 *  Closes the files opened during a transfer.
 *
 * Parameters:
 *	curlData: The pointer to the struct with the transfer data.
 *
 *----------------------------------------------------------------------
 */
void
curlCloseFiles(struct curlObjData *curlData) {
    if (curlData->outHandle!=NULL) {
        fclose(curlData->outHandle);
        curlData->outHandle=NULL;
    }
    if (curlData->inHandle!=NULL) {
        fclose(curlData->inHandle);
        curlData->inHandle=NULL;
    }
    if (curlData->headerHandle!=NULL) {
        fclose(curlData->headerHandle);
        curlData->headerHandle=NULL;
    }
    if (curlData->stderrHandle!=NULL) {
        fclose(curlData->stderrHandle);
        curlData->stderrHandle=NULL;
    }
}

/*----------------------------------------------------------------------
 *
 * curlOpenFile --
 *
 *  Opens a file to be used during a transfer.
 *
 * Parameter:
 *  fileName: name of the file.
 *  handle: the handle for the file
 *  writing: '0' if reading, '1' if writing.
 *  text:    '0' if binary, '1' if text.
 *
 * Results:
 *  '0' all went well, '1' in case of error.
 *----------------------------------------------------------------------
 */
int
curlOpenFile(Tcl_Interp *interp,char *fileName, FILE **handle, int writing, int text) {
    Tcl_Obj        *resultObjPtr;

#ifdef _WIN32
    Tcl_DString     nativeString;
    TCHAR          *nativeFile=Tcl_WinUtfToTChar(fileName,-1,&nativeString);
#endif

    if (*handle!=NULL) {
        fclose(*handle);
    }
    if (writing==1) {
#ifdef _WIN32
        *handle=_tfopen(nativeFile, text==1 ? _T("w") : _T("wb"));
#else
        *handle=fopen(fileName, text==1 ? "w" : "wb");
#endif
    } else {
#ifdef _WIN32
        *handle=_tfopen(nativeFile, text==1 ? _T("r") : _T("rb"));
#else
        *handle=fopen(fileName, text==1 ? "r" : "rb");
#endif
    }
#ifdef _WIN32
    Tcl_DStringFree(&nativeString);
#endif
    if (*handle==NULL) {
        resultObjPtr=Tcl_ObjPrintf("Couldn't open file %s",fileName);
        Tcl_SetObjResult(interp,resultObjPtr);
        return 1;
    }
    return 0;
}

/*----------------------------------------------------------------------
 *
 * curlSetBodyVarName --
 *
 *  After performing a transfer, this function is invoked to set the 
 *  body of the recieved transfer into a user defined Tcl variable.
 *
 * Parameter:
 *  interp: The Tcl interpreter we are using.
 *  curlData: A pointer to the struct with the transfer data.
 *----------------------------------------------------------------------
 */
void
curlSetBodyVarName(Tcl_Interp *interp,struct curlObjData *curlDataPtr) {
    Tcl_Obj    *bodyVarNameObjPtr, *bodyVarObjPtr;

    bodyVarNameObjPtr=Tcl_NewStringObj(curlDataPtr->bodyVarName,-1);
    bodyVarObjPtr=Tcl_NewByteArrayObj((unsigned char *)curlDataPtr->bodyVar.memory,
            curlDataPtr->bodyVar.size);

    Tcl_ObjSetVar2(interp,bodyVarNameObjPtr,(Tcl_Obj *)NULL,bodyVarObjPtr,0);

    Tcl_Free(curlDataPtr->bodyVar.memory);
    curlDataPtr->bodyVar.memory=NULL;
    curlDataPtr->bodyVar.size=0;
}

/*----------------------------------------------------------------------
 *
 * curlstrdup --
 *	The same as strdup, but won't seg fault if the string to copy is NULL.
 *
 * Parameters:
 *	old: The original one.
 *
 * Results:
 *	Returns a pointer to the new string.
 *
 *----------------------------------------------------------------------
 */
char
*curlstrdup (char *old) {
    char    *tmpPtr;

    if (old == NULL) {
        return NULL;
    }
    tmpPtr = Tcl_Alloc(strlen(old)+1);
    strcpy(tmpPtr,old);

    return tmpPtr;
}

/*
 *----------------------------------------------------------------------
 *
 * curlCreateShareObjCmd --
 *
 *  Looks for the first free share handle (scurl1, scurl2,...) and
 *  creates a Tcl command for it.
 *
 * Results:
 *  A string with the name of the handle, don't forget to free it.
 *
 * Side effects:
 *  See the user documentation.
 *
 *----------------------------------------------------------------------
 */
Tcl_Obj *
curlCreateShareObjCmd (Tcl_Interp *interp,struct shcurlObjData  *shcurlData) {
    char                shandleName[32];
    int                 i;
    Tcl_CmdInfo         info;
    Tcl_Command         cmdToken;

    /* We try with scurl1, if it already exists with scurl2...*/
    for (i=1;;i++) {
        snprintf(shandleName,sizeof(shandleName),"scurl%d",i);
        if (!Tcl_GetCommandInfo(interp,shandleName,&info)) {
            cmdToken=Tcl_CreateObjCommand(interp,shandleName,curlShareObjCmd,
                                (ClientData)shcurlData,
                                (Tcl_CmdDeleteProc *)curlCleanUpShareCmd);
            break;
        }
    }
    shcurlData->token=cmdToken;

    return Tcl_NewStringObj(shandleName,-1);
}

/*
 *----------------------------------------------------------------------
 *
 * curlShareInitObjCmd --
 *
 *  This procedure is invoked to process the "curl::shareinit" Tcl command.
 *  See the user documentation for details on what it does.
 *
 * Results:
 *  A standard Tcl result.
 *
 * Side effects:
 *  See the user documentation.
 *
 *----------------------------------------------------------------------
 */

int
curlShareInitObjCmd (ClientData clientData, Tcl_Interp *interp,
        int objc,Tcl_Obj *const objv[]) {

    Tcl_Obj               *resultPtr;
    CURLSH                *shcurlHandle;
    struct shcurlObjData  *shcurlData;
    Tcl_Obj               *shandleObj;

    shcurlData=(struct shcurlObjData *)Tcl_Alloc(sizeof(struct shcurlObjData));
    if (shcurlData == NULL) {
        resultPtr=Tcl_NewStringObj("Couldn't allocate memory",-1);
        Tcl_SetObjResult(interp,resultPtr);
        return TCL_ERROR;
    }

    memset(shcurlData, 0, sizeof(struct shcurlObjData));

    shcurlHandle=curl_share_init();
    if (shcurlHandle == NULL) {
        resultPtr=Tcl_NewStringObj("Couldn't create share handle",-1);
        Tcl_SetObjResult(interp,resultPtr);
        return TCL_ERROR;
    }

    shandleObj=curlCreateShareObjCmd(interp,shcurlData);

    shcurlData->shandle=shcurlHandle;

    Tcl_SetObjResult(interp,shandleObj);

#ifdef TCL_THREADS
    curl_share_setopt(shcurlHandle, CURLSHOPT_LOCKFUNC, curlShareLockFunc);
    curl_share_setopt(shcurlHandle, CURLSHOPT_UNLOCKFUNC, curlShareUnLockFunc);
#endif

    return TCL_OK;
}

#ifdef TCL_THREADS
/*
 *----------------------------------------------------------------------
 *
 * curlShareLockFunc --
 *
 *  This will be the function invoked by libcurl when it wants to lock
 *  some data for the share interface.
 *
 * Side effects:
 *  See the user documentation.
 *
 *----------------------------------------------------------------------
 */

void
curlShareLockFunc (CURL *handle, curl_lock_data data, curl_lock_access access,
                   void *userptr) {

    switch(data) {
        case CURL_LOCK_DATA_COOKIE:
            Tcl_MutexLock(&cookieLock);
            break;
        case CURL_LOCK_DATA_DNS:
            Tcl_MutexLock(&dnsLock);
            break;
        case CURL_LOCK_DATA_SSL_SESSION:
            Tcl_MutexLock(&sslLock);
            break;
        case CURL_LOCK_DATA_CONNECT:
            Tcl_MutexLock(&connectLock);
            break;
        default:
            /* Prevent useless compile warnings */
            break;
    }
}

/*
 *----------------------------------------------------------------------
 *
 * curlShareUnLockFunc --
 *
 *  This will be the function invoked by libcurl when it wants to unlock
 *  the previously locked data.
 *
 * Side effects:
 *  See the user documentation.
 *
 *----------------------------------------------------------------------
 */
void
curlShareUnLockFunc(CURL *handle, curl_lock_data data, void *userptr) {

    switch(data) {
        case CURL_LOCK_DATA_COOKIE:
            Tcl_MutexUnlock(&cookieLock);
            break;
        case CURL_LOCK_DATA_DNS:
            Tcl_MutexUnlock(&dnsLock);
            break;
        case CURL_LOCK_DATA_SSL_SESSION:
            Tcl_MutexUnlock(&sslLock);
            break;
        case CURL_LOCK_DATA_CONNECT:
            Tcl_MutexUnlock(&connectLock);
            break;
        default:
            break;
    }
}

#endif

/*
 *----------------------------------------------------------------------
 *
 * curlShareObjCmd --
 *
 *   This procedure is invoked to process the "share curl" commands.
 *   See the user documentation for details on what it does.
 *
 * Results:
 *   A standard Tcl result.
 *
 * Side effects:
 *   See the user documentation.
 *
 *----------------------------------------------------------------------
 */
int
curlShareObjCmd (ClientData clientData, Tcl_Interp *interp,
                 int objc,Tcl_Obj *const objv[]) {

    struct shcurlObjData     *shcurlData = (struct shcurlObjData *)clientData;
    CURLSH                   *shcurlHandle = shcurlData->shandle;
    int                       tableIndex, dataIndex;
    int                       dataToLock=0;

    if (objc<2) {
        Tcl_WrongNumArgs(interp,1,objv,"option arg ?arg?");
        return TCL_ERROR;
    }

    if (Tcl_GetIndexFromObj(interp, objv[1], shareCmd, "option",TCL_EXACT,&tableIndex)==TCL_ERROR) {
        return TCL_ERROR;
    }

    switch(tableIndex) {
        case 0:
        case 1:
            if (objc != 3) {
                Tcl_WrongNumArgs(interp,2,objv,"arg");
                return TCL_ERROR;
            }
            if (Tcl_GetIndexFromObj(interp, objv[2], lockData,
                                    "data to lock ",TCL_EXACT,&dataIndex) == TCL_ERROR) {
                return TCL_ERROR;
            }
            switch (dataIndex) {
                case 0:
                    dataToLock = CURL_LOCK_DATA_COOKIE;
                    break;
                case 1:
                    dataToLock = CURL_LOCK_DATA_DNS;
                    break;
            }
            if (tableIndex == 0) {
                curl_share_setopt(shcurlHandle, CURLSHOPT_SHARE,   dataToLock);
            } else {
                curl_share_setopt(shcurlHandle, CURLSHOPT_UNSHARE, dataToLock);
            }
            break;
        case 2:
            if (objc != 2) {
                Tcl_WrongNumArgs(interp,2,objv,"");
                return TCL_ERROR;
            }
            if (shcurlData->users != NULL) {
                Tcl_SetObjResult(interp,
                    Tcl_NewStringObj("Share handle is still attached to one or more easy handles.", -1));
                return TCL_ERROR;
            }
            Tcl_DeleteCommandFromToken(interp,shcurlData->token);
            break;
    }
    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * curlCleanUpShareCmd --
 *
 *   This procedure is invoked when curl share handle is deleted.
 *
 * Results:
 *   A standard Tcl result.
 *
 * Side effects:
 *   Cleans the curl share handle and frees the memory.
 *
 *----------------------------------------------------------------------
 */
int
curlCleanUpShareCmd(ClientData clientData) {
    struct shcurlObjData     *shcurlData=(struct shcurlObjData *)clientData;
    CURLSH                   *shcurlHandle=shcurlData->shandle;
    struct curlObjData       *curlData = shcurlData->users;

    while (curlData != NULL) {
        struct curlObjData *next = curlData->nextSharedHandle;

        curlData->shareToken = NULL;
        curlData->nextSharedHandle = NULL;
        if (curlData->curl != NULL) {
            curl_easy_setopt(curlData->curl, CURLOPT_SHARE, NULL);
        }
        curlData = next;
    }

    curl_share_cleanup(shcurlHandle);
    Tcl_Free((char *)shcurlData);

    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * curlErrorStrings --
 *
 *  All the commands to return the error string from the error code have
 *  this function in common.
 *
 * Results:
 *  '0': All went well.
 *  '1': The error code didn't make sense.
 *----------------------------------------------------------------------
 */
int
curlErrorStrings (Tcl_Interp *interp, Tcl_Obj *const objv,int type) {

    Tcl_Obj               *resultPtr;
    int                    errorCode;

    if (Tcl_GetIntFromObj(interp,objv,&errorCode)) {
        resultPtr=Tcl_ObjPrintf("Invalid error code: %s",Tcl_GetString(objv));
        Tcl_SetObjResult(interp,resultPtr);
        return 1;
    }
    switch(type) {
        case 0:
            resultPtr=Tcl_NewStringObj(curl_easy_strerror(errorCode),-1);
            break;
        case 1:
            resultPtr=Tcl_NewStringObj(curl_share_strerror(errorCode),-1);
            break;
        case 2:
            resultPtr=Tcl_NewStringObj(curl_multi_strerror(errorCode),-1);
            break;
        default:
            resultPtr=Tcl_NewStringObj("You're kidding,right?",-1);
    }
    Tcl_SetObjResult(interp,resultPtr);

    return 0;
}

/*
 *----------------------------------------------------------------------
 *
 * curlEasyStringError --
 *
 *  This function is invoked to process the "curl::easystrerror" Tcl command.
 *  It will return a string with an explanation of the error code given.
 *
 * Results:
 *  A standard Tcl result.
 *
 * Side effects:
 *  The interpreter will contain as a result the string with the error
 *  message.
 *
 *----------------------------------------------------------------------
 */
int
curlEasyStringError (ClientData clientData, Tcl_Interp *interp, 
                     int objc,Tcl_Obj *const objv[]) {

    if (objc<2) {
        Tcl_WrongNumArgs(interp,1,objv,"errorCode");
        return TCL_ERROR;
    }

    if (curlErrorStrings(interp,objv[1],0)) {
        return TCL_ERROR;
    }
    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * curlShareStringError --
 *
 *  This function is invoked to process the "curl::sharestrerror" Tcl command.
 *  It will return a string with an explanation of the error code given.
 *
 * Results:
 *  A standard Tcl result.
 *
 * Side effects:
 *  The interpreter will contain as a result the string with the error
 *  message.
 *
 *----------------------------------------------------------------------
 */
int
curlShareStringError (ClientData clientData, Tcl_Interp *interp,
        int objc,Tcl_Obj *const objv[]) {

    if (objc<2) {
        Tcl_WrongNumArgs(interp,1,objv,"errorCode");
        return TCL_ERROR;
    }

    if (curlErrorStrings(interp,objv[1],1)) {
        return TCL_ERROR;
    }
    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * curlMultiStringError --
 *
 *  This function is invoked to process the "curl::multirerror" Tcl command.
 *  It will return a string with an explanation of the error code given.
 *
 * Results:
 *  A standard Tcl result.
 *
 * Side effects:
 *  The interpreter will contain as a result the string with the error
 *  message.
 *
 *----------------------------------------------------------------------
 */
int
curlMultiStringError (ClientData clientData, Tcl_Interp *interp,
        int objc,Tcl_Obj *const objv[]) {

    if (objc<2) {
        Tcl_WrongNumArgs(interp,1,objv,"errorCode");
        return TCL_ERROR;
    }

    if (curlErrorStrings(interp,objv[1],2)) {
        return TCL_ERROR;
    }
    return TCL_OK;
}
