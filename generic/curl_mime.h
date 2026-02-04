#ifndef __curl_mime_h__
#define __curl_mime_h__

typedef struct TclCurlOptionDef TclCurlOptionDef;
typedef struct TclCurlOptsArgs TclCurlOptsArgs;

int               TclCurl_SetPostData(Tcl_Interp *interp,struct curlObjData *curlDataPtr);
void              TclCurl_ResetPostData(struct curlObjData *curlDataPtr);
void              TclCurl_ResetFormArray(struct curl_forms *formArray);
/* Internal helper used by the setopts handler table. */
int               TclCurl_HandleHttpPost(TclCurlOptsArgs *args);

#endif
