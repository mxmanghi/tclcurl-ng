#ifndef __curl_mime_h__
#define __curl_mime_h__

typedef struct TclCurlOptionDef TclCurlOptionDef;

int               TclCurl_SetPostData(Tcl_Interp *interp,struct curlObjData *curlDataPtr);
void              TclCurl_ResetPostData(struct curlObjData *curlDataPtr);
void              TclCurl_ResetFormArray(struct curl_forms *formArray);
int               TclCurl_HandleHttpPost(Tcl_Interp *interp, struct curlObjData *curlData,
                                         Tcl_Obj *const objv, int tableIndex, const TclCurlOptionDef *def);

#endif
