#include <string.h>
#include "tcpm.h"
#include "stringlist.h"
#include "socketpuller.h"
#include "utils.h"
#include "logs.h"
#include "udpfrontend.h"
#include "timedtask.h"
#include "dnscache.h"
#include "ipmisc.h"
#include "domainstatistic.h"
#include "ptimer.h"

static void SweepWorks(IHeader *h, int Number, TcpM *Module)
{
    ShowTimeOutMessage(h, 'T');
    DomainStatistic_Add(h, STATISTIC_TYPE_REFUSED);

    if( Number == 1 && Module->SocksProxies == NULL )
    {
        AddressList_Advance(&(Module->ServiceList));
    }
}

static SOCKET TcpM_Connect(struct sockaddr  **ServerAddressesList,
                           sa_family_t      *FamiliesList,
                           const char       *Type
                           )
{
#   define  CONNECT_TIMEOUT 5
#   define  NUMBER_OF_SOCKETS 5

#ifdef WIN32
	PTimer  t;
#endif

    SocketPuller p;
    int i; /* Loop */
    SOCKET Final;

    struct timeval TimeLimit = {CONNECT_TIMEOUT, 0};

    if( SocketPuller_Init(&p) != 0 )
    {
        return -23;
    }

    INFO("Connecting to %s ...\n", Type);

#ifdef WIN32
	PTimer_Start(&t);
#endif

    for( i = 0; i < NUMBER_OF_SOCKETS; ++i )
    {
        SOCKET s;

		if( ServerAddressesList[i] == NULL )
		{
			break;
		}

		s = socket(FamiliesList[i], SOCK_STREAM, IPPROTO_TCP);
        if( s == INVALID_SOCKET )
        {
			continue;
        }

        SetSocketNonBlock(s, TRUE);

		if( connect(s,
                    ServerAddressesList[i],
                    GetAddressLength(FamiliesList[i])
                    )
            != 0 )
		{
			if( GET_LAST_ERROR() != CONNECT_FUNCTION_BLOCKED )
			{
				CLOSE_SOCKET(s);
				continue;
			}
		}

        p.Add(&p, s, NULL, 0);
    }

    if( p.IsEmpty(&p) )
    {
        p.Free(&p);
        INFO("Connecting to %s failed, 90.\n", Type);
        return INVALID_SOCKET;
    }

    Final = p.Select(&p, &TimeLimit, NULL, FALSE, TRUE);

    p.CloseAll(&p, Final);
    p.FreeWithoutClose(&p);

    if( Final == INVALID_SOCKET )
    {
        INFO("Connecting to %s timed out.\n", Type);
    } else {
#ifdef WIN32
        INFO("TCP connection to %s established. Time consumed : %lums\n",
             Type,
             PTimer_End(&t)
             );
#else
        INFO("TCP connection to %s established. Time consumed : %d.%ds\n",
             Type,
             CONNECT_TIMEOUT == TimeLimit.tv_sec ? 0 : ((int)(CONNECT_TIMEOUT - 1 - TimeLimit.tv_sec)),
             CONNECT_TIMEOUT == TimeLimit.tv_sec ? 0 : ((int)(1000000 - TimeLimit.tv_usec))
             );
#endif
    }

    return Final;
}

static int TcpM_SendWrapper(SOCKET Sock, const char *Start, int Length)
{
#define DEFAULT_TIME_OUT__SEND 2000 /* ms */
    while( send(Sock, Start, Length, MSG_NOSIGNAL) != Length )
	{
		int LastError = GET_LAST_ERROR();
        if( FatalErrorDecideding(LastError) != 0 ||
                !SocketIsWritable(Sock, DEFAULT_TIME_OUT__SEND)
                )
        {
            ShowSocketError("Sending to TCP server or proxy failed", LastError);
            return (-1) * LastError;
        }
    }

#undef DEFAULT_TIME_OUT__SEND
	return Length;
}

