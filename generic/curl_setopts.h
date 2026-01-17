/*
 *
 *
 */

#ifndef __curl_setopts_h__
#define __curl_setopts_h__

enum curlOptionsIndices
{
    TCLCURLOPT_URL = 0,
    TCLCURLOPT_FILE,
    TCLCURLOPT_READDATA,
    TCLCURLOPT_USERAGENT,
    TCLCURLOPT_REFERER,
    TCLCURLOPT_VERBOSE,
    TCLCURLOPT_HEADER,
    TCLCURLOPT_NOBODY,
    TCLCURLOPT_PROXY,
    TCLCURLOPT_PROXYPORT,
    TCLCURLOPT_HTTPPROXYTUNNEL,
    TCLCURLOPT_FAILONERROR,
    TCLCURLOPT_TIMEOUT,
    TCLCURLOPT_LOW_SPEED_LIMIT,
    TCLCURLOPT_LOW_SPEED_TIME,
    TCLCURLOPT_RESUME_FROM,
    TCLCURLOPT_INFILESIZE,
    TCLCURLOPT_UPLOAD,
    TCLCURLOPT_FTPLISTONLY,
    TCLCURLOPT_FTPAPPEND,
    TCLCURLOPT_NETRC,
    TCLCURLOPT_FOLLOWLOCATION,
    TCLCURLOPT_TRANSFERTEXT,
    TCLCURLOPT_PUT,
    TCLCURLOPT_MUTE,
    TCLCURLOPT_USERPWD,
    TCLCURLOPT_PROXYUSERPWD,
    TCLCURLOPT_RANGE,
    TCLCURLOPT_ERRORBUFFER,
    TCLCURLOPT_HTTPGET,
    TCLCURLOPT_POST,
    TCLCURLOPT_POSTFIELDS,
    TCLCURLOPT_POSTFIELDSIZE_LARGE,
    TCLCURLOPT_FTPPORT,
    TCLCURLOPT_COOKIE,
    TCLCURLOPT_COOKIEFILE,
    TCLCURLOPT_HTTPHEADER,
    TCLCURLOPT_HTTPPOST,
    TCLCURLOPT_SSLCERT,
    TCLCURLOPT_SSLCERTPASSWD,
    TCLCURLOPT_SSLVERSION,
    TCLCURLOPT_CRLF,
    TCLCURLOPT_QUOTE,
    TCLCURLOPT_POSTQUOTE,
    TCLCURLOPT_WRITEHEADER,
    TCLCURLOPT_TIMECONDITION,
    TCLCURLOPT_TIMEVALUE,
    TCLCURLOPT_CUSTOMREQUEST,
    TCLCURLOPT_STDERR,
    TCLCURLOPT_INTERFACE,
    TCLCURLOPT_KRB4LEVEL,
    TCLCURLOPT_SSL_VERIFYPEER,
    TCLCURLOPT_CAINFO,
    TCLCURLOPT_FILETIME,
    TCLCURLOPT_MAXREDIRS,
    TCLCURLOPT_MAXCONNECTS,
    TCLCURLOPT_CLOSEPOLICY,
    TCLCURLOPT_RANDOM_FILE,
    TCLCURLOPT_EGDSOCKET,
    TCLCURLOPT_CONNECTTIMEOUT,
    TCLCURLOPT_NOPROGRESS,
    TCLCURLOPT_HEADERVAR,
    TCLCURLOPT_BODYVAR,
    TCLCURLOPT_PROGRESSPROC,
    TCLCURLOPT_CANCELTRANSVARNAME,
    TCLCURLOPT_WRITEPROC,
    TCLCURLOPT_READPROC,
    TCLCURLOPT_SSL_VERIFYHOST,
    TCLCURLOPT_COOKIEJAR,
    TCLCURLOPT_SSL_CIPHER_LIST,
    TCLCURLOPT_HTTP_VERSION,
    TCLCURLOPT_FTP_USE_EPSV,
    TCLCURLOPT_SSLCERTTYPE,
    TCLCURLOPT_SSLKEY,
    TCLCURLOPT_SSLKEYTYPE,
    TCLCURLOPT_SSLKEYPASSWD,
    TCLCURLOPT_SSL_ENGINE,
    TCLCURLOPT_SSL_ENGINEDEFAULT,
    TCLCURLOPT_PREQUOTE,
    TCLCURLOPT_DEBUGPROC,
    TCLCURLOPT_DNS_CACHE_TIMEOUT,
    TCLCURLOPT_DNS_USE_GLOBAL_CACHE,
    TCLCURLOPT_COOKIESESSION,
    TCLCURLOPT_CAPATH,
    TCLCURLOPT_BUFFERSIZE,
    TCLCURLOPT_NOSIGNAL,
    TCLCURLOPT_ENCODING,
    TCLCURLOPT_PROXYTYPE,
    TCLCURLOPT_HTTP200ALIASES,
    TCLCURLOPT_UNRESTRICTED_AUTH,
    TCLCURLOPT_FTP_USE_EPRT,
    TCLCURLOPT_NOSUCHOPTION,
    TCLCURLOPT_HTTPAUTH,
    TCLCURLOPT_FTP_CREATE_MISSING_DIRS,
    TCLCURLOPT_PROXYAUTH,
    TCLCURLOPT_FTP_RESPONSE_TIMEOUT,
    TCLCURLOPT_IPRESOLVE,
    TCLCURLOPT_MAXFILESIZE,
    TCLCURLOPT_NETRC_FILE,
    TCLCURLOPT_FTP_SSL,
    TCLCURLOPT_SHARE,
    TCLCURLOPT_PORT,
    TCLCURLOPT_TCP_NODELAY,
    TCLCURLOPT_AUTOREFERER,
    TCLCURLOPT_SOURCE_HOST,
    TCLCURLOPT_SOURCE_USERPWD,
    TCLCURLOPT_SOURCE_PATH,
    TCLCURLOPT_SOURCE_PORT,
    TCLCURLOPT_PASV_HOST,
    TCLCURLOPT_SOURCE_PREQUOTE,
    TCLCURLOPT_SOURCE_POSTQUOTE,
    TCLCURLOPT_FTPSSLAUTH,
    TCLCURLOPT_SOURCE_URL,
    TCLCURLOPT_SOURCE_QUOTE,
    TCLCURLOPT_FTP_ACCOUNT,
    TCLCURLOPT_IGNORE_CONTENT_LENGTH,
    TCLCURLOPT_COOKIELIST,
    TCLCURLOPT_FTP_SKIP_PASV_IP,
    TCLCURLOPT_FTP_FILEMETHOD,
    TCLCURLOPT_LOCALPORT,
    TCLCURLOPT_LOCALPORTRANGE,
    TCLCURLOPT_MAX_SEND_SPEED_LARGE,
    TCLCURLOPT_MAX_RECV_SPEED_LARGE,
    TCLCURLOPT_FTP_ALTERNATIVE_TO_USER,
    TCLCURLOPT_SSL_SESSIONID_CACHE,
    TCLCURLOPT_SSH_AUTH_TYPES,
    TCLCURLOPT_SSH_PUBLIC_KEYFILE,
    TCLCURLOPT_SSH_PRIVATE_KEYFILE,
    TCLCURLOPT_TIMEOUT_MS,
    TCLCURLOPT_CONNECTTIMEOUT_MS,
    TCLCURLOPT_HTTP_CONTENT_DECODING,
    TCLCURLOPT_HTTP_TRANSFER_DECODING,
    TCLCURLOPT_KRBLEVEL,
    TCLCURLOPT_NEW_FILE_PERMS,
    TCLCURLOPT_NEW_DIRECTORY_PERMS,
    TCLCURLOPT_KEYPASSWD,
    TCLCURLOPT_APPEND,
    TCLCURLOPT_DIRLISTONLY,
    TCLCURLOPT_USE_SSL,
    TCLCURLOPT_POST301,
    TCLCURLOPT_SSH_HOST_PUBLIC_KEY_MD5,
    TCLCURLOPT_PROXY_TRANSFER_MODE,
    TCLCURLOPT_CRLFILE,
    TCLCURLOPT_ISSUERCERT,
    TCLCURLOPT_ADDRESS_SCOPE,
    TCLCURLOPT_CERTINFO,
    TCLCURLOPT_POSTREDIR,
    TCLCURLOPT_USERNAME,
    TCLCURLOPT_PASSWORD,
    TCLCURLOPT_PROXYUSERNAME,
    TCLCURLOPT_PROXYPASSWORD,
    TCLCURLOPT_TFTP_BLKSIZE,
    TCLCURLOPT_SOCKS5_GSSAPI_SERVICE,
    TCLCURLOPT_SOCKS5_GSSAPI_NEC,
    TCLCURLOPT_PROTOCOLS,
    TCLCURLOPT_REDIR_PROTOCOLS,
    TCLCURLOPT_FTP_SSL_CCC,
    TCLCURLOPT_SSH_KNOWNHOSTS,
    TCLCURLOPT_SSH_KEYFUNCTION,
    TCLCURLOPT_MAIL_FROM,
    TCLCURLOPT_MAIL_RCPT,
    TCLCURLOPT_FTP_USE_PRET,
    TCLCURLOPT_WILDCARDMATCH,
    TCLCURLOPT_CHUNK_BGN_PROC,
    TCLCURLOPT_CHUNK_BGN_VAR,
    TCLCURLOPT_CHUNK_END_PROC,
    TCLCURLOPT_FNMATCH_PROC,
    TCLCURLOPT_RESOLVE,
    TCLCURLOPT_TLSAUTH_USERNAME,
    TCLCURLOPT_TLSAUTH_PASSWORD,
    TCLCURLOPT_TLSAUTH_TYPE,
    TCLCURLOPT_TRANSFER_ENCODING,
    TCLCURLOPT_GSSAPI_DELEGATION,
    TCLCURLOPT_NOPROXY,
    TCLCURLOPT_TELNETOPTIONS,
    TCLCURLOPT_CAINFO_BLOB
};

