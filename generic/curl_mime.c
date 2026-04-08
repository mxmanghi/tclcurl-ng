/*
 * curl_mime.c --
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

#include "tclcurl.h"
#include "curl_mime.h"
#include "curl_setopts.h"

enum curlFormIndices {
    NAME_HTTP_OPT, CONTENTS_HTTP_OPT, FILE_HTTP_OPT, 
    CONTENTTYPE_HTTP_OPT, CONTENTHEADER_HTTP_OPT, FILENAME_HTTP_OPT,
    BUFFERNAME_HTTP_OPT, BUFFER_HTTP_OPT, FILECONTENT_HTTP_OPT
};

const static char *curlFormTable[] = {
    "name",         "contents",         "file", 
    "contenttype",  "contentheader",    "filename",
    "bufferName",   "buffer",           "filecontent", (char *)NULL
};

/*----------------------------------------------------------------------
 *
 * TclCurl_SetPostData --
 *
 *  In case there is going to be a post transfer, this function sets the
 *  data that is going to be posted.
 *
 * Parameters:
 *	interp: Tcl interpreter we are using.
 *	curlData: A pointer to the struct with the transfer data.
 *
 * Results:
 *  A standard Tcl result.
 *----------------------------------------------------------------------
 */
int
TclCurl_SetPostData(Tcl_Interp *interp,struct curlObjData *curlDataPtr) {
    Tcl_Obj *errorMsgObjPtr;

    if (curlDataPtr->postFields != NULL) {
        if (curlDataPtr->postFieldSize >= 0) {
            if (curl_easy_setopt(curlDataPtr->curl,CURLOPT_POSTFIELDSIZE_LARGE,
                                 curlDataPtr->postFieldSize)) {
                errorMsgObjPtr = Tcl_NewStringObj("Error setting POST field size",-1);
                Tcl_SetObjResult(interp,errorMsgObjPtr);
                return TCL_ERROR;
            }
        }
        if (curl_easy_setopt(curlDataPtr->curl,CURLOPT_COPYPOSTFIELDS,
                             curlDataPtr->postFields)) {
            errorMsgObjPtr = Tcl_NewStringObj("Error setting the data to post",-1);
            Tcl_SetObjResult(interp,errorMsgObjPtr);
            return TCL_ERROR;
        }
    }

#ifdef CURL_PRE_7_56_DEPR
    if (curlDataPtr->postListFirst != NULL) {
        if (curl_easy_setopt(curlDataPtr->curl,CURLOPT_MIMEPOST,curlDataPtr->postListFirst)) {
            curl_formfree(curlDataPtr->postListFirst);
            errorMsgObjPtr = Tcl_NewStringObj("Error setting the data to post",-1);
            Tcl_SetObjResult(interp,errorMsgObjPtr);
            return TCL_ERROR;
        }
    }
#else
    if (curlDataPtr->mime != NULL) {
        if (curl_easy_setopt(curlDataPtr->curl,CURLOPT_MIMEPOST,curlDataPtr->mime)) {
            curl_mime_free(curlDataPtr->mime);
            curlDataPtr->mime = NULL;
            errorMsgObjPtr = Tcl_NewStringObj("Error setting the data to post",-1);
            Tcl_SetObjResult(interp,errorMsgObjPtr);
            return TCL_ERROR;
        }
    }
#endif
    return TCL_OK;
}

/*----------------------------------------------------------------------
 *
 * TclCurl_ResetPostData --
 *
 *  After performing a transfer, this function is invoked to erease the
 *  posr data.
 *
 * Parameter:
 *  curlData: A pointer to the struct with the transfer data.
 *----------------------------------------------------------------------
 */

#ifdef CURL_PRE_7_56_DEPR
void
TclCurl_ResetPostData(struct curlObjData *curlDataPtr) {
    struct formArrayStruct *tmpPtr;

    Tcl_Free(curlDataPtr->postFields);
    curlDataPtr->postFields = NULL;
    curlDataPtr->postFieldSize = -1;
    curl_easy_setopt(curlDataPtr->curl,CURLOPT_COPYPOSTFIELDS,NULL);
    curl_easy_setopt(curlDataPtr->curl,CURLOPT_POSTFIELDSIZE_LARGE,(curl_off_t)-1);

    if (curlDataPtr->postListFirst) {
        curl_formfree(curlDataPtr->postListFirst);
        curlDataPtr->postListFirst = NULL;
        curlDataPtr->postListLast  = NULL;
        curl_easy_setopt(curlDataPtr->curl,CURLOPT_MIMEPOST,NULL);

        while (curlDataPtr->formArray != NULL) {
            if (curlDataPtr->formArray->formHeaderList != NULL) {
                curl_slist_free_all(curlDataPtr->formArray->formHeaderList);
                curlDataPtr->formArray->formHeaderList = NULL;
            }
            curlResetFormArray(curlDataPtr->formArray->formArray);
            tmpPtr=curlDataPtr->formArray->next;
            Tcl_Free((char *)curlDataPtr->formArray);
            curlDataPtr->formArray=tmpPtr;
        }
    }
}

