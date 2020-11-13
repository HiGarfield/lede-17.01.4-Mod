#ifndef UDPM_H_INCLUDED
#define UDPM_H_INCLUDED

#include "common.h"
#include "addresslist.h"
#include "readconfig.h"
#include "mcontext.h"

typedef struct _UdpM UdpM;

struct _UdpM {
    /* private */
    volatile SOCKET  Departure;
    ModuleContext Context;

    EFFECTIVE_LOCK  Lock;

    ThreadHandle    WorkThread;

    AddressList     AddrList;
    struct { /* parallel query informations */
        /* When these two are not NULL, parallel-query is enabled */
        struct sockaddr **addrs; /* Free it when no longer needed */
        sa_family_t familiy;
        int addrlen;
    } Parallels;

    int CountOfTimeout;

    /* public */
    int (*Send)(UdpM *m,
                IHeader *h, /* Entity followed */
                int BufferLength
                );
};

int UdpM_Init(UdpM *m, const char *Services, BOOL Parallel);

#endif // UDPM_H_INCLUDED
