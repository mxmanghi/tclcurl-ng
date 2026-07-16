/*
 * multi.h --
 *
 * Header file for the part of the TclCurl extension that deals with libcurl's
 * 'multi' interface.
 *
 * Copyright (c) 2002-2011 Andres Garcia Garcia.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#ifndef __multi_h__
#define __multi_h__

#include "tclcurl.h"

#ifdef  __cplusplus
extern "C" {
#endif 

struct easyHandleList {
    CURL                    *curl;
    char                    *name;
    struct easyHandleList   *next;
};

struct curlMultiObjData {
    CURLM                 *mcurl;
    Tcl_Command            token;
    Tcl_Interp            *interp;
    struct easyHandleList *handleListFirst;
    struct easyHandleList *handleListLast;
    fd_set                 fdread;
    fd_set                 fdwrite;
    fd_set                 fdexcep;
    int                    runningTransfers;
    char                  *postCommand;    
};

struct curlEvent {
    Tcl_EventProc           *proc;
    struct Tcl_Event        *nextPtr;
    struct curlMultiObjData *curlMultiData;
};

#define TCLCURL_MULTI_COMMAND_LIST(X) \
    X(TCLCURL_MULTI_CMD_ADDHANDLE, "addhandle", TclCurl_MultiDispatchAddHandle) \
    X(TCLCURL_MULTI_CMD_REMOVEHANDLE, "removehandle", TclCurl_MultiDispatchRemoveHandle) \
    X(TCLCURL_MULTI_CMD_PERFORM, "perform", TclCurl_MultiDispatchPerform) \
    X(TCLCURL_MULTI_CMD_CLEANUP, "cleanup", TclCurl_MultiDispatchCleanup) \
    X(TCLCURL_MULTI_CMD_GETINFO, "getinfo", TclCurl_MultiDispatchGetInfo) \
    X(TCLCURL_MULTI_CMD_ACTIVE, "active", TclCurl_MultiDispatchActive) \
    X(TCLCURL_MULTI_CMD_AUTO, "auto", TclCurl_MultiDispatchAuto) \
    X(TCLCURL_MULTI_CMD_CONFIGURE, "configure", TclCurl_MultiDispatchConfigure)

typedef enum {
#define TCLCURL_MULTI_COMMAND_ENUM_ENTRY(name, label, proc) name,
    TCLCURL_MULTI_COMMAND_LIST(TCLCURL_MULTI_COMMAND_ENUM_ENTRY)
#undef TCLCURL_MULTI_COMMAND_ENUM_ENTRY
    TCLCURL_MULTI_CMD_COUNT
} TclCurlMultiCommandId;

const static char *multiCommandTable[TCLCURL_MULTI_CMD_COUNT + 1] = {
#define TCLCURL_MULTI_COMMAND_TABLE_ENTRY(name, label, proc) [name] = label,
    TCLCURL_MULTI_COMMAND_LIST(TCLCURL_MULTI_COMMAND_TABLE_ENTRY)
#undef TCLCURL_MULTI_COMMAND_TABLE_ENTRY
    NULL
};

#define TCLCURL_MULTI_CONFIG_LIST(X) \
    X(TCLCURL_MULTI_CONFIG_PIPELINING, "-pipelining", CURLMOPT_PIPELINING, SetMultiOptLong) \
    X(TCLCURL_MULTI_CONFIG_MAXCONNECTS, "-maxconnects", CURLMOPT_MAXCONNECTS, SetMultiOptLong)

typedef enum {
#define TCLCURL_MULTI_CONFIG_ENUM_ENTRY(name, label, opt, proc) name,
    TCLCURL_MULTI_CONFIG_LIST(TCLCURL_MULTI_CONFIG_ENUM_ENTRY)
#undef TCLCURL_MULTI_CONFIG_ENUM_ENTRY
    TCLCURL_MULTI_CONFIG_COUNT
} TclCurlMultiConfigId;

const static char *multiConfigTable[TCLCURL_MULTI_CONFIG_COUNT + 1] = {
#define TCLCURL_MULTI_CONFIG_TABLE_ENTRY(name, label, opt, proc) [name] = label,
    TCLCURL_MULTI_CONFIG_LIST(TCLCURL_MULTI_CONFIG_TABLE_ENTRY)
#undef TCLCURL_MULTI_CONFIG_TABLE_ENTRY
    NULL
};

char *curlCreateMultiObjCmd (Tcl_Interp *interp,struct curlMultiObjData *curlMultiData);
int Tclcurl_MultiInit (Tcl_Interp *interp);
int curlMultiDeleteCmd(ClientData clientData);

int curlInitMultiObjCmd (ClientData clientData, Tcl_Interp *interp,
        int objc,Tcl_Obj *const objv[]);

int curlMultiObjCmd (ClientData clientData, Tcl_Interp *interp,
    int objc,Tcl_Obj *const objv[]);

CURLMcode curlAddMultiHandle(Tcl_Interp *interp,CURLM *curlMultiHandle
        ,Tcl_Obj *objvPtr);

CURLMcode curlRemoveMultiHandle(Tcl_Interp *interp,CURLM *curlMultiHandle
        ,Tcl_Obj *objvPtr);

int curlMultiPerform(Tcl_Interp *interp,CURLM *curlMultiHandle);

int curlMultiGetInfo(Tcl_Interp *interp,CURLM *curlMultiHandlePtr);

int curlMultiGetActiveTransfers( struct curlMultiObjData *curlMultiData);
int curlMultiActiveTransfers(Tcl_Interp *interp, struct curlMultiObjData *curlMultiData);

struct curlObjData *curlGetEasyHandle(Tcl_Interp *interp,Tcl_Obj *nameObjPtr);

void curlMultiFreeSpace(struct curlMultiObjData *curlMultiData);

int curlReturnCURLMcode(Tcl_Interp *interp,CURLMcode errorCode);

void curlEasyHandleListAdd(struct curlMultiObjData *multiDataPtr,CURL *easyHandle,char *name);
void curlEasyHandleListRemove(struct curlMultiObjData *multiDataPtr,CURL *easyHandle);
char *curlGetEasyName(struct curlMultiObjData *multiDataPtr,CURL *easyHandle);

int curlMultiAutoTransfer(Tcl_Interp *interp, struct curlMultiObjData *curlMultiData, int objc,Tcl_Obj *const objv[]);
int curlMultiSetOpts(Tcl_Interp *interp, struct curlMultiObjData *curlMultiData,Tcl_Obj *const objv,int tableIndex);
int SetMultiOptLong(Tcl_Interp *interp,CURLM *curlMultiHandle,CURLMoption opt,
        int tableIndex,Tcl_Obj *tclObj);

int curlMultiConfigTransfer(Tcl_Interp *interp, struct curlMultiObjData *curlMultiData, int objc,Tcl_Obj *const objv[]);

void curlEventSetup(ClientData clientData, int flags);

void curlEventCheck(ClientData clientData, int flags);

int curlEventProc(Tcl_Event *evPtr,int flags);

#ifdef  __cplusplus
}
#endif

#endif /* __multi_h __ */
