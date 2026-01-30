/*
 * curl_mime.h --
 *
 * Header file for the TclCurl extension to enable Tcl interpreters
 * to access libcurl.
 *
 * Copyright (c) 2001-2011 Andres Garcia Garcia.
 * Copyright (c) 2026 Massimo Manghi
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include "tclcurl.h"
#include "curl_mime.h"
#include "curl_setopts.h"

const static char *curlFormTable[] = {
    "name",  "contents", "file", "contenttype", "contentheader", "filename",
    "bufferName", "buffer", "filecontent", (char *)NULL
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

    if (curlDataPtr->postListFirst != NULL) {
        if (curl_easy_setopt(curlDataPtr->curl,CURLOPT_MIMEPOST,curlDataPtr->postListFirst)) {
            curl_formfree(curlDataPtr->postListFirst);
            errorMsgObjPtr=Tcl_NewStringObj("Error setting the data to post",-1);
            Tcl_SetObjResult(interp,errorMsgObjPtr);
            return TCL_ERROR;
        }
    }
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
void
TclCurl_ResetPostData(struct curlObjData *curlDataPtr) {
    struct formArrayStruct *tmpPtr;

    if (curlDataPtr->postListFirst) {
        curl_formfree(curlDataPtr->postListFirst);
        curlDataPtr->postListFirst = NULL;
        curlDataPtr->postListLast  = NULL;
        curl_easy_setopt(curlDataPtr->curl,CURLOPT_MIMEPOST,NULL);

        while (curlDataPtr->formArray!=NULL) {
            if (curlDataPtr->formArray->formHeaderList!=NULL) {
                curl_slist_free_all(curlDataPtr->formArray->formHeaderList);
                curlDataPtr->formArray->formHeaderList=NULL;
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
void
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
int
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
    formaddError = 0;
    newFormArray = (struct formArrayStruct *)Tcl_Alloc(sizeof(struct formArrayStruct));
    formArray = (struct curl_forms *)Tcl_Alloc(post_data_numel*(sizeof(struct curl_forms)));
    formArrayIndex = 0;

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

                    if (TclCurl_TclSize2SizeT(curlformBufferSize,&buffer_size) == 0) {
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
        formaddError=curl_formadd(&(curlData->postListFirst),
                                  &(curlData->postListLast), CURLFORM_ARRAY, formArray,
                                  CURLFORM_END);
    }
    if (formaddError != CURL_FORMADD_OK) {
        TclCurl_ResetFormArray(formArray);
        curlData->formArray=newFormArray->next;
        Tcl_Free((char *)newFormArray);
        resultObjPtr=Tcl_ObjPrintf("%d",formaddError);
        Tcl_SetObjResult(interp,resultObjPtr);
        Tcl_Free(tmpStr);
        return TCL_ERROR;
    }
    return TCL_OK;
}
