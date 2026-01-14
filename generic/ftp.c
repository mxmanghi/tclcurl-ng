/*
 *
 */

#include <curl/curl.h>
#include <curl/easy.h>
#include "tclcompat.h"


const static char *ftpsslccc[] = {
    "none", "passive", "active", (char *)NULL
};

long  TclCurl_FTPClearCommandChannelOpt(Tcl_Interp* interp,Tcl_Obj* opt_o)
{
    int intNumber;

    if (Tcl_GetIndexFromObj(interp, opt_o, ftpsslccc, "Clear Command Channel option ",
                            TCL_EXACT, &intNumber) == TCL_ERROR) {
        return -1;
    }

    switch(intNumber) {
        case 0:
            return CURLFTPSSL_CCC_NONE;
        case 1:
            return CURLFTPSSL_CCC_PASSIVE;
        case 2:
            return CURLFTPSSL_CCC_ACTIVE;
    }

    /* It's not supposed to ever get here */

    return -1;
}

