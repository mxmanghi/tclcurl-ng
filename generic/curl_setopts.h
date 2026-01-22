/*
 *
 *
 */

#ifndef __curl_setopts_h__
#define __curl_setopts_h__

#define TCLCURL_OPTIONS(X)                                        \
    X(TCLCURLOPT_URL,"-url")                                      \
    X(TCLCURLOPT_FILE,"-file")                                    \
    X(TCLCURLOPT_READDATA,"-infile")                              \
    X(TCLCURLOPT_USERAGENT,"-useragent")                          \
    X(TCLCURLOPT_REFERER,"-referer")                              \
    X(TCLCURLOPT_VERBOSE,"-verbose")                              \
    X(TCLCURLOPT_HEADER,"-header")                                \
    X(TCLCURLOPT_NOBODY,"-nobody")                                \
    X(TCLCURLOPT_PROXY,"-proxy")                                  \
    X(TCLCURLOPT_PROXYPORT,"-proxyport")                          \
    X(TCLCURLOPT_HTTPPROXYTUNNEL,"-httpproxytunnel")              \
    X(TCLCURLOPT_FAILONERROR,"-failonerror")                      \
    X(TCLCURLOPT_TIMEOUT,"-timeout")                              \
    X(TCLCURLOPT_LOW_SPEED_LIMIT,"-lowspeedlimit")                \
    X(TCLCURLOPT_LOW_SPEED_TIME,"-lowspeedtime")                  \
    X(TCLCURLOPT_RESUME_FROM,"-resumefrom")                       \
    X(TCLCURLOPT_INFILESIZE,"-infilesize")                        \
    X(TCLCURLOPT_UPLOAD,"-upload")                                \
    X(TCLCURLOPT_FTPLISTONLY,"-ftplistonly")                      \
    X(TCLCURLOPT_FTPAPPEND,"-ftpappend")                          \
    X(TCLCURLOPT_NETRC,"-netrc")                                  \
    X(TCLCURLOPT_FOLLOWLOCATION,"-followlocation")                \
    X(TCLCURLOPT_TRANSFERTEXT,"-transfertext")                    \
    X(TCLCURLOPT_PUT,"-put")                                      \
    X(TCLCURLOPT_MUTE,"-mute")                                    \
    X(TCLCURLOPT_USERPWD,"-userpwd")                              \
    X(TCLCURLOPT_PROXYUSERPWD,"-proxyuserpwd")                    \
    X(TCLCURLOPT_RANGE,"-range")                                  \
    X(TCLCURLOPT_ERRORBUFFER,"-errorbuffer")                      \
    X(TCLCURLOPT_HTTPGET,"-httpget")                              \
    X(TCLCURLOPT_POST,"-post")                                    \
    X(TCLCURLOPT_POSTFIELDS,"-postfields")                        \
    X(TCLCURLOPT_POSTFIELDSIZE_LARGE,"-postfieldsize")            \
    X(TCLCURLOPT_FTPPORT,"-ftpport")                              \
    X(TCLCURLOPT_COOKIE,"-cookie")                                \
    X(TCLCURLOPT_COOKIEFILE,"-cookiefile")                        \
    X(TCLCURLOPT_HTTPHEADER,"-httpheader")                        \
    X(TCLCURLOPT_HTTPPOST,"-httppost")                            \
    X(TCLCURLOPT_SSLCERT,"-sslcert")                              \
    X(TCLCURLOPT_SSLCERTPASSWD,"-sslcertpasswd")                  \
    X(TCLCURLOPT_SSLVERSION,"-sslversion")                        \
    X(TCLCURLOPT_CRLF,"-crlf")                                    \
    X(TCLCURLOPT_QUOTE,"-quote")                                  \
    X(TCLCURLOPT_POSTQUOTE,"-postquote")                          \
    X(TCLCURLOPT_WRITEHEADER,"-writeheader")                      \
    X(TCLCURLOPT_TIMECONDITION,"-timecondition")                  \
    X(TCLCURLOPT_TIMEVALUE,"-timevalue")                          \
    X(TCLCURLOPT_CUSTOMREQUEST,"-customrequest")                  \
    X(TCLCURLOPT_STDERR,"-stderr")                                \
    X(TCLCURLOPT_INTERFACE,"-interface")                          \
    X(TCLCURLOPT_KRB4LEVEL,"-krb4level")                          \
    X(TCLCURLOPT_SSL_VERIFYPEER,"-sslverifypeer")                 \
    X(TCLCURLOPT_CAINFO,"-cainfo")                                \
    X(TCLCURLOPT_FILETIME,"-filetime")                            \
    X(TCLCURLOPT_MAXREDIRS,"-maxredirs")                          \
    X(TCLCURLOPT_MAXCONNECTS,"-maxconnects")                      \
    X(TCLCURLOPT_CLOSEPOLICY,"-closepolicy")                      \
    X(TCLCURLOPT_RANDOM_FILE,"-randomfile")                       \
    X(TCLCURLOPT_EGDSOCKET,"-egdsocket")                          \
    X(TCLCURLOPT_CONNECTTIMEOUT,"-connecttimeout")                \
    X(TCLCURLOPT_NOPROGRESS,"-noprogress")                        \
    X(TCLCURLOPT_HEADERVAR,"-headervar")                          \
    X(TCLCURLOPT_BODYVAR,"-bodyvar")                              \
    X(TCLCURLOPT_PROGRESSPROC,"-progressproc")                    \
    X(TCLCURLOPT_CANCELTRANSVARNAME,"-canceltransvarname")        \
    X(TCLCURLOPT_WRITEPROC,"-writeproc")                          \
    X(TCLCURLOPT_READPROC,"-readproc")                            \
    X(TCLCURLOPT_SSL_VERIFYHOST,"-sslverifyhost")                 \
    X(TCLCURLOPT_COOKIEJAR,"-cookiejar")                          \
    X(TCLCURLOPT_SSL_CIPHER_LIST,"-sslcipherlist")                \
    X(TCLCURLOPT_HTTP_VERSION,"-httpversion")                     \
    X(TCLCURLOPT_FTP_USE_EPSV,"-ftpuseepsv")                      \
    X(TCLCURLOPT_SSLCERTTYPE,"-sslcerttype")                      \
    X(TCLCURLOPT_SSLKEY,"-sslkey")                                \
    X(TCLCURLOPT_SSLKEYTYPE,"-sslkeytype")                        \
    X(TCLCURLOPT_SSLKEYPASSWD,"-sslkeypasswd")                    \
    X(TCLCURLOPT_SSL_ENGINE,"-sslengine")                         \
    X(TCLCURLOPT_SSL_ENGINEDEFAULT,"-sslenginedefault")           \
    X(TCLCURLOPT_PREQUOTE,"-prequote")                            \
    X(TCLCURLOPT_DEBUGPROC,"-debugproc")                          \
    X(TCLCURLOPT_DNS_CACHE_TIMEOUT,"-dnscachetimeout")            \
    X(TCLCURLOPT_DNS_USE_GLOBAL_CACHE,"-dnsuseglobalcache")       \
    X(TCLCURLOPT_COOKIESESSION,"-cookiesession")                  \
    X(TCLCURLOPT_CAPATH,"-capath")                                \
    X(TCLCURLOPT_BUFFERSIZE,"-buffersize")                        \
    X(TCLCURLOPT_NOSIGNAL,"-nosignal")                            \
    X(TCLCURLOPT_ENCODING,"-encoding")                            \
    X(TCLCURLOPT_PROXYTYPE,"-proxytype")                          \
    X(TCLCURLOPT_HTTP200ALIASES,"-http200aliases")                \
    X(TCLCURLOPT_UNRESTRICTED_AUTH,"-unrestrictedauth")           \
    X(TCLCURLOPT_FTP_USE_EPRT,"-ftpuseeprt")                      \
    X(TCLCURLOPT_NOSUCHOPTION,"-command")                         \
    X(TCLCURLOPT_HTTPAUTH,"-httpauth")                            \
    X(TCLCURLOPT_FTP_CREATE_MISSING_DIRS,"-ftpcreatemissingdirs") \
    X(TCLCURLOPT_PROXYAUTH,"-proxyauth")                          \
    X(TCLCURLOPT_FTP_RESPONSE_TIMEOUT,"-ftpresponsetimeout")      \
    X(TCLCURLOPT_IPRESOLVE,"-ipresolve")                          \
    X(TCLCURLOPT_MAXFILESIZE,"-maxfilesize")                      \
    X(TCLCURLOPT_NETRC_FILE,"-netrcfile")                         \
    X(TCLCURLOPT_FTP_SSL,"-ftpssl")                               \
    X(TCLCURLOPT_SHARE,"-share")                                  \
    X(TCLCURLOPT_PORT,"-port")                                    \
    X(TCLCURLOPT_TCP_NODELAY,"-tcpnodelay")                       \
    X(TCLCURLOPT_AUTOREFERER,"-autoreferer")                      \
    X(TCLCURLOPT_SOURCE_HOST,"-sourcehost")                       \
    X(TCLCURLOPT_SOURCE_USERPWD,"-sourceuserpwd")                 \
    X(TCLCURLOPT_SOURCE_PATH,"-sourcepath")                       \
    X(TCLCURLOPT_SOURCE_PORT,"-sourceport")                       \
    X(TCLCURLOPT_PASV_HOST,"-pasvhost")                           \
    X(TCLCURLOPT_SOURCE_PREQUOTE,"-sourceprequote")               \
    X(TCLCURLOPT_SOURCE_POSTQUOTE,"-sourcepostquote")             \
    X(TCLCURLOPT_FTPSSLAUTH,"-ftpsslauth")                        \
    X(TCLCURLOPT_SOURCE_URL,"-sourceurl")                         \
    X(TCLCURLOPT_SOURCE_QUOTE,"-sourcequote")                     \
    X(TCLCURLOPT_FTP_ACCOUNT,"-ftpaccount")                       \
    X(TCLCURLOPT_IGNORE_CONTENT_LENGTH,"-ignorecontentlength")    \
    X(TCLCURLOPT_COOKIELIST,"-cookielist")                        \
    X(TCLCURLOPT_FTP_SKIP_PASV_IP,"-ftpskippasvip")               \
    X(TCLCURLOPT_FTP_FILEMETHOD,"-ftpfilemethod")                 \
    X(TCLCURLOPT_LOCALPORT,"-localport")                          \
    X(TCLCURLOPT_LOCALPORTRANGE,"-localportrange")                \
    X(TCLCURLOPT_MAX_SEND_SPEED_LARGE,"-maxsendspeed")            \
    X(TCLCURLOPT_MAX_RECV_SPEED_LARGE,"-maxrecvspeed")            \
    X(TCLCURLOPT_FTP_ALTERNATIVE_TO_USER,"-ftpalternativetouser") \
    X(TCLCURLOPT_SSL_SESSIONID_CACHE,"-sslsessionidcache")        \
    X(TCLCURLOPT_SSH_AUTH_TYPES,"-sshauthtypes")                  \
    X(TCLCURLOPT_SSH_PUBLIC_KEYFILE,"-sshpublickeyfile")          \
    X(TCLCURLOPT_SSH_PRIVATE_KEYFILE,"-sshprivatekeyfile")        \
    X(TCLCURLOPT_TIMEOUT_MS,"-timeoutms")                         \
    X(TCLCURLOPT_CONNECTTIMEOUT_MS,"-connecttimeoutms")           \
    X(TCLCURLOPT_HTTP_CONTENT_DECODING,"-contentdecoding")        \
    X(TCLCURLOPT_HTTP_TRANSFER_DECODING,"-transferdecoding")      \
    X(TCLCURLOPT_KRBLEVEL,"-krblevel")                            \
    X(TCLCURLOPT_NEW_FILE_PERMS,"-newfileperms")                  \
    X(TCLCURLOPT_NEW_DIRECTORY_PERMS,"-newdirectoryperms")        \
    X(TCLCURLOPT_KEYPASSWD,"-keypasswd")                          \
    X(TCLCURLOPT_APPEND,"-append")                                \
    X(TCLCURLOPT_DIRLISTONLY,"-dirlistonly")                      \
    X(TCLCURLOPT_USE_SSL,"-usessl")                               \
    X(TCLCURLOPT_POST301,"-post301")                              \
    X(TCLCURLOPT_SSH_HOST_PUBLIC_KEY_MD5,"-sshhostpublickeymd5")  \
    X(TCLCURLOPT_PROXY_TRANSFER_MODE,"-proxytransfermode")        \
    X(TCLCURLOPT_CRLFILE,"-crlfile")                              \
    X(TCLCURLOPT_ISSUERCERT,"-issuercert")                        \
    X(TCLCURLOPT_ADDRESS_SCOPE,"-addressscope")                   \
    X(TCLCURLOPT_CERTINFO,"-certinfo")                            \
    X(TCLCURLOPT_POSTREDIR,"-postredir")                          \
    X(TCLCURLOPT_USERNAME,"-username")                            \
    X(TCLCURLOPT_PASSWORD,"-password")                            \
    X(TCLCURLOPT_PROXYUSERNAME,"-proxyuser")                      \
    X(TCLCURLOPT_PROXYPASSWORD,"-proxypassword")                  \
    X(TCLCURLOPT_TFTP_BLKSIZE,"-tftpblksize")                     \
    X(TCLCURLOPT_SOCKS5_GSSAPI_SERVICE,"-socks5gssapiservice")    \
    X(TCLCURLOPT_SOCKS5_GSSAPI_NEC,"-socks5gssapinec")            \
    X(TCLCURLOPT_PROTOCOLS,"-protocols")                          \
    X(TCLCURLOPT_REDIR_PROTOCOLS,"-redirprotocols")               \
    X(TCLCURLOPT_FTP_SSL_CCC,"-ftpsslccc")                        \
    X(TCLCURLOPT_SSH_KNOWNHOSTS,"-sshknownhosts")                 \
    X(TCLCURLOPT_SSH_KEYFUNCTION,"-sshkeyproc")                   \
    X(TCLCURLOPT_MAIL_FROM,"-mailfrom")                           \
    X(TCLCURLOPT_MAIL_RCPT,"-mailrcpt")                           \
    X(TCLCURLOPT_FTP_USE_PRET,"-ftpusepret")                      \
    X(TCLCURLOPT_WILDCARDMATCH,"-wildcardmatch")                  \
    X(TCLCURLOPT_CHUNK_BGN_PROC,"-chunkbgnproc")                  \
    X(TCLCURLOPT_CHUNK_BGN_VAR,"-chunkbgnvar")                    \
    X(TCLCURLOPT_CHUNK_END_PROC,"-chunkendproc")                  \
    X(TCLCURLOPT_FNMATCH_PROC,"-fnmatchproc")                     \
    X(TCLCURLOPT_RESOLVE,"-resolve")                              \
    X(TCLCURLOPT_TLSAUTH_USERNAME,"-tlsauthusername")             \
    X(TCLCURLOPT_TLSAUTH_PASSWORD,"-tlsauthpassword")             \
    X(TCLCURLOPT_TLSAUTH_TYPE,"-tlsauthtype")                     \
    X(TCLCURLOPT_TRANSFER_ENCODING,"-transferencoding")           \
    X(TCLCURLOPT_GSSAPI_DELEGATION,"-gssapidelegation")           \
    X(TCLCURLOPT_NOPROXY,"-noproxy")                              \
    X(TCLCURLOPT_TELNETOPTIONS,"-telnetoptions")                  \
    X(TCLCURLOPT_CAINFO_BLOB,"-cainfoblob")

typedef enum {
#define X(sym,option) sym,
    TCLCURL_OPTIONS(X)
#undef X
    TCLCURL_OPTIONS_COUNT
} tclcurl_options_indices;

static const char* const configTable[TCLCURL_OPTIONS_COUNT] {
#define X(sym,option) [sym] = option,
    TCLCURL_OPTIONS(X)
#undef X
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