static int TcpM_RecvWrapper(SOCKET Sock, char *Buffer, int BufferSize)
{
#define DEFAULT_TIME_OUT__RECV 2000 /* ms */
	int Recvlength;

	while( (Recvlength = recv(Sock, Buffer, BufferSize, 0)) < 0 )
	{
		int LastError = GET_LAST_ERROR();
        if( FatalErrorDecideding(LastError) != 0 ||
                !SocketIsStillReadable(Sock, DEFAULT_TIME_OUT__RECV)
                )
        {
            ShowSocketError("Receiving from TCP server or proxy failed", LastError);
            return (-1) * LastError;
        }
	}
#undef DEFAULT_TIME_OUT__RECV
	return Recvlength;
}

static int TcpM_ProxyPreparation(SOCKET Sock,
                                 const struct sockaddr  *NestedAddress,
                                 sa_family_t Family
                                 )
{
    char AddressInfos[4 + 1 + LENGTH_OF_IPV6_ADDRESS_ASCII + 2 + 1];
    char *AddressString = AddressInfos + 5;
    char NumberOfCharacter;
    unsigned short Port;
    char RecvBuffer[16];

	if( TcpM_SendWrapper(Sock, "\x05\x01\x00", 3) != 3 )
	{
		ERRORMSG("Cannot communicate with TCP proxy, negotiation error.\n");
		return -1;
	}

    if( TcpM_RecvWrapper(Sock, RecvBuffer, 2) != 2 )
    {
		ERRORMSG("Cannot communicate with TCP proxy, negotiation error.\n");
        return -2;
    }

	if( RecvBuffer[0] != '\x05' || RecvBuffer[1] != '\x00' )
	{
		/*printf("---------3 : %x %x\n", RecvBuffer[0], RecvBuffer[1]);*/
		ERRORMSG("Cannot communicate with TCP proxy, negotiation error.\n");
		return -3;
	}

	memcpy(AddressInfos, "\x05\x01\x00\x03", 4);

    if( Family == AF_INET )
    {
        IPv4AddressToAsc(&(((const struct sockaddr_in *)NestedAddress)->sin_addr), AddressString);
		Port = ((const struct sockaddr_in *)NestedAddress)->sin_port;
    } else {
		IPv6AddressToAsc(&(((const struct sockaddr_in6 *)NestedAddress)->sin6_addr), AddressString);
		Port = ((const struct sockaddr_in6 *)NestedAddress)->sin6_port;
    }

    NumberOfCharacter = strlen(AddressString);
    memcpy(AddressInfos + 4, &NumberOfCharacter, 1);
    memcpy(AddressInfos + 5 + NumberOfCharacter,
           (const char *)&Port,
           sizeof(Port)
           );

	INFO("Connecting to TCP server.\n");

	if( TcpM_SendWrapper(Sock,
                         AddressInfos,
                         4 + 1 + NumberOfCharacter + 2
                         )
     != 4 + 1 + NumberOfCharacter + 2 )
	{
	    ERRORMSG("Cannot communicate with TCP proxy, connection to TCP server error.\n");
		return -4;
	}

/*
	if( TcpM_SendWrapper(Sock, "\x05\x01\x00\x03", 4) != 4 )
	{
	    ERRORMSG("Cannot communicate with TCP proxy, connection to TCP server error.\n");
		return -4;
	}

	if( TcpM_SendWrapper(Sock, &NumberOfCharacter, 1) != 1 )
	{
		ERRORMSG("Cannot communicate with TCP proxy, connection to TCP server error.\n");
		return -5;
	}
	if( TcpM_SendWrapper(Sock, AddressString, NumberOfCharacter) != NumberOfCharacter )
	{
		ERRORMSG("Cannot communicate with TCP proxy, connection to TCP server error.\n");
		return -6;
	}
	if( TcpM_SendWrapper(Sock, (const char *)&Port, sizeof(Port)) != sizeof(Port) )
	{
		ERRORMSG("Cannot communicate with TCP proxy, connection to TCP server error.\n");
		return -7;
	}
*/
    if( TcpM_RecvWrapper(Sock, RecvBuffer, 4) != 4 )
    {
		ERRORMSG("Cannot communicate with TCP proxy, connection to TCP server error.\n");
		return -9;
    }

	if( RecvBuffer[1] != '\x00' )
	{
		ERRORMSG("Cannot communicate with TCP proxy, connection to TCP server error.\n");
		return -10;
	}

	switch( RecvBuffer[3] )
	{
		case 0x01:
			NumberOfCharacter = 6;
			break;

		case 0x03:
			TcpM_RecvWrapper(Sock, &NumberOfCharacter, 1);
			NumberOfCharacter += 2;
			break;

		case 0x04:
			NumberOfCharacter = 18;
			break;

		default:
			/*printf("------Here : %d %d %d %d\n", RecvBuffer[0], RecvBuffer[1], RecvBuffer[2], RecvBuffer[3]);*/
			ERRORMSG("Cannot communicate with TCP proxy, connection to TCP server error.\n");
			return -11;
	}
	ClearTCPSocketBuffer(Sock, NumberOfCharacter);

	INFO("Connected to TCP server.\n");

	return 0;

}

