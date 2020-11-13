#ifdef WIN32
#include "winmsgque.h"
#include "ptimer.h"

static int WinMsgQue_Post(WinMsgQue *q, const void *Data)
{
    EFFECTIVE_LOCK_GET(q->l);
    if( q->q.Add(&(q->q), Data) != 0 )
    {
        EFFECTIVE_LOCK_RELEASE(q->l);
        return -9;
    }
    SetEvent(q->e);
    EFFECTIVE_LOCK_RELEASE(q->l);
    return 0;
}

/* Remember to free it */
static void *WinMsgQue_Wait(WinMsgQue *q, DWORD *Milliseconds)
{
    void *Peek;
    DWORD TimeWaiting;
    void *ret = NULL;

    EFFECTIVE_LOCK_GET(q->l);
    Peek = q->q.Get(&(q->q));
    EFFECTIVE_LOCK_RELEASE(q->l);

    if( Peek != NULL )
    {
        return Peek;
    } else {
        PTimer t;

        TimeWaiting = Milliseconds == NULL ? INFINITE : *Milliseconds;
        PTimer_Start(&t);
        if( WaitForSingleObject(q->e, TimeWaiting) == WAIT_OBJECT_0 )
        {
            if( Milliseconds != NULL )
            {
                unsigned long ElapsedTime = PTimer_End(&t);

                if( ElapsedTime > *Milliseconds )
                {
                    *Milliseconds = 0;
                } else {
                    *Milliseconds -= ElapsedTime;
                }
            }

            EFFECTIVE_LOCK_GET(q->l);
            ret = q->q.Get(&(q->q));
            EFFECTIVE_LOCK_RELEASE(q->l);
        } else {
            *Milliseconds = 0;
            ret = NULL;
        }

        return ret;
    }
}

static int Compare(const void *One, const void *Two)
{
    return 0;
}

int WinMsgQue_Init(WinMsgQue *q, int MsgSize)
{
    if( LinkedQueue_Init(&(q->q), MsgSize, Compare) != 0 )
    {
        return -8;
    }

    q->e = CreateEvent(NULL, FALSE, FALSE, NULL);
    if( q->e == NULL )
    {
        q->q.Free(&(q->q));
        return -15;
    }

    EFFECTIVE_LOCK_INIT(q->l);

    q->Post = WinMsgQue_Post;
    q->Wait = WinMsgQue_Wait;

    return 0;
}

#endif /* WIN32 */