/*----------------------------------------------------------------------
 *
 * TclCurl_ResetFormArray --
 *
 *  Cleans the contents of the formArray, it is done after a transfer or
 *  if 'curl_formadd' returns an error.
 *
 * Parameter:
 *  formArray: A pointer to the array to clean up.
 *----------------------------------------------------------------------
 */
static void
TclCurl_ResetFormArray(struct curl_forms *formArray) {
    int i;

    for (i=0;formArray[i].option!=CURLFORM_END;i++) {
        switch (formArray[i].option) {
            case CURLFORM_COPYNAME:
            case CURLFORM_COPYCONTENTS:
            case CURLFORM_FILE:
            case CURLFORM_CONTENTTYPE:
            case CURLFORM_FILENAME:
            case CURLFORM_FILECONTENT:
            case CURLFORM_BUFFER:
            case CURLFORM_BUFFERPTR:
                Tcl_Free((char *)(formArray[i].value));
                break;
            default:
                break;
        }
    }
    Tcl_Free((char *)formArray);
}
#else
void
TclCurl_ResetPostData(struct curlObjData *curlDataPtr) {
    Tcl_Free(curlDataPtr->postFields);
    curlDataPtr->postFields = NULL;
    curlDataPtr->postFieldSize = -1;
    curl_easy_setopt(curlDataPtr->curl,CURLOPT_COPYPOSTFIELDS,NULL);
    curl_easy_setopt(curlDataPtr->curl,CURLOPT_POSTFIELDSIZE_LARGE,(curl_off_t)-1);

    if (curlDataPtr->mime != NULL) {
        curl_mime_free(curlDataPtr->mime);
        curlDataPtr->mime = NULL;
    }
    curl_easy_setopt(curlDataPtr->curl,CURLOPT_MIMEPOST,NULL);
}
#endif

/*----------------------------------------------------------------------
 *
 * TclCurl_HandleHttpPost --
 *
 *  Performs the heavyweght job of building the HTTP post request data
 *
 * Parameter:
 *  formArray: A pointer to the array to clean up.
 *----------------------------------------------------------------------
 */
