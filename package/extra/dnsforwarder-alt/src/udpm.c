#include <string.h>
#include <time.h>
#include "udpm.h"
#include "logs.h"
#include "utils.h"
#include "dnscache.h"
#include "ipmisc.h"
#include "domainstatistic.h"
#include "timedtask.h"

static void SweepWorks(IHeader *h, int Number, UdpM *Module)
{
    ShowTimeOutMessage(h, 'U');
    DomainStatistic_Add(h, STATISTIC_TYPE_REFUSED);
    ++(Module->CountOfTimeout);

    if( Number == 1 )
    {
        AddressList_Advance(&(Module->AddrList));
    }
}

static int SwepTask(UdpM *m, SwepCallback cb)
{
    EFFECTIVE_LOCK_GET(m->Lock);
    m->Context.Swep(&(m->Context), cb, m);
    EFFECTIVE_LOCK_RELEASE(m->Lock);

    return 0;
}

static void UdpM_Works(UdpM *m)
{
    static const struct timeval	ShortTime = {10, 0};
    struct timeval Timeout;

    struct sockaddr *addr;

    #define BUF_LENGTH  2048
    char *ReceiveBuffer;
    IHeader *Header;

    #define LEFT_LENGTH  (BUF_LENGTH - sizeof(IHeader))
    char *Entity;

    fd_set	ReadSet, ReadySet;

    ReceiveBuffer = SafeMalloc(BUF_LENGTH);
    if( ReceiveBuffer == NULL )
    {
        ERRORMSG("Fatal error 30.\n");
        return;
    }

    Header = (IHeader *)ReceiveBuffer;
    Entity = ReceiveBuffer + sizeof(IHeader);

    while( TRUE )
    {
        int RecvState;
        int ContextState;

        /* Set up socket */
        if( m->Departure == INVALID_SOCKET )
        {
            EFFECTIVE_LOCK_GET(m->Lock);
            if( m->Parallels.addrs == NULL )
            {
                sa_family_t	family;

                addr = AddressList_GetOne(&(m->AddrList), &family);
                if( addr == NULL )
                {
                    ERRORMSG("Fatal error 53.\n");
                    EFFECTIVE_LOCK_RELEASE(m->Lock);
                    return;
                }

                m->Departure = socket(family, SOCK_DGRAM, IPPROTO_UDP);
            } else { /* Parallel query */
                m->Departure = socket(m->Parallels.familiy,
                                      SOCK_DGRAM,
                                      IPPROTO_UDP
                                      );
            }

            if( m->Departure == INVALID_SOCKET )
            {
                ERRORMSG("Fatal error 68.\n");
                EFFECTIVE_LOCK_RELEASE(m->Lock);
                return;
            }

            m->CountOfTimeout = 0;

            EFFECTIVE_LOCK_RELEASE(m->Lock);

            FD_ZERO(&ReadSet);
            FD_SET(m->Departure, &ReadSet);
        }

        ReadySet = ReadSet;
        Timeout = ShortTime;
        switch( select(m->Departure + 1, &ReadySet, NULL, NULL, &Timeout) )
        {
            case SOCKET_ERROR:
                WARNING("SOCKET_ERROR reached, 98.\n");
                FD_CLR(m->Departure, &ReadSet);
                CLOSE_SOCKET(m->Departure);
                m->Departure = INVALID_SOCKET;
                continue;
                break;

            case 0:
                #define RECREATION_THRESHOLD    8
                if( m->CountOfTimeout > RECREATION_THRESHOLD )
                {
                    FD_CLR(m->Departure, &ReadSet);
                    CLOSE_SOCKET(m->Departure);
                    m->Departure = INVALID_SOCKET;

                    WARNING("UDP socket is about to be recreated.\n");
                }
                continue;
                break;

            default:
                /* Goto recv job */
                break;
        }

        /* recv */
        RecvState = recvfrom(m->Departure,
                             Entity,
                             LEFT_LENGTH,
                             0,
                             NULL,
                             NULL
                             );

        if( RecvState <= 0 )
        {
            ShowErrorMessage(Header, 'U');
            continue;
        }

        m->CountOfTimeout = 0;

        /* Fill IHeader */
        IHeader_Fill(Header,
                     FALSE,
                     Entity,
                     RecvState,
                     NULL,
                     INVALID_SOCKET,
                     AF_UNSPEC,
                     NULL
                     );

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

        if( IHeader_Blocked(Header) )
        {
            ShowBlockedMessage(Header, "False package, discarded");
            DomainStatistic_Add(Header, STATISTIC_TYPE_BLOCKEDMSG);
            continue;
        }

        /* Fetch context item */
        EFFECTIVE_LOCK_GET(m->Lock);
        ContextState = m->Context.FindAndRemove(&(m->Context),
                                                Header,
                                                Header
                                                );
        EFFECTIVE_LOCK_RELEASE(m->Lock);
        if( ContextState == 0 )
        {
            int SentState;

            SentState = IHeader_SendBack(Header);

            if( SentState != 0 )
            {
                ShowErrorMessage(Header, 'U');
                continue;
            }

            ShowNormalMessage(Header, 'U');
            DNSCache_AddItemsToCache(Header);
            DomainStatistic_Add(Header, STATISTIC_TYPE_UDP);
        }
    }

    SafeFree(ReceiveBuffer);
}

