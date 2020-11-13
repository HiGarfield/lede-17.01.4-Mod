#ifndef TLSM_C_INCLUDED
#define TLSM_C_INCLUDED

#ifdef WIN32
#else /* WIN32 */
#include <curl/curl.h>
#endif /* WIN32 */

#include "mcontext.h"
#include "socketpuller.h"
#include "stringlist.h"

typedef struct _TlsM TlsM;

struct _TlsM {
    /* private */
#ifdef WIN32
#else /* WIN32 */
    CURL            *Departure;
#endif /* WIN32 */

    StringList      ServiceList;
    const char      **Services;

    SOCKET          Incoming;
    Address_Type    IncomingAddr;
    SocketPuller    Puller;

    ModuleContext   Context;

    ThreadHandle    WorkThread;

    /* public */
    int (*Send)(TlsM *m,
                IHeader *h, /* Entity followed */
                int BufferLength
                );
};

int TlsM_Init(TlsM *m, const char *Services);

#endif // TLSM_C_INCLUDED
