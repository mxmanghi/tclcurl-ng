/*
 * curl_setopts.h --
 *
 * Definition and central X-macro based table of symbols and functions for
 * the CLI option management. 
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

#ifndef __curl_setopts_h__
#define __curl_setopts_h__

#include "curl_mime.h"

#ifdef CURL_PRE_7_84_DEPR
#define TCLCURL_OPTION_LIST_PRE_7_84_DEPR(entry) entry
#else
#define TCLCURL_OPTION_LIST_PRE_7_84_DEPR(entry) \

#endif

#ifdef CURL_PRE_7_11_1_DEPR
#define TCLCURL_OPTION_LIST_PRE_7_11_1_DEPR(entry) entry
#else
#define TCLCURL_OPTION_LIST_PRE_7_11_1_DEPR(entry) \

#endif

#ifdef CURL_PRE_8_17_0_DEPR
#define TCLCURL_OPTION_LIST_PRE_8_17_0_DEPR(entry) entry
#else
#define TCLCURL_OPTION_LIST_PRE_8_17_0_DEPR(entry) \

#endif

#define TCLCURL_OPTION_LIST(X) \
    X(TCLCURLOPT_URL,"CURLOPT_URL","-url",TclCurl_HandleSetoptChar,CURLOPT_URL,NULL) \
    X(TCLCURLOPT_FILE,"CURLOPT_FILE","-file",TclCurl_HandleOutFile,0,NULL) \
    X(TCLCURLOPT_READDATA,"CURLOPT_READDATA","-infile",TclCurl_HandleReadData,0,NULL) \
    X(TCLCURLOPT_USERAGENT,"CURLOPT_USERAGENT","-useragent",TclCurl_HandleSetoptChar,CURLOPT_USERAGENT,NULL) \
    X(TCLCURLOPT_REFERER,"CURLOPT_REFERER","-referer",TclCurl_HandleSetoptChar,CURLOPT_REFERER,NULL) \
    X(TCLCURLOPT_VERBOSE,"CURLOPT_VERBOSE","-verbose",TclCurl_HandleSetoptInt,CURLOPT_VERBOSE,NULL) \
    X(TCLCURLOPT_HEADER,"CURLOPT_HEADER","-header",TclCurl_HandleSetoptInt,CURLOPT_HEADER,NULL) \
    X(TCLCURLOPT_NOBODY,"CURLOPT_NOBODY","-nobody",TclCurl_HandleSetoptInt,CURLOPT_NOBODY,NULL) \
    X(TCLCURLOPT_PROXY,"CURLOPT_PROXY","-proxy",TclCurl_HandleSetoptChar,CURLOPT_PROXY,NULL) \
    X(TCLCURLOPT_PROXYPORT,"CURLOPT_PROXYPORT","-proxyport",TclCurl_HandleSetoptLong,CURLOPT_PROXYPORT,NULL) \
    X(TCLCURLOPT_HTTPPROXYTUNNEL,"CURLOPT_HTTPPROXYTUNNEL","-httpproxytunnel",TclCurl_HandleSetoptInt,CURLOPT_HTTPPROXYTUNNEL,NULL) \
    X(TCLCURLOPT_FAILONERROR,"CURLOPT_FAILONERROR","-failonerror",TclCurl_HandleSetoptInt,CURLOPT_FAILONERROR,NULL) \
    X(TCLCURLOPT_TIMEOUT,"CURLOPT_TIMEOUT","-timeout",TclCurl_HandleSetoptLong,CURLOPT_TIMEOUT,NULL) \
    X(TCLCURLOPT_LOW_SPEED_LIMIT,"CURLOPT_LOW_SPEED_LIMIT","-lowspeedlimit",TclCurl_HandleSetoptLong,CURLOPT_LOW_SPEED_LIMIT,NULL) \
    X(TCLCURLOPT_LOW_SPEED_TIME,"CURLOPT_LOW_SPEED_TIME","-lowspeedtime",TclCurl_HandleSetoptLong,CURLOPT_LOW_SPEED_TIME,NULL) \
    X(TCLCURLOPT_RESUME_FROM,"CURLOPT_RESUME_FROM","-resumefrom",TclCurl_HandleSetoptLong,CURLOPT_RESUME_FROM,NULL) \
    X(TCLCURLOPT_INFILESIZE,"CURLOPT_INFILESIZE","-infilesize",TclCurl_HandleSetoptLong,CURLOPT_INFILESIZE,NULL) \
    X(TCLCURLOPT_UPLOAD,"CURLOPT_UPLOAD","-upload",TclCurl_HandleSetoptInt,CURLOPT_UPLOAD,NULL) \
    X(TCLCURLOPT_FTPLISTONLY,"CURLOPT_FTPLISTONLY","-ftplistonly",TclCurl_HandleSetoptInt,CURLOPT_DIRLISTONLY,NULL) \
    X(TCLCURLOPT_FTPAPPEND,"CURLOPT_FTPAPPEND","-ftpappend",TclCurl_HandleSetoptInt,CURLOPT_APPEND,NULL) \
    X(TCLCURLOPT_NETRC,"CURLOPT_NETRC","-netrc",TclCurl_HandleNetrc,0,NULL) \
    X(TCLCURLOPT_FOLLOWLOCATION,"CURLOPT_FOLLOWLOCATION","-followlocation",TclCurl_HandleSetoptInt,CURLOPT_FOLLOWLOCATION,NULL) \
    X(TCLCURLOPT_TRANSFERTEXT,"CURLOPT_TRANSFERTEXT","-transfertext",TclCurl_HandleTransferText,CURLOPT_TRANSFERTEXT,NULL) \
    X(TCLCURLOPT_PUT,"CURLOPT_PUT","-put",TclCurl_HandleSetoptInt,CURLOPT_UPLOAD,NULL) \
    X(TCLCURLOPT_MUTE,"CURLOPT_MUTE","-mute",TclCurl_HandleMute,0,NULL) \
    X(TCLCURLOPT_USERPWD,"CURLOPT_USERPWD","-userpwd",TclCurl_HandleSetoptChar,CURLOPT_USERPWD,NULL) \
    X(TCLCURLOPT_PROXYUSERPWD, "CURLOPT_PROXYUSERPWD","-proxyuserpwd", TclCurl_HandleSetoptChar, CURLOPT_PROXYUSERPWD, NULL) \
    X(TCLCURLOPT_RANGE,"CURLOPT_RANGE","-range",TclCurl_HandleSetoptChar,CURLOPT_RANGE,NULL) \
    X(TCLCURLOPT_ERRORBUFFER,"CURLOPT_ERRORBUFFER","-errorbuffer",TclCurl_HandleErrorBuffer,0,NULL) \
    X(TCLCURLOPT_HTTPGET,"CURLOPT_HTTPGET","-httpget",TclCurl_HandleSetoptLong,CURLOPT_HTTPGET,NULL) \
    X(TCLCURLOPT_POST,"CURLOPT_POST","-post",TclCurl_HandleSetoptInt,CURLOPT_POST,NULL) \
    X(TCLCURLOPT_POSTFIELDS,"CURLOPT_POSTFIELDS","-postfields",TclCurl_HandlePostFields,0,NULL) \
    X(TCLCURLOPT_POSTFIELDSIZE_LARGE,"CURLOPT_POSTFIELDSIZE_LARGE","-postfieldsize",TclCurl_HandlePostFieldSize,0,NULL) \
    X(TCLCURLOPT_FTPPORT,"CURLOPT_FTPPORT","-ftpport",TclCurl_HandleSetoptChar,CURLOPT_FTPPORT,NULL) \
    X(TCLCURLOPT_COOKIE,"CURLOPT_COOKIE","-cookie",TclCurl_HandleSetoptChar,CURLOPT_COOKIE,NULL) \
    X(TCLCURLOPT_COOKIEFILE,"CURLOPT_COOKIEFILE","-cookiefile",TclCurl_HandleSetoptChar,CURLOPT_COOKIEFILE,NULL) \
    X(TCLCURLOPT_HTTPHEADER,"CURLOPT_HTTPHEADER","-httpheader",TclCurl_HandleHttpHeaderList,0,NULL) \
    X(TCLCURLOPT_HTTPPOST,"CURLOPT_HTTPPOST","-httppost",TclCurl_HandleHttpPost,0,NULL) \
    X(TCLCURLOPT_SSLCERT,"CURLOPT_SSLCERT","-sslcert",TclCurl_HandleSetoptChar,CURLOPT_SSLCERT,NULL) \
    X(TCLCURLOPT_SSLCERTPASSWD, "CURLOPT_SSLCERTPASSWD","-sslcertpasswd",TclCurl_HandleSetoptChar,CURLOPT_SSLCERTPASSWD,NULL) \
    X(TCLCURLOPT_SSLVERSION, "CURLOPT_SSLVERSION", "-sslversion", TclCurl_HandleSslVersion,0,NULL) \
    X(TCLCURLOPT_CRLF,"CURLOPT_CRLF","-crlf",TclCurl_HandleSetoptInt,CURLOPT_CRLF,NULL) \
    X(TCLCURLOPT_QUOTE,"CURLOPT_QUOTE","-quote",TclCurl_HandleQuoteList,0,NULL) \
    X(TCLCURLOPT_POSTQUOTE,"CURLOPT_POSTQUOTE","-postquote",TclCurl_HandlePostQuoteList,0,NULL) \
    X(TCLCURLOPT_WRITEHEADER,"CURLOPT_WRITEHEADER","-writeheader",TclCurl_HandleWriteHeader,0,NULL) \
    X(TCLCURLOPT_TIMECONDITION,"CURLOPT_TIMECONDITION","-timecondition",TclCurl_HandleTimeCondition,0,NULL) \
    X(TCLCURLOPT_TIMEVALUE,"CURLOPT_TIMEVALUE","-timevalue",TclCurl_HandleSetoptLong,CURLOPT_TIMEVALUE,NULL) \
    X(TCLCURLOPT_CUSTOMREQUEST,"CURLOPT_CUSTOMREQUEST","-customrequest",TclCurl_HandleSetoptChar,CURLOPT_CUSTOMREQUEST,NULL) \
    X(TCLCURLOPT_STDERR,"CURLOPT_STDERR","-stderr",TclCurl_HandleStderrFile,0,NULL) \
    X(TCLCURLOPT_INTERFACE,"CURLOPT_INTERFACE","-interface",TclCurl_HandleSetoptChar,CURLOPT_INTERFACE,NULL) \
    TCLCURL_OPTION_LIST_PRE_8_17_0_DEPR(X(TCLCURLOPT_KRB4LEVEL,"CURLOPT_KRB4LEVEL","-krb4level",TclCurl_HandleSetoptChar,CURLOPT_KRBLEVEL,NULL)) \
    X(TCLCURLOPT_SSL_VERIFYPEER,"CURLOPT_SSL_VERIFYPEER","-sslverifypeer",TclCurl_HandleSetoptLong,CURLOPT_SSL_VERIFYPEER,NULL) \
    X(TCLCURLOPT_CAINFO,"CURLOPT_CAINFO","-cainfo",TclCurl_HandleSetoptChar,CURLOPT_CAINFO,NULL) \
    X(TCLCURLOPT_FILETIME,"CURLOPT_FILETIME","-filetime",TclCurl_HandleSetoptLong,CURLOPT_FILETIME,NULL) \
    X(TCLCURLOPT_MAXREDIRS,"CURLOPT_MAXREDIRS","-maxredirs",TclCurl_HandleSetoptLong,CURLOPT_MAXREDIRS,NULL) \
    X(TCLCURLOPT_MAXCONNECTS,"CURLOPT_MAXCONNECTS","-maxconnects",TclCurl_HandleSetoptLong,CURLOPT_MAXCONNECTS,NULL) \
    X(TCLCURLOPT_CLOSEPOLICY,"CURLOPT_CLOSEPOLICY","-closepolicy",TclCurl_HandleObsolete,0,"option is obsolete") \
    TCLCURL_OPTION_LIST_PRE_7_84_DEPR(X(TCLCURLOPT_RANDOM_FILE,"CURLOPT_RANDOM_FILE","-randomfile",TclCurl_HandleSetoptChar,CURLOPT_RANDOM_FILE,NULL)) \
    TCLCURL_OPTION_LIST_PRE_7_84_DEPR(X(TCLCURLOPT_EGDSOCKET,"CURLOPT_EGDSOCKET","-egdsocket",TclCurl_HandleSetoptChar,CURLOPT_EGDSOCKET,NULL)) \
    X(TCLCURLOPT_CONNECTTIMEOUT,"CURLOPT_CONNECTTIMEOUT","-connecttimeout",TclCurl_HandleSetoptLong,CURLOPT_CONNECTTIMEOUT,NULL) \
    X(TCLCURLOPT_NOPROGRESS,"CURLOPT_NOPROGRESS","-noprogress",TclCurl_HandleSetoptLong,CURLOPT_NOPROGRESS,NULL) \
    X(TCLCURLOPT_HEADERVAR,"CURLOPT_HEADERVAR","-headervar",TclCurl_HandleHeaderVar,0,NULL) \
    X(TCLCURLOPT_BODYVAR,"CURLOPT_BODYVAR","-bodyvar",TclCurl_HandleBodyVar,0,NULL) \
    X(TCLCURLOPT_PROGRESSPROC,"CURLOPT_PROGRESSPROC","-progressproc",TclCurl_HandleProgressProc,0,NULL) \
    X(TCLCURLOPT_CANCELTRANSVARNAME,"CURLOPT_CANCELTRANSVARNAME","-canceltransvarname",TclCurl_HandleCancelTransVarName,0,NULL) \
    X(TCLCURLOPT_WRITEPROC,"CURLOPT_WRITEPROC","-writeproc",TclCurl_HandleWriteProc,0,NULL) \
    X(TCLCURLOPT_READPROC,"CURLOPT_READPROC","-readproc",TclCurl_HandleReadProc,0,NULL) \
    X(TCLCURLOPT_SSL_VERIFYHOST,"CURLOPT_SSL_VERIFYHOST","-sslverifyhost",TclCurl_HandleSetoptLong,CURLOPT_SSL_VERIFYHOST,NULL) \
    X(TCLCURLOPT_COOKIEJAR,"CURLOPT_COOKIEJAR","-cookiejar",TclCurl_HandleSetoptChar, CURLOPT_COOKIEJAR, NULL) \
    X(TCLCURLOPT_SSL_CIPHER_LIST, "CURLOPT_SSL_CIPHER_LIST","-sslcipherlist",TclCurl_HandleSetoptChar,CURLOPT_SSL_CIPHER_LIST,NULL) \
    X(TCLCURLOPT_HTTP_VERSION, "CURLOPT_HTTP_VERSION", "-httpversion", TclCurl_HandleHttpVersion, CURLOPT_HTTP_VERSION, NULL) \
    X(TCLCURLOPT_FTP_USE_EPSV, "CURLOPT_FTP_USE_EPSV", "-ftpuseepsv", TclCurl_HandleSetoptLong, CURLOPT_FTP_USE_EPSV, NULL) \
    X(TCLCURLOPT_SSLCERTTYPE, "CURLOPT_SSLCERTTYPE", "-sslcerttype", TclCurl_HandleSetoptChar, CURLOPT_SSLCERTTYPE, NULL) \
    X(TCLCURLOPT_SSLKEY, "CURLOPT_SSLKEY", "-sslkey", TclCurl_HandleSetoptChar, CURLOPT_SSLKEY, NULL) \
    X(TCLCURLOPT_SSLKEYTYPE, "CURLOPT_SSLKEYTYPE", "-sslkeytype", TclCurl_HandleSetoptChar, CURLOPT_SSLKEYTYPE, NULL) \
    X(TCLCURLOPT_SSLKEYPASSWD, "CURLOPT_SSLKEYPASSWD", "-sslkeypasswd", TclCurl_HandleSetoptChar, CURLOPT_KEYPASSWD, NULL) \
    X(TCLCURLOPT_SSL_ENGINE, "CURLOPT_SSL_ENGINE", "-sslengine", TclCurl_HandleSetoptChar, CURLOPT_SSLENGINE, NULL) \
    X(TCLCURLOPT_SSL_ENGINEDEFAULT, "CURLOPT_SSL_ENGINEDEFAULT", "-sslenginedefault", TclCurl_HandleSetoptLong, CURLOPT_SSLENGINE_DEFAULT, NULL) \
    X(TCLCURLOPT_PREQUOTE, "CURLOPT_PREQUOTE", "-prequote", TclCurl_HandlePrequoteList, 0, NULL) \
    X(TCLCURLOPT_DEBUGPROC, "CURLOPT_DEBUGPROC", "-debugproc", TclCurl_HandleDebugProc, 0, NULL) \
    X(TCLCURLOPT_DNS_CACHE_TIMEOUT, "CURLOPT_DNS_CACHE_TIMEOUT", "-dnscachetimeout", TclCurl_HandleSetoptLong, CURLOPT_DNS_CACHE_TIMEOUT, NULL) \
    TCLCURL_OPTION_LIST_PRE_7_11_1_DEPR(X(TCLCURLOPT_DNS_USE_GLOBAL_CACHE,"CURLOPT_DNS_USE_GLOBAL_CACHE","-dnsuseglobalcache",TclCurl_HandleSetoptLong,CURLOPT_DNS_USE_GLOBAL_CACHE,NULL)) \
    X(TCLCURLOPT_COOKIESESSION, "CURLOPT_COOKIESESSION", "-cookiesession", TclCurl_HandleSetoptLong, CURLOPT_COOKIESESSION, NULL) \
    X(TCLCURLOPT_CAPATH, "CURLOPT_CAPATH", "-capath", TclCurl_HandleSetoptChar, CURLOPT_CAPATH, NULL) \
    X(TCLCURLOPT_BUFFERSIZE, "CURLOPT_BUFFERSIZE", "-buffersize", TclCurl_HandleSetoptLong, CURLOPT_BUFFERSIZE, NULL) \
    X(TCLCURLOPT_NOSIGNAL, "CURLOPT_NOSIGNAL", "-nosignal", TclCurl_HandleSetoptLong, CURLOPT_NOSIGNAL, NULL) \
    X(TCLCURLOPT_ENCODING, "CURLOPT_ENCODING", "-encoding", TclCurl_HandleEncoding, CURLOPT_ACCEPT_ENCODING, NULL) \
    X(TCLCURLOPT_PROXYTYPE, "CURLOPT_PROXYTYPE", "-proxytype", TclCurl_HandleProxyType, 0, NULL) \
    X(TCLCURLOPT_HTTP200ALIASES, "CURLOPT_HTTP200ALIASES", "-http200aliases", TclCurl_HandleHttp200Aliases, 0, NULL) \
    X(TCLCURLOPT_UNRESTRICTED_AUTH, "CURLOPT_UNRESTRICTED_AUTH", "-unrestrictedauth", TclCurl_HandleSetoptInt, CURLOPT_UNRESTRICTED_AUTH, NULL) \
    X(TCLCURLOPT_FTP_USE_EPRT, "CURLOPT_FTP_USE_EPRT", "-ftpuseeprt", TclCurl_HandleSetoptLong, CURLOPT_FTP_USE_EPRT, NULL) \
    X(TCLCURLOPT_NOSUCHOPTION, "CURLOPT_NOSUCHOPTION", "-command", TclCurl_HandleCommand, 0, NULL) \
    X(TCLCURLOPT_HTTPAUTH, "CURLOPT_HTTPAUTH", "-httpauth", TclCurl_HandleHttpAuth, 0, NULL) \
    X(TCLCURLOPT_FTP_CREATE_MISSING_DIRS,"CURLOPT_FTP_CREATE_MISSING_DIRS","-ftpcreatemissingdirs",TclCurl_HandleSetoptLong,CURLOPT_FTP_CREATE_MISSING_DIRS, NULL) \
    X(TCLCURLOPT_PROXYAUTH, "CURLOPT_PROXYAUTH", "-proxyauth", TclCurl_HandleProxyAuth, 0, NULL) \
    X(TCLCURLOPT_FTP_RESPONSE_TIMEOUT,"CURLOPT_FTP_RESPONSE_TIMEOUT","-ftpresponsetimeout",TclCurl_HandleSetoptLong,CURLOPT_FTP_RESPONSE_TIMEOUT,NULL) \
    X(TCLCURLOPT_IPRESOLVE, "CURLOPT_IPRESOLVE", "-ipresolve", TclCurl_HandleIpResolve, 0, NULL) \
    X(TCLCURLOPT_MAXFILESIZE, "CURLOPT_MAXFILESIZE", "-maxfilesize", TclCurl_HandleSetoptLong, CURLOPT_MAXFILESIZE, NULL) \
    X(TCLCURLOPT_NETRC_FILE, "CURLOPT_NETRC_FILE", "-netrcfile", TclCurl_HandleSetoptChar, CURLOPT_NETRC_FILE, NULL) \
    X(TCLCURLOPT_FTP_SSL, "CURLOPT_FTP_SSL", "-ftpssl", TclCurl_HandleFtpSsl, CURLOPT_USE_SSL, NULL) \
    X(TCLCURLOPT_SHARE, "CURLOPT_SHARE", "-share", TclCurl_HandleSetoptSHandle, CURLOPT_SHARE, NULL) \
    X(TCLCURLOPT_PORT, "CURLOPT_PORT", "-port", TclCurl_HandleSetoptLong, CURLOPT_PORT, NULL) \
    X(TCLCURLOPT_TCP_NODELAY, "CURLOPT_TCP_NODELAY", "-tcpnodelay",TclCurl_HandleSetoptLong,CURLOPT_TCP_NODELAY, NULL) \
    X(TCLCURLOPT_AUTOREFERER, "CURLOPT_AUTOREFERER", "-autoreferer",TclCurl_HandleSetoptLong,CURLOPT_AUTOREFERER, NULL) \
    X(TCLCURLOPT_SOURCE_HOST, "CURLOPT_SOURCE_HOST", "-sourcehost", TclCurl_HandleObsolete, 0, "option is obsolete") \
    X(TCLCURLOPT_SOURCE_USERPWD, "CURLOPT_SOURCE_USERPWD", "-sourceuserpwd", TclCurl_HandleObsolete, 0, "option is obsolete") \
    X(TCLCURLOPT_SOURCE_PATH, "CURLOPT_SOURCE_PATH", "-sourcepath", TclCurl_HandleObsolete, 0, "option is obsolete") \
    X(TCLCURLOPT_SOURCE_PORT, "CURLOPT_SOURCE_PORT", "-sourceport", TclCurl_HandleObsolete, 0, "option is obsolete, check '-ftpport'") \
    X(TCLCURLOPT_PASV_HOST, "CURLOPT_PASV_HOST", "-pasvhost", TclCurl_HandleObsolete, 0, "option is obsolete, check '-ftpport'") \
    X(TCLCURLOPT_SOURCE_PREQUOTE, "CURLOPT_SOURCE_PREQUOTE", "-sourceprequote", TclCurl_HandleObsolete, 0, "option is obsolete") \
    X(TCLCURLOPT_SOURCE_POSTQUOTE, "CURLOPT_SOURCE_POSTQUOTE", "-sourcepostquote", TclCurl_HandleObsolete, 0, "option is obsolete") \
    X(TCLCURLOPT_FTPSSLAUTH, "CURLOPT_FTPSSLAUTH", "-ftpsslauth", TclCurl_HandleFtpSslAuth, 0, NULL) \
    X(TCLCURLOPT_SOURCE_URL, "CURLOPT_SOURCE_URL", "-sourceurl", TclCurl_HandleObsolete, 0, "option is obsolete") \
    X(TCLCURLOPT_SOURCE_QUOTE, "CURLOPT_SOURCE_QUOTE", "-sourcequote", TclCurl_HandleObsolete, 0, "option is obsolete") \
    X(TCLCURLOPT_FTP_ACCOUNT, "CURLOPT_FTP_ACCOUNT", "-ftpaccount", TclCurl_HandleSetoptChar, CURLOPT_FTP_ACCOUNT, NULL) \
    X(TCLCURLOPT_IGNORE_CONTENT_LENGTH,"CURLOPT_IGNORE_CONTENT_LENGTH","-ignorecontentlength",TclCurl_HandleSetoptLong,CURLOPT_IGNORE_CONTENT_LENGTH,NULL) \
    X(TCLCURLOPT_COOKIELIST,"CURLOPT_COOKIELIST","-cookielist",TclCurl_HandleSetoptChar,CURLOPT_COOKIELIST,NULL) \
    X(TCLCURLOPT_FTP_SKIP_PASV_IP,"CURLOPT_FTP_SKIP_PASV_IP","-ftpskippasvip",TclCurl_HandleSetoptLong,CURLOPT_FTP_SKIP_PASV_IP,NULL) \
    X(TCLCURLOPT_FTP_FILEMETHOD,"CURLOPT_FTP_FILEMETHOD","-ftpfilemethod",TclCurl_HandleFtpFileMethod,0,NULL) \
    X(TCLCURLOPT_LOCALPORT,"CURLOPT_LOCALPORT","-localport",TclCurl_HandleSetoptLong,CURLOPT_LOCALPORT,NULL) \
    X(TCLCURLOPT_LOCALPORTRANGE,"CURLOPT_LOCALPORTRANGE","-localportrange",TclCurl_HandleSetoptLong,CURLOPT_LOCALPORTRANGE, NULL) \
    X(TCLCURLOPT_MAX_SEND_SPEED_LARGE, "CURLOPT_MAX_SEND_SPEED_LARGE", "-maxsendspeed", TclCurl_HandleSetoptCurlOffT, CURLOPT_MAX_SEND_SPEED_LARGE, NULL) \
    X(TCLCURLOPT_MAX_RECV_SPEED_LARGE, "CURLOPT_MAX_RECV_SPEED_LARGE", "-maxrecvspeed", TclCurl_HandleSetoptCurlOffT, CURLOPT_MAX_RECV_SPEED_LARGE, NULL) \
    X(TCLCURLOPT_FTP_ALTERNATIVE_TO_USER, "CURLOPT_FTP_ALTERNATIVE_TO_USER", "-ftpalternativetouser", TclCurl_HandleSetoptChar, CURLOPT_FTP_ALTERNATIVE_TO_USER, NULL) \
    X(TCLCURLOPT_SSL_SESSIONID_CACHE,"CURLOPT_SSL_SESSIONID_CACHE","-sslsessionidcache",TclCurl_HandleSetoptLong,CURLOPT_SSL_SESSIONID_CACHE,NULL) \
    X(TCLCURLOPT_SSH_AUTH_TYPES,"CURLOPT_SSH_AUTH_TYPES","-sshauthtypes",TclCurl_HandleSshAuthTypes,0,NULL) \
    X(TCLCURLOPT_SSH_PUBLIC_KEYFILE, "CURLOPT_SSH_PUBLIC_KEYFILE", "-sshpublickeyfile", TclCurl_HandleSetoptChar, CURLOPT_SSH_PUBLIC_KEYFILE, NULL) \
    X(TCLCURLOPT_SSH_PRIVATE_KEYFILE, "CURLOPT_SSH_PRIVATE_KEYFILE", "-sshprivatekeyfile", TclCurl_HandleSetoptChar, CURLOPT_SSH_PRIVATE_KEYFILE, NULL) \
    X(TCLCURLOPT_TIMEOUT_MS, "CURLOPT_TIMEOUT_MS", "-timeoutms", TclCurl_HandleSetoptLong, CURLOPT_TIMEOUT_MS, NULL) \
    X(TCLCURLOPT_CONNECTTIMEOUT_MS, "CURLOPT_CONNECTTIMEOUT_MS", "-connecttimeoutms", TclCurl_HandleSetoptLong, CURLOPT_CONNECTTIMEOUT_MS, NULL) \
    X(TCLCURLOPT_HTTP_CONTENT_DECODING, "CURLOPT_HTTP_CONTENT_DECODING", "-contentdecoding", TclCurl_HandleSetoptLong, CURLOPT_HTTP_CONTENT_DECODING, NULL) \
    X(TCLCURLOPT_HTTP_TRANSFER_DECODING, "CURLOPT_HTTP_TRANSFER_DECODING", "-transferdecoding", TclCurl_HandleSetoptLong, CURLOPT_HTTP_TRANSFER_DECODING, NULL) \
    TCLCURL_OPTION_LIST_PRE_8_17_0_DEPR(X(TCLCURLOPT_KRBLEVEL, "CURLOPT_KRBLEVEL", "-krblevel", TclCurl_HandleSetoptChar, CURLOPT_KRBLEVEL, NULL)) \
    X(TCLCURLOPT_NEW_FILE_PERMS, "CURLOPT_NEW_FILE_PERMS", "-newfileperms", TclCurl_HandleSetoptLong, CURLOPT_NEW_FILE_PERMS, NULL) \
    X(TCLCURLOPT_NEW_DIRECTORY_PERMS, "CURLOPT_NEW_DIRECTORY_PERMS", "-newdirectoryperms", TclCurl_HandleSetoptLong, CURLOPT_NEW_DIRECTORY_PERMS, NULL) \
    X(TCLCURLOPT_KEYPASSWD, "CURLOPT_KEYPASSWD", "-keypasswd", TclCurl_HandleSetoptChar, CURLOPT_KEYPASSWD, NULL) \
    X(TCLCURLOPT_APPEND, "CURLOPT_APPEND", "-append", TclCurl_HandleSetoptInt, CURLOPT_APPEND, NULL) \
    X(TCLCURLOPT_DIRLISTONLY, "CURLOPT_DIRLISTONLY", "-dirlistonly", TclCurl_HandleSetoptInt, CURLOPT_DIRLISTONLY, NULL) \
    X(TCLCURLOPT_USE_SSL, "CURLOPT_USE_SSL", "-usessl", TclCurl_HandleFtpSsl, CURLOPT_USE_SSL, NULL) \
    X(TCLCURLOPT_POST301, "CURLOPT_POST301", "-post301", TclCurl_HandlePostRedir, 0, NULL) \
    X(TCLCURLOPT_SSH_HOST_PUBLIC_KEY_MD5,"CURLOPT_SSH_HOST_PUBLIC_KEY_MD5","-sshhostpublickeymd5",TclCurl_HandleSetoptChar,CURLOPT_SSH_HOST_PUBLIC_KEY_MD5,NULL) \
    X(TCLCURLOPT_PROXY_TRANSFER_MODE, "CURLOPT_PROXY_TRANSFER_MODE", "-proxytransfermode", TclCurl_HandleSetoptLong, CURLOPT_PROXY_TRANSFER_MODE, NULL) \
    X(TCLCURLOPT_CRLFILE, "CURLOPT_CRLFILE", "-crlfile", TclCurl_HandleSetoptChar, CURLOPT_CRLFILE, NULL) \
    X(TCLCURLOPT_ISSUERCERT, "CURLOPT_ISSUERCERT", "-issuercert", TclCurl_HandleSetoptChar, CURLOPT_ISSUERCERT, NULL) \
    X(TCLCURLOPT_ADDRESS_SCOPE, "CURLOPT_ADDRESS_SCOPE", "-addressscope", TclCurl_HandleSetoptLong, CURLOPT_ADDRESS_SCOPE, NULL) \
    X(TCLCURLOPT_CERTINFO, "CURLOPT_CERTINFO", "-certinfo", TclCurl_HandleSetoptLong, CURLOPT_CERTINFO, NULL) \
    X(TCLCURLOPT_POSTREDIR, "CURLOPT_POSTREDIR", "-postredir", TclCurl_HandlePostRedir, 0, NULL) \
    X(TCLCURLOPT_USERNAME, "CURLOPT_USERNAME", "-username", TclCurl_HandleSetoptChar, CURLOPT_USERNAME, NULL) \
    X(TCLCURLOPT_PASSWORD, "CURLOPT_PASSWORD", "-password", TclCurl_HandleSetoptChar, CURLOPT_PASSWORD, NULL) \
    X(TCLCURLOPT_PROXYUSERNAME, "CURLOPT_PROXYUSERNAME", "-proxyuser", TclCurl_HandleSetoptChar, CURLOPT_PROXYUSERNAME, NULL) \
    X(TCLCURLOPT_PROXYPASSWORD, "CURLOPT_PROXYPASSWORD", "-proxypassword", TclCurl_HandleSetoptChar, CURLOPT_PROXYPASSWORD, NULL) \
    X(TCLCURLOPT_TFTP_BLKSIZE, "CURLOPT_TFTP_BLKSIZE", "-tftpblksize", TclCurl_HandleSetoptLong, CURLOPT_TFTP_BLKSIZE, NULL) \
    X(TCLCURLOPT_PROXY_SERVICE_NAME, "CURLOPT_PROXY_SERVICE_NAME", "-socks5gssapiservice", TclCurl_HandleSetoptChar, CURLOPT_PROXY_SERVICE_NAME, NULL) \
    X(TCLCURLOPT_SOCKS5_GSSAPI_NEC, "CURLOPT_SOCKS5_GSSAPI_NEC", "-socks5gssapinec", TclCurl_HandleSetoptLong, CURLOPT_SOCKS5_GSSAPI_NEC, NULL) \
    X(TCLCURLOPT_PROTOCOLS_STR, "CURLOPT_PROTOCOLS_STR", "-protocols", TclCurl_HandleProtocolMask, CURLOPT_PROTOCOLS_STR, NULL) \
    X(TCLCURLOPT_REDIR_PROTOCOLS_STR, "CURLOPT_REDIR_PROTOCOLS_STR", "-redirprotocols", TclCurl_HandleProtocolMask, CURLOPT_REDIR_PROTOCOLS_STR, NULL) \
    X(TCLCURLOPT_FTP_SSL_CCC, "CURLOPT_FTP_SSL_CCC", "-ftpsslccc", TclCurl_HandleFtpSslCcc, 0, NULL) \
    X(TCLCURLOPT_SSH_KNOWNHOSTS, "CURLOPT_SSH_KNOWNHOSTS", "-sshknownhosts", TclCurl_HandleSetoptChar, CURLOPT_SSH_KNOWNHOSTS, NULL) \
    X(TCLCURLOPT_SSH_KEYFUNCTION, "CURLOPT_SSH_KEYFUNCTION", "-sshkeyproc", TclCurl_HandleSshKeyFunction, 0, NULL) \
    X(TCLCURLOPT_MAIL_FROM, "CURLOPT_MAIL_FROM", "-mailfrom", TclCurl_HandleSetoptChar, CURLOPT_MAIL_FROM, NULL) \
    X(TCLCURLOPT_MAIL_RCPT, "CURLOPT_MAIL_RCPT", "-mailrcpt", TclCurl_HandleMailRcpt, 0, NULL) \
    X(TCLCURLOPT_FTP_USE_PRET, "CURLOPT_FTP_USE_PRET", "-ftpusepret", TclCurl_HandleSetoptLong, CURLOPT_FTP_USE_PRET, NULL) \
    X(TCLCURLOPT_WILDCARDMATCH, "CURLOPT_WILDCARDMATCH", "-wildcardmatch", TclCurl_HandleSetoptLong, CURLOPT_WILDCARDMATCH, NULL) \
    X(TCLCURLOPT_CHUNK_BGN_PROC, "CURLOPT_CHUNK_BGN_PROC", "-chunkbgnproc", TclCurl_HandleChunkBgnProc, 0, NULL) \
    X(TCLCURLOPT_CHUNK_BGN_VAR, "CURLOPT_CHUNK_BGN_VAR", "-chunkbgnvar", TclCurl_HandleChunkBgnVar, 0, NULL) \
    X(TCLCURLOPT_CHUNK_END_PROC, "CURLOPT_CHUNK_END_PROC", "-chunkendproc", TclCurl_HandleChunkEndProc, 0, NULL) \
    X(TCLCURLOPT_FNMATCH_PROC, "CURLOPT_FNMATCH_PROC", "-fnmatchproc", TclCurl_HandleFnmatchProc, 0, NULL) \
    X(TCLCURLOPT_RESOLVE, "CURLOPT_RESOLVE", "-resolve", TclCurl_HandleResolveList, 0, NULL) \
    X(TCLCURLOPT_TLSAUTH_USERNAME, "CURLOPT_TLSAUTH_USERNAME", "-tlsauthusername", TclCurl_HandleSetoptChar, CURLOPT_TLSAUTH_USERNAME, NULL) \
    X(TCLCURLOPT_TLSAUTH_PASSWORD, "CURLOPT_TLSAUTH_PASSWORD", "-tlsauthpassword", TclCurl_HandleSetoptChar, CURLOPT_TLSAUTH_PASSWORD, NULL) \
    X(TCLCURLOPT_TLSAUTH_TYPE, "CURLOPT_TLSAUTH_TYPE", "-tlsauthtype", TclCurl_HandleTlsAuthType, 0, NULL) \
    X(TCLCURLOPT_TRANSFER_ENCODING, "CURLOPT_TRANSFER_ENCODING", "-transferencoding", TclCurl_HandleSetoptLong, CURLOPT_TRANSFER_ENCODING, NULL) \
    X(TCLCURLOPT_GSSAPI_DELEGATION, "CURLOPT_GSSAPI_DELEGATION", "-gssapidelegation", TclCurl_HandleGssApiDelegation, 0, NULL) \
    X(TCLCURLOPT_NOPROXY, "CURLOPT_NOPROXY", "-noproxy", TclCurl_HandleSetoptChar, CURLOPT_NOPROXY, NULL) \
    X(TCLCURLOPT_TELNETOPTIONS, "CURLOPT_TELNETOPTIONS", "-telnetoptions", TclCurl_HandleTelnetOptions, 0, NULL) \
    X(TCLCURLOPT_CAINFO_BLOB,"CURLOPT_CAINFO_BLOB","-cainfoblob",TclCurl_HandleCainfoBlob,0,NULL)

enum curlOptionsIndices {
#define TCLCURLOPT_ENUM_ENTRY(option, optname, configname, handler, curlopt, message) option,
    TCLCURL_OPTION_LIST(TCLCURLOPT_ENUM_ENTRY)
#undef TCLCURLOPT_ENUM_ENTRY
    TCLCURLOPT_COUNT
};

const static char *configTable[] = {
#define TCLCURLOPT_CONFIG_ENTRY(option, optname, configname, handler, curlopt, message) configname,
    TCLCURL_OPTION_LIST(TCLCURLOPT_CONFIG_ENTRY)
#undef TCLCURLOPT_CONFIG_ENTRY
    (char *) NULL
};

typedef struct TclCurlOptionDef TclCurlOptionDef;
typedef struct TclCurlOptsArgs  TclCurlOptsArgs;

struct TclCurlOptsArgs {
    Tcl_Interp*             interp;
    struct curlObjData*     curlData;
    Tcl_Obj *const          objv;
    int                     curlOptsIndex;
    const TclCurlOptionDef* def;
};

typedef int (*TclCurlOptionHandler)(TclCurlOptsArgs *args);

struct TclCurlOptionDef {
    const char           *optionName;
    const char           *configName;
    TclCurlOptionHandler  handler;
    CURLoption            curlOpt;
    const char           *errorMessage;
};

#endif