static int UdpM_Send(UdpM *m,
                     IHeader *h, /* Entity followed */
                     int BufferLength
                     )
{
    int ret = 0;

    IHeader_AddFakeEdns(h, BufferLength);

    EFFECTIVE_LOCK_GET(m->Lock);
    if( m->Context.Add(&(m->Context), h) != 0 )
    {
        EFFECTIVE_LOCK_RELEASE(m->Lock);
        return -242;
    }
    EFFECTIVE_LOCK_RELEASE(m->Lock);

    if( m->Departure != INVALID_SOCKET )
    {
        if( m->Parallels.addrs != NULL )
        { /* Parallel query */
            struct sockaddr **a = m->Parallels.addrs;

            ret = 0;

            while( *a != NULL )
            {
                int State;

                State = sendto(m->Departure,
                               (const void *)(h + 1),
                               h->EntityLength,
                               MSG_NOSIGNAL,
                               *a,
                               m->Parallels.addrlen
                               );

                ret |= (State > 0);

                ++a;
            }

        } else {
            struct sockaddr *a;
            sa_family_t	family;

            int State;

            a = AddressList_GetOne(&(m->AddrList), &family);
            if( a == NULL )
            {
                ERRORMSG("Fatal error 205.\n");
                ret = -277;
            }

            State = sendto(m->Departure,
                           (const void *)(h + 1),
                           h->EntityLength,
                           MSG_NOSIGNAL,
                           a,
                           GetAddressLength(family)
                           );

            ret = (State > 0);

            /** TODO: Error handlings */

        }
    }

    /*EFFECTIVE_LOCK_RELEASE(m->Lock);*/
    return !ret;
}

int UdpM_Init(UdpM *m, const char *Services, BOOL Parallel)
{
    StringList	Addresses;
    StringListIterator  sli;
    const char *Itr;

    if( m == NULL || Services == NULL )
    {
        return -141;
    }

    m->Departure = INVALID_SOCKET;
    if( StringList_Init(&Addresses, Services, ", ") != 0 )
    {
        return -364;
    }

    Addresses.TrimAll(&Addresses, "\t .");

    if( StringListIterator_Init(&sli, &Addresses) != 0 )
    {
        Addresses.Free(&Addresses);
        return -169;
    }

    if( AddressList_Init(&(m->AddrList)) != 0 )
    {
        Addresses.Free(&Addresses);
        return -171;
    }

    Itr = sli.Next(&sli);
    while( Itr != NULL )
    {
        AddressList_Add_From_String(&(m->AddrList), Itr, 53);
        Itr = sli.Next(&sli);
    }

    Addresses.Free(&Addresses);

    if( Parallel )
    {
        if( AddressList_GetOneBySubscript(&(m->AddrList),
                                          &(m->Parallels.familiy),
                                          0
                                          )
           == NULL )
        {
            return -184;
        }

        m->Parallels.addrs =
            AddressList_GetPtrListOfFamily(&(m->AddrList),
                                           m->Parallels.familiy
                                           );

        m->Parallels.addrlen = GetAddressLength(m->Parallels.familiy);

    } else {
        m->Parallels.addrs = NULL;
        m->Parallels.familiy = AF_UNSPEC;
        m->Parallels.addrlen = 0;
    }

    if( ModuleContext_Init(&(m->Context)) != 0 )
    {
        return -143;
    }

    m->CountOfTimeout = 0;

    EFFECTIVE_LOCK_INIT(m->Lock);

    m->Send = UdpM_Send;

    CREATE_THREAD(UdpM_Works, m, m->WorkThread);

    TimedTask_Add(TRUE,
                  FALSE,
                  10000,
                  (TaskFunc)SwepTask,
                  m,
                  SweepWorks,
                  FALSE
                  );

    return 0;
}