static int TcpM_Send_Actual(TcpM *m, IHeader *h /* Entity followed */)
{
    uint16_t TCPLength;

    if( m->Context.Add(&(m->Context), h) != 0 )
    {
        return -11;
    }

    /* Set up socket */
    if( m->Departure == INVALID_SOCKET )
    {
        SOCKET s = INVALID_SOCKET;
        if( m->SocksProxies == NULL )
        {
            /* Non-proxied */
            s = TcpM_Connect(m->Services, m->ServiceFamilies, "server");
            if( s == INVALID_SOCKET )
            {
                return -122;
            }
        } else {
            /* Proxied */
            struct sockaddr *addr;
            sa_family_t family;

            s = TcpM_Connect(m->SocksProxies,
                             m->SocksProxyFamilies,
                             "proxy"
                             );

            if( s == INVALID_SOCKET )
            {
                return -187;
            }

            addr = AddressList_GetOne(&(m->ServiceList), &family);
            if( addr == NULL )
            {
                CLOSE_SOCKET(s);
                return -324;
            }

            if( TcpM_ProxyPreparation(s, addr, family) != 0 )
            {
                CLOSE_SOCKET(s);
                AddressList_Advance(&(m->ServiceList));
                return -330;
            }
        }

        m->Departure = s;
        m->Puller.Add(&(m->Puller), m->Departure, NULL, 0);
    }

    TCPLength = htons(h->EntityLength);
    memcpy((char *)(IHEADER_TAIL(h)) - 2, &TCPLength, 2);

    if( TcpM_SendWrapper(m->Departure,
                         (char *)(IHEADER_TAIL(h)) - 2,
                         h->EntityLength + 2
                         )
        < 0 )
    {
        if( m->SocksProxies != NULL )
        {
            AddressList_Advance(&(m->ServiceList));
        }

        return -174;
    }

    return 0;
}

PUBFUNC int TcpM_Send(TcpM *m,
                      IHeader *h, /* Entity followed */
                      int BufferLength
                      )
{
    int State;

    State = sendto(m->Incoming,
                   (const char *)h,
                   sizeof(IHeader) + h->EntityLength,
                   MSG_NOSIGNAL,
                   (const struct sockaddr *)&(m->IncomingAddr.Addr),
                   GetAddressLength(m->IncomingAddr.family)
                   );

    return !(State > 0);
}

