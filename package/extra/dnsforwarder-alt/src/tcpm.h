#ifndef TCPM_C_INCLUDED
#define TCPM_C_INCLUDED

#include "mcontext.h"
#include "addresslist.h"
#include "socketpuller.h"

typedef struct _TcpM TcpM;

struct _TcpM {
    /* private */
    volatile SOCKET Departure;

    SOCKET          Incoming;
    Address_Type    IncomingAddr;
    SocketPuller    Puller;

    ModuleContext   Context;

    ThreadHandle    WorkThread;

    AddressList     ServiceList;
    struct sockaddr **Services;
    sa_family_t     *ServiceFamilies;

    AddressList     SocksProxyList;
    struct sockaddr **SocksProxies;
    sa_family_t     *SocksProxyFamilies;

    /* public */
    int (*Send)(TcpM *m,
                IHeader *h, /* Entity followed */
                int BufferLength
                );
};

int TcpM_Init(TcpM *m, const char *Services, const char *SocksProxies);

#endif // TCPM_C_INCLUDED
