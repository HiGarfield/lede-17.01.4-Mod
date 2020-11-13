#include "socketpuller.h"

PUBFUNC int SocketPuller_Add(SocketPuller *p,
                             SOCKET s,
                             const void *Data,
                             int DataLength
                             )
{
    if( s == INVALID_SOCKET )
    {
        return -11;
    }

    if( p->p.Add(&(p->p), s, Data, DataLength) != 0 )
    {
        return -16;
    }

    if( s > p->Max )
    {
        p->Max = s;
    }

    FD_SET(s, &(p->s));

    return 0;
}

PUBFUNC int SocketPuller_Del(SocketPuller *p, SOCKET Sock)
{
    if( p->p.Del(&(p->p), Sock) != 0 )
    {
        return -33;
    }

    FD_CLR(Sock, &(p->s));

    return 0;
}

PUBFUNC SOCKET SocketPuller_Select(SocketPuller *p,
                                   struct timeval *tv,
                                   void **Data,
                                   BOOL Reading,
                                   BOOL Writing
                                   )
{
    fd_set ReadySet;

    ReadySet = p->s;

    while( TRUE )
    {
        switch( select(p->Max + 1,
                       Reading ? &ReadySet : NULL,
                       Writing ? &ReadySet : NULL,
                       NULL,
                       tv)
                )
        {
        case SOCKET_ERROR:
            if( FatalErrorDecideding(GET_LAST_ERROR()) == 0 )
            {
                continue;
            }
            /* No break; */
        case 0:
            return INVALID_SOCKET;
            break;

        default:
            return p->p.FetchOnSet(&(p->p), &ReadySet, Data);
            break;
        }
    }
}

PUBFUNC BOOL SocketPuller_IsEmpty(SocketPuller *p)
{
    return !(p->Max >= 0);
}

PUBFUNC void SocketPuller_CloseAll(SocketPuller *p, SOCKET ExceptFor)
{
    p->p.CloseAll(&(p->p), ExceptFor);
}

PUBFUNC void SocketPuller_Free(SocketPuller *p)
{
    p->p.Free(&(p->p), TRUE);
}

PUBFUNC void SocketPuller_FreeWithoutClose(SocketPuller *p)
{
    p->p.Free(&(p->p), FALSE);
}

int SocketPuller_Init(SocketPuller *p)
{
    p->Max = -1;

    p->Add = SocketPuller_Add;
    p->Del = SocketPuller_Del;
    p->Select = SocketPuller_Select;
    p->IsEmpty = SocketPuller_IsEmpty;
    p->CloseAll = SocketPuller_CloseAll;
    p->Free = SocketPuller_Free;
    p->FreeWithoutClose = SocketPuller_FreeWithoutClose;

    FD_ZERO(&(p->s));
    return SocketPool_Init(&(p->p));
}
