/*
 * curl_setopts.c
 *
 *
 */

#include "tclcurl.h"
#include "curl_setopts.h"

/* size_t -> Tcl_Size */
static inline int SizeT2TclSize(size_t in, Tcl_Size *out) {
    if (in > (size_t)TCL_SIZE_MAX) return 0;  /* overflow for Tcl_Size */
    *out = (Tcl_Size)in;
    return 1;
}

/* Tcl_Size -> size_t */
static inline int TclSize2SizeT(Tcl_Size in, size_t *out) {
    if (in < 0) return 0;  /* negative not representable as size_t */
    *out = (size_t)in;
    return 1;
}

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
curlSetOptsTransfer(Tcl_Interp *interp, struct curlObjData *curlData,
        int objc, Tcl_Obj *const objv[]) {

    int            tableIndex;

    if (Tcl_GetIndexFromObj(interp, objv[2], optionTable, "option", 
            TCL_EXACT, &tableIndex)==TCL_ERROR) {
        return TCL_ERROR;
    }

    return  TclCurl_SetOpts(interp,curlData,objv[3],tableIndex);
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

    int              tableIndex;
    int              i,j;
    Tcl_Obj         *resultPtr;

    for(i=2,j=3;i<objc;i=i+2,j=j+2) {
        if (Tcl_GetIndexFromObj(interp, objv[i], configTable, "option", 
                TCL_EXACT, &tableIndex)==TCL_ERROR) {
            return TCL_ERROR;
        }
        if (i==objc-1) {
            resultPtr=Tcl_ObjPrintf("Empty value for %s",configTable[tableIndex]);
            Tcl_SetObjResult(interp,resultPtr);            
            return TCL_ERROR;
        }
        if (TclCurl_SetOpts(interp,curlData,objv[j],tableIndex)==TCL_ERROR) {
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
int
SetoptInt(Tcl_Interp *interp,CURL *curlHandle,CURLoption opt,
        int tableIndex,Tcl_Obj *tclObj) {
    int        intNumber;

    if (Tcl_GetIntFromObj(interp,tclObj,&intNumber)) {
        curlErrorSetOpt(interp,configTable,tableIndex,Tcl_GetString(tclObj));
        return 1;
    }
    if (curl_easy_setopt(curlHandle,opt,intNumber)) {
        curlErrorSetOpt(interp,configTable,tableIndex,Tcl_GetString(tclObj));
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
int
SetoptLong(Tcl_Interp *interp,CURL *curlHandle,CURLoption opt,
        int tableIndex,Tcl_Obj *tclObj) {
    long         longNumber;

    if (Tcl_GetLongFromObj(interp,tclObj,&longNumber)) {
        curlErrorSetOpt(interp,configTable,tableIndex,Tcl_GetString(tclObj));
        return 1;
    }
    if (curl_easy_setopt(curlHandle,opt,longNumber)) {
        curlErrorSetOpt(interp,configTable,tableIndex,Tcl_GetString(tclObj));
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
        int tableIndex,Tcl_Obj *tclObj) {
    Tcl_WideInt wideNumber;

    if (Tcl_GetWideIntFromObj(interp,tclObj,&wideNumber)) {
        curlErrorSetOpt(interp,configTable,tableIndex,Tcl_GetString(tclObj));
        return 1;
    }

    if (curl_easy_setopt(curlHandle,opt,(curl_off_t)wideNumber)) {
        curlErrorSetOpt(interp,configTable,tableIndex,Tcl_GetString(tclObj));
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
int
SetoptChar(Tcl_Interp *interp,CURL *curlHandle,
        CURLoption opt,int tableIndex,Tcl_Obj *tclObj) {
    char    *optionPtr;

    optionPtr=curlstrdup(Tcl_GetString(tclObj));
    if (curl_easy_setopt(curlHandle,opt,optionPtr)) {
        curlErrorSetOpt(interp,configTable,tableIndex,optionPtr);
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
int
SetoptBlob(Tcl_Interp *interp,CURL *curlHandle,
        CURLoption opt,int tableIndex,Tcl_Obj *tclObj) {
    struct curl_blob   optionBlob;
    Tcl_Size           len;

    optionBlob.data = Tcl_GetByteArrayFromObj(tclObj,&len);
    if (optionBlob.data) {
        optionBlob.len = len;
        optionBlob.flags = CURL_BLOB_COPY;
        if (curl_easy_setopt(curlHandle,opt,&optionBlob)) {
            curlErrorSetOpt(interp,configTable,tableIndex,"...");
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
SetoptSHandle(Tcl_Interp *interp,CURL *curlHandle,
        CURLoption opt,int tableIndex,Tcl_Obj *tclObj) {

    char                    *shandleName;
    Tcl_CmdInfo             *infoPtr=(Tcl_CmdInfo *)Tcl_Alloc(sizeof(Tcl_CmdInfo));
    struct shcurlObjData    *shandleDataPtr;

    shandleName=Tcl_GetString(tclObj);
    if (0==Tcl_GetCommandInfo(interp,shandleName,infoPtr)) {
        return 1;
    }
    shandleDataPtr=(struct shcurlObjData *)(infoPtr->objClientData);
    Tcl_Free((char *)infoPtr);
    if (curl_easy_setopt(curlHandle,opt,shandleDataPtr->shandle)) {
        curlErrorSetOpt(interp,configTable,tableIndex,shandleName);
        return 1;
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

typedef struct TclCurlOptionDef TclCurlOptionDef;
typedef int (*TclCurlOptionHandler)(Tcl_Interp *interp,
        struct curlObjData *curlData, Tcl_Obj *const objv, int tableIndex,
        const TclCurlOptionDef *def);

struct TclCurlOptionDef {
    const char           *optionName;
    const char           *configName;
    TclCurlOptionHandler  handler;
    CURLoption            curlOpt;
    const char           *errorMessage;
};

static int TclCurl_HandleSetoptChar(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def);
static int TclCurl_HandleSetoptInt(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def);
static int TclCurl_HandleSetoptLong(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def);
static int TclCurl_HandleSetoptCurlOffT(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def);
static int TclCurl_HandleSetoptSHandle(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def);
static int TclCurl_HandleOutFile(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def);
static int TclCurl_HandleReadData(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def);
static int TclCurl_HandleNetrc(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def);
static int TclCurl_HandleTransferText(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def);
static int TclCurl_HandleMute(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def);
static int TclCurl_HandleErrorBuffer(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def);
static int TclCurl_HandleHttpHeaderList(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def);
static int TclCurl_HandleHttpPost(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def);
static int TclCurl_HandleSslVersion(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def);
static int TclCurl_HandleQuoteList(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def);
static int TclCurl_HandlePostQuoteList(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def);
static int TclCurl_HandleWriteHeader(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def);
static int TclCurl_HandleTimeCondition(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def);
static int TclCurl_HandleStderrFile(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def);
static int TclCurl_HandleHeaderVar(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def);
static int TclCurl_HandleBodyVar(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def);
static int TclCurl_HandleProgressProc(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def);
static int TclCurl_HandleCancelTransVarName(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def);
static int TclCurl_HandleWriteProc(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def);
static int TclCurl_HandleReadProc(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def);
static int TclCurl_HandleHttpVersion(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def);
static int TclCurl_HandlePrequoteList(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def);
static int TclCurl_HandleDebugProc(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def);
static int TclCurl_HandleEncoding(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def);
static int TclCurl_HandleProxyType(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def);
static int TclCurl_HandleHttp200Aliases(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def);
static int TclCurl_HandleCommand(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def);
static int TclCurl_HandleHttpAuth(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def);
static int TclCurl_HandleProxyAuth(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def);
static int TclCurl_HandleIpResolve(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def);
static int TclCurl_HandleFtpSsl(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def);
static int TclCurl_HandleObsolete(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def);
static int TclCurl_HandleFtpSslAuth(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def);
static int TclCurl_HandleFtpFileMethod(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def);
static int TclCurl_HandleSshAuthTypes(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def);
static int TclCurl_HandlePostRedir(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def);
static int TclCurl_HandleProtocolMask(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def);
static int TclCurl_HandleFtpSslCcc(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def);
static int TclCurl_HandleSshKeyFunction(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def);
static int TclCurl_HandleMailRcpt(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def);
static int TclCurl_HandleChunkBgnProc(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def);
static int TclCurl_HandleChunkBgnVar(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def);
static int TclCurl_HandleChunkEndProc(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def);
static int TclCurl_HandleFnmatchProc(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def);
static int TclCurl_HandleResolveList(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def);
static int TclCurl_HandleTlsAuthType(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def);
static int TclCurl_HandleGssApiDelegation(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def);
static int TclCurl_HandleTelnetOptions(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def);
static int TclCurl_HandleCainfoBlob(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def);

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
 *    tableIndex: The index of the option in the options table.
 *
 * Results:
 *  A standard Tcl result.
 *----------------------------------------------------------------------
 */

int
TclCurl_SetOpts(Tcl_Interp *interp, struct curlObjData *curlData,
                Tcl_Obj *const objv, int tableIndex)
{
    if (tableIndex < 0 || tableIndex >= TCLCURLOPT_COUNT) {
        return TCL_ERROR;
    }
    return curlOptionDefs[tableIndex].handler(interp, curlData, objv, tableIndex,
            &curlOptionDefs[tableIndex]);
}

static int
TclCurl_HandleSetoptChar(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def)
{
    CURL *curlHandle = curlData->curl;

    if (SetoptChar(interp, curlHandle, def->curlOpt, tableIndex, objv)) {
        return TCL_ERROR;
    }
    return TCL_OK;
}

static int
TclCurl_HandleSetoptInt(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def)
{
    CURL *curlHandle = curlData->curl;

    if (SetoptInt(interp, curlHandle, def->curlOpt, tableIndex, objv)) {
        return TCL_ERROR;
    }
    return TCL_OK;
}

static int
TclCurl_HandleSetoptLong(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def)
{
    CURL *curlHandle = curlData->curl;

    if (SetoptLong(interp, curlHandle, def->curlOpt, tableIndex, objv)) {
        return TCL_ERROR;
    }
    return TCL_OK;
}

static int
TclCurl_HandleSetoptCurlOffT(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def)
{
    CURL *curlHandle = curlData->curl;

    if (SetoptCurlOffT(interp, curlHandle, def->curlOpt, tableIndex, objv)) {
        return TCL_ERROR;
    }
    return TCL_OK;
}

static int
TclCurl_HandleSetoptSHandle(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def)
{
    CURL *curlHandle = curlData->curl;

    if (SetoptSHandle(interp, curlHandle, def->curlOpt, tableIndex, objv)) {
        return TCL_ERROR;
    }
    return TCL_OK;
}

static int
TclCurl_HandleOutFile(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def)
{
    CURL *curlHandle = curlData->curl;

    (void)tableIndex;
    (void)def;

    Tcl_Free(curlData->outFile);
    curlData->outFile=curlstrdup(Tcl_GetString(objv));
    if (curlData->outFlag) {
        if (curlData->outHandle!=NULL) {
            fclose(curlData->outHandle);
            curlData->outHandle=NULL;
        }
    }
    if ((strcmp(curlData->outFile,""))&&(strcmp(curlData->outFile,"stdout"))) {
        curlData->outFlag=1;
    } else {
        curlData->outFlag=0;
        curl_easy_setopt(curlHandle,CURLOPT_WRITEDATA,stdout);
        curlData->outFile=NULL;
    }
    curl_easy_setopt(curlHandle,CURLOPT_WRITEFUNCTION,NULL);
    return TCL_OK;
}

static int
TclCurl_HandleReadData(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def)
{
    CURL *curlHandle = curlData->curl;

    (void)tableIndex;
    (void)def;

    Tcl_Free(curlData->inFile);
    curlData->inFile=curlstrdup(Tcl_GetString(objv));
    if (curlData->inFlag) {
        if (curlData->inHandle!=NULL) {
            fclose(curlData->inHandle);
            curlData->inHandle=NULL;
        }
    }
    if ((strcmp(curlData->inFile,""))&&(strcmp(curlData->inFile,"stdin"))) {
        curlData->inFlag=1;
    } else {
        curl_easy_setopt(curlHandle,CURLOPT_READDATA,stdin);
        curlData->inFlag=0;
        curlData->inFile=NULL;
    }
    curl_easy_setopt(curlHandle,CURLOPT_READFUNCTION,NULL);
    return TCL_OK;
}

static int
TclCurl_HandleNetrc(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def)
{
    CURL *curlHandle = curlData->curl;
    int  curlTableIndex;

    (void)def;

    if (Tcl_GetIndexFromObj(interp, objv, netrcTable,
            "netrc option",TCL_EXACT,&curlTableIndex)==TCL_ERROR) {
        return TCL_ERROR;
    }
    if (curl_easy_setopt(curlHandle,CURLOPT_NETRC,curlTableIndex)) {
        curlErrorSetOpt(interp,configTable,tableIndex,netrcTable[curlTableIndex]);
        return 1;
    }
    return TCL_OK;
}

static int
TclCurl_HandleTransferText(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def)
{
    CURL *curlHandle = curlData->curl;

    if (SetoptInt(interp, curlHandle, def->curlOpt, tableIndex, objv)) {
        return TCL_ERROR;
    }
    Tcl_GetIntFromObj(interp,objv,&curlData->transferText);
    return TCL_OK;
}

static int
TclCurl_HandleMute(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def)
{
    (void)interp;
    (void)curlData;
    (void)objv;
    (void)tableIndex;
    (void)def;

    return TCL_OK;
}

static int
TclCurl_HandleErrorBuffer(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def)
{
    (void)tableIndex;
    (void)def;

    if (TclCurl_ErrorBuffer(interp,curlData,objv) == TCL_ERROR) {
        return TCL_ERROR;
    }
    return TCL_OK;
}

static int
TclCurl_HandleHttpHeaderList(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def)
{
    CURL *curlHandle = curlData->curl;

    (void)def;

    if(SetoptsList(interp,&curlData->headerList,objv)) {
        curlErrorSetOpt(interp,configTable,tableIndex,"Header list invalid");
        return TCL_ERROR;
    }
    if (curl_easy_setopt(curlHandle,CURLOPT_HTTPHEADER,curlData->headerList)) {
        curl_slist_free_all(curlData->headerList);
        curlData->headerList=NULL;
        return TCL_ERROR;
    }
    return TCL_OK;
}

static int
TclCurl_HandleHttpPost(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def)
{
    Tcl_Obj *resultObjPtr;
    Tcl_Size i,j;
    Tcl_Size post_data_numel;
    Tcl_Obj **httpPostData;
    int curlTableIndex;
    int formaddError,formArrayIndex;
    struct formArrayStruct   *newFormArray;
    struct curl_forms        *formArray;
    Tcl_Size                  curlformBufferSize;
    size_t                    contentslen;
    unsigned char            *tmpUStr;
    char*                     tmpStr = NULL;

    (void)def;

    if (Tcl_ListObjGetElements(interp,objv,&post_data_numel,&httpPostData) == TCL_ERROR) {
        return TCL_ERROR;
    }
    formaddError=0;
    newFormArray=(struct formArrayStruct *)Tcl_Alloc(sizeof(struct formArrayStruct));
    formArray=(struct curl_forms *)Tcl_Alloc(post_data_numel*(sizeof(struct curl_forms)));
    formArrayIndex=0;

    newFormArray->next=curlData->formArray;
    newFormArray->formArray=formArray;
    newFormArray->formHeaderList=NULL;

    for (i=0,j=0;i<post_data_numel;i+=2,j+=1) {
        if (Tcl_GetIndexFromObj(interp,httpPostData[i],curlFormTable,
                "CURLFORM option",TCL_EXACT,&curlTableIndex)==TCL_ERROR) {
            formaddError=1;
            break;
        }
        switch (curlTableIndex) {
            case NAME_HTTP_OPT:
                formArray[formArrayIndex].option = CURLFORM_COPYNAME;
                formArray[formArrayIndex].value  = curlstrdup(Tcl_GetString(httpPostData[i+1]));
                break;
            case CONTENTS_HTTP_OPT:
            {
                tmpStr=Tcl_GetStringFromObj(httpPostData[i+1],&curlformBufferSize);
                formArray[formArrayIndex].option = CURLFORM_COPYCONTENTS;

                formArray[formArrayIndex].value = Tcl_Alloc((curlformBufferSize > 0) ? curlformBufferSize : 1);

                if (curlformBufferSize > 0) {
                    size_t buffer_size;

                    if (TclSize2SizeT(curlformBufferSize,&buffer_size) == 0) {
                        curlErrorSetOpt(interp,configTable,tableIndex,"Inconsistent buffer size");
                        return TCL_ERROR;
                    }
                    memcpy((char *)formArray[formArrayIndex].value,tmpStr,buffer_size);
                } else {
                    memset((char *)formArray[formArrayIndex].value,0,1);
                }

                formArrayIndex++;
                formArray[formArrayIndex].option = CURLFORM_CONTENTSLENGTH;
                contentslen=curlformBufferSize++;
                formArray[formArrayIndex].value  = (char *)contentslen;
                break;
            }
            case FILE_HTTP_OPT:
                formArray[formArrayIndex].option = CURLFORM_FILE;
                formArray[formArrayIndex].value  = curlstrdup(Tcl_GetString(httpPostData[i+1]));
                break;
            case CONTENTTYPE_HTTP_OPT:
                formArray[formArrayIndex].option = CURLFORM_CONTENTTYPE;
                formArray[formArrayIndex].value  = curlstrdup(Tcl_GetString(httpPostData[i+1]));
                break;
            case CONTENTHEADER_HTTP_OPT:
                formArray[formArrayIndex].option = CURLFORM_CONTENTHEADER;
                if(SetoptsList(interp,&newFormArray->formHeaderList,httpPostData[i+1])) {
                    curlErrorSetOpt(interp,configTable,tableIndex,"Header list invalid");
                    formaddError=1;
                    return TCL_ERROR;
                }
                formArray[formArrayIndex].value  = (char *)newFormArray->formHeaderList;
                break;
            case FILENAME_HTTP_OPT:
                formArray[formArrayIndex].option = CURLFORM_FILENAME;
                formArray[formArrayIndex].value  = curlstrdup(Tcl_GetString(httpPostData[i+1]));
                break;
            case BUFFERNAME_HTTP_OPT:
                formArray[formArrayIndex].option = CURLFORM_BUFFER;
                formArray[formArrayIndex].value  = curlstrdup(Tcl_GetString(httpPostData[i+1]));
                break;
            case BUFFER_HTTP_OPT:
                tmpUStr=Tcl_GetByteArrayFromObj(httpPostData[i+1],&curlformBufferSize);
                formArray[formArrayIndex].option = CURLFORM_BUFFERPTR;
                formArray[formArrayIndex].value  = (char *)
                        memcpy(Tcl_Alloc(curlformBufferSize), tmpUStr, curlformBufferSize);
                formArrayIndex++;
                formArray[formArrayIndex].option = CURLFORM_BUFFERLENGTH;
                contentslen=curlformBufferSize;
                formArray[formArrayIndex].value  = (char *)contentslen;
                break;
            case FILECONTENT_HTTP_OPT:
                formArray[formArrayIndex].option = CURLFORM_FILECONTENT;
                formArray[formArrayIndex].value  = curlstrdup(Tcl_GetString(httpPostData[i+1]));
                break;
        }
        formArrayIndex++;
    }
    formArray[formArrayIndex].option=CURLFORM_END;
    curlData->formArray=newFormArray;

    if (0==formaddError) {
        formaddError=curl_formadd(&(curlData->postListFirst)
                ,&(curlData->postListLast), CURLFORM_ARRAY, formArray
                , CURLFORM_END);
    }
    if (formaddError!=CURL_FORMADD_OK) {
        curlResetFormArray(formArray);
        curlData->formArray=newFormArray->next;
        Tcl_Free((char *)newFormArray);
        resultObjPtr=Tcl_ObjPrintf("%d",formaddError);
        Tcl_SetObjResult(interp,resultObjPtr);
        Tcl_Free(tmpStr);
        return TCL_ERROR;
    }
    return TCL_OK;
}

static int
TclCurl_HandleSslVersion(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def)
{
    CURL *curlHandle = curlData->curl;
    Tcl_Obj *tmpObjPtr;
    long longNumber=0;
    int intNumber;

    (void)def;

    if (Tcl_GetIndexFromObj(interp, objv, sslversion,
        "sslversion ",TCL_EXACT,&intNumber)==TCL_ERROR) {
        return TCL_ERROR;
    }
    switch(intNumber) {
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
    if (SetoptLong(interp,curlHandle,CURLOPT_SSLVERSION,
                tableIndex,tmpObjPtr)) {
        Tcl_DecrRefCount(tmpObjPtr);
        return TCL_ERROR;
    }
    Tcl_DecrRefCount(tmpObjPtr);
    return TCL_OK;
}

static int
TclCurl_HandleQuoteList(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def)
{
    CURL *curlHandle = curlData->curl;

    (void)def;

    if(SetoptsList(interp,&curlData->quote,objv)) {
        curlErrorSetOpt(interp,configTable,tableIndex,"quote list invalid");
        return TCL_ERROR;
    }
    if (curl_easy_setopt(curlHandle,CURLOPT_QUOTE,curlData->quote)) {
        curlErrorSetOpt(interp,configTable,tableIndex,"quote list invalid");
        curl_slist_free_all(curlData->quote);
        curlData->quote=NULL;
        return TCL_ERROR;
    }
    return TCL_OK;
}

static int
TclCurl_HandlePostQuoteList(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def)
{
    CURL *curlHandle = curlData->curl;

    (void)def;

    if(SetoptsList(interp,&curlData->postquote,objv)) {
        curlErrorSetOpt(interp,configTable,tableIndex,"postquote invalid");
        return TCL_ERROR;
    }
    if (curl_easy_setopt(curlHandle,CURLOPT_POSTQUOTE,curlData->postquote)) {
        curlErrorSetOpt(interp,configTable,tableIndex,"postquote invalid");
        curl_slist_free_all(curlData->postquote);
        curlData->postquote=NULL;
        return TCL_ERROR;
    }
    return TCL_OK;
}

static int
TclCurl_HandleWriteHeader(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def)
{
    CURL *curlHandle = curlData->curl;

    (void)tableIndex;
    (void)def;

    Tcl_Free(curlData->headerFile);
    curlData->headerFile=curlstrdup(Tcl_GetString(objv));
    if (curlData->headerFlag) {
        if (curlData->headerHandle!=NULL) {
            fclose(curlData->headerHandle);
            curlData->headerHandle=NULL;
        }
        curl_easy_setopt(curlHandle,CURLOPT_HEADERDATA,NULL);
    }
    if ((strcmp(curlData->headerFile,""))&&(strcmp(curlData->headerFile,"stdout"))
            &&(strcmp(curlData->headerFile,"stderr"))) {
        curlData->headerFlag=1;
    } else {
        if ((strcmp(curlData->headerFile,"stdout"))) {
            curl_easy_setopt(curlHandle,CURLOPT_HEADERDATA,stderr);
        } else {
            curl_easy_setopt(curlHandle,CURLOPT_HEADERDATA,stdout);
        }
        curlData->headerFlag=0;
        curlData->headerFile=NULL;
    }
    return TCL_OK;
}

static int
TclCurl_HandleTimeCondition(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def)
{
    CURL *curlHandle = curlData->curl;
    long longNumber=0;
    int intNumber;

    (void)tableIndex;
    (void)def;

    if (Tcl_GetIndexFromObj(interp, objv, timeCond,
        "time cond option",TCL_EXACT, &intNumber)==TCL_ERROR) {
        return TCL_ERROR;
    }
    if (intNumber==0) {
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
TclCurl_HandleStderrFile(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def)
{
    CURL *curlHandle = curlData->curl;

    (void)tableIndex;
    (void)def;

    Tcl_Free(curlData->stderrFile);
    curlData->stderrFile=curlstrdup(Tcl_GetString(objv));
    if ((strcmp(curlData->stderrFile,""))&&(strcmp(curlData->stderrFile,"stdout"))
            &&(strcmp(curlData->stderrFile,"stderr"))) {
        curlData->stderrFlag=1;
    } else {
        curlData->stderrFlag=0;
        if (strcmp(curlData->stderrFile,"stdout")) {
            curl_easy_setopt(curlHandle,CURLOPT_STDERR,stderr);
        } else {
            curl_easy_setopt(curlHandle,CURLOPT_STDERR,stdout);
        }
        curlData->stderrFile=NULL;
    }
    return TCL_OK;
}

static int
TclCurl_HandleHeaderVar(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def)
{
    CURL *curlHandle = curlData->curl;

    (void)tableIndex;
    (void)def;

    if (curlData->headerFlag) {
        if (curlData->headerHandle!=NULL) {
            fclose(curlData->headerHandle);
            curlData->headerHandle=NULL;
        }
        curl_easy_setopt(curlHandle,CURLOPT_HEADERDATA,NULL);
        curlData->headerFlag=0;
    }
    if (curl_easy_setopt(curlHandle,CURLOPT_HEADERFUNCTION,
            curlHeaderReader)) {
        return TCL_ERROR;
    }
    Tcl_Free(curlData->headerVar);
    curlData->headerVar=curlstrdup(Tcl_GetString(objv));
    if (curl_easy_setopt(curlHandle,CURLOPT_HEADERDATA,
            (FILE *)curlData)) {
        return TCL_ERROR;
    }
    return TCL_OK;
}

static int
TclCurl_HandleBodyVar(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def)
{
    CURL *curlHandle = curlData->curl;

    (void)tableIndex;
    (void)def;

    Tcl_Free(curlData->bodyVarName);
    curlData->bodyVarName=curlstrdup(Tcl_GetString(objv));
    if (curlData->outFlag) {
        if (curlData->outHandle!=NULL) {
            fclose(curlData->outHandle);
            curlData->outHandle=NULL;
        }
        curl_easy_setopt(curlHandle,CURLOPT_WRITEDATA,NULL);
    }
    curlData->outFlag=0;
    if (curl_easy_setopt(curlHandle,CURLOPT_WRITEFUNCTION,
            curlBodyReader)) {
        return TCL_ERROR;
    }
    if (curl_easy_setopt(curlHandle,CURLOPT_WRITEDATA,curlData)) {
        return TCL_ERROR;
    }
    return TCL_OK;
}

static int
TclCurl_HandleProgressProc(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def)
{
    CURL *curlHandle = curlData->curl;

    (void)tableIndex;
    (void)def;

    Tcl_Free(curlData->progressProc);
    curlData->progressProc=curlstrdup(Tcl_GetString(objv));
    if (strcmp(curlData->progressProc,"")) {
        if (curl_easy_setopt(curlHandle,CURLOPT_PROGRESSFUNCTION,
                curlProgressCallback)) {
            return TCL_ERROR;
        }
        if (curl_easy_setopt(curlHandle,CURLOPT_PROGRESSDATA,
                curlData)) {
            return TCL_ERROR;
        }
    } else {
        if (curl_easy_setopt(curlHandle,CURLOPT_PROGRESSFUNCTION,NULL)) {
            return TCL_ERROR;
        }
    }
    return TCL_OK;
}

static int
TclCurl_HandleCancelTransVarName(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def)
{
    (void)tableIndex;
    (void)def;

    if (curlData->cancelTransVarName) {
        Tcl_UnlinkVar(curlData->interp,curlData->cancelTransVarName);
        Tcl_Free(curlData->cancelTransVarName);
    }
    curlData->cancelTransVarName=curlstrdup(Tcl_GetString(objv));
    Tcl_LinkVar(interp,curlData->cancelTransVarName,
            (char *)&(curlData->cancelTrans),TCL_LINK_INT);
    return TCL_OK;
}

static int
TclCurl_HandleWriteProc(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def)
{
    CURL *curlHandle = curlData->curl;

    (void)tableIndex;
    (void)def;

    curlData->writeProc=curlstrdup(Tcl_GetString(objv));
    if (curlData->outFlag) {
        if (curlData->outHandle!=NULL) {
            fclose(curlData->outHandle);
            curlData->outHandle=NULL;
        }
        curl_easy_setopt(curlHandle,CURLOPT_WRITEDATA,NULL);
    }
    curlData->outFlag=0;
    if (curl_easy_setopt(curlHandle,CURLOPT_WRITEFUNCTION,
            curlWriteProcInvoke)) {
        curl_easy_setopt(curlHandle,CURLOPT_WRITEFUNCTION,NULL);
        return TCL_ERROR;
    }
    if (curl_easy_setopt(curlHandle,CURLOPT_WRITEDATA,curlData)) {
        curl_easy_setopt(curlHandle,CURLOPT_WRITEFUNCTION,NULL);
        return TCL_ERROR;
    }
    return TCL_OK;
}

static int
TclCurl_HandleReadProc(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def)
{
    CURL *curlHandle = curlData->curl;

    (void)tableIndex;
    (void)def;

    curlData->readProc=curlstrdup(Tcl_GetString(objv));
    if (curlData->inFlag) {
        if (curlData->inHandle!=NULL) {
            fclose(curlData->inHandle);
            curlData->inHandle=NULL;
        }
        curl_easy_setopt(curlHandle,CURLOPT_READDATA,NULL);
    }
    curlData->inFlag=0;
    if (strcmp(curlData->readProc,"")) {
        if (curl_easy_setopt(curlHandle,CURLOPT_READFUNCTION,
                curlReadProcInvoke)) {
            return TCL_ERROR;
        }
    } else {
        curl_easy_setopt(curlHandle,CURLOPT_READFUNCTION,NULL);
        return TCL_OK;
    }
    if (curl_easy_setopt(curlHandle,CURLOPT_READDATA,curlData)) {
        return TCL_ERROR;
    }
    return TCL_OK;
}

static int
TclCurl_HandleHttpVersion(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def)
{
    CURL *curlHandle = curlData->curl;
    char* tmpStr = NULL;
    int versionIndex;

    (void)def;

    if (Tcl_GetIndexFromObj(interp, objv, httpVersionTable,
        "http version",TCL_EXACT,&versionIndex)==TCL_ERROR) {
        return TCL_ERROR;
    }
    if (curl_easy_setopt(curlHandle,CURLOPT_HTTP_VERSION,
                versionIndex)) {
        tmpStr=curlstrdup(Tcl_GetString(objv));
        curlErrorSetOpt(interp,configTable,tableIndex,tmpStr);
        Tcl_Free(tmpStr);
        return TCL_ERROR;
    }
    return TCL_OK;
}

static int
TclCurl_HandlePrequoteList(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def)
{
    CURL *curlHandle = curlData->curl;

    (void)def;

    if(SetoptsList(interp,&curlData->prequote,objv)) {
        curlErrorSetOpt(interp,configTable,tableIndex,"prequote invalid");
        return TCL_ERROR;
    }
    if (curl_easy_setopt(curlHandle,CURLOPT_PREQUOTE,curlData->prequote)) {
        curlErrorSetOpt(interp,configTable,tableIndex,"prequote invalid");
        curl_slist_free_all(curlData->prequote);
        curlData->prequote=NULL;
        return TCL_ERROR;
    }
    return TCL_OK;
}

static int
TclCurl_HandleDebugProc(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def)
{
    CURL *curlHandle = curlData->curl;

    (void)tableIndex;
    (void)def;

    curlData->debugProc=curlstrdup(Tcl_GetString(objv));
    if (curl_easy_setopt(curlHandle,CURLOPT_DEBUGFUNCTION,
            curlDebugProcInvoke)) {
        return TCL_ERROR;
    }
    if (curl_easy_setopt(curlHandle,CURLOPT_DEBUGDATA,curlData)) {
        return TCL_ERROR;
    }
    return TCL_OK;
}

static int
TclCurl_HandleEncoding(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def)
{
    CURL *curlHandle = curlData->curl;
    int encodingIndex;

    (void)def;

    if (Tcl_GetIndexFromObj(interp, objv, encodingTable,
        "encoding",TCL_EXACT,&encodingIndex)==TCL_ERROR) {
        return TCL_ERROR;
    }
    if (encodingIndex==2) {
        if (curl_easy_setopt(curlHandle,CURLOPT_ACCEPT_ENCODING,"")) {
            curlErrorSetOpt(interp,configTable,tableIndex,"all");
            return 1;
        }
    } else {
        if (SetoptChar(interp,curlHandle,CURLOPT_ACCEPT_ENCODING,tableIndex,objv)) {
            return TCL_ERROR;
        }
    }
    return TCL_OK;
}

static int
TclCurl_HandleProxyType(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def)
{
    CURL *curlHandle = curlData->curl;
    int proxyIndex;

    (void)interp;
    (void)tableIndex;
    (void)def;

    if (Tcl_GetIndexFromObj(interp, objv, proxyTypeTable,
        "proxy type",TCL_EXACT,&proxyIndex)==TCL_ERROR) {
        return TCL_ERROR;
    }
    switch(proxyIndex) {
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
TclCurl_HandleHttp200Aliases(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def)
{
    CURL *curlHandle = curlData->curl;

    (void)def;

    if(SetoptsList(interp,&curlData->http200aliases,objv)) {
        curlErrorSetOpt(interp,configTable,tableIndex,"http200aliases invalid");
        return TCL_ERROR;
    }
    if (curl_easy_setopt(curlHandle,CURLOPT_HTTP200ALIASES,curlData->http200aliases)) {
        curlErrorSetOpt(interp,configTable,tableIndex,"http200aliases invalid");
        curl_slist_free_all(curlData->http200aliases);
        curlData->http200aliases=NULL;
        return TCL_ERROR;
    }
    return TCL_OK;
}

static int
TclCurl_HandleCommand(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def)
{
    (void)interp;
    (void)tableIndex;
    (void)def;

    Tcl_Free(curlData->command);
    curlData->command=curlstrdup(Tcl_GetString(objv));
    return TCL_OK;
}

static int
TclCurl_HandleHttpAuth(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def)
{
    CURL *curlHandle = curlData->curl;
    Tcl_Obj *tmpObjPtr;
    long longNumber=0;
    int intNumber;

    (void)def;

    if (Tcl_GetIndexFromObj(interp, objv, httpAuthMethods,
        "authentication method",TCL_EXACT,&intNumber)==TCL_ERROR) {
        return TCL_ERROR;
    }
    curlData->anyAuthFlag=0;
    switch(intNumber) {
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
            curlData->anyAuthFlag=1;
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
    if (SetoptLong(interp,curlHandle,CURLOPT_HTTPAUTH
            ,tableIndex,tmpObjPtr)) {
        Tcl_DecrRefCount(tmpObjPtr);
        return TCL_ERROR;
    }
    Tcl_DecrRefCount(tmpObjPtr);
    return TCL_OK;
}

static int
TclCurl_HandleProxyAuth(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def)
{
    CURL *curlHandle = curlData->curl;
    Tcl_Obj *tmpObjPtr;
    long longNumber=0;
    int intNumber;

    (void)curlData;
    (void)def;

    if (Tcl_GetIndexFromObj(interp, objv, httpAuthMethods,
        "authentication method",TCL_EXACT,&intNumber)==TCL_ERROR) {
        return TCL_ERROR;
    }
    switch(intNumber) {
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
    if (SetoptLong(interp,curlHandle,CURLOPT_PROXYAUTH
            ,tableIndex,tmpObjPtr)) {
        Tcl_DecrRefCount(tmpObjPtr);
        return TCL_ERROR;
    }
    Tcl_DecrRefCount(tmpObjPtr);
    return TCL_OK;
}

static int
TclCurl_HandleIpResolve(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def)
{
    CURL *curlHandle = curlData->curl;
    Tcl_Obj *tmpObjPtr;
    long longNumber=0;
    int curlTableIndex;

    (void)def;

    if (Tcl_GetIndexFromObj(interp, objv, ipresolve,
        "ip version",TCL_EXACT,&curlTableIndex)==TCL_ERROR) {
        return TCL_ERROR;
    }
    switch(curlTableIndex) {
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
    if (SetoptLong(interp,curlHandle,CURLOPT_IPRESOLVE
            ,tableIndex,tmpObjPtr)) {
        Tcl_DecrRefCount(tmpObjPtr);
        return TCL_ERROR;
    }
    Tcl_DecrRefCount(tmpObjPtr);
    return TCL_OK;
}

static int
TclCurl_HandleFtpSsl(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def)
{
    CURL *curlHandle = curlData->curl;
    Tcl_Obj *tmpObjPtr;
    long longNumber = TclCurl_FTPSSLMethod(interp,objv);

    (void)def;

    if (longNumber == -1) {
        return TCL_ERROR;
    }

    tmpObjPtr=Tcl_NewLongObj(longNumber);
    Tcl_IncrRefCount(tmpObjPtr);
    if (SetoptLong(interp,curlHandle,CURLOPT_USE_SSL,
                tableIndex,tmpObjPtr)) {
        Tcl_DecrRefCount(tmpObjPtr);
        return TCL_ERROR;
    }
    Tcl_DecrRefCount(tmpObjPtr);
    return TCL_OK;
}

static int
TclCurl_HandleObsolete(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def)
{
    (void)curlData;
    (void)objv;

    curlErrorSetOpt(interp,configTable,tableIndex,
            def->errorMessage ? def->errorMessage : "option is obsolete");
    return TCL_ERROR;
}

static int
TclCurl_HandleFtpSslAuth(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def)
{
    CURL *curlHandle = curlData->curl;
    Tcl_Obj *tmpObjPtr;
    long longNumber=0;
    int intNumber;

    (void)def;

    if (Tcl_GetIndexFromObj(interp, objv, ftpsslauth,
        "ftpsslauth method ",TCL_EXACT,&intNumber)==TCL_ERROR) {
        return TCL_ERROR;
    }
    switch(intNumber) {
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
    if (SetoptLong(interp,curlHandle,CURLOPT_FTPSSLAUTH,
                tableIndex,tmpObjPtr)) {
        Tcl_DecrRefCount(tmpObjPtr);
        return TCL_ERROR;
    }
    Tcl_DecrRefCount(tmpObjPtr);
    return TCL_OK;
}

static int
TclCurl_HandleFtpFileMethod(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def)
{
    CURL *curlHandle = curlData->curl;
    Tcl_Obj *tmpObjPtr;
    long longNumber=0;
    int intNumber;

    (void)def;

    if (Tcl_GetIndexFromObj(interp, objv, ftpfilemethod,
        "ftp file method ",TCL_EXACT,&intNumber)==TCL_ERROR) {
        return TCL_ERROR;
    }
    switch(intNumber) {
        case 0:
        case 1:
            longNumber=1;
            break;
        case 2:
            longNumber=2;
            break;
        case 3:
            longNumber=3;
            break;
    }
    tmpObjPtr=Tcl_NewLongObj(longNumber);
    Tcl_IncrRefCount(tmpObjPtr);
    if (SetoptLong(interp,curlHandle,CURLOPT_FTP_FILEMETHOD,
                tableIndex,tmpObjPtr)) {
        Tcl_DecrRefCount(tmpObjPtr);
        return TCL_ERROR;
    }
    Tcl_DecrRefCount(tmpObjPtr);
    return TCL_OK;
}

static int
TclCurl_HandleSshAuthTypes(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def)
{
    CURL *curlHandle = curlData->curl;
    Tcl_Obj *tmpObjPtr;
    long longNumber=0;
    int intNumber;

    (void)def;

    if (Tcl_GetIndexFromObj(interp, objv, sshauthtypes,
        "ssh auth type ",TCL_EXACT,&intNumber)==TCL_ERROR) {
        return TCL_ERROR;
    }
    switch(intNumber) {
        case 0:
            longNumber=CURLSSH_AUTH_PUBLICKEY;
            break;
        case 1:
            longNumber=CURLSSH_AUTH_PASSWORD;
            break;
        case 2:
            longNumber=CURLSSH_AUTH_HOST;
            break;
        case 3:
            longNumber=CURLSSH_AUTH_KEYBOARD;
            break;
        case 4:
            longNumber=CURLSSH_AUTH_ANY;
            break;
        case 5:
            longNumber=CURLSSH_AUTH_NONE;
            break;
        case 6:
            longNumber=CURLSSH_AUTH_AGENT;
            break;
        case 7:
            longNumber=CURLSSH_AUTH_DEFAULT;
            break;
    }
    tmpObjPtr=Tcl_NewLongObj(longNumber);
    Tcl_IncrRefCount(tmpObjPtr);
    if (SetoptLong(interp,curlHandle,CURLOPT_SSH_AUTH_TYPES,
                tableIndex,tmpObjPtr)) {
        Tcl_DecrRefCount(tmpObjPtr);
        return TCL_ERROR;
    }
    Tcl_DecrRefCount(tmpObjPtr);
    return TCL_OK;
}

static int
TclCurl_HandlePostRedir(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def)
{
    CURL *curlHandle = curlData->curl;
    Tcl_Obj *tmpObjPtr;
    long longNumber=0;
    int intNumber;

    (void)def;

    if (Tcl_GetIndexFromObj(interp, objv, postredir,
        "Postredir option ",TCL_EXACT,&intNumber)==TCL_ERROR) {
        return TCL_ERROR;
    }
    switch(intNumber) {
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
    if (SetoptLong(interp,curlHandle,CURLOPT_POSTREDIR,
                tableIndex,tmpObjPtr)) {
        Tcl_DecrRefCount(tmpObjPtr);
        return TCL_ERROR;
    }
    Tcl_DecrRefCount(tmpObjPtr);
    return TCL_OK;
}

static int
TclCurl_HandleProtocolMask(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def)
{
    CURL *curlHandle = curlData->curl;
    unsigned long int protocolMask;
    Tcl_Obj **protocols;
    Tcl_Size protocols_c;
    Tcl_Obj *tmpObjPtr;

    if (Tcl_ListObjGetElements(interp,objv,&protocols_c,&protocols) == TCL_ERROR) {
        return TCL_ERROR;
    }

    protocolMask = TclCurl_BuildProtocolMask(interp,protocols,protocols_c);

    tmpObjPtr=Tcl_NewLongObj(protocolMask);
    Tcl_IncrRefCount(tmpObjPtr);
    if (SetoptLong(interp,curlHandle,def->curlOpt,tableIndex,tmpObjPtr)) {
        Tcl_DecrRefCount(tmpObjPtr);
        return TCL_ERROR;
    }
    Tcl_DecrRefCount(tmpObjPtr);
    return TCL_OK;
}

static int
TclCurl_HandleFtpSslCcc(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def)
{
    CURL *curlHandle = curlData->curl;
    Tcl_Obj *tmpObjPtr;
    long longNumber = TclCurl_FTPClearCommandChannelOpt(interp,objv);

    (void)def;

    if (longNumber < 0) {
        return TCL_ERROR;
    }

    tmpObjPtr = Tcl_NewLongObj(longNumber);
    Tcl_IncrRefCount(tmpObjPtr);
    if (SetoptLong(interp,curlHandle,CURLOPT_FTP_SSL_CCC,tableIndex,tmpObjPtr)) {
        Tcl_DecrRefCount(tmpObjPtr);
        return TCL_ERROR;
    }
    Tcl_DecrRefCount(tmpObjPtr);
    return TCL_OK;
}

static int
TclCurl_HandleSshKeyFunction(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def)
{
    CURL *curlHandle = curlData->curl;

    (void)interp;
    (void)tableIndex;
    (void)def;

    if (curl_easy_setopt(curlHandle,CURLOPT_SSH_KEYFUNCTION,curlsshkeycallback)) {
        return TCL_ERROR;
    }
    if (curl_easy_setopt(curlHandle,CURLOPT_SSH_KEYDATA,curlData)) {
        return TCL_ERROR;
    }
    curlData->sshkeycallProc=curlstrdup(Tcl_GetString(objv));
    return TCL_OK;
}

static int
TclCurl_HandleMailRcpt(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def)
{
    CURL *curlHandle = curlData->curl;

    (void)def;

    if (SetoptsList(interp,&curlData->mailrcpt,objv)) {
        curlErrorSetOpt(interp,configTable,tableIndex,"mailrcpt invalid");
        return TCL_ERROR;
    }
    if (curl_easy_setopt(curlHandle,CURLOPT_MAIL_RCPT,curlData->mailrcpt)) {
        curlErrorSetOpt(interp,configTable,tableIndex,"mailrcpt invalid");
        curl_slist_free_all(curlData->mailrcpt);
        curlData->mailrcpt=NULL;
        return TCL_ERROR;
    }
    return TCL_OK;
}

static int
TclCurl_HandleChunkBgnProc(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def)
{
    CURL *curlHandle = curlData->curl;

    (void)interp;
    (void)tableIndex;
    (void)def;

    curlData->chunkBgnProc=curlstrdup(Tcl_GetString(objv));
    if (strcmp(curlData->chunkBgnProc,"")) {
        if (curl_easy_setopt(curlHandle,CURLOPT_CHUNK_BGN_FUNCTION,
                curlChunkBgnProcInvoke)) {
            return TCL_ERROR;
        }
    } else {
        curl_easy_setopt(curlHandle,CURLOPT_CHUNK_BGN_FUNCTION,NULL);
        return TCL_OK;
    }
    if (curl_easy_setopt(curlHandle,CURLOPT_CHUNK_DATA,curlData)) {
        return TCL_ERROR;
    }
    return TCL_OK;
}

static int
TclCurl_HandleChunkBgnVar(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def)
{
    (void)interp;
    (void)tableIndex;
    (void)def;

    curlData->chunkBgnVar=curlstrdup(Tcl_GetString(objv));
    if (!strcmp(curlData->chunkBgnVar,"")) {
        curlErrorSetOpt(interp,configTable,tableIndex,"invalid var name");
        return TCL_ERROR;
    }
    return TCL_OK;
}

static int
TclCurl_HandleChunkEndProc(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def)
{
    CURL *curlHandle = curlData->curl;

    (void)interp;
    (void)tableIndex;
    (void)def;

    curlData->chunkEndProc=curlstrdup(Tcl_GetString(objv));
    if (strcmp(curlData->chunkEndProc,"")) {
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
TclCurl_HandleFnmatchProc(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def)
{
    CURL *curlHandle = curlData->curl;

    (void)interp;
    (void)tableIndex;
    (void)def;

    curlData->fnmatchProc=curlstrdup(Tcl_GetString(objv));
    if (strcmp(curlData->fnmatchProc,"")) {
        if (curl_easy_setopt(curlHandle,CURLOPT_FNMATCH_FUNCTION,
                curlfnmatchProcInvoke)) {
            return TCL_ERROR;
        }
    } else {
        curl_easy_setopt(curlHandle,CURLOPT_FNMATCH_FUNCTION,NULL);
        return TCL_OK;
    }
    if (curl_easy_setopt(curlHandle,CURLOPT_FNMATCH_DATA,curlData)) {
        return TCL_ERROR;
    }
    return TCL_OK;
}

static int
TclCurl_HandleResolveList(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def)
{
    CURL *curlHandle = curlData->curl;

    (void)def;

    if (SetoptsList(interp,&curlData->resolve,objv)) {
        curlErrorSetOpt(interp,configTable,tableIndex,"invalid list");
        return TCL_ERROR;
    }
    if (curl_easy_setopt(curlHandle,CURLOPT_RESOLVE,curlData->resolve)) {
        curlErrorSetOpt(interp,configTable,tableIndex,"resolve list invalid");
        curl_slist_free_all(curlData->resolve);
        curlData->resolve=NULL;
        return TCL_ERROR;
    }
    return TCL_OK;
}

static int
TclCurl_HandleTlsAuthType(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def)
{
    CURL *curlHandle = curlData->curl;
    Tcl_Obj *tmpObjPtr;
    long longNumber=0;
    int intNumber;

    (void)def;

    if (Tcl_GetIndexFromObj(interp, objv, tlsauth,
        "TSL auth option ",TCL_EXACT,&intNumber)==TCL_ERROR) {
        return TCL_ERROR;
    }
    switch(intNumber) {
        case 0:
            longNumber=CURL_TLSAUTH_NONE;
            break;
        case 1:
            longNumber=CURL_TLSAUTH_SRP;
    }
    tmpObjPtr=Tcl_NewLongObj(longNumber);
    Tcl_IncrRefCount(tmpObjPtr);
    if (SetoptLong(interp,curlHandle,CURLOPT_TLSAUTH_TYPE,
                tableIndex,tmpObjPtr)) {
        Tcl_DecrRefCount(tmpObjPtr);
        return TCL_ERROR;
    }
    Tcl_DecrRefCount(tmpObjPtr);
    return TCL_OK;
}

static int
TclCurl_HandleGssApiDelegation(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def)
{
    CURL *curlHandle = curlData->curl;
    Tcl_Obj *tmpObjPtr;
    long longNumber=0;
    int intNumber;

    (void)def;

    if (Tcl_GetIndexFromObj(interp, objv, gssapidelegation,
        "GSS API delegation option ",TCL_EXACT,&intNumber)==TCL_ERROR) {
        return TCL_ERROR;
    }
    switch(intNumber) {
        case 0:
            longNumber=CURLGSSAPI_DELEGATION_FLAG;
            break;
        case 1:
            longNumber=CURLGSSAPI_DELEGATION_POLICY_FLAG;
    }
    tmpObjPtr=Tcl_NewLongObj(longNumber);
    Tcl_IncrRefCount(tmpObjPtr);
    if (SetoptLong(interp,curlHandle,CURLOPT_GSSAPI_DELEGATION,
                tableIndex,tmpObjPtr)) {
        Tcl_DecrRefCount(tmpObjPtr);
        return TCL_ERROR;
    }
    Tcl_DecrRefCount(tmpObjPtr);
    return TCL_OK;
}

static int
TclCurl_HandleTelnetOptions(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def)
{
    CURL *curlHandle = curlData->curl;

    (void)def;

    if (SetoptsList(interp,&curlData->telnetoptions,objv)) {
        curlErrorSetOpt(interp,configTable,tableIndex,"invalid list");
        return TCL_ERROR;
    }
    if (curl_easy_setopt(curlHandle,CURLOPT_TELNETOPTIONS,curlData->telnetoptions)) {
        curlErrorSetOpt(interp,configTable,tableIndex,"telnetoptions list invalid");
        curl_slist_free_all(curlData->telnetoptions);
        curlData->telnetoptions=NULL;
        return TCL_ERROR;
    }
    return TCL_OK;
}

static int
TclCurl_HandleCainfoBlob(Tcl_Interp *interp, struct curlObjData *curlData,
        Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def)
{
    (void)def;

#if CURL_AT_LEAST_VERSION(7, 77, 0)
    if (SetoptBlob(interp,curlData->curl,CURLOPT_CAINFO_BLOB,
            tableIndex,objv)) {
        return TCL_ERROR;
    }
    return TCL_OK;
#else
    (void)interp;
    (void)curlData;
    (void)objv;
    (void)tableIndex;
    return TCL_ERROR;
#endif
}
