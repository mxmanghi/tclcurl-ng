/*
 * curl_setopts.c --
 *
 * Central CLI option management. 
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
 * Portions of this file were drafted or revised with AI assistance
 * under the direction and review of Massimo Manghi.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "tclcurl.h"
#include "curl_setopts.h"

#ifndef TCL_UNUSED
#define TCL_UNUSED(x) (void)(x)
#endif

/*
 * The optionTable array is built straight from the X-macro
 * table data defined in curl_setopts.h
 */

const static char *optionTable[] = {
#define TCLCURLOPT_OPTION_ENTRY(option, optname, configname, handler, curlopt, message) optname,
    TCLCURL_OPTION_LIST(TCLCURLOPT_OPTION_ENTRY)
#undef TCLCURLOPT_OPTION_ENTRY
    (char *)NULL
};

const static char *gssapidelegation[] = {
    "flag", "policyflag", (char *) NULL
};

const static char *tlsauth[] = {
    "none", "srp", (char *)NULL
};

const static char *postredir[] = {
    "301", "302", "all", (char *)NULL
};

const static char *sshauthtypes[] = {
    "publickey", "password", "host", "keyboard", "any", (char *)NULL
};

const static char *ftpfilemethod[] = {
    "default", "multicwd", "nocwd", "singlecwd", (char *)NULL
};

const static char *sslversion[] = {
    "default", "tlsv1", "sslv2", "sslv3", "tlsv1_0", "tlsv1_1", "tlsv1_2", "tlsv1_3",
    "maxdefault", "maxtlsv1_0", "maxtlsv1_1", "maxtlsv1_2", "maxtlsv1_3", (char *)NULL
};

const static char *ftpsslauth[] = {
    "default", "ssl", "tls", (char *)NULL
};

const static char *ipresolve[] = {
    "whatever", "v4", "v6", (char *)NULL
};

const static char *httpAuthMethods[] = {
    "basic", "digest", "digestie", "gssnegotiate", "ntlm", "any", "anysafe", "ntlmwb", (char *)NULL
};

const static char *proxyTypeTable[] = {
    "http", "http1.0", "socks4", "socks4a", "socks5", "socks5h", (char *)NULL
};

const static char *encodingTable[] = {
    "identity", "deflated", "all", (char *)NULL
};

const static char *netrcTable[] = {
    "optional", "ignored", "required", (char *)NULL
};

CONST static char   *httpVersionTable[] = {
    "none",     /* CURL_HTTP_VERSION_NONE */
    "1.0",      /* CURL_HTTP_VERSION_1_0  */
    "1.1",      /* CURL_HTTP_VERSION_1_1  */
    "2.0",      /* CURL_HTTP_VERSION_2_0  */
    "2TLS",     /* CURL_HTTP_VERSION_2TLS */
    "2_PRIOR_KNOWLEDGE",  /* CURL_HTTP_VERSION_2_PRIOR_KNOWLEDGE */
    (char *)NULL
};

const static char *timeCond[] = {
    "ifmodsince", "ifunmodsince",
    (char *)NULL
};

/*
 *----------------------------------------------------------------------
 *
 * curlSetOptsTransfer --
 *
 *  This procedure is invoked when the user invokes the 'setopt'
 *  command, it is used to set the 'curl' options 
 *
 *  Parameter:
 *    interp: Pointer to the interpreter we are using.
 *    curlHandle: the curl handle for which the option is set.
 *    objc and objv: The usual in Tcl.
 *
 * Results:
 *  A standard Tcl result.
 *----------------------------------------------------------------------
 */
int
curlSetOptsTransfer(Tcl_Interp *interp,struct curlObjData *curlData,int objc,Tcl_Obj *const objv[]) {
    int optionTableIndex;

    if (Tcl_GetIndexFromObj(interp,objv[2],optionTable,"option",TCL_EXACT,&optionTableIndex)==TCL_ERROR) {
        return TCL_ERROR;
    }

    return  TclCurl_SetOpts(interp,curlData,objv[3],optionTableIndex);
}

/*
 *----------------------------------------------------------------------
 *
 * curlConfigTransfer --
 *
 *  This procedure is invoked by the user command 'configure', it reads 
 *  the options passed by the user to configure a transfer, and passes
 *  then, one by one to 'curlSetOpts'.
 *
 *  Parameter:
 *      interp: Pointer to the interpreter we are using.
 *      curlHandle: the curl handle for which the option is set.
 *      objc and objv: The usual in Tcl.
 *
 * Results:
 *  A standard Tcl result.
 *----------------------------------------------------------------------
 */