#ifndef CURL_PRE_7_56_DEPR
int TclCurl_HandleHttpPost(TclCurlOptsArgs *coa)
{
    curl_mimepart*  part;
    Tcl_Size        post_data_numel;
    Tcl_Obj**       httpPostData;
    int             curlTableIndex;
    int             arg_p = 0;

    if (Tcl_ListObjGetElements(coa->interp,coa->objv,&post_data_numel,&httpPostData) == TCL_ERROR) {
        return TCL_ERROR;
    }

    if (coa->curlData->mime == NULL) {
        coa->curlData->mime = curl_mime_init(coa->curlData->curl);
        if (coa->curlData->mime == NULL) {
            curlErrorSetOpt(coa->interp,configTable,coa->curlOptsIndex,"Could not initialize MIME data");
            return TCL_ERROR;
        }
    }
    part = curl_mime_addpart(coa->curlData->mime);
    if (part == NULL) {
        curlErrorSetOpt(coa->interp,configTable,coa->curlOptsIndex,"Could not append MIME part");
        return TCL_ERROR;
    }

    while (arg_p < post_data_numel)
    {
        if (Tcl_GetIndexFromObj(coa->interp,httpPostData[arg_p],curlFormTable,
                "CURLFORM option",TCL_EXACT,&curlTableIndex) == TCL_ERROR) {
            /* TODO: add meaningful error information */
            return TCL_ERROR;
        }

        switch (curlTableIndex) {
            case NAME_HTTP_OPT:
            {
                Tcl_Size data_l;

                curl_mime_name(part,Tcl_GetStringFromObj(httpPostData[++arg_p],&data_l));
                break;
            }
            case CONTENTS_HTTP_OPT:
            {
                Tcl_Size    buffer_l;
                const char* tmpStr;
                size_t buffer_size;

                tmpStr = Tcl_GetStringFromObj(httpPostData[++arg_p],&buffer_l);
                if (TclCurl_TclSize2SizeT(buffer_l,&buffer_size) == 0) {
                    curlErrorSetOpt(coa->interp,configTable,coa->curlOptsIndex,"Inconsistent buffer size");
                    return TCL_ERROR;
                }
                curl_mime_data(part,tmpStr,buffer_size);
                break;
            }
            case FILE_HTTP_OPT:
            {
                Tcl_Size data_l;
                curl_mime_filedata(part,Tcl_GetStringFromObj(httpPostData[++arg_p],&data_l));
                break;
            }
            case CONTENTTYPE_HTTP_OPT:
            {
                Tcl_Size data_l;
                curl_mime_type(part,Tcl_GetStringFromObj(httpPostData[++arg_p],&data_l));
                break;
            }
            case CONTENTHEADER_HTTP_OPT:
            {
                struct curl_slist* curl_content_headers_l = NULL;

                if(SetoptsList(coa->interp,&curl_content_headers_l,httpPostData[++arg_p])) {
                    curlErrorSetOpt(coa->interp,configTable,coa->curlOptsIndex,"Header list invalid");
                    return TCL_ERROR;
                }

                curl_mime_headers(part,curl_content_headers_l,1);
                break;
            }
            case BUFFERNAME_HTTP_OPT:
            case FILENAME_HTTP_OPT:
            {
                Tcl_Size data_l;
                curl_mime_filename(part,Tcl_GetStringFromObj(httpPostData[++arg_p],&data_l));
                break;
            }
            case BUFFER_HTTP_OPT:
            {
                Tcl_Size    tcl_buffer_l;
                size_t      buffer_size;
                const char* tmpStr;

                tmpStr = (const char *) Tcl_GetByteArrayFromObj(httpPostData[++arg_p],&tcl_buffer_l);
                if (TclCurl_TclSize2SizeT(tcl_buffer_l,&buffer_size) == 0) {
                    curlErrorSetOpt(coa->interp,configTable,coa->curlOptsIndex,"Inconsistent buffer size");
                    return TCL_ERROR;
                }

                curl_mime_data(part,tmpStr,buffer_size);
                break;
            }
            case FILECONTENT_HTTP_OPT:
            {
                Tcl_Size data_l;
                curl_mime_filedata(part,Tcl_GetStringFromObj(httpPostData[++arg_p],&data_l));
                break;
            }
        }

        arg_p++;
    }

    return TCL_OK;
}
#else
int
TclCurl_HandleHttpPost(TclCurlOptsArgs *args)
{
    Tcl_Obj*                resultObjPtr;
    Tcl_Size                i,j;
    Tcl_Size                post_data_numel;
    Tcl_Obj**               httpPostData;
    int                     curlTableIndex;
    int                     formaddError,formArrayIndex;
    struct formArrayStruct* newFormArray;
    struct curl_forms*      formArray;
    Tcl_Size                curlformBufferSize;
    size_t                  contentslen;
    unsigned char*          tmpUStr;
    char*                   tmpStr = NULL;
    if (Tcl_ListObjGetElements(args->interp,args->objv,&post_data_numel,&httpPostData) == TCL_ERROR) {
        return TCL_ERROR;
    }
    formaddError = 0;
    newFormArray = (struct formArrayStruct *)Tcl_Alloc(sizeof(struct formArrayStruct));
    formArray    = (struct curl_forms *)Tcl_Alloc(post_data_numel*(sizeof(struct curl_forms)));
    formArrayIndex = 0;

    newFormArray->next=args->curlData->formArray;
    newFormArray->formArray=formArray;
    newFormArray->formHeaderList=NULL;

    for (i=0,j=0;i<post_data_numel;i+=2,j+=1) {
        if (Tcl_GetIndexFromObj(args->interp,httpPostData[i],curlFormTable,
                "CURLFORM option",TCL_EXACT,&curlTableIndex) == TCL_ERROR) {
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

                    if (TclCurl_TclSize2SizeT(curlformBufferSize,&buffer_size) == 0) {
                        curlErrorSetOpt(args->interp,configTable,args->tableIndex,"Inconsistent buffer size");
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
                if(SetoptsList(args->interp,&newFormArray->formHeaderList,httpPostData[i+1])) {
                    curlErrorSetOpt(args->interp,configTable,args->tableIndex,"Header list invalid");
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
    args->curlData->formArray=newFormArray;

    if (formaddError == 0) {
        formaddError=curl_formadd(&(args->curlData->postListFirst),
                                  &(args->curlData->postListLast), CURLFORM_ARRAY, formArray,
                                  CURLFORM_END);
    }
    if (formaddError != CURL_FORMADD_OK) {
        TclCurl_ResetFormArray(formArray);
        args->curlData->formArray=newFormArray->next;
        Tcl_Free((char *)newFormArray);
        resultObjPtr=Tcl_ObjPrintf("%d",formaddError);
        Tcl_SetObjResult(args->interp,resultObjPtr);
        Tcl_Free(tmpStr);
        return TCL_ERROR;
    }
    return TCL_OK;
}
#endif
