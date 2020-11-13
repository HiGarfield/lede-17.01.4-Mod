#include <string.h>
#include "tlsm.h"
#include "socketpuller.h"
#include "utils.h"
#include "logs.h"
#include "udpfrontend.h"
#include "timedtask.h"
#include "dnscache.h"
#include "ipmisc.h"
#include "domainstatistic.h"
#include "ptimer.h"

static void SwepWorks(IHeader *h, int Number, TlsM *Module)
{
    ShowTimeOutMessage(h, 'S');
    DomainStatistic_Add(h, STATISTIC_TYPE_REFUSED);

    if( Number == 1  )
    {
        /** TODO */
    }
}

static int TlsM_SendWrapper(TlsM *m, SOCKET s, char *Start, int Count)
{
    size_t nsentall = 0;

    while( TRUE )
    {
        CURLcode State;
        size_t ns = 0;

        State = curl_easy_send(m->Departure,
                               Start + nsentall,
                               Count - nsentall,
                               &ns
                               );

        nsentall += ns;

        if( State == CURLE_AGAIN )
        {
            if( !SocketIsWritable(s, 2000) )
            {
                return -45;
            }
        } else if( State == CURLE_OK )
        {
            break;
        } else {
            return -51;
        }

    }

    if( nsentall > INT_MAX )
    {
        return -58;
    } else {
        return (int)nsentall;
    }
}

static int TlsM_RecvWrapper(TlsM *m, SOCKET s, char *Buffer, int BufferLength)
{
    size_t nr = 0;
    CURLcode State;

    do {
        State = curl_easy_recv(m->Departure,
                               Buffer,
                               BufferLength,
                               &nr
                               );

        if( State == CURLE_AGAIN && !SocketIsStillReadable(s, 2000) )
        {
            return -81;
        }

    } while( State == CURLE_AGAIN );

    if( nr > INT_MAX )
    {
        return -86;
    } else {
        return (int)nr;
    }
}

static int TlsM_Send_Actual(TlsM *m, IHeader *h /* Entity followed */)
{
    uint16_t TCPLength;
    curl_socket_t s;

    if( m->Context.Add(&(m->Context), h) != 0 )
    {
        return -11;
    }

    /* Set up connection */
    if( m->Departure == NULL )
    {
        m->Departure = curl_easy_init();
        if(  m->Departure == NULL )
        {
            ERRORMSG("Fatal error 40.\n");
            return -41;
        }

        curl_easy_setopt(m->Departure, CURLOPT_URL, m->Services[0]);
        curl_easy_setopt(m->Departure, CURLOPT_CONNECT_ONLY, 1L);
        curl_easy_setopt(m->Departure,
                         CURLOPT_SSLVERSION,
                         CURL_SSLVERSION_TLSv1_2
                         );

        if( curl_easy_perform(m->Departure) != CURLE_OK )
        {
            return -52;
        }

        if( curl_easy_getinfo(m->Departure, CURLINFO_ACTIVESOCKET, &s)
           != CURLE_OK
           )
        {
            return -58;
        }

        m->Puller.Add(&(m->Puller), s, NULL, 0);
    } else {
        if( curl_easy_getinfo(m->Departure, CURLINFO_ACTIVESOCKET, &s)
           != CURLE_OK
           )
        {
            return -68;
        }
    }

    /* Preparing content */
    TCPLength = htons(h->EntityLength);
    memcpy((char *)(IHEADER_TAIL(h)) - 2, &TCPLength, 2);

    /* Sending content */
    if( TlsM_SendWrapper(m,
                         s,
                         (char *)(IHEADER_TAIL(h)) - 2,
                         h->EntityLength + 2
                         )
        != h->EntityLength + 2 )
    {
        return -120;
    }

    return 0;
}

static void TlsM_CloseConnection(TlsM *m)
{
    curl_socket_t s;

    if( curl_easy_getinfo(m->Departure, CURLINFO_ACTIVESOCKET, &s)
       != CURLE_OK
       )
    {
        return;
    }

    m->Puller.Del(&(m->Puller), s);

    curl_easy_cleanup(m->Departure);
    m->Departure = NULL;
}

