/*
 *----------------------------------------------------------------------
 *
 * curlSetOpts --
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

#include "tclcurl.h"
#include "curlopts.h"

int
TclCurl_SetOpts(Tcl_Interp *interp, struct curlObjData *curlData,
                Tcl_Obj *const objv,int tableIndex) {

    int            exitCode;
    CURL           *curlHandle=curlData->curl;

    Tcl_Obj        *resultObjPtr;
    Tcl_Obj        *tmpObjPtr;

    Tcl_RegExp      regExp;

    int             charLength;
    long            longNumber=0;
    int             intNumber;
    unsigned char  *tmpUStr;

    int                       formaddError,formArrayIndex;
    struct formArrayStruct   *newFormArray;
    struct curl_forms        *formArray;
    Tcl_Size                  curlformBufferSize;
    size_t                    contentslen;


    switch(tableIndex) {
        case TCLCURLOPT_URL:
            if (SetoptChar(interp,curlHandle,CURLOPT_URL,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case TCLCURLOPT_FILE:
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
            break;
        case TCLCURLOPT_READDATA:
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
            break;
        case TCLCURLOPT_USERAGENT:
            if (SetoptChar(interp,curlHandle,CURLOPT_USERAGENT,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case TCLCURLOPT_REFERER:
            if (SetoptChar(interp,curlHandle,CURLOPT_REFERER,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case TCLCURLOPT_VERBOSE:
            if (SetoptInt(interp,curlHandle,CURLOPT_VERBOSE,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case TCLCURLOPT_HEADER:
            if (SetoptInt(interp,curlHandle,CURLOPT_HEADER,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case TCLCURLOPT_NOBODY:
            if (SetoptInt(interp,curlHandle,CURLOPT_NOBODY,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case TCLCURLOPT_PROXY:
            if (SetoptChar(interp,curlHandle,CURLOPT_PROXY,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case TCLCURLOPT_PROXYPORT:
            if (SetoptLong(interp,curlHandle,CURLOPT_PROXYPORT,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case TCLCURLOPT_HTTPPROXYTUNNEL:
            if (SetoptInt(interp,curlHandle,CURLOPT_HTTPPROXYTUNNEL,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case TCLCURLOPT_FAILONERROR:
            if (SetoptInt(interp,curlHandle,CURLOPT_FAILONERROR,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 12:
            if (SetoptLong(interp,curlHandle,CURLOPT_TIMEOUT,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 13:
            if (SetoptLong(interp,curlHandle,CURLOPT_LOW_SPEED_LIMIT,tableIndex,
                        objv)) {
                return TCL_ERROR;
            }
            break;
        case 14:
            if (SetoptLong(interp,curlHandle,CURLOPT_LOW_SPEED_TIME,tableIndex,
                        objv)) {
                return TCL_ERROR;
            }
            break;
        case 15:
            if (SetoptLong(interp,curlHandle,CURLOPT_RESUME_FROM,tableIndex,
                        objv)) {
                return TCL_ERROR;
            }
            break;
        case 16:
            if (SetoptLong(interp,curlHandle,CURLOPT_INFILESIZE,tableIndex,
                        objv)) {
                return TCL_ERROR;
            }
            break;
        case 17:
            if (SetoptInt(interp,curlHandle,CURLOPT_UPLOAD,tableIndex,
                    objv)) {
                return TCL_ERROR;
            }
            break;
        case 137:
        case 18:
            if (SetoptInt(interp,curlHandle,CURLOPT_DIRLISTONLY,tableIndex,
                    objv)) {
                return TCL_ERROR;
            }
            break;
        case 136:
        case 19:
            if (SetoptInt(interp,curlHandle,CURLOPT_APPEND,tableIndex,
                    objv)) {
                return TCL_ERROR;
            }
            break;
        case 20:
        {
            int curlTableIndex;

            if (Tcl_GetIndexFromObj(interp, objv, netrcTable,
                    "netrc option",TCL_EXACT,&curlTableIndex)==TCL_ERROR) {
                return TCL_ERROR;
            }
            if (curl_easy_setopt(curlHandle,CURLOPT_NETRC,curlTableIndex)) {
                curlErrorSetOpt(interp,configTable,tableIndex,netrcTable[curlTableIndex]);
                return 1;
            }
            break;
        }
        case 21:
            if (SetoptInt(interp,curlHandle,CURLOPT_FOLLOWLOCATION,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 22:
            if (SetoptInt(interp,curlHandle,CURLOPT_TRANSFERTEXT,tableIndex,
                    objv)) {
                return TCL_ERROR;
            }
            Tcl_GetIntFromObj(interp,objv,&curlData->transferText);
            break;
        case 23:
            if (SetoptInt(interp,curlHandle,CURLOPT_UPLOAD,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 24: /* The CURLOPT_MUTE option no longer does anything.*/
            break;
        case 25:
            if (SetoptChar(interp,curlHandle,CURLOPT_USERPWD,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 26:
            if (SetoptChar(interp,curlHandle,CURLOPT_PROXYUSERPWD,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case TCLCURLOPT_RANGE:
            if (SetoptChar(interp,curlHandle,CURLOPT_RANGE,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case TCLCURLOPT_ERRORBUFFER:
        {
            const char     *startPtr;
            const char     *endPtr;
            char           *tmpStr = NULL;

            tmpStr   = curlstrdup(Tcl_GetString(objv));
            regExp   = Tcl_RegExpCompile(interp,"(.*)(?:\\()(.*)(?:\\))");
            exitCode = Tcl_RegExpExec(interp,regExp,tmpStr,tmpStr);

            switch (exitCode) {
                case -1:
                    Tcl_Free((char *)tmpStr);
                    return TCL_ERROR;
                    break;
                case 0:
                    if (*tmpStr!=0) {
                        curlData->errorBufferName=curlstrdup(tmpStr);
                    } else {
                        curlData->errorBuffer=NULL;
                    }
                    curlData->errorBufferKey=NULL;
                    break;
                case 1:
                    Tcl_RegExpRange(regExp,1,&startPtr,&endPtr);
                    charLength=endPtr-startPtr;
                    curlData->errorBufferName=Tcl_Alloc(charLength+1);
                    strncpy(curlData->errorBufferName,startPtr,charLength);
                    curlData->errorBufferName[charLength]=0;
                    Tcl_RegExpRange(regExp,2,&startPtr,&endPtr);
                    charLength=endPtr-startPtr;
                    curlData->errorBufferKey=Tcl_Alloc(charLength+1);
                    strncpy(curlData->errorBufferKey,startPtr,charLength);
                    curlData->errorBufferKey[charLength]=0;
                    break;
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
            break;
        }
        case TCLCURLOPT_HTTPGET:
            if (SetoptLong(interp,curlHandle,CURLOPT_HTTPGET,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case TCLCURLOPT_POST:
            if (SetoptInt(interp,curlHandle,CURLOPT_POST,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case TCLCURLOPT_COPYPOSTFIELDS:
            if (SetoptChar(interp,curlHandle,CURLOPT_COPYPOSTFIELDS,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 32:
            if (SetoptCurlOffT(interp,curlHandle,CURLOPT_POSTFIELDSIZE_LARGE,
                        tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 33:
            if (SetoptChar(interp,curlHandle,
                    CURLOPT_FTPPORT,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 34:
            if (SetoptChar(interp,curlHandle,CURLOPT_COOKIE,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 35:
            if (SetoptChar(interp,curlHandle,CURLOPT_COOKIEFILE,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 36:
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
            break;
        case TCLCURLOPT_HTTPPOST:
        {
            Tcl_Size    i,j;
            Tcl_Size    post_data_numel;
            Tcl_Obj**   httpPostData;
            int         curlTableIndex;
            char*       tmpStr = NULL;

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
/*                        fprintf(stdout,"Section name: %s\n",Tcl_GetString(httpPostData[i+1]));*/
                        formArray[formArrayIndex].option = CURLFORM_COPYNAME;
                        formArray[formArrayIndex].value  = curlstrdup(Tcl_GetString(httpPostData[i+1]));
                        break;
                    case CONTENTS_HTTP_OPT:
                    {
/*                        fprintf(stdout,"Section contents: %s\n",Tcl_GetString(httpPostData[i+1]));*/
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
/*                        fprintf(stdout,"File name %d: %s\n",formArrayIndex,Tcl_GetString(httpPostData[i+1]));*/
                        formArray[formArrayIndex].option = CURLFORM_FILE;
                        formArray[formArrayIndex].value  = curlstrdup(Tcl_GetString(httpPostData[i+1]));
                        break;
                    case CONTENTTYPE_HTTP_OPT:
/*                        fprintf(stdout,"Data type: %s\n",Tcl_GetString(httpPostData[i+1]));*/
                        formArray[formArrayIndex].option = CURLFORM_CONTENTTYPE;
                        formArray[formArrayIndex].value  = curlstrdup(Tcl_GetString(httpPostData[i+1]));
                        break;
                    case CONTENTHEADER_HTTP_OPT:
/*                        fprintf(stdout,"ContentHeader: %s\n",Tcl_GetString(httpPostData[i+1]));*/
                        formArray[formArrayIndex].option = CURLFORM_CONTENTHEADER;
                        if(SetoptsList(interp,&newFormArray->formHeaderList,httpPostData[i+1])) {
                            curlErrorSetOpt(interp,configTable,tableIndex,"Header list invalid");
                            formaddError=1;
                            return TCL_ERROR;
                        }
                        formArray[formArrayIndex].value  = (char *)newFormArray->formHeaderList;
                        break;
                    case FILENAME_HTTP_OPT:
/*                        fprintf(stdout,"FileName: %s\n",Tcl_GetString(httpPostData[i+1])); */
                        formArray[formArrayIndex].option = CURLFORM_FILENAME;
                        formArray[formArrayIndex].value  = curlstrdup(Tcl_GetString(httpPostData[i+1]));
                        break;
                    case BUFFERNAME_HTTP_OPT:
/*                        fprintf(stdout,"BufferName: %s\n",Tcl_GetString(httpPostData[i+1])); */
                        formArray[formArrayIndex].option = CURLFORM_BUFFER;
                        formArray[formArrayIndex].value  = curlstrdup(Tcl_GetString(httpPostData[i+1]));
                        break;
                    case BUFFER_HTTP_OPT:
/*                        fprintf(stdout,"Buffer: %s\n",Tcl_GetString(httpPostData[i+1])); */
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
/*                        fprintf(stdout,"FileContent: %s\n",Tcl_GetString(httpPostData[i+1])); */
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
            break;
        }
        case 38:
            if (SetoptChar(interp,curlHandle,CURLOPT_SSLCERT,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 39:
            if (SetoptChar(interp,curlHandle,CURLOPT_SSLCERTPASSWD,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 40:
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
            break;
        case 41:
            if (SetoptInt(interp,curlHandle,CURLOPT_CRLF,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 42:
            if(SetoptsList(interp,&curlData->quote,objv)) {
                curlErrorSetOpt(interp,configTable,tableIndex,"quote list invalid");
                return TCL_ERROR;
            }
            if (curl_easy_setopt(curlHandle,CURLOPT_QUOTE,curlData->quote)) {
                curl_slist_free_all(curlData->quote);
                curlData->quote=NULL;
                return TCL_ERROR;
            }
            return TCL_OK;
            break;
        case 43:
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
            break;
        case 44:
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
            break;
        case 45:
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
            break;
        case 46:
            if (SetoptLong(interp,curlHandle,CURLOPT_TIMEVALUE,tableIndex,
                        objv)) {
                return TCL_ERROR;
            }
            break;
        case 47:
            if (SetoptChar(interp,curlHandle,CURLOPT_CUSTOMREQUEST,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 48:
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
            break;
        case 49:
            if (SetoptChar(interp,curlHandle,CURLOPT_INTERFACE,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 50:
        case 132:
            if (SetoptChar(interp,curlHandle,CURLOPT_KRBLEVEL,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 51:
            if (SetoptLong(interp,curlHandle,CURLOPT_SSL_VERIFYPEER,tableIndex,
                        objv)) {
                return TCL_ERROR;
            }
            break;
        case 52:
            if (SetoptChar(interp,curlHandle,CURLOPT_CAINFO,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 53:
            if (SetoptLong(interp,curlHandle,CURLOPT_FILETIME,tableIndex,
                        objv)) {
                return TCL_ERROR;
            }
            break;
        case 54:
            if (SetoptLong(interp,curlHandle,CURLOPT_MAXREDIRS,tableIndex,
                        objv)) {
                return TCL_ERROR;
            }
            break;
        case 55:
            if (SetoptLong(interp,curlHandle,CURLOPT_MAXCONNECTS,tableIndex,
                        objv)) {
                return TCL_ERROR;
            }
            break;
        case 56:
            curlErrorSetOpt(interp,configTable,tableIndex,"option is obsolete");
            return TCL_ERROR;
            break;
        case 57:
            if (SetoptChar(interp,curlHandle,CURLOPT_RANDOM_FILE,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 58:
            if (SetoptChar(interp,curlHandle,CURLOPT_EGDSOCKET,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 59:
            if (SetoptLong(interp,curlHandle,CURLOPT_CONNECTTIMEOUT,
                        tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 60:
            if (SetoptLong(interp,curlHandle,CURLOPT_NOPROGRESS,
                        tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 61:
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
            break;
        case 62:
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
            break;
        case 63:
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
            break;
        case 64:
            if (curlData->cancelTransVarName) {
                Tcl_UnlinkVar(curlData->interp,curlData->cancelTransVarName);
                Tcl_Free(curlData->cancelTransVarName);
            }
            curlData->cancelTransVarName=curlstrdup(Tcl_GetString(objv));
            Tcl_LinkVar(interp,curlData->cancelTransVarName,
                    (char *)&(curlData->cancelTrans),TCL_LINK_INT);
            break;
        case 65:
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
            break;
        case 66:
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
            break;
        case 67:
            if (SetoptLong(interp,curlHandle,CURLOPT_SSL_VERIFYHOST,
                        tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 68:
            if (SetoptChar(interp,curlHandle,CURLOPT_COOKIEJAR,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 69:
            if (SetoptChar(interp,curlHandle,CURLOPT_SSL_CIPHER_LIST,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 70:
        {
            char*   tmpStr = NULL;

            if (Tcl_GetIndexFromObj(interp, objv, httpVersionTable,
                "http version",TCL_EXACT,&tableIndex)==TCL_ERROR) {
                return TCL_ERROR;
            }
            if (curl_easy_setopt(curlHandle,CURLOPT_HTTP_VERSION,
                        tableIndex)) {
                tmpStr=curlstrdup(Tcl_GetString(objv));
                curlErrorSetOpt(interp,configTable,70,tmpStr);
                Tcl_Free(tmpStr);
                return TCL_ERROR;
            }
            break;
        }
        case 71:
            if (SetoptLong(interp,curlHandle,CURLOPT_FTP_USE_EPSV,
                        tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 72:
            if (SetoptChar(interp,curlHandle,CURLOPT_SSLCERTTYPE,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 73:
            if (SetoptChar(interp,curlHandle,CURLOPT_SSLKEY,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 74:
            if (SetoptChar(interp,curlHandle,CURLOPT_SSLKEYTYPE,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 135:
        case 75:
            if (SetoptChar(interp,curlHandle,CURLOPT_KEYPASSWD,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 76:
            if (SetoptChar(interp,curlHandle,CURLOPT_SSLENGINE,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 77:
            if (SetoptLong(interp,curlHandle,CURLOPT_SSLENGINE_DEFAULT,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 78:
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
            break;
        case 79:
            curlData->debugProc=curlstrdup(Tcl_GetString(objv));
            if (curl_easy_setopt(curlHandle,CURLOPT_DEBUGFUNCTION,
                    curlDebugProcInvoke)) {    
                return TCL_ERROR;
            }
            if (curl_easy_setopt(curlHandle,CURLOPT_DEBUGDATA,curlData)) {
                return TCL_ERROR;
            }
            break;
        case 80:
            if (SetoptLong(interp,curlHandle,CURLOPT_DNS_CACHE_TIMEOUT,
                        tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 81:
            if (SetoptLong(interp,curlHandle,CURLOPT_DNS_USE_GLOBAL_CACHE,
                        tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 82:
            if (SetoptLong(interp,curlHandle,CURLOPT_COOKIESESSION,
                        tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 83:
            if (SetoptChar(interp,curlHandle,CURLOPT_CAPATH,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 84:
            if (SetoptLong(interp,curlHandle,CURLOPT_BUFFERSIZE,
                        tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 85:
            if (SetoptLong(interp,curlHandle,CURLOPT_NOSIGNAL,
                        tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 86:
            if (Tcl_GetIndexFromObj(interp, objv, encodingTable,
                "encoding",TCL_EXACT,&tableIndex)==TCL_ERROR) {
                return TCL_ERROR;
            }
            if (tableIndex==2) {
                if (curl_easy_setopt(curlHandle,CURLOPT_ACCEPT_ENCODING,"")) {
                    curlErrorSetOpt(interp,configTable,86,"all");
                    return 1;
                }
            } else {
                if (SetoptChar(interp,curlHandle,CURLOPT_ACCEPT_ENCODING,86,objv)) {
                    return TCL_ERROR;
                }
            }
            break;
        case 87:
            if (Tcl_GetIndexFromObj(interp, objv, proxyTypeTable,
                "proxy type",TCL_EXACT,&tableIndex)==TCL_ERROR) {
                return TCL_ERROR;
            }
            switch(tableIndex) {
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
            break;
        case 88:
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
            break;
        case 89:
            if (SetoptInt(interp,curlHandle,CURLOPT_UNRESTRICTED_AUTH
                    ,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 90:
            if (SetoptLong(interp,curlHandle,CURLOPT_FTP_USE_EPRT,
                        tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 91:
            Tcl_Free(curlData->command);
            curlData->command=curlstrdup(Tcl_GetString(objv));
            break;
        case 92:
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
            break;
        case 93:
            if (SetoptLong(interp,curlHandle,CURLOPT_FTP_CREATE_MISSING_DIRS,
                        tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 94:
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
            break;
        case 95:
            if (SetoptLong(interp,curlHandle,CURLOPT_FTP_RESPONSE_TIMEOUT,
                        tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 96:
        {
            int curlTableIndex;

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
            break;
        }
        case 97:
            if (SetoptLong(interp,curlHandle,CURLOPT_MAXFILESIZE,
                        tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 98:
            if (SetoptChar(interp,curlHandle,CURLOPT_NETRC_FILE,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 99:
        case 138:
        {
            long longNumber = TclCurl_FTPSSLMethod(interp,objv);
            if (longNumber == -1) { return TCL_ERROR; }

            tmpObjPtr=Tcl_NewLongObj(longNumber);
            Tcl_IncrRefCount(tmpObjPtr);
            if (SetoptLong(interp,curlHandle,CURLOPT_USE_SSL,
                        tableIndex,tmpObjPtr)) {
                Tcl_DecrRefCount(tmpObjPtr);
                return TCL_ERROR;
            }
            Tcl_DecrRefCount(tmpObjPtr);
            break;
        }
        case 100:
            if (SetoptSHandle(interp,curlHandle,CURLOPT_SHARE,
                    tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 101:
            if (SetoptLong(interp,curlHandle,CURLOPT_PORT,
                        tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 102:
            if (SetoptLong(interp,curlHandle,CURLOPT_TCP_NODELAY,
                        tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 103:
            if (SetoptLong(interp,curlHandle,CURLOPT_AUTOREFERER,
                        tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 104:
            curlErrorSetOpt(interp,configTable,tableIndex,"option is obsolete");
            return TCL_ERROR;
            break;
        case 105:
            curlErrorSetOpt(interp,configTable,tableIndex,"option is obsolete");
            return TCL_ERROR;
            break;
        case 106:
            curlErrorSetOpt(interp,configTable,tableIndex,"option is obsolete");
            return TCL_ERROR;
            break;
        case 107:
            curlErrorSetOpt(interp,configTable,tableIndex,"option is obsolete, check '-ftpport'");
            return TCL_ERROR;
            break;
        case 108:
            curlErrorSetOpt(interp,configTable,tableIndex,"option is obsolete, check '-ftpport'");
            return TCL_ERROR;
            break;
        case 109:
            curlErrorSetOpt(interp,configTable,tableIndex,"option is obsolete");
            return TCL_ERROR;
            break;
        case 110:
            curlErrorSetOpt(interp,configTable,tableIndex,"option is obsolete");
            return TCL_ERROR;
            break;
        case 111:
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
            break;
        case 112:
            curlErrorSetOpt(interp,configTable,tableIndex,"option is obsolete");
            return TCL_ERROR;
            break;
        case 113:
            curlErrorSetOpt(interp,configTable,tableIndex,"option is obsolete");
            return TCL_ERROR;
            break;
        case 114:
            if (SetoptChar(interp,curlHandle,CURLOPT_FTP_ACCOUNT,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 115:
            if (SetoptLong(interp,curlHandle,CURLOPT_IGNORE_CONTENT_LENGTH,
                        tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 116:
            if (SetoptChar(interp,curlHandle,CURLOPT_COOKIELIST,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 117:
            if (SetoptLong(interp,curlHandle,CURLOPT_FTP_SKIP_PASV_IP,
                        tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 118:
            if (Tcl_GetIndexFromObj(interp, objv, ftpfilemethod,
                "ftp file method ",TCL_EXACT,&intNumber)==TCL_ERROR) {
                return TCL_ERROR;
            }
            switch(intNumber) {
                case 0:
                case 1:
                    longNumber=1;                /* FTPFILE_MULTICWD  */
                    break;
                case 2:
                    longNumber=2;                /* FTPFILE_NOCWD     */
                    break;
                case 3:
                    longNumber=3;                /* FTPFILE_SINGLECWD */
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
            break;
        case 119:
            if (SetoptLong(interp,curlHandle,CURLOPT_LOCALPORT,
                        tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 120:
            if (SetoptLong(interp,curlHandle,CURLOPT_LOCALPORTRANGE,
                        tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 121:
            if (SetoptCurlOffT(interp,curlHandle,CURLOPT_MAX_SEND_SPEED_LARGE,
                        tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
         case 122:
            if (SetoptCurlOffT(interp,curlHandle,CURLOPT_MAX_RECV_SPEED_LARGE,
                        tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 123:
            if (SetoptChar(interp,curlHandle,
                    CURLOPT_FTP_ALTERNATIVE_TO_USER,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 124:
            if (SetoptLong(interp,curlHandle,CURLOPT_SSL_SESSIONID_CACHE,
                        tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 125:
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
            break;
        case 126:
            if (SetoptChar(interp,curlHandle,CURLOPT_SSH_PUBLIC_KEYFILE,
                    tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 127:
            if (SetoptChar(interp,curlHandle,CURLOPT_SSH_PRIVATE_KEYFILE,
                    tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 128:
            if (SetoptLong(interp,curlHandle,CURLOPT_TIMEOUT_MS,
                        tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 129:
            if (SetoptLong(interp,curlHandle,CURLOPT_CONNECTTIMEOUT_MS,
                        tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 130:
            if (SetoptLong(interp,curlHandle,CURLOPT_HTTP_CONTENT_DECODING,
                        tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 131:
            if (SetoptLong(interp,curlHandle,CURLOPT_HTTP_TRANSFER_DECODING,
                        tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        /* 132 is together with case 50 */
        case 133:
            if (SetoptLong(interp,curlHandle,CURLOPT_NEW_FILE_PERMS,
                        tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 134:
            if (SetoptLong(interp,curlHandle,CURLOPT_NEW_DIRECTORY_PERMS,
                        tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        /* case 135 with 75, case 136 with 19, case 137 with 18 and case 138 with 99 */
        case 139:
        case 146:
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
            break;
        case 140:
            if (SetoptChar(interp,curlHandle,CURLOPT_SSH_HOST_PUBLIC_KEY_MD5,
                    tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 141:
            if (SetoptLong(interp,curlHandle,CURLOPT_PROXY_TRANSFER_MODE,
                        tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 142:
            if (SetoptChar(interp,curlHandle,CURLOPT_CRLFILE,
                    tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 143:
            if (SetoptChar(interp,curlHandle,CURLOPT_ISSUERCERT,
                    tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 144:
            if (SetoptLong(interp,curlHandle,CURLOPT_ADDRESS_SCOPE,
                        tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 145:
            if (SetoptLong(interp,curlHandle,CURLOPT_CERTINFO,
                        tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        /* case 146 is together with 139*/
        case 147:
            if (SetoptChar(interp,curlHandle,CURLOPT_USERNAME,
                    tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 148:
            if (SetoptChar(interp,curlHandle,CURLOPT_PASSWORD,
                    tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 149:
            if (SetoptChar(interp,curlHandle,CURLOPT_PROXYUSERNAME,
                    tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 150:
            if (SetoptChar(interp,curlHandle,CURLOPT_PROXYPASSWORD,
                    tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 151:
            if (SetoptLong(interp,curlHandle,CURLOPT_TFTP_BLKSIZE,
                        tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 152:
            if (SetoptChar(interp,curlHandle,CURLOPT_SOCKS5_GSSAPI_SERVICE,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 153:
            if (SetoptLong(interp,curlHandle,CURLOPT_SOCKS5_GSSAPI_NEC,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 154:
        case 155:
        {
            unsigned long int protocolMask;
            Tcl_Obj**         protocols;
            Tcl_Size          protocols_c;
            long              longNumber = 0;

            if (Tcl_ListObjGetElements(interp,objv,&protocols_c,&protocols) == TCL_ERROR) {
                return TCL_ERROR;
            }

            protocolMask = TclCurl_BuildProtocolMask(interp,protocols,protocols_c);

            tmpObjPtr=Tcl_NewLongObj(protocolMask);
            Tcl_IncrRefCount(tmpObjPtr);
            if (tableIndex == 154) {
                longNumber=CURLOPT_PROTOCOLS;
            } else {
                longNumber=CURLOPT_REDIR_PROTOCOLS;
            }
            if (SetoptLong(interp,curlHandle,longNumber,tableIndex,tmpObjPtr)) {
                Tcl_DecrRefCount(tmpObjPtr);
                return TCL_ERROR;
            }
            Tcl_DecrRefCount(tmpObjPtr);
            break;
        }
        case 156:
        {
            /* Select the Curl defined option value for the specified clear channel setting */

            long longNumber = TclCurl_FTPClearCommandChannelOpt(interp,objv);

            if (longNumber < 0) { return TCL_ERROR; }

            tmpObjPtr = Tcl_NewLongObj(longNumber);
            Tcl_IncrRefCount(tmpObjPtr);
            if (SetoptLong(interp,curlHandle,CURLOPT_FTP_SSL_CCC,tableIndex,tmpObjPtr)) {
                Tcl_DecrRefCount(tmpObjPtr);
                return TCL_ERROR;
            }
            Tcl_DecrRefCount(tmpObjPtr);
            break;
        }
        case 157:
            if (SetoptChar(interp,curlHandle,CURLOPT_SSH_KNOWNHOSTS,tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 158:
            if (curl_easy_setopt(curlHandle,CURLOPT_SSH_KEYFUNCTION,curlsshkeycallback)) {
                return TCL_ERROR;
            }
            if (curl_easy_setopt(curlHandle,CURLOPT_SSH_KEYDATA,curlData)) {
                return TCL_ERROR;
            }
            curlData->sshkeycallProc=curlstrdup(Tcl_GetString(objv));
            break;
        case 159:
            if (SetoptChar(interp,curlHandle,CURLOPT_MAIL_FROM,
                    tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 160:
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
            break;
        case 161:
            if (SetoptLong(interp,curlHandle,CURLOPT_FTP_USE_PRET,
                        tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 162:
            if (SetoptLong(interp,curlHandle,CURLOPT_WILDCARDMATCH,
                        tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 163:
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
            break;
        case 164:
            curlData->chunkBgnVar=curlstrdup(Tcl_GetString(objv));
            if (!strcmp(curlData->chunkBgnVar,"")) {
                curlErrorSetOpt(interp,configTable,tableIndex,"invalid var name");
                return TCL_ERROR;
            }
            break;
        case 165:
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
            break;
        case 166:
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
            break;
        case 167:
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
            break;
        case 168:
            if (SetoptChar(interp,curlHandle,CURLOPT_TLSAUTH_USERNAME,
                    tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 169:
            if (SetoptChar(interp,curlHandle,CURLOPT_TLSAUTH_PASSWORD,
                    tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 170:
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
            break;
        case 171:
            if (SetoptLong(interp,curlHandle,CURLOPT_TRANSFER_ENCODING,
                        tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case TCLCURLOPT_GSSAPI_DELEGATION:
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
            break;
        case 173:
            if (SetoptChar(interp,curlHandle,CURLOPT_NOPROXY,
                    tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
        case 174:
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
            break;
        case 175:
#if CURL_AT_LEAST_VERSION(7, 77, 0)
            if (SetoptBlob(interp,curlHandle,CURLOPT_CAINFO_BLOB,
                    tableIndex,objv)) {
                return TCL_ERROR;
            }
            break;
#else
            return TCL_ERROR;
#endif
    }
    return TCL_OK;
}