static int TcpM_Works(TcpM *m)
{
    SOCKET  s;

    #define BUF_LENGTH  2048
    char *ReceiveBuffer;
    IHeader *Header;

    #define LEFT_LENGTH  (BUF_LENGTH - sizeof(IHeader))
    char *Entity;

    static const struct timeval TimeLimit = {5, 0};
    struct timeval TimeOut;

    time_t  LastRecvFromServer = 0;

    BOOL Retried = FALSE;

    int NumberOfCumulated = 0;

    ReceiveBuffer = SafeMalloc(BUF_LENGTH);
    if( ReceiveBuffer == NULL )
    {
        ERRORMSG("Fatal error 381.\n");
        return -383;
    }

    Header = (IHeader *)ReceiveBuffer;
    Entity = ReceiveBuffer + sizeof(IHeader);

    while( TRUE )
    {
        TimeOut = TimeLimit;
        s = m->Puller.Select(&(m->Puller), &TimeOut, NULL, TRUE, FALSE);

        if( s == INVALID_SOCKET )
        {
            m->Context.Swep(&(m->Context), (SwepCallback)SweepWorks, m);
            NumberOfCumulated = 0;
        } else if( s == m->Departure )
        {
            int State;
            uint16_t TCPLength;

            State = TcpM_RecvWrapper(s, (char *)&TCPLength, 2);
            if( State < 2 )
            {
                m->Puller.Del(&(m->Puller), s);
                CLOSE_SOCKET(s);
                m->Departure = INVALID_SOCKET;

                if( m->SocksProxies == NULL )
                {
                    INFO("TCP server closed the connection.\n");
                } else {
                    INFO("TCP proxy closed the connection.\n");
                }

                /* Try again */
                if( !Retried )
                {
                    INFO("TCP query retrying...\n");

                    TcpM_Send_Actual(m, Header);

                    Retried = TRUE;
                }

                continue;
            }

            LastRecvFromServer = time(NULL);

            Retried = TRUE;

            TCPLength = ntohs(TCPLength);

            if( TCPLength > LEFT_LENGTH )
            {
                WARNING("TCP segment is too large, discarded.\n");
                m->Puller.Del(&(m->Puller), s);
                CLOSE_SOCKET(s);
                m->Departure = INVALID_SOCKET;
                continue;
            }

            State = TcpM_RecvWrapper(s, Entity, TCPLength);
            if( State != TCPLength )
            {
                m->Puller.Del(&(m->Puller), s);
                CLOSE_SOCKET(s);
                m->Departure = INVALID_SOCKET;
                continue;
            }

            IHeader_Fill(Header,
                         FALSE,
                         Entity,
                         State,
                         NULL,
                         INVALID_SOCKET,
                         AF_UNSPEC,
                         NULL
                         );

            if( m->Context.FindAndRemove(&(m->Context), Header, Header) != 0 )
            {
                continue;
            }

            switch( IPMiscSingleton_Process(Header) )
            {
            case IP_MISC_NOTHING:
                break;

            case IP_MISC_FILTERED_IP:
                ShowBlockedMessage(Header, "Bad package, discarded");
                DomainStatistic_Add(Header, STATISTIC_TYPE_BLOCKEDMSG);
                continue;
                break;

            case IP_MISC_NEGATIVE_RESULT:
                ShowBlockedMessage(Header, "Negative result, discarded");
                DomainStatistic_Add(Header, STATISTIC_TYPE_BLOCKEDMSG);
                continue;
                break;

            default:
                ERRORMSG("Fatal error 155.\n");
                continue;
                break;
            }

            State = IHeader_SendBack(Header);

            if( State != 0 )
            {
                ShowErrorMessage(Header, 'T');
                continue;
            }

            ShowNormalMessage(Header, 'T');
            DNSCache_AddItemsToCache(Header);
            DomainStatistic_Add(Header, STATISTIC_TYPE_TCP);

        } else /* s == m->Incoming */ {
            int State;

            if( NumberOfCumulated > 1024 )
            {
                m->Context.Swep(&(m->Context), (SwepCallback)SweepWorks, m);
                NumberOfCumulated = 0;
            }

            State = recvfrom(s,
                             ReceiveBuffer, /* Receiving a header */
                             BUF_LENGTH,
                             0,
                             NULL,
                             NULL
                             );

            if( State <= 0 )
            {
                Retried = TRUE;
                continue;
            }

            ++NumberOfCumulated;

            Retried = FALSE;

            if( m->Departure != INVALID_SOCKET &&
                (time(NULL) - LastRecvFromServer > 5 /*||
                 !SocketIsWritable(m->Departure, 3000)*/
                 )
                )
            {
                m->Puller.Del(&(m->Puller), m->Departure);
                CLOSE_SOCKET(m->Departure);
                m->Departure = INVALID_SOCKET;
            }

            if( TcpM_Send_Actual(m, Header) != 0 )
            {
                m->Puller.Del(&(m->Puller), m->Departure);
                CLOSE_SOCKET(m->Departure);
                m->Departure = INVALID_SOCKET;

                /* Try again */
                INFO("TCP query retrying...\n");

                TcpM_Send_Actual(m, Header);

                Retried = TRUE;
            }
        }
    }
}