PUBFUNC int TlsM_Send(TlsM *m,
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

static int TlsM_Works(TlsM *m)
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
        ERRORMSG("Fatal error 127.\n");
        return -128;
    }

    Header = (IHeader *)ReceiveBuffer;
    Entity = ReceiveBuffer + sizeof(IHeader);

    while( TRUE )
    {
        TimeOut = TimeLimit;
        s = m->Puller.Select(&(m->Puller), &TimeOut, NULL, TRUE, FALSE);

        if( s == INVALID_SOCKET )
        {
            m->Context.Swep(&(m->Context), (SwepCallback)SwepWorks, m);
            NumberOfCumulated = 0;
        } else if( s == m->Incoming )
        {
            int State;

            if( NumberOfCumulated > 1024 )
            {
                m->Context.Swep(&(m->Context), (SwepCallback)SwepWorks, m);
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

            if( m->Departure != NULL &&
                time(NULL) - LastRecvFromServer > 5 )
            {
                TlsM_CloseConnection(m);
            }

            if( TlsM_Send_Actual(m, Header) != 0 )
            {
                TlsM_CloseConnection(m);

                /* Try again */
                if( TlsM_Send_Actual(m, Header) != 0 )
                {
                    TlsM_CloseConnection(m);
                }

                Retried = TRUE;
            }

        } else /* Departure socket */ {
            int State;
            uint16_t TcpLength;

            if( TlsM_RecvWrapper(m, s, (char *)&TcpLength, 2) != 2 )
            {
                TlsM_CloseConnection(m);
                INFO("TLS server closed the connection.\n");

                if( !Retried )
                {
                    INFO("TLS query retrying...\n");

                    if( TlsM_Send_Actual(m, Header) != 0 )
                    {
                        TlsM_CloseConnection(m);
                    }

                    Retried = TRUE;
                }

                continue;
            }

            TcpLength = ntohs(TcpLength);

            if( TcpLength > LEFT_LENGTH )
            {
                WARNING("TLS segment is too large, discarded.\n");
                TlsM_CloseConnection(m);
                continue;
            }

            if( TlsM_RecvWrapper(m,
                                 s,
                                 Entity,
                                 TcpLength
                                 )
                != TcpLength )
            {
                TlsM_CloseConnection(m);
                continue;
            }

            if( m->Context.FindAndRemove(&(m->Context), Header, Header) != 0 )
            {
                continue;
            }

            switch( IPMiscSingleton_Process(Header) )
            {
            case IP_MISC_ACTION_NOTHING:
                break;

            case IP_MISC_ACTION_BLOCK:
                ShowBlockedMessage(Header, "Bad package, discarded");
                continue;
                break;

            default:
                ERRORMSG("Fatal error 298.\n");
                continue;
                break;
            }

            State = IHeader_SendBack(Header);

            if( State != 0 )
            {
                ShowErrorMessage(Header, 'S');
                continue;
            }

            ShowNormalMessage(Header, 'S');
            DNSCache_AddItemsToCache(Header);
            DomainStatistic_Add(Header, STATISTIC_TYPE_TCP);
        }
    }
}

int TlsM_Init(TlsM *m, const char *Services)
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

    m->Incoming = TryBindLocal(Ipv6_Aviliable(), 10500, &(m->IncomingAddr));
    if( m->Incoming == INVALID_SOCKET )
    {
        return -357;
    }

    m->Puller.Add(&(m->Puller), m->Incoming, NULL, 0);

    m->Departure = NULL;

    if( StringList_Init(&(m->ServiceList), NULL, NULL) != 0 )
    {
        StringList l;
        StringListIterator i;
        const char *one;

        if( StringList_Init(&l, Services, ",") != 0 )
        {
            return -170;
        }

        if( StringListIterator_Init(&i, &l) != 0 )
        {
            return -175;
        }

        while( (one = i.Next(&i)) != NULL )
        {
            char n[512];

            snprintf(n, sizeof(n), "https://%s", one);
            n[sizeof(n) - 1] = '\0';

            m->ServiceList.Add(&(m->ServiceList), n, NULL);
        }

        l.Free(&l);
    }

    m->Services = m->ServiceList.ToCharPtrArray(&(m->ServiceList));
    if( m->Services == NULL )
    {
        return -316;
    }

    m->Send = TlsM_Send;

    CREATE_THREAD(TlsM_Works, m, m->WorkThread);

    return 0;
}
