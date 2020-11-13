#ifndef TLSSERVER_H_INCLUDED
#define TLSSERVER_H_INCLUDED

#ifdef WIN32
#else /* WIN32 */
#include <curl/curl.h>
#endif /* WIN32 */

typedef struct _TlsServer {
    struct curl_slist *hl;
    const char *p;
} TlsServer;



#endif // TLSSERVER_H_INCLUDED