const static char *optionTable[] = {
    "CURLOPT_URL",           "CURLOPT_FILE",            "CURLOPT_READDATA",
    "CURLOPT_USERAGENT",     "CURLOPT_REFERER",         "CURLOPT_VERBOSE",
    "CURLOPT_HEADER",        "CURLOPT_NOBODY",          "CURLOPT_PROXY",
    "CURLOPT_PROXYPORT",     "CURLOPT_HTTPPROXYTUNNEL", "CURLOPT_FAILONERROR",
    "CURLOPT_TIMEOUT",       "CURLOPT_LOW_SPEED_LIMIT", "CURLOPT_LOW_SPEED_TIME",
    "CURLOPT_RESUME_FROM",   "CURLOPT_INFILESIZE",      "CURLOPT_UPLOAD",
    "CURLOPT_FTPLISTONLY",   "CURLOPT_FTPAPPEND",       "CURLOPT_NETRC",
    "CURLOPT_FOLLOWLOCATION","CURLOPT_TRANSFERTEXT",    "CURLOPT_PUT",
    "CURLOPT_MUTE",          "CURLOPT_USERPWD",         "CURLOPT_PROXYUSERPWD",
    "CURLOPT_RANGE",         "CURLOPT_ERRORBUFFER",     "CURLOPT_HTTPGET",
    "CURLOPT_POST",          "CURLOPT_POSTFIELDS",      "CURLOPT_POSTFIELDSIZE_LARGE",
    "CURLOPT_FTPPORT",       "CURLOPT_COOKIE",          "CURLOPT_COOKIEFILE",
    "CURLOPT_HTTPHEADER",    "CURLOPT_HTTPPOST",        "CURLOPT_SSLCERT",
    "CURLOPT_SSLCERTPASSWD", "CURLOPT_SSLVERSION",      "CURLOPT_CRLF",
    "CURLOPT_QUOTE",         "CURLOPT_POSTQUOTE",       "CURLOPT_WRITEHEADER",
    "CURLOPT_TIMECONDITION", "CURLOPT_TIMEVALUE",       "CURLOPT_CUSTOMREQUEST",
    "CURLOPT_STDERR",        "CURLOPT_INTERFACE",       "CURLOPT_KRB4LEVEL",
    "CURLOPT_SSL_VERIFYPEER","CURLOPT_CAINFO",          "CURLOPT_FILETIME",
    "CURLOPT_MAXREDIRS",     "CURLOPT_MAXCONNECTS",     "CURLOPT_CLOSEPOLICY",
    "CURLOPT_RANDOM_FILE",   "CURLOPT_EGDSOCKET",       "CURLOPT_CONNECTTIMEOUT",
    "CURLOPT_NOPROGRESS",    "CURLOPT_HEADERVAR",       "CURLOPT_BODYVAR",
    "CURLOPT_PROGRESSPROC","CURLOPT_CANCELTRANSVARNAME","CURLOPT_WRITEPROC",
    "CURLOPT_READPROC",      "CURLOPT_SSL_VERIFYHOST",  "CURLOPT_COOKIEJAR",
    "CURLOPT_SSL_CIPHER_LIST","CURLOPT_HTTP_VERSION",   "CURLOPT_FTP_USE_EPSV",
    "CURLOPT_SSLCERTTYPE",    "CURLOPT_SSLKEY",         "CURLOPT_SSLKEYTYPE",
    "CURLOPT_SSLKEYPASSWD",   "CURLOPT_SSL_ENGINE",     "CURLOPT_SSL_ENGINEDEFAULT",
    "CURLOPT_PREQUOTE",       "CURLOPT_DEBUGPROC",      "CURLOPT_DNS_CACHE_TIMEOUT",
    "CURLOPT_DNS_USE_GLOBAL_CACHE", "CURLOPT_COOKIESESSION","CURLOPT_CAPATH",
    "CURLOPT_BUFFERSIZE",     "CURLOPT_NOSIGNAL",       "CURLOPT_ENCODING",
    "CURLOPT_PROXYTYPE",      "CURLOPT_HTTP200ALIASES", "CURLOPT_UNRESTRICTED_AUTH",
    "CURLOPT_FTP_USE_EPRT",   "CURLOPT_NOSUCHOPTION",   "CURLOPT_HTTPAUTH",
    "CURLOPT_FTP_CREATE_MISSING_DIRS",                  "CURLOPT_PROXYAUTH",
    "CURLOPT_FTP_RESPONSE_TIMEOUT",                     "CURLOPT_IPRESOLVE",
    "CURLOPT_MAXFILESIZE",    "CURLOPT_NETRC_FILE",     "CURLOPT_FTP_SSL",
    "CURLOPT_SHARE",          "CURLOPT_PORT",           "CURLOPT_TCP_NODELAY",
    "CURLOPT_AUTOREFERER",    "CURLOPT_SOURCE_HOST",    "CURLOPT_SOURCE_USERPWD",
    "CURLOPT_SOURCE_PATH",    "CURLOPT_SOURCE_PORT",    "CURLOPT_PASV_HOST",
    "CURLOPT_SOURCE_PREQUOTE","CURLOPT_SOURCE_POSTQUOTE",
    "CURLOPT_FTPSSLAUTH",     "CURLOPT_SOURCE_URL",     "CURLOPT_SOURCE_QUOTE",
    "CURLOPT_FTP_ACCOUNT",    "CURLOPT_IGNORE_CONTENT_LENGTH",
    "CURLOPT_COOKIELIST",     "CURLOPT_FTP_SKIP_PASV_IP",
    "CURLOPT_FTP_FILEMETHOD", "CURLOPT_LOCALPORT",      "CURLOPT_LOCALPORTRANGE",
    "CURLOPT_MAX_SEND_SPEED_LARGE",                     "CURLOPT_MAX_RECV_SPEED_LARGE",
    "CURLOPT_FTP_ALTERNATIVE_TO_USER",                  "CURLOPT_SSL_SESSIONID_CACHE",
    "CURLOPT_SSH_AUTH_TYPES",                           "CURLOPT_SSH_PUBLIC_KEYFILE",
    "CURLOPT_SSH_PRIVATE_KEYFILE",                      "CURLOPT_TIMEOUT_MS",
    "CURLOPT_CONNECTTIMEOUT_MS",                        "CURLOPT_HTTP_CONTENT_DECODING",
    "CURLOPT_HTTP_TRANSFER_DECODING",                   "CURLOPT_KRBLEVEL",
    "CURLOPT_NEW_FILE_PERMS",                           "CURLOPT_NEW_DIRECTORY_PERMS",
    "CURLOPT_KEYPASSWD",      "CURLOPT_APPEND",         "CURLOPT_DIRLISTONLY",
    "CURLOPT_USE_SSL",        "CURLOPT_POST301",        "CURLOPT_SSH_HOST_PUBLIC_KEY_MD5",
    "CURLOPT_PROXY_TRANSFER_MODE",                      "CURLOPT_CRLFILE",
    "CURLOPT_ISSUERCERT",     "CURLOPT_ADDRESS_SCOPE",  "CURLOPT_CERTINFO",
    "CURLOPT_POSTREDIR",      "CURLOPT_USERNAME",       "CURLOPT_PASSWORD",
    "CURLOPT_PROXYUSERNAME",  "CURLOPT_PROXYPASSWORD",  "CURLOPT_TFTP_BLKSIZE",
    "CURLOPT_SOCKS5_GSSAPI_SERVICE",                    "CURLOPT_SOCKS5_GSSAPI_NEC",
    "CURLOPT_PROTOCOLS",      "CURLOPT_REDIR_PROTOCOLS","CURLOPT_FTP_SSL_CCC",
    "CURLOPT_SSH_KNOWNHOSTS", "CURLOPT_SSH_KEYFUNCTION","CURLOPT_MAIL_FROM",
    "CURLOPT_MAIL_RCPT",      "CURLOPT_FTP_USE_PRET",   "CURLOPT_WILDCARDMATCH",
    "CURLOPT_CHUNK_BGN_PROC", "CURLOPT_CHUNK_BGN_VAR",  "CURLOPT_CHUNK_END_PROC",
    "CURLOPT_FNMATCH_PROC",   "CURLOPT_RESOLVE",        "CURLOPT_TLSAUTH_USERNAME",
    "CURLOPT_TLSAUTH_PASSWORD", "CURLOPT_TLSAUTH_TYPE", "CURLOPT_TRANSFER_ENCODING",
    "CURLOPT_GSSAPI_DELEGATION", "CURLOPT_NOPROXY",     "CURLOPT_TELNETOPTIONS",
    "CURLOPT_CAINFO_BLOB",
    (char *)NULL
};