int
curlConfigTransfer(Tcl_Interp *interp, struct curlObjData *curlData,
                   int objc, Tcl_Obj *const objv[]) {

    int              configOptionIndex;
    int              i,j;
    Tcl_Obj         *resultPtr;

    for(i=2,j=3;i<objc;i=i+2,j=j+2) {
        if (Tcl_GetIndexFromObj(interp, objv[i],
                                configTable, "option", TCL_EXACT, &configOptionIndex) == TCL_ERROR) {
            return TCL_ERROR;
        }
        if (i == objc-1) {
            resultPtr=Tcl_ObjPrintf("Empty value for %s",configTable[configOptionIndex]);
            Tcl_SetObjResult(interp,resultPtr);            
            return TCL_ERROR;
        }
        if (TclCurl_SetOpts(interp,curlData,objv[j],configOptionIndex)==TCL_ERROR) {
            return TCL_ERROR;
        }
    }
    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * SetoptInt --
 *
 *   Sets the curl options that require an int
 *
 * Parameters:
 *   interp: The interpreter we are working with.
 *   curlHandle: and the curl handle
 *   opt: the option to set
 *   tclObj: The Tcl with the value for the option.
 *
 * Results:
 *  0 if all went well.
 *  1 in case of error.
 *
 *----------------------------------------------------------------------
 */
static int
SetoptInt(Tcl_Interp *interp,CURL *curlHandle,CURLoption opt,
        int configOptionIndex,Tcl_Obj *tclObj) {
    int intNumber;

    if (Tcl_GetIntFromObj(interp,tclObj,&intNumber)) {
        curlErrorSetOpt(interp,configTable,configOptionIndex,Tcl_GetString(tclObj));
        return 1;
    }
    if (curl_easy_setopt(curlHandle,opt,intNumber)) {
        curlErrorSetOpt(interp,configTable,configOptionIndex,Tcl_GetString(tclObj));
        return 1;
    }
    return 0;
}

/*
 *----------------------------------------------------------------------
 *
 * SetoptLong --
 *
 *  Set the curl options that require a long
 *
 * Parameters:
 *  interp: The interpreter we are working with.
 *  curlHandle: and the curl handle
 *  opt: the option to set
 *  tclObj: The Tcl with the value for the option.
 *
 * Results:
 *  0 if all went well.
 *  1 in case of error.
 *
 *----------------------------------------------------------------------
 */
static int
SetoptLong(Tcl_Interp *interp,CURL *curlHandle,CURLoption opt,int configOptionIndex,Tcl_Obj *tclObj) {
    long longNumber;

    if (Tcl_GetLongFromObj(interp,tclObj,&longNumber)) {
        curlErrorSetOpt(interp,configTable,configOptionIndex,Tcl_GetString(tclObj));
        return 1;
    }
    if (curl_easy_setopt(curlHandle,opt,longNumber)) {
        curlErrorSetOpt(interp,configTable,configOptionIndex,Tcl_GetString(tclObj));
        return 1;
    }

    return 0;
}

/*
 *----------------------------------------------------------------------
 *
 * curlSetoptCurlOffT --
 *
 *  Set the curl options that require a curl_off_t
 *
 * Parameters:
 *  interp: The interpreter we are working with.
 *  curlHandle: and the curl handle
 *  opt: the option to set
 *  tclObj: The Tcl with the value for the option.
 *
 * Results:
 *  0 if all went well.
 *  1 in case of error.
 *
 *----------------------------------------------------------------------
 */
int
SetoptCurlOffT(Tcl_Interp *interp,CURL *curlHandle,CURLoption opt,
        int configOptionIndex,Tcl_Obj *tclObj) {
    Tcl_WideInt wideNumber;

    if (Tcl_GetWideIntFromObj(interp,tclObj,&wideNumber)) {
        curlErrorSetOpt(interp,configTable,configOptionIndex,Tcl_GetString(tclObj));
        return 1;
    }

    if (curl_easy_setopt(curlHandle,opt,(curl_off_t)wideNumber)) {
        curlErrorSetOpt(interp,configTable,configOptionIndex,Tcl_GetString(tclObj));
        return 1;
    }

    return 0;
}

/*
 *----------------------------------------------------------------------
 *
 * SetoptChar --
 *
 *  Set the curl options that require a string
 *
 * Parameters:
 *  interp: The interpreter we are working with.
 *  curlHandle: and the curl handle
 *  opt: the option to set
 *  tclObj: The Tcl with the value for the option.
 *
 * Results:
 *  0 if all went well.
 *  1 in case of error.
 *
 *----------------------------------------------------------------------
 */
static int
SetoptChar(Tcl_Interp *interp,CURL *curlHandle,
           CURLoption opt,int configOptionIndex,Tcl_Obj *tclObj) {
    char *optionPtr;

    optionPtr=curlstrdup(Tcl_GetString(tclObj));
    if (curl_easy_setopt(curlHandle,opt,optionPtr)) {
        curlErrorSetOpt(interp,configTable,configOptionIndex,optionPtr);
        Tcl_Free(optionPtr);
        return 1;
    }
    Tcl_Free(optionPtr);
    return 0;
}

/*
 *----------------------------------------------------------------------
 *
 * SetoptBlob --
 *
 *	Set the curl options that require a blob
 *
 * Parameters:
 *	interp: The interpreter we are working with.
 *	curlHandle: and the curl handle
 *	opt: the option to set
 *	tclObj: The Tcl with the value for the option.
 *
 * Results:
 *	0 if all went well.
 *	1 in case of error.
 *
 *----------------------------------------------------------------------
 */
static int
SetoptBlob(Tcl_Interp *interp,CURL *curlHandle,
        CURLoption opt,int configOptionIndex,Tcl_Obj *tclObj) {
    struct curl_blob   optionBlob;
    Tcl_Size           len;

    optionBlob.data = Tcl_GetByteArrayFromObj(tclObj,&len);
    if (optionBlob.data) {
        optionBlob.len = len;
        optionBlob.flags = CURL_BLOB_COPY;
        if (curl_easy_setopt(curlHandle,opt,&optionBlob)) {
            curlErrorSetOpt(interp,configTable,configOptionIndex,"...");
            return 1;
        }
    }
    return 0;
}

/*
 *----------------------------------------------------------------------
 *
 * SetoptSHandle --
 *
 *  Set the curl options that require a share handle (there is only
 *  one but you never know.
 *
 * Parameters:
 *  interp: The interpreter we are working with.
 *  curlHandle: the curl handle
 *  opt: the option to set
 *  tclObj: The Tcl with the value for the option.
 *
 * Results:
 *  0 if all went well.
 *  1 in case of error.
 *
 *----------------------------------------------------------------------
 */
int
SetoptSHandle(Tcl_Interp *interp,struct curlObjData *curlData,
        CURLoption opt,int configOptionIndex,Tcl_Obj *tclObj) {
    char                    *shandleName;
    Tcl_CmdInfo             *infoPtr=(Tcl_CmdInfo *)Tcl_Alloc(sizeof(Tcl_CmdInfo));
    struct shcurlObjData    *shandleDataPtr;
    CURL                    *curlHandle = curlData->curl;

    shandleName=Tcl_GetString(tclObj);
    if (Tcl_GetCommandInfo(interp,shandleName,infoPtr) == 0) {
        Tcl_Free((char *)infoPtr);
        return 1;
    }
    shandleDataPtr=(struct shcurlObjData *)(infoPtr->objClientData);
    if (curl_easy_setopt(curlHandle,opt,shandleDataPtr->shandle)) {
        Tcl_Free((char *)infoPtr);
        curlErrorSetOpt(interp,configTable,configOptionIndex,shandleName);
        return 1;
    }
    Tcl_Free((char *)infoPtr);

    if (curlData->shareToken != shandleDataPtr->token) {
        curlDetachShareHandle(curlData);
        curlData->shareToken = shandleDataPtr->token;
        curlData->nextSharedHandle = shandleDataPtr->users;
        shandleDataPtr->users = curlData;
    }

    return 0;
}

/*
 *----------------------------------------------------------------------
 *
 * TclCurl option dispatch --
 *
 *  These procedures take care of setting the transfer options.
 *
 * Results:
 *  A standard Tcl result.
 *----------------------------------------------------------------------
 */

#define TCLCURL_OPT_HANDLER(handler) handler(TclCurlOptsArgs *args);

static int TCLCURL_OPT_HANDLER(TclCurl_HandleSetoptChar)
static int TCLCURL_OPT_HANDLER(TclCurl_HandleSetoptInt)
static int TCLCURL_OPT_HANDLER(TclCurl_HandleSetoptLong)
static int TCLCURL_OPT_HANDLER(TclCurl_HandleSetoptCurlOffT)
static int TCLCURL_OPT_HANDLER(TclCurl_HandlePostFields)
static int TCLCURL_OPT_HANDLER(TclCurl_HandlePostFieldSize)
static int TCLCURL_OPT_HANDLER(TclCurl_HandleSetoptSHandle)
static int TCLCURL_OPT_HANDLER(TclCurl_HandleOutFile)
static int TCLCURL_OPT_HANDLER(TclCurl_HandleReadData)
static int TCLCURL_OPT_HANDLER(TclCurl_HandleNetrc)
static int TCLCURL_OPT_HANDLER(TclCurl_HandleTransferText)
static int TCLCURL_OPT_HANDLER(TclCurl_HandleMute)
static int TCLCURL_OPT_HANDLER(TclCurl_HandleErrorBuffer)
static int TCLCURL_OPT_HANDLER(TclCurl_HandleHttpHeaderList)
static int TCLCURL_OPT_HANDLER(TclCurl_HandleSslVersion)
static int TCLCURL_OPT_HANDLER(TclCurl_HandleQuoteList)
static int TCLCURL_OPT_HANDLER(TclCurl_HandlePostQuoteList)
static int TCLCURL_OPT_HANDLER(TclCurl_HandleWriteHeader)
static int TCLCURL_OPT_HANDLER(TclCurl_HandleTimeCondition)
static int TCLCURL_OPT_HANDLER(TclCurl_HandleStderrFile)
static int TCLCURL_OPT_HANDLER(TclCurl_HandleHeaderVar)
static int TCLCURL_OPT_HANDLER(TclCurl_HandleBodyVar)
static int TCLCURL_OPT_HANDLER(TclCurl_HandleProgressProc)
static int TCLCURL_OPT_HANDLER(TclCurl_HandleCancelTransVarName)
static int TCLCURL_OPT_HANDLER(TclCurl_HandleWriteProc)
static int TCLCURL_OPT_HANDLER(TclCurl_HandleReadProc)
static int TCLCURL_OPT_HANDLER(TclCurl_HandleHttpVersion)
static int TCLCURL_OPT_HANDLER(TclCurl_HandlePrequoteList)
static int TCLCURL_OPT_HANDLER(TclCurl_HandleDebugProc)
static int TCLCURL_OPT_HANDLER(TclCurl_HandleEncoding)
static int TCLCURL_OPT_HANDLER(TclCurl_HandleProxyType)
static int TCLCURL_OPT_HANDLER(TclCurl_HandleHttp200Aliases)
static int TCLCURL_OPT_HANDLER(TclCurl_HandleCommand)
static int TCLCURL_OPT_HANDLER(TclCurl_HandleHttpAuth)
static int TCLCURL_OPT_HANDLER(TclCurl_HandleProxyAuth)
static int TCLCURL_OPT_HANDLER(TclCurl_HandleIpResolve)
static int TCLCURL_OPT_HANDLER(TclCurl_HandleFtpSsl)
static int TCLCURL_OPT_HANDLER(TclCurl_HandleObsolete)
static int TCLCURL_OPT_HANDLER(TclCurl_HandleFtpSslAuth)
static int TCLCURL_OPT_HANDLER(TclCurl_HandleFtpFileMethod)
static int TCLCURL_OPT_HANDLER(TclCurl_HandleSshAuthTypes)
static int TCLCURL_OPT_HANDLER(TclCurl_HandlePostRedir)
static int TCLCURL_OPT_HANDLER(TclCurl_HandleProtocolMask)
static int TCLCURL_OPT_HANDLER(TclCurl_HandleFtpSslCcc)
static int TCLCURL_OPT_HANDLER(TclCurl_HandleSshKeyFunction)
static int TCLCURL_OPT_HANDLER(TclCurl_HandleMailRcpt)
static int TCLCURL_OPT_HANDLER(TclCurl_HandleChunkBgnProc)
static int TCLCURL_OPT_HANDLER(TclCurl_HandleChunkBgnVar)
static int TCLCURL_OPT_HANDLER(TclCurl_HandleChunkEndProc)
static int TCLCURL_OPT_HANDLER(TclCurl_HandleFnmatchProc)
static int TCLCURL_OPT_HANDLER(TclCurl_HandleResolveList)
static int TCLCURL_OPT_HANDLER(TclCurl_HandleTlsAuthType)
static int TCLCURL_OPT_HANDLER(TclCurl_HandleGssApiDelegation)
static int TCLCURL_OPT_HANDLER(TclCurl_HandleTelnetOptions)
static int TCLCURL_OPT_HANDLER(TclCurl_HandleCainfoBlob)
       int TCLCURL_OPT_HANDLER(TclCurl_HandleHttpPost)

#undef TCLCURL_OPT_HANDLER

static const TclCurlOptionDef curlOptionDefs[] = {
#define TCLCURLOPT_DEF_ENTRY(option, optname, configname, handler, curlopt, message) \
    [option] = { optname, configname, handler, curlopt, message },
    TCLCURL_OPTION_LIST(TCLCURLOPT_DEF_ENTRY)
#undef TCLCURLOPT_DEF_ENTRY
};

/*
 *----------------------------------------------------------------------
 *
 * TclCurl_SetOpts --
 *
 *  This procedure takes care of setting the transfer options.
 *
 * Parameter:
 *    interp: Pointer to the interpreter we are using.
 *    curlHandle: the curl handle for which the option is set.
 *    objv: A pointer to the object where the data to set is stored.
 *    curlOptsIndex: The index of the option in the TclCurl option table.
 *
 * Results:
 *  A standard Tcl result.
 *----------------------------------------------------------------------
 */

int
TclCurl_SetOpts(Tcl_Interp *interp, struct curlObjData *curlData,
                Tcl_Obj *const objv, int curlOptsIndex)
{
    if (curlOptsIndex < 0 || curlOptsIndex >= TCLCURLOPT_COUNT) {
        return TCL_ERROR;
    }
    TclCurlOptsArgs args = {
        .interp        = interp,
        .curlData      = curlData,
        .objv          = objv,
        .curlOptsIndex = curlOptsIndex,
        .def           = &curlOptionDefs[curlOptsIndex]
    };

    return curlOptionDefs[curlOptsIndex].handler(&args);
}

static int
TclCurl_HandleSetoptChar(TclCurlOptsArgs *args)
{
    CURL *curlHandle = args->curlData->curl;

    if (SetoptChar(args->interp, curlHandle, args->def->curlOpt, args->curlOptsIndex, args->objv)) {
        return TCL_ERROR;
    }
    return TCL_OK;
}

static int
TclCurl_HandleSetoptInt(TclCurlOptsArgs *args)
{
    CURL *curlHandle = args->curlData->curl;

    if (SetoptInt(args->interp, curlHandle, args->def->curlOpt, args->curlOptsIndex, args->objv)) {
        return TCL_ERROR;
    }
    return TCL_OK;
}

static int
TclCurl_HandleSetoptLong(TclCurlOptsArgs *args)
{
    CURL *curlHandle = args->curlData->curl;

    if (SetoptLong(args->interp,curlHandle,args->def->curlOpt,args->curlOptsIndex,args->objv)) {
        return TCL_ERROR;
    }
    return TCL_OK;
}

static int
TclCurl_HandleSetoptCurlOffT(TclCurlOptsArgs *args)
{
    CURL *curlHandle = args->curlData->curl;

    if (SetoptCurlOffT(args->interp,curlHandle,args->def->curlOpt,args->curlOptsIndex,args->objv)) {
        return TCL_ERROR;
    }
    return TCL_OK;
}

static int
TclCurl_HandlePostFields(TclCurlOptsArgs *args)
{
    Tcl_Free(args->curlData->postFields);
    args->curlData->postFields = curlstrdup(Tcl_GetString(args->objv));
    return TCL_OK;
}

static int
TclCurl_HandlePostFieldSize(TclCurlOptsArgs *args)
{
    Tcl_WideInt wideNumber;

    if (Tcl_GetWideIntFromObj(args->interp,args->objv,&wideNumber)) {
        curlErrorSetOpt(args->interp,configTable,args->curlOptsIndex,Tcl_GetString(args->objv));
        return TCL_ERROR;
    }

    args->curlData->postFieldSize = (curl_off_t) wideNumber;
    return TCL_OK;
}

static int
TclCurl_HandleSetoptSHandle(TclCurlOptsArgs *args)
{
    if (SetoptSHandle(args->interp, args->curlData, args->def->curlOpt, args->curlOptsIndex, args->objv)) {
        return TCL_ERROR;
    }
    return TCL_OK;
}

static int
TclCurl_HandleOutFile(TclCurlOptsArgs *args)
{
    CURL *curlHandle = args->curlData->curl;

    Tcl_Free(args->curlData->outFile);
    args->curlData->outFile=curlstrdup(Tcl_GetString(args->objv));
    if (args->curlData->outFlag) {
        if (args->curlData->outHandle!=NULL) {
            fclose(args->curlData->outHandle);
            args->curlData->outHandle=NULL;
        }
    }
    if ((strcmp(args->curlData->outFile,""))&&(strcmp(args->curlData->outFile,"stdout"))) {
        args->curlData->outFlag=1;
    } else {
        args->curlData->outFlag=0;
        curl_easy_setopt(curlHandle,CURLOPT_WRITEDATA,stdout);
        args->curlData->outFile=NULL;
    }
    curl_easy_setopt(curlHandle,CURLOPT_WRITEFUNCTION,NULL);
    return TCL_OK;
}

static int
TclCurl_HandleReadData(TclCurlOptsArgs *args)
{
    CURL *curlHandle = args->curlData->curl;

    Tcl_Free(args->curlData->inFile);
    args->curlData->inFile=curlstrdup(Tcl_GetString(args->objv));
    if (args->curlData->inFlag) {
        if (args->curlData->inHandle!=NULL) {
            fclose(args->curlData->inHandle);
            args->curlData->inHandle=NULL;
        }
    }
    if ((strcmp(args->curlData->inFile,""))&&(strcmp(args->curlData->inFile,"stdin"))) {
        args->curlData->inFlag=1;
    } else {
        curl_easy_setopt(curlHandle,CURLOPT_READDATA,stdin);
        args->curlData->inFlag=0;
        args->curlData->inFile=NULL;
    }
    curl_easy_setopt(curlHandle,CURLOPT_READFUNCTION,NULL);
    return TCL_OK;
}

static int
TclCurl_HandleNetrc(TclCurlOptsArgs *args)
{
    CURL *curlHandle = args->curlData->curl;
    int  netrcIndex;

    if (Tcl_GetIndexFromObj(args->interp, args->objv, netrcTable,
            "netrc option",TCL_EXACT,&netrcIndex)==TCL_ERROR) {
        return TCL_ERROR;
    }
    if (curl_easy_setopt(curlHandle,CURLOPT_NETRC,netrcIndex)) {
        curlErrorSetOpt(args->interp,configTable,args->curlOptsIndex,netrcTable[netrcIndex]);
        return 1;
    }
    return TCL_OK;
}

static int
TclCurl_HandleTransferText(TclCurlOptsArgs *args)
{
    CURL *curlHandle = args->curlData->curl;

    if (SetoptInt(args->interp, curlHandle, args->def->curlOpt, args->curlOptsIndex, args->objv)) {
        return TCL_ERROR;
    }
    Tcl_GetIntFromObj(args->interp,args->objv,&args->curlData->transferText);
    return TCL_OK;
}

static int
TclCurl_HandleMute(TclCurlOptsArgs *args)
{
    TCL_UNUSED(args);
    return TCL_OK;
}

static int
TclCurl_HandleErrorBuffer(TclCurlOptsArgs *args)
{
    if (TclCurl_ErrorBuffer(args->interp,args->curlData,args->objv) == TCL_ERROR) {
        return TCL_ERROR;
    }
    return TCL_OK;
}

static int
TclCurl_HandleHttpHeaderList(TclCurlOptsArgs *args)
{
    CURL *curlHandle = args->curlData->curl;

    if(SetoptsList(args->interp,&args->curlData->headerList,args->objv)) {
        curlErrorSetOpt(args->interp,configTable,args->curlOptsIndex,"Header list invalid");
        return TCL_ERROR;
    }
    if (curl_easy_setopt(curlHandle,CURLOPT_HTTPHEADER,args->curlData->headerList)) {
        curl_slist_free_all(args->curlData->headerList);
        args->curlData->headerList=NULL;
        return TCL_ERROR;
    }
    return TCL_OK;
}

static int
TclCurl_HandleSslVersion(TclCurlOptsArgs *args)
{
    CURL *curlHandle = args->curlData->curl;
    Tcl_Obj *tmpObjPtr;
    long longNumber=0;
    int  sslVersionIndex;

    if (Tcl_GetIndexFromObj(args->interp, args->objv, sslversion,
        "sslversion ",TCL_EXACT,&sslVersionIndex)==TCL_ERROR) {
        return TCL_ERROR;
    }
    switch(sslVersionIndex) {
        case 0:
            longNumber=CURL_SSLVERSION_DEFAULT;
            break;
        case 1:
            longNumber=CURL_SSLVERSION_TLSv1;
            break;
        case 2:
            longNumber=CURL_SSLVERSION_SSLv2;
            break;
        case 3:
            longNumber=CURL_SSLVERSION_SSLv3;
            break;
        case 4:
            longNumber=CURL_SSLVERSION_TLSv1_0;
            break;
        case 5:
            longNumber=CURL_SSLVERSION_TLSv1_1;
            break;
        case 6:
            longNumber=CURL_SSLVERSION_TLSv1_2;
            break;
        case 7:
            longNumber=CURL_SSLVERSION_TLSv1_3;
            break;
        case 8:
            longNumber=CURL_SSLVERSION_MAX_DEFAULT;
            break;
        case 9:
            longNumber=CURL_SSLVERSION_MAX_TLSv1_0;
            break;
        case 10:
            longNumber=CURL_SSLVERSION_MAX_TLSv1_1;
            break;
        case 11:
            longNumber=CURL_SSLVERSION_MAX_TLSv1_2;
            break;
        case 12:
            longNumber=CURL_SSLVERSION_MAX_TLSv1_3;
    }
    tmpObjPtr=Tcl_NewLongObj(longNumber);
    Tcl_IncrRefCount(tmpObjPtr);
    if (SetoptLong(args->interp,curlHandle,CURLOPT_SSLVERSION,
                args->curlOptsIndex,tmpObjPtr)) {
        Tcl_DecrRefCount(tmpObjPtr);
        return TCL_ERROR;
    }
    Tcl_DecrRefCount(tmpObjPtr);
    return TCL_OK;
}

static int
TclCurl_HandleQuoteList(TclCurlOptsArgs *args)
{
    CURL *curlHandle = args->curlData->curl;

    if(SetoptsList(args->interp,&args->curlData->quote,args->objv)) {
        curlErrorSetOpt(args->interp,configTable,args->curlOptsIndex,"quote list invalid");
        return TCL_ERROR;
    }
    if (curl_easy_setopt(curlHandle,CURLOPT_QUOTE,args->curlData->quote)) {
        curlErrorSetOpt(args->interp,configTable,args->curlOptsIndex,"quote list invalid");
        curl_slist_free_all(args->curlData->quote);
        args->curlData->quote=NULL;
        return TCL_ERROR;
    }
    return TCL_OK;
}

static int
TclCurl_HandlePostQuoteList(TclCurlOptsArgs *args)
{
    CURL *curlHandle = args->curlData->curl;

    if(SetoptsList(args->interp,&args->curlData->postquote,args->objv)) {
        curlErrorSetOpt(args->interp,configTable,args->curlOptsIndex,"postquote invalid");
        return TCL_ERROR;
    }
    if (curl_easy_setopt(curlHandle,CURLOPT_POSTQUOTE,args->curlData->postquote)) {
        curlErrorSetOpt(args->interp,configTable,args->curlOptsIndex,"postquote invalid");
        curl_slist_free_all(args->curlData->postquote);
        args->curlData->postquote=NULL;
        return TCL_ERROR;
    }
    return TCL_OK;
}

static int
TclCurl_HandleWriteHeader(TclCurlOptsArgs *args)
{
    CURL *curlHandle = args->curlData->curl;

    Tcl_Free(args->curlData->headerFile);
    args->curlData->headerFile=curlstrdup(Tcl_GetString(args->objv));
    if (args->curlData->headerFlag) {
        if (args->curlData->headerHandle!=NULL) {
            fclose(args->curlData->headerHandle);
            args->curlData->headerHandle=NULL;
        }
        curl_easy_setopt(curlHandle,CURLOPT_HEADERDATA,NULL);
    }
    if ((strcmp(args->curlData->headerFile,""))&&(strcmp(args->curlData->headerFile,"stdout"))
            &&(strcmp(args->curlData->headerFile,"stderr"))) {
        args->curlData->headerFlag=1;
    } else {
        if ((strcmp(args->curlData->headerFile,"stdout"))) {
            curl_easy_setopt(curlHandle,CURLOPT_HEADERDATA,stderr);
        } else {
            curl_easy_setopt(curlHandle,CURLOPT_HEADERDATA,stdout);
        }
        args->curlData->headerFlag=0;
        args->curlData->headerFile=NULL;
    }
    return TCL_OK;
}

static int
TclCurl_HandleTimeCondition(TclCurlOptsArgs *args)
{
    CURL *curlHandle = args->curlData->curl;
    long longNumber=0;
    int  timeConditionIndex;

    if (Tcl_GetIndexFromObj(args->interp, args->objv, timeCond,
        "time cond option",TCL_EXACT, &timeConditionIndex)==TCL_ERROR) {
        return TCL_ERROR;
    }
    if (timeConditionIndex==0) {
        longNumber=CURL_TIMECOND_IFMODSINCE;
    } else {
        longNumber=CURL_TIMECOND_IFUNMODSINCE;
    }
    if (curl_easy_setopt(curlHandle,CURLOPT_TIMECONDITION,longNumber)) {
        return TCL_ERROR;
    }
    return TCL_OK;
}

static int
TclCurl_HandleStderrFile(TclCurlOptsArgs *args)
{
    CURL *curlHandle = args->curlData->curl;

    Tcl_Free(args->curlData->stderrFile);
    args->curlData->stderrFile=curlstrdup(Tcl_GetString(args->objv));
    if ((strcmp(args->curlData->stderrFile,""))&&(strcmp(args->curlData->stderrFile,"stdout"))
            &&(strcmp(args->curlData->stderrFile,"stderr"))) {
        args->curlData->stderrFlag=1;
    } else {
        args->curlData->stderrFlag=0;
        if (strcmp(args->curlData->stderrFile,"stdout")) {
            curl_easy_setopt(curlHandle,CURLOPT_STDERR,stderr);
        } else {
            curl_easy_setopt(curlHandle,CURLOPT_STDERR,stdout);
        }
        args->curlData->stderrFile=NULL;
    }
    return TCL_OK;
}

static int
TclCurl_HandleHeaderVar(TclCurlOptsArgs *args)
{
    CURL *curlHandle = args->curlData->curl;

    if (args->curlData->headerFlag) {
        if (args->curlData->headerHandle!=NULL) {
            fclose(args->curlData->headerHandle);
            args->curlData->headerHandle=NULL;
        }
        curl_easy_setopt(curlHandle,CURLOPT_HEADERDATA,NULL);
        args->curlData->headerFlag=0;
    }
    if (curl_easy_setopt(curlHandle,CURLOPT_HEADERFUNCTION,
            curlHeaderReader)) {
        return TCL_ERROR;
    }
    Tcl_Free(args->curlData->headerVar);
    args->curlData->headerVar=curlstrdup(Tcl_GetString(args->objv));
    if (curl_easy_setopt(curlHandle,CURLOPT_HEADERDATA,
            (FILE *)args->curlData)) {
        return TCL_ERROR;
    }
    return TCL_OK;
}

static int
TclCurl_HandleBodyVar(TclCurlOptsArgs *args)
{
    CURL *curlHandle = args->curlData->curl;

    Tcl_Free(args->curlData->bodyVarName);
    args->curlData->bodyVarName=curlstrdup(Tcl_GetString(args->objv));
    if (args->curlData->outFlag) {
        if (args->curlData->outHandle!=NULL) {
            fclose(args->curlData->outHandle);
            args->curlData->outHandle=NULL;
        }
        curl_easy_setopt(curlHandle,CURLOPT_WRITEDATA,NULL);
    }
    args->curlData->outFlag=0;
    if (curl_easy_setopt(curlHandle,CURLOPT_WRITEFUNCTION,
            curlBodyReader)) {
        return TCL_ERROR;
    }
    if (curl_easy_setopt(curlHandle,CURLOPT_WRITEDATA,args->curlData)) {
        return TCL_ERROR;
    }
    return TCL_OK;
}

static int
TclCurl_HandleProgressProc(TclCurlOptsArgs *args)
{
    CURL *curlHandle = args->curlData->curl;

    Tcl_Free(args->curlData->progressProc);
    args->curlData->progressProc=curlstrdup(Tcl_GetString(args->objv));
    if (strcmp(args->curlData->progressProc,"")) {
        if (curl_easy_setopt(curlHandle,CURLOPT_XFERINFOFUNCTION,
                curlProgressCallback)) {
            return TCL_ERROR;
        }
        if (curl_easy_setopt(curlHandle,CURLOPT_XFERINFODATA,
                args->curlData)) {
            return TCL_ERROR;
        }
    } else {
        if (curl_easy_setopt(curlHandle,CURLOPT_XFERINFOFUNCTION,NULL)) {
            return TCL_ERROR;
        }
    }
    return TCL_OK;
}

static int
TclCurl_HandleCancelTransVarName(TclCurlOptsArgs *args)
{
    if (args->curlData->cancelTransVarName) {
        Tcl_UnlinkVar(args->interp,args->curlData->cancelTransVarName);
        Tcl_Free(args->curlData->cancelTransVarName);
    }
    args->curlData->cancelTransVarName=curlstrdup(Tcl_GetString(args->objv));
    Tcl_LinkVar(args->interp,args->curlData->cancelTransVarName,
            (char *)&(args->curlData->cancelTrans),TCL_LINK_INT);
    return TCL_OK;
}

static int
TclCurl_HandleWriteProc(TclCurlOptsArgs *args)
{
    CURL *curlHandle = args->curlData->curl;

    args->curlData->writeProc=curlstrdup(Tcl_GetString(args->objv));
    if (args->curlData->outFlag) {
        if (args->curlData->outHandle!=NULL) {
            fclose(args->curlData->outHandle);
            args->curlData->outHandle=NULL;
        }
        curl_easy_setopt(curlHandle,CURLOPT_WRITEDATA,NULL);
    }
    args->curlData->outFlag=0;
    if (curl_easy_setopt(curlHandle,CURLOPT_WRITEFUNCTION,
            curlWriteProcInvoke)) {
        curl_easy_setopt(curlHandle,CURLOPT_WRITEFUNCTION,NULL);
        return TCL_ERROR;
    }
    if (curl_easy_setopt(curlHandle,CURLOPT_WRITEDATA,args->curlData)) {
        curl_easy_setopt(curlHandle,CURLOPT_WRITEFUNCTION,NULL);
        return TCL_ERROR;
    }
    return TCL_OK;
}

static int
TclCurl_HandleReadProc(TclCurlOptsArgs *args)
{
    CURL *curlHandle = args->curlData->curl;

    args->curlData->readProc=curlstrdup(Tcl_GetString(args->objv));
    if (args->curlData->inFlag) {
        if (args->curlData->inHandle!=NULL) {
            fclose(args->curlData->inHandle);
            args->curlData->inHandle=NULL;
        }
        curl_easy_setopt(curlHandle,CURLOPT_READDATA,NULL);
    }
    args->curlData->inFlag=0;
    if (strcmp(args->curlData->readProc,"")) {
        if (curl_easy_setopt(curlHandle,CURLOPT_READFUNCTION,
                curlReadProcInvoke)) {
            return TCL_ERROR;
        }
    } else {
        curl_easy_setopt(curlHandle,CURLOPT_READFUNCTION,NULL);
        return TCL_OK;
    }
    if (curl_easy_setopt(curlHandle,CURLOPT_READDATA,args->curlData)) {
        return TCL_ERROR;
    }
    return TCL_OK;
}

static int
TclCurl_HandleHttpVersion(TclCurlOptsArgs *args)
{
    CURL *curlHandle = args->curlData->curl;
    char* tmpStr = NULL;
    int httpVersionIndex;

    if (Tcl_GetIndexFromObj(args->interp, args->objv, httpVersionTable,
        "http version",TCL_EXACT,&httpVersionIndex)==TCL_ERROR) {
        return TCL_ERROR;
    }
    if (curl_easy_setopt(curlHandle,CURLOPT_HTTP_VERSION,
                httpVersionIndex)) {
        tmpStr=curlstrdup(Tcl_GetString(args->objv));
        curlErrorSetOpt(args->interp,configTable,args->curlOptsIndex,tmpStr);
        Tcl_Free(tmpStr);
        return TCL_ERROR;
    }
    return TCL_OK;
}

static int
TclCurl_HandlePrequoteList(TclCurlOptsArgs *args)
{
    CURL *curlHandle = args->curlData->curl;

    if(SetoptsList(args->interp,&args->curlData->prequote,args->objv)) {
        curlErrorSetOpt(args->interp,configTable,args->curlOptsIndex,"prequote invalid");
        return TCL_ERROR;
    }
    if (curl_easy_setopt(curlHandle,CURLOPT_PREQUOTE,args->curlData->prequote)) {
        curlErrorSetOpt(args->interp,configTable,args->curlOptsIndex,"prequote invalid");
        curl_slist_free_all(args->curlData->prequote);
        args->curlData->prequote=NULL;
        return TCL_ERROR;
    }
    return TCL_OK;
}

static int
TclCurl_HandleDebugProc(TclCurlOptsArgs *args)
{
    CURL *curlHandle = args->curlData->curl;

    args->curlData->debugProc=curlstrdup(Tcl_GetString(args->objv));
    if (curl_easy_setopt(curlHandle,CURLOPT_DEBUGFUNCTION,
            curlDebugProcInvoke)) {
        return TCL_ERROR;
    }
    if (curl_easy_setopt(curlHandle,CURLOPT_DEBUGDATA,args->curlData)) {
        return TCL_ERROR;
    }
    return TCL_OK;
}

static int
TclCurl_HandleEncoding(TclCurlOptsArgs *args)
{
    CURL *curlHandle = args->curlData->curl;
    int encodingTableIndex;

    if (Tcl_GetIndexFromObj(args->interp, args->objv, encodingTable,
        "encoding",TCL_EXACT,&encodingTableIndex)==TCL_ERROR) {
        return TCL_ERROR;
    }
    if (encodingTableIndex==2) {
        if (curl_easy_setopt(curlHandle,CURLOPT_ACCEPT_ENCODING,"")) {
            curlErrorSetOpt(args->interp,configTable,args->curlOptsIndex,"all");
            return 1;
        }
    } else {
        if (SetoptChar(args->interp,curlHandle,CURLOPT_ACCEPT_ENCODING,args->curlOptsIndex,args->objv)) {
            return TCL_ERROR;
        }
    }
    return TCL_OK;
}

static int
TclCurl_HandleProxyType(TclCurlOptsArgs *args)
{
    CURL *curlHandle = args->curlData->curl;
    int proxyTypeIndex;

    if (Tcl_GetIndexFromObj(args->interp, args->objv, proxyTypeTable,
        "proxy type",TCL_EXACT,&proxyTypeIndex)==TCL_ERROR) {
        return TCL_ERROR;
    }
    switch(proxyTypeIndex) {
        case 0:
            curl_easy_setopt(curlHandle,CURLOPT_PROXYTYPE,
                    CURLPROXY_HTTP);
            break;
        case 1:
            curl_easy_setopt(curlHandle,CURLOPT_PROXYTYPE,
                    CURLPROXY_HTTP_1_0);
            break;
        case 2:
            curl_easy_setopt(curlHandle,CURLOPT_PROXYTYPE,
                    CURLPROXY_SOCKS4);
            break;
        case 3:
            curl_easy_setopt(curlHandle,CURLOPT_PROXYTYPE,
                    CURLPROXY_SOCKS4A);
            break;
        case 4:
            curl_easy_setopt(curlHandle,CURLOPT_PROXYTYPE,
                    CURLPROXY_SOCKS5);
            break;
        case 5:
            curl_easy_setopt(curlHandle,CURLOPT_PROXYTYPE,
                    CURLPROXY_SOCKS5_HOSTNAME);
            break;
    }
    return TCL_OK;
}

static int
TclCurl_HandleHttp200Aliases(TclCurlOptsArgs *args)
{
    CURL *curlHandle = args->curlData->curl;

    if(SetoptsList(args->interp,&args->curlData->http200aliases,args->objv)) {
        curlErrorSetOpt(args->interp,configTable,args->curlOptsIndex,"http200aliases invalid");
        return TCL_ERROR;
    }
    if (curl_easy_setopt(curlHandle,CURLOPT_HTTP200ALIASES,args->curlData->http200aliases)) {
        curlErrorSetOpt(args->interp,configTable,args->curlOptsIndex,"http200aliases invalid");
        curl_slist_free_all(args->curlData->http200aliases);
        args->curlData->http200aliases=NULL;
        return TCL_ERROR;
    }
    return TCL_OK;
}

static int
TclCurl_HandleCommand(TclCurlOptsArgs *args)
{
    Tcl_Free(args->curlData->command);
    args->curlData->command=curlstrdup(Tcl_GetString(args->objv));
    return TCL_OK;
}

static int
TclCurl_HandleHttpAuth(TclCurlOptsArgs *args)
{
    CURL *curlHandle = args->curlData->curl;
    Tcl_Obj *tmpObjPtr;
    long longNumber=0;
    int httpAuthMethodIndex;

    if (Tcl_GetIndexFromObj(args->interp, args->objv, httpAuthMethods,
        "authentication method",TCL_EXACT,&httpAuthMethodIndex)==TCL_ERROR) {
        return TCL_ERROR;
    }
    args->curlData->anyAuthFlag=0;
    switch(httpAuthMethodIndex) {
        case 0:
            longNumber=CURLAUTH_BASIC;
            break;
        case 1:
            longNumber=CURLAUTH_DIGEST;
            break;
        case 2:
            longNumber=CURLAUTH_DIGEST_IE;
            break;
        case 3:
            longNumber=CURLAUTH_GSSNEGOTIATE;
            break;
        case 4:
            longNumber=CURLAUTH_NTLM;
            break;
        case 5:
            longNumber=CURLAUTH_ANY;
            args->curlData->anyAuthFlag=1;
            break;
        case 6:
            longNumber=CURLAUTH_ANYSAFE;
            break;
        case 7:
            longNumber=CURLAUTH_NTLM_WB;
            break;
    }
    tmpObjPtr=Tcl_NewLongObj(longNumber);
    Tcl_IncrRefCount(tmpObjPtr);
    if (SetoptLong(args->interp,curlHandle,CURLOPT_HTTPAUTH
            ,args->curlOptsIndex,tmpObjPtr)) {
        Tcl_DecrRefCount(tmpObjPtr);
        return TCL_ERROR;
    }
    Tcl_DecrRefCount(tmpObjPtr);
    return TCL_OK;
}

static int
TclCurl_HandleProxyAuth(TclCurlOptsArgs *args)
{
    CURL *curlHandle = args->curlData->curl;
    Tcl_Obj *tmpObjPtr;
    long longNumber=0;
    int proxyAuthMethodIndex;

    if (Tcl_GetIndexFromObj(args->interp, args->objv, httpAuthMethods,
        "authentication method",TCL_EXACT,&proxyAuthMethodIndex)==TCL_ERROR) {
        return TCL_ERROR;
    }
    switch(proxyAuthMethodIndex) {
        case 0:
            longNumber=CURLAUTH_BASIC;
            break;
        case 1:
            longNumber=CURLAUTH_DIGEST;
            break;
        case 2:
            longNumber=CURLAUTH_GSSNEGOTIATE;
            break;
        case 3:
            longNumber=CURLAUTH_NTLM;
            break;
        case 5:
            longNumber=CURLAUTH_ANYSAFE;
            break;
        case 4:
        default:
            longNumber=CURLAUTH_ANY;
            break;
    }
    tmpObjPtr=Tcl_NewLongObj(longNumber);
    Tcl_IncrRefCount(tmpObjPtr);
    if (SetoptLong(args->interp,curlHandle,CURLOPT_PROXYAUTH
            ,args->curlOptsIndex,tmpObjPtr)) {
        Tcl_DecrRefCount(tmpObjPtr);
        return TCL_ERROR;
    }
    Tcl_DecrRefCount(tmpObjPtr);
    return TCL_OK;
}

static int
TclCurl_HandleIpResolve(TclCurlOptsArgs *args)
{
    CURL *curlHandle = args->curlData->curl;
    Tcl_Obj *tmpObjPtr;
    long longNumber=0;
    int ipResolveIndex;

    if (Tcl_GetIndexFromObj(args->interp, args->objv, ipresolve,
        "ip version",TCL_EXACT,&ipResolveIndex)==TCL_ERROR) {
        return TCL_ERROR;
    }
    switch(ipResolveIndex) {
        case 0:
            longNumber=CURL_IPRESOLVE_WHATEVER;
            break;
        case 1:
            longNumber=CURL_IPRESOLVE_V4;
            break;
        case 2:
            longNumber=CURL_IPRESOLVE_V6;
            break;
    }
    tmpObjPtr=Tcl_NewLongObj(longNumber);
    Tcl_IncrRefCount(tmpObjPtr);
    if (SetoptLong(args->interp,curlHandle,CURLOPT_IPRESOLVE,
                   args->curlOptsIndex,tmpObjPtr)) {
        Tcl_DecrRefCount(tmpObjPtr);
        return TCL_ERROR;
    }
    Tcl_DecrRefCount(tmpObjPtr);
    return TCL_OK;
}

static int
TclCurl_HandleFtpSsl(TclCurlOptsArgs *args)
{
    CURL *curlHandle = args->curlData->curl;
    Tcl_Obj *tmpObjPtr;
    long longNumber = TclCurl_FTPSSLMethod(args->interp,args->objv);

    if (longNumber == -1) {
        return TCL_ERROR;
    }

    tmpObjPtr=Tcl_NewLongObj(longNumber);
    Tcl_IncrRefCount(tmpObjPtr);
    if (SetoptLong(args->interp,curlHandle,CURLOPT_USE_SSL,
                args->curlOptsIndex,tmpObjPtr)) {
        Tcl_DecrRefCount(tmpObjPtr);
        return TCL_ERROR;
    }
    Tcl_DecrRefCount(tmpObjPtr);
    return TCL_OK;
}

static int
TclCurl_HandleObsolete(TclCurlOptsArgs *args)
{
    curlErrorSetOpt(args->interp,configTable,args->curlOptsIndex,
            args->def->errorMessage ? args->def->errorMessage : "option is obsolete");
    return TCL_ERROR;
}

static int
TclCurl_HandleFtpSslAuth(TclCurlOptsArgs *args)
{
    CURL *curlHandle = args->curlData->curl;
    Tcl_Obj *tmpObjPtr;
    long longNumber=0;
    int ftpSslAuthIndex;

    if (Tcl_GetIndexFromObj(args->interp, args->objv, ftpsslauth,
                            "ftpsslauth method ",TCL_EXACT,&ftpSslAuthIndex) == TCL_ERROR) {
        return TCL_ERROR;
    }
    switch(ftpSslAuthIndex) {
        case 0:
            longNumber=CURLFTPAUTH_DEFAULT;
            break;
        case 1:
            longNumber=CURLFTPAUTH_SSL;
            break;
        case 2:
            longNumber=CURLFTPAUTH_TLS;
            break;
    }
    tmpObjPtr=Tcl_NewLongObj(longNumber);
    Tcl_IncrRefCount(tmpObjPtr);
    if (SetoptLong(args->interp,curlHandle,CURLOPT_FTPSSLAUTH,
                args->curlOptsIndex,tmpObjPtr)) {
        Tcl_DecrRefCount(tmpObjPtr);
        return TCL_ERROR;
    }
    Tcl_DecrRefCount(tmpObjPtr);
    return TCL_OK;
}

static int
TclCurl_HandleFtpFileMethod(TclCurlOptsArgs *args)
{
    CURL *curlHandle = args->curlData->curl;
    Tcl_Obj *tmpObjPtr;
    long longNumber=0;
    int ftpFileMethodIndex;

    if (Tcl_GetIndexFromObj(args->interp, args->objv, ftpfilemethod,
        "ftp file method ",TCL_EXACT,&ftpFileMethodIndex)==TCL_ERROR) {
        return TCL_ERROR;
    }
    switch(ftpFileMethodIndex) {
        case 0:
        case 1:
            longNumber = 1;
            break;
        case 2:
            longNumber = 2;
            break;
        case 3:
            longNumber = 3;
            break;
    }
    tmpObjPtr=Tcl_NewLongObj(longNumber);
    Tcl_IncrRefCount(tmpObjPtr);
    if (SetoptLong(args->interp,curlHandle,CURLOPT_FTP_FILEMETHOD,
                args->curlOptsIndex,tmpObjPtr)) {
        Tcl_DecrRefCount(tmpObjPtr);
        return TCL_ERROR;
    }
    Tcl_DecrRefCount(tmpObjPtr);
    return TCL_OK;
}

static int
TclCurl_HandleSshAuthTypes(TclCurlOptsArgs *args)
{
    CURL *curlHandle = args->curlData->curl;
    Tcl_Obj *tmpObjPtr;
    long longNumber=0;
    int sshAuthTypeIndex;

    if (Tcl_GetIndexFromObj(args->interp, args->objv, sshauthtypes,
        "ssh auth type ",TCL_EXACT,&sshAuthTypeIndex)==TCL_ERROR) {
        return TCL_ERROR;
    }
    switch(sshAuthTypeIndex) {
        case 0:
            longNumber = CURLSSH_AUTH_PUBLICKEY;
            break;
        case 1:
            longNumber = CURLSSH_AUTH_PASSWORD;
            break;
        case 2:
            longNumber = CURLSSH_AUTH_HOST;
            break;
        case 3:
            longNumber = CURLSSH_AUTH_KEYBOARD;
            break;
        case 4:
            longNumber = CURLSSH_AUTH_ANY;
            break;
        case 5:
            longNumber = CURLSSH_AUTH_NONE;
            break;
        case 6:
            longNumber = CURLSSH_AUTH_AGENT;
            break;
        case 7:
            longNumber = CURLSSH_AUTH_DEFAULT;
            break;
    }
    tmpObjPtr=Tcl_NewLongObj(longNumber);
    Tcl_IncrRefCount(tmpObjPtr);
    if (SetoptLong(args->interp,curlHandle,CURLOPT_SSH_AUTH_TYPES,
                args->curlOptsIndex,tmpObjPtr)) {
        Tcl_DecrRefCount(tmpObjPtr);
        return TCL_ERROR;
    }
    Tcl_DecrRefCount(tmpObjPtr);
    return TCL_OK;
}

static int
TclCurl_HandlePostRedir(TclCurlOptsArgs *args)
{
    CURL *curlHandle = args->curlData->curl;
    Tcl_Obj *tmpObjPtr;
    long longNumber=0;
    int postRedirIndex;

    if (Tcl_GetIndexFromObj(args->interp, args->objv, postredir,
        "Postredir option ",TCL_EXACT,&postRedirIndex)==TCL_ERROR) {
        return TCL_ERROR;
    }
    switch(postRedirIndex) {
        case 0:
            longNumber=CURL_REDIR_POST_301;
            break;
        case 1:
            longNumber=CURL_REDIR_POST_302;
            break;
        case 2:
            longNumber=CURL_REDIR_POST_303;
            break;
        case 3:
            longNumber=CURL_REDIR_POST_ALL;
            break;
    }
    tmpObjPtr=Tcl_NewLongObj(longNumber);
    Tcl_IncrRefCount(tmpObjPtr);
    if (SetoptLong(args->interp,curlHandle,CURLOPT_POSTREDIR,args->curlOptsIndex,tmpObjPtr)) {
        Tcl_DecrRefCount(tmpObjPtr);
        return TCL_ERROR;
    }
    Tcl_DecrRefCount(tmpObjPtr);
    return TCL_OK;
}

static int
TclCurl_HandleProtocolMask(TclCurlOptsArgs *args)
{
    CURL *curlHandle = args->curlData->curl;
    Tcl_Obj **protocols;
    Tcl_Size protocols_c;
    Tcl_Obj *tmpObjPtr;

    if (Tcl_ListObjGetElements(args->interp,args->objv,&protocols_c,&protocols) == TCL_ERROR) {
        return TCL_ERROR;
    }

    tmpObjPtr = TclCurl_JoinList(protocols,protocols_c,",");
    Tcl_IncrRefCount(tmpObjPtr);
    curl_easy_setopt(curlHandle,CURLOPT_PROTOCOLS_STR,Tcl_GetString(tmpObjPtr));
    Tcl_DecrRefCount(tmpObjPtr);

    return TCL_OK;
}

static int
TclCurl_HandleFtpSslCcc(TclCurlOptsArgs *args)
{
    CURL *curlHandle = args->curlData->curl;
    Tcl_Obj *tmpObjPtr;
    long longNumber = TclCurl_FTPClearCommandChannelOpt(args->interp,args->objv);

    if (longNumber < 0) {
        return TCL_ERROR;
    }

    tmpObjPtr = Tcl_NewLongObj(longNumber);
    Tcl_IncrRefCount(tmpObjPtr);
    if (SetoptLong(args->interp,curlHandle,CURLOPT_FTP_SSL_CCC,args->curlOptsIndex,tmpObjPtr)) {
        Tcl_DecrRefCount(tmpObjPtr);
        return TCL_ERROR;
    }
    Tcl_DecrRefCount(tmpObjPtr);
    return TCL_OK;
}

static int
TclCurl_HandleSshKeyFunction(TclCurlOptsArgs *args)
{
    CURL *curlHandle = args->curlData->curl;

    if (curl_easy_setopt(curlHandle,CURLOPT_SSH_KEYFUNCTION,curlsshkeycallback)) {
        return TCL_ERROR;
    }
    if (curl_easy_setopt(curlHandle,CURLOPT_SSH_KEYDATA,args->curlData)) {
        return TCL_ERROR;
    }
    args->curlData->sshkeycallProc=curlstrdup(Tcl_GetString(args->objv));
    return TCL_OK;
}

static int
TclCurl_HandleMailRcpt(TclCurlOptsArgs *args)
{
    CURL *curlHandle = args->curlData->curl;

    if (SetoptsList(args->interp,&args->curlData->mailrcpt,args->objv)) {
        curlErrorSetOpt(args->interp,configTable,args->curlOptsIndex,"mailrcpt invalid");
        return TCL_ERROR;
    }
    if (curl_easy_setopt(curlHandle,CURLOPT_MAIL_RCPT,args->curlData->mailrcpt)) {
        curlErrorSetOpt(args->interp,configTable,args->curlOptsIndex,"mailrcpt invalid");
        curl_slist_free_all(args->curlData->mailrcpt);
        args->curlData->mailrcpt=NULL;
        return TCL_ERROR;
    }
    return TCL_OK;
}

static int
TclCurl_HandleChunkBgnProc(TclCurlOptsArgs *args)
{
    CURL *curlHandle = args->curlData->curl;

    args->curlData->chunkBgnProc=curlstrdup(Tcl_GetString(args->objv));
    if (strcmp(args->curlData->chunkBgnProc,"")) {
        if (curl_easy_setopt(curlHandle,CURLOPT_CHUNK_BGN_FUNCTION,
                curlChunkBgnProcInvoke)) {
            return TCL_ERROR;
        }
    } else {
        curl_easy_setopt(curlHandle,CURLOPT_CHUNK_BGN_FUNCTION,NULL);
        return TCL_OK;
    }
    if (curl_easy_setopt(curlHandle,CURLOPT_CHUNK_DATA,args->curlData)) {
        return TCL_ERROR;
    }
    return TCL_OK;
}

static int
TclCurl_HandleChunkBgnVar(TclCurlOptsArgs *args)
{
    args->curlData->chunkBgnVar=curlstrdup(Tcl_GetString(args->objv));
    if (!strcmp(args->curlData->chunkBgnVar,"")) {
        curlErrorSetOpt(args->interp,configTable,args->curlOptsIndex,"invalid var name");
        return TCL_ERROR;
    }
    return TCL_OK;
}

static int
TclCurl_HandleChunkEndProc(TclCurlOptsArgs *args)
{
    CURL *curlHandle = args->curlData->curl;

    args->curlData->chunkEndProc=curlstrdup(Tcl_GetString(args->objv));
    if (strcmp(args->curlData->chunkEndProc,"")) {
        if (curl_easy_setopt(curlHandle,CURLOPT_CHUNK_END_FUNCTION,
                curlChunkEndProcInvoke)) {
            return TCL_ERROR;
        }
    } else {
        curl_easy_setopt(curlHandle,CURLOPT_CHUNK_END_FUNCTION,NULL);
        return TCL_OK;
    }
    return TCL_OK;
}

static int
TclCurl_HandleFnmatchProc(TclCurlOptsArgs *args)
{
    CURL *curlHandle = args->curlData->curl;

    args->curlData->fnmatchProc=curlstrdup(Tcl_GetString(args->objv));
    if (strcmp(args->curlData->fnmatchProc,"")) {
        if (curl_easy_setopt(curlHandle,CURLOPT_FNMATCH_FUNCTION,
                curlfnmatchProcInvoke)) {
            return TCL_ERROR;
        }
    } else {
        curl_easy_setopt(curlHandle,CURLOPT_FNMATCH_FUNCTION,NULL);
        return TCL_OK;
    }
    if (curl_easy_setopt(curlHandle,CURLOPT_FNMATCH_DATA,args->curlData)) {
        return TCL_ERROR;
    }
    return TCL_OK;
}

static int
TclCurl_HandleResolveList(TclCurlOptsArgs *args)
{
    CURL *curlHandle = args->curlData->curl;

    if (SetoptsList(args->interp,&args->curlData->resolve,args->objv)) {
        curlErrorSetOpt(args->interp,configTable,args->curlOptsIndex,"invalid list");
        return TCL_ERROR;
    }
    if (curl_easy_setopt(curlHandle,CURLOPT_RESOLVE,args->curlData->resolve)) {
        curlErrorSetOpt(args->interp,configTable,args->curlOptsIndex,"resolve list invalid");
        curl_slist_free_all(args->curlData->resolve);
        args->curlData->resolve=NULL;
        return TCL_ERROR;
    }
    return TCL_OK;
}

static int
TclCurl_HandleTlsAuthType(TclCurlOptsArgs *args)
{
    CURL *curlHandle = args->curlData->curl;
    Tcl_Obj *tmpObjPtr;
    long longNumber=0;
    int tlsAuthTypeIndex;

    if (Tcl_GetIndexFromObj(args->interp, args->objv, tlsauth,
        "TSL auth option ",TCL_EXACT,&tlsAuthTypeIndex)==TCL_ERROR) {
        return TCL_ERROR;
    }
    switch(tlsAuthTypeIndex) {
        case 0:
            longNumber=CURL_TLSAUTH_NONE;
            break;
        case 1:
            longNumber=CURL_TLSAUTH_SRP;
    }
    tmpObjPtr=Tcl_NewLongObj(longNumber);
    Tcl_IncrRefCount(tmpObjPtr);
    if (SetoptLong(args->interp,curlHandle,CURLOPT_TLSAUTH_TYPE,
                args->curlOptsIndex,tmpObjPtr)) {
        Tcl_DecrRefCount(tmpObjPtr);
        return TCL_ERROR;
    }
    Tcl_DecrRefCount(tmpObjPtr);
    return TCL_OK;
}

static int
TclCurl_HandleGssApiDelegation(TclCurlOptsArgs *args)
{
    CURL *curlHandle = args->curlData->curl;
    Tcl_Obj *tmpObjPtr;
    long longNumber=0;
    int gssApiDelegationIndex;

    if (Tcl_GetIndexFromObj(args->interp, args->objv, gssapidelegation,
        "GSS API delegation option ",TCL_EXACT,&gssApiDelegationIndex)==TCL_ERROR) {
        return TCL_ERROR;
    }
    switch(gssApiDelegationIndex) {
        case 0:
            longNumber=CURLGSSAPI_DELEGATION_FLAG;
            break;
        case 1:
            longNumber=CURLGSSAPI_DELEGATION_POLICY_FLAG;
    }
    tmpObjPtr=Tcl_NewLongObj(longNumber);
    Tcl_IncrRefCount(tmpObjPtr);
    if (SetoptLong(args->interp,curlHandle,CURLOPT_GSSAPI_DELEGATION,
                args->curlOptsIndex,tmpObjPtr)) {
        Tcl_DecrRefCount(tmpObjPtr);
        return TCL_ERROR;
    }
    Tcl_DecrRefCount(tmpObjPtr);
    return TCL_OK;
}

static int
TclCurl_HandleTelnetOptions(TclCurlOptsArgs *args)
{
    CURL *curlHandle = args->curlData->curl;

    if (SetoptsList(args->interp,&args->curlData->telnetoptions,args->objv)) {
        curlErrorSetOpt(args->interp,configTable,args->curlOptsIndex,"invalid list");
        return TCL_ERROR;
    }
    if (curl_easy_setopt(curlHandle,CURLOPT_TELNETOPTIONS,args->curlData->telnetoptions)) {
        curlErrorSetOpt(args->interp,configTable,args->curlOptsIndex,"telnetoptions list invalid");
        curl_slist_free_all(args->curlData->telnetoptions);
        args->curlData->telnetoptions = NULL;
        return TCL_ERROR;
    }
    return TCL_OK;
}

static int
TclCurl_HandleCainfoBlob(TclCurlOptsArgs *args)
{
#if CURL_AT_LEAST_VERSION(7, 77, 0)
    if (SetoptBlob(args->interp,args->curlData->curl,CURLOPT_CAINFO_BLOB,
            args->curlOptsIndex,args->objv)) {
        return TCL_ERROR;
    }
    return TCL_OK;
#else
    return TCL_ERROR;
#endif
}
