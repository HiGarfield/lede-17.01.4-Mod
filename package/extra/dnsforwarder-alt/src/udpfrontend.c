#include "udpfrontend.h"
#include "socketpuller.h"
#include "addresslist.h"
#include "utils.h"
#include "mmgr.h"
#include "logs.h"

static BOOL Ipv6_Enabled = FALSE;

static SocketPuller Frontend;

static void UdpFrontend_Work(void *Unused)
{
    /* Buffer */
    #define BUF_LENGTH  2048
    char *ReceiveBuffer;
    IHeader *Header;

    #define LEFT_LENGTH  (BUF_LENGTH - sizeof(IHeader))
    char *Entity;

    ReceiveBuffer = SafeMalloc(BUF_LENGTH);
    if( ReceiveBuffer == NULL )
    {
        ERRORMSG("No enough memory, 26.\n");
        return;
    }

    Header = (IHeader *)ReceiveBuffer;
    Entity = ReceiveBuffer + sizeof(IHeader);

    /* Loop */
    while( TRUE )
    {
        /* Address */
        char AddressBuffer[sizeof(Address_Type)];
        struct sockaddr *IncomingAddress = (struct sockaddr *)AddressBuffer;

        SOCKET sock;
        const sa_family_t *f;

        int RecvState;

        socklen_t AddrLen;

        char Agent[sizeof(Header->Agent)];

        sock = Frontend.Select(&Frontend,
                               NULL,
                               (void **)&f,
                               TRUE,
                               FALSE
                               );
        if( sock == INVALID_SOCKET )
        {
            ERRORMSG("Fatal error 57.\n");
            return;
        }

        AddrLen = sizeof(Address_Type);

        RecvState = recvfrom(sock,
                             Entity,
                             LEFT_LENGTH,
                             0,
                             IncomingAddress,
                             &AddrLen
                             );

        if( *f == AF_INET )
        {
            IPv4AddressToAsc(&(((struct sockaddr_in *)IncomingAddress)->sin_addr),
                             Agent
                             );
        } else {
            IPv6AddressToAsc(&(((struct sockaddr_in6 *)IncomingAddress)->sin6_addr),
                             Agent
                             );
        }

        if( RecvState < 0 )
        {
            INFO("An error occured while receiving from UDP client %s, not a big deal.\n",
                 Agent
                 );
            continue;
        }

        IHeader_Fill(Header,
                     FALSE,
                     Entity,
                     RecvState,
                     IncomingAddress,
                     sock,
                     *f,
                     Agent
                     );

        MMgr_Send(Header, BUF_LENGTH);
    }
}

void UdpFrontend_StartWork(void)
{
    ThreadHandle t;

    CREATE_THREAD(UdpFrontend_Work, NULL, t);
    DETACH_THREAD(t);
}

int UdpFrontend_Init(ConfigFileInfo *ConfigInfo, BOOL StartWork)
{
    StringList *UDPLocal;
    StringListIterator i;
    const char *One;

    int Count = 0;

    UDPLocal = ConfigGetStringList(ConfigInfo, "UDPLocal");
    if( UDPLocal == NULL )
    {
        ERRORMSG("No UDP interface specified.\n");
        return -11;
    }

    if( StringListIterator_Init(&i, UDPLocal) != 0 )
    {
        return -20;
    }

    if( SocketPuller_Init(&Frontend) != 0 )
    {
        return -19;
    }

    while( (One = i.Next(&i)) != NULL )
    {
        Address_Type a;
        sa_family_t f;

        SOCKET sock;

        f = AddressList_ConvertFromString(&a, One, 53);
        if( f == AF_UNSPEC )
        {
            ERRORMSG("Invalid `UDPLocal' option : %s .\n", One);
            continue;
        }

        sock = socket(f, SOCK_DGRAM, IPPROTO_UDP);
        if( sock == INVALID_SOCKET )
        {
            continue;
        }

        if( bind(sock,
                 (const struct sockaddr *)&(a.Addr),
                 GetAddressLength(f)
                 )
            != 0 )
        {
            char p[128];

            snprintf(p, sizeof(p), "Opening UDP interface %s failed", One);
            p[sizeof(p) - 1] = '\0';

            ShowSocketError(p, GET_LAST_ERROR());
            CLOSE_SOCKET(sock);
            continue;
        }

        if( f == AF_INET6 )
        {
            Ipv6_Enabled = TRUE;
        }

        Frontend.Add(&Frontend, sock, &f, sizeof(sa_family_t));
        INFO("UDP interface %s opened.\n", One);
        ++Count;
    }

    if( Count == 0 )
    {
        ERRORMSG("No UDP interface opened.\n");
        return -163;
    }

    if( StartWork )
    {
        UdpFrontend_StartWork();
    }

    return 0;
}

BOOL Ipv6_Aviliable(void)
{
    return Ipv6_Enabled;
}