const static char *configTable[] = {
    "-url",               "-file",               "-infile",
    "-useragent",         "-referer",            "-verbose",
    "-header",            "-nobody",             "-proxy",
    "-proxyport",         "-httpproxytunnel",    "-failonerror",
    "-timeout",           "-lowspeedlimit",      "-lowspeedtime",
    "-resumefrom",        "-infilesize",         "-upload",
    "-ftplistonly",       "-ftpappend",          "-netrc",
    "-followlocation",    "-transfertext",       "-put",
    "-mute",              "-userpwd",            "-proxyuserpwd",
    "-range",             "-errorbuffer",        "-httpget",
    "-post",              "-postfields",         "-postfieldsize",
    "-ftpport",           "-cookie",             "-cookiefile",
    "-httpheader",        "-httppost",           "-sslcert",
    "-sslcertpasswd",     "-sslversion",         "-crlf",
    "-quote",             "-postquote",          "-writeheader",
    "-timecondition",     "-timevalue",          "-customrequest",
    "-stderr",            "-interface",          "-krb4level",
    "-sslverifypeer",     "-cainfo",             "-filetime",
    "-maxredirs",         "-maxconnects",        "-closepolicy",
    "-randomfile",        "-egdsocket",          "-connecttimeout",
    "-noprogress",        "-headervar",          "-bodyvar",
    "-progressproc",      "-canceltransvarname", "-writeproc",
    "-readproc",          "-sslverifyhost",      "-cookiejar",
    "-sslcipherlist",     "-httpversion",        "-ftpuseepsv",
    "-sslcerttype",       "-sslkey",             "-sslkeytype",
    "-sslkeypasswd",      "-sslengine",          "-sslenginedefault",
    "-prequote",          "-debugproc",          "-dnscachetimeout",
    "-dnsuseglobalcache", "-cookiesession",      "-capath",
    "-buffersize",        "-nosignal",           "-encoding",
    "-proxytype",         "-http200aliases",     "-unrestrictedauth",
    "-ftpuseeprt",        "-command",            "-httpauth",
    "-ftpcreatemissingdirs",                     "-proxyauth",
    "-ftpresponsetimeout",                       "-ipresolve",
    "-maxfilesize",       "-netrcfile",          "-ftpssl",
    "-share",             "-port",               "-tcpnodelay",
    "-autoreferer",       "-sourcehost",         "-sourceuserpwd",
    "-sourcepath",        "-sourceport",         "-pasvhost",
    "-sourceprequote",    "-sourcepostquote",
    "-ftpsslauth",        "-sourceurl",          "-sourcequote",
    "-ftpaccount",        "-ignorecontentlength",
    "-cookielist",        "-ftpskippasvip",
    "-ftpfilemethod",     "-localport",          "-localportrange",
    "-maxsendspeed",                             "-maxrecvspeed",
    "-ftpalternativetouser",                     "-sslsessionidcache",
    "-sshauthtypes",      "-sshpublickeyfile",   "-sshprivatekeyfile",
    "-timeoutms",         "-connecttimeoutms",   "-contentdecoding",
    "-transferdecoding",  "-krblevel",           "-newfileperms",
    "-newdirectoryperms", "-keypasswd",          "-append",
    "-dirlistonly",       "-usessl",             "-post301",
    "-sshhostpublickeymd5",                      "-proxytransfermode",
    "-crlfile",           "-issuercert",         "-addressscope",
    "-certinfo",          "-postredir",          "-username",
    "-password",          "-proxyuser",          "-proxypassword",
    "-tftpblksize",       "-socks5gssapiservice","-socks5gssapinec",
    "-protocols",         "-redirprotocols",     "-ftpsslccc",
    "-sshknownhosts",     "-sshkeyproc",         "-mailfrom",
    "-mailrcpt",          "-ftpusepret",         "-wildcardmatch",
    "-chunkbgnproc",      "-chunkbgnvar",        "-chunkendproc",
    "-fnmatchproc",       "-resolve",            "-tlsauthusername",
    "-tlsauthpassword",   "-tlsauthtype",        "-transferencoding",
    "-gssapidelegation",  "-noproxy",            "-telnetoptions",
    "-cainfoblob",
    (char *) NULL
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

CONST static char   *curlFormTable[] = {
    "name",  "contents", "file", "contenttype", "contentheader", "filename",
    "bufferName", "buffer", "filecontent", (char *)NULL
};

const static char *timeCond[] = {
    "ifmodsince", "ifunmodsince",
    (char *)NULL
};


#endif