int TcpM_Init(TcpM *m, const char *Services, const char *SocksProxies)
{
    if( m == NULL || Services == NULL )
    {
        return -7;
    }

    if( ModuleContext_Init(&(m->Context)) != 0 )
    {
        return -12;
    }

    if( SocketPuller_Init(&(m->Puller)) != 0 )
    {
        return -389;
    }

    m->Incoming = TryBindLocal(Ipv6_Aviliable(), 10400, &(m->IncomingAddr));
    if( m->Incoming == INVALID_SOCKET )
    {
        return -357;
    }

    m->Puller.Add(&(m->Puller), m->Incoming, NULL, 0);

    m->Departure = INVALID_SOCKET;

    if( AddressList_Init(&(m->ServiceList)) != 0 )
    {
        return -17;
    } else {
        StringList l;
        StringListIterator i;
        const char *Itr;

        if( StringList_Init(&l, Services, ", ") != 0 )
        {
            return -23;
        }

        l.TrimAll(&l, "\t .");

        if( StringListIterator_Init(&i, &l) != 0 )
        {
            return -29;
        }

        while( (Itr = i.Next(&i)) != NULL )
        {
            AddressList_Add_From_String(&(m->ServiceList), Itr, 53);
        }

        l.Free(&l);

        if( SocksProxies == NULL )
        {
            m->Services = AddressList_GetPtrList(&(m->ServiceList),
                                                 &(m->ServiceFamilies)
                                                 );

            if( m->Services == NULL )
            {
                return -45;
            }
        }
    }

    if( SocksProxies != NULL )
    {
        /* Proxied */
        if( AddressList_Init(&(m->SocksProxyList)) != 0 )
        {
            return -53;
        } else {
            StringList l;
            StringListIterator i;
            const char *Itr;

            if( StringList_Init(&l, SocksProxies, ", ") != 0 )
            {
                return -61;
            }

            l.TrimAll(&l, "\t .");

            if( StringListIterator_Init(&i, &l) != 0 )
            {
                return -58;
            }

            while( (Itr = i.Next(&i)) != NULL )
            {
                AddressList_Add_From_String(&(m->SocksProxyList), Itr, 1080);
            }

            l.Free(&l);

            m->SocksProxies = AddressList_GetPtrList(&(m->SocksProxyList),
                                                      &(m->SocksProxyFamilies)
                                                      );

            if( m->SocksProxies == NULL )
            {
                return -84;
            }
        }
    } else {
        /* Non-proxied */
        m->SocksProxies = NULL;
        m->SocksProxyFamilies = NULL;
    }

    m->Send = TcpM_Send;

    CREATE_THREAD(TcpM_Works, m, m->WorkThread);

    return 0;
}
