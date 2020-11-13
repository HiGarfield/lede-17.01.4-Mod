#include "timedtask.h"
#include "linkedqueue.h"
#include "pipes.h"
#include "logs.h"

#ifdef WIN32
#include "winmsgque.h"
#endif /* WIN32 */

typedef struct _TaskInfo{
    TaskFunc    Task;

    void    *Arg1;
    void    *Arg2;

#ifdef WIN32
    DWORD   TimeOut;
    DWORD   LeftTime;
#else /* WIN32 */
    struct timeval  TimeOut;
    struct timeval  LeftTime;
#endif /* WIN32 */

    BOOL    Persistent;
    BOOL    Asynchronous;
} TaskInfo;

static LinkedQueue  TimeQueue;

#ifdef WIN32
static WinMsgQue    MsgQue;
#else /* WIN32 */
static PIPE_HANDLE  WriteTo, ReadFrom;
#endif /* WIN32 */

#ifndef WIN32
static int Tv_Comapre(const struct timeval *one, const struct timeval *two)
{
    if( one->tv_sec == two->tv_sec )
    {
        return one->tv_usec - two->tv_usec;
    } else {
        return one->tv_sec - two->tv_sec;
    }
}

static int Tv_Subtract(struct timeval *Minuend,
                       const struct timeval *Subtrahend
                       )
{
    if( Tv_Comapre(Minuend, Subtrahend) <= 0 )
    {
        Minuend->tv_sec = 0;
        Minuend->tv_usec = 0;
        return 0;
    } else {
        if( Minuend->tv_usec >= Subtrahend->tv_usec )
        {
            Minuend->tv_usec -= Subtrahend->tv_usec;
            Minuend->tv_sec -= Subtrahend->tv_sec;
        } else {
            Minuend->tv_sec -= (1 + Subtrahend->tv_sec);
            Minuend->tv_usec = 1000000 - Subtrahend->tv_usec + Minuend->tv_usec;
        }
        return 1;
    }
}
#endif /* WIN32 */

#ifdef WIN32
static void TimeTask_ReduceTime(const DWORD tv)
{
    LinkedQueueIterator i;
    TaskInfo *ti;

    if( LinkedQueueIterator_Init(&i, &TimeQueue) != 0 )
    {
        /** TODO: Show fatal error */
        return;
    }

    while( (ti = i.Next(&i)) != NULL )
    {
        if( tv > ti->LeftTime )
        {
            ti->LeftTime = 0;
        } else {
            ti->LeftTime -= tv;
        }
    }
}
#else /* WIN32 */
static void TimeTask_ReduceTime(const struct timeval *tv)
{
    LinkedQueueIterator i;
    TaskInfo *ti;

    if( tv == NULL )
    {
        /** TODO: Show fatal error */
        return;
    }

    if( LinkedQueueIterator_Init(&i, &TimeQueue) != 0 )
    {
        /** TODO: Show fatal error */
        return;
    }

    while( (ti = i.Next(&i)) != NULL )
    {
        Tv_Subtract(&(ti->LeftTime), tv);
    }
}
#endif /* WIN32 */

static int TimeTask_ReallyAdd(TaskInfo *i)
{
    return TimeQueue.Add(&TimeQueue, i);
}

static void TimeTask_RunTack(void *i)
{
    TaskInfo *Info = (TaskInfo *)i;

    Info->Task(Info->Arg1, Info->Arg2);

    if( Info->Persistent )
    {
        Info->LeftTime = Info->TimeOut;
        if( Info->Asynchronous )
        {
#ifdef WIN32
            if( MsgQue.Post(&MsgQue, Info) != 0 )
            {
                /** TODO: Show fatal error */
            }
#else /* WIN32 */
            if( WRITE_PIPE(WriteTo, Info, sizeof(TaskInfo)) < 0 )
            {
                /** TODO: Show fatal error */
            }
#endif /* WIN32 */
        } else {
            if( TimeTask_ReallyAdd(Info) != 0 )
            {
                /** TODO: Show fatal error */
            }
        }
    }

    LinkedQueue_FreeNode(i);
}

/* Only the particular one thread execute the function */
static void TimeTask_Work(void *Unused)
{
#ifdef WIN32
    static TaskInfo *i = NULL;
    static DWORD *tv = NULL;
    static DWORD BeforeWaiting = 0;

    while( TRUE )
    {
        static TaskInfo *New;

        i = TimeQueue.Get(&TimeQueue);
        if( i == NULL )
        {
            tv = NULL;
        } else {
            tv = &(i->LeftTime);
            BeforeWaiting = *tv;
        }

        New = MsgQue.Wait(&MsgQue, tv);
        if( New == NULL )
        {
            /* Run the task */
            TimeTask_ReduceTime(BeforeWaiting - *tv);

            if( i->Asynchronous )
            {
                ThreadHandle t;
                CREATE_THREAD(TimeTask_RunTack, i, t);
                DETACH_THREAD(t);
            } else {
                TimeTask_RunTack(i);
            }

        } else {
            /* Receive a new task from other thread */
            if( tv != NULL )
            {
                TimeTask_ReduceTime(BeforeWaiting - *tv);
            }

            if( TimeTask_ReallyAdd(New) != 0 )
            {
                /** TODO: Show fatal error */
                break;
            }

            if( i != NULL )
            {
                if( TimeTask_ReallyAdd(i) != 0 )
                {
                    /** TODO: Show fatal error */
                    break;
                }
                LinkedQueue_FreeNode(i);
            }

            WinMsgQue_FreeMsg(New);
        }
    }
#else /* WIN32 */
    static fd_set   ReadSet, ReadySet;

    static TaskInfo *i = NULL;
    static struct timeval   *tv = NULL;
    static struct timeval   Elapsed = {0, 0};

    FD_ZERO(&ReadSet);
    FD_SET(ReadFrom, &ReadSet);

    while( TRUE )
    {
        i = TimeQueue.Get(&TimeQueue);

        if( i == NULL )
        {
            tv = NULL;
        } else {
            tv = &(i->LeftTime);
            Elapsed = *tv;
        }

        ReadySet = ReadSet;
        switch( select(ReadFrom + 1, &ReadySet, NULL, NULL, tv) )
        {
        case SOCKET_ERROR:
            /** TODO: Show fatal error */
            while( TRUE )
            {
                SLEEP(32767);
            }
            break;

        case 0:
            /* Run the task */
            Tv_Subtract(&Elapsed, tv);
            TimeTask_ReduceTime(&Elapsed);

            if( i->Asynchronous )
            {
                ThreadHandle t;
                CREATE_THREAD(TimeTask_RunTack, i, t);
                DETACH_THREAD(t);
            } else {
                TimeTask_RunTack(i);
            }

            break;

        default:
            /* Receive a new task from other thread */
            if( tv != NULL )
            {
                Tv_Subtract(&Elapsed, tv);
                TimeTask_ReduceTime(&Elapsed);
            }

            {
                static TaskInfo ni;

                if( READ_PIPE(ReadFrom, &ni, sizeof(TaskInfo)) < 0 )
                {
                    /** TODO: Show fatal error */
                    break;
                }

                if( TimeTask_ReallyAdd(&ni) != 0 )
                {
                    /** TODO: Show fatal error */
                    break;
                }
            }

            if( i != NULL )
            {
                if( TimeTask_ReallyAdd(i) != 0 )
                {
                    /** TODO: Show fatal error */
                    break;
                }
                LinkedQueue_FreeNode(i);
            }

            break;
        }
    }
#endif /* WIN32 */
}

int TimedTask_Add(BOOL Persistent,
                 BOOL Asynchronous,
                 int Milliseconds,
                 TaskFunc Func,
                 void *Arg1,
                 void *Arg2,
                 BOOL Immediate
                 )
{
    TaskInfo i;

    if( Func == NULL )
    {
        return -33;
    }

    i.Task = Func;
    i.Arg1 = Arg1;
    i.Arg2 = Arg2;
    i.Persistent = Persistent;
    i.Asynchronous = Asynchronous;
#ifdef WIN32
    i.TimeOut = Milliseconds;
#else /* WIN32 */
    i.TimeOut.tv_usec = (Milliseconds % 1000) * 1000;
    i.TimeOut.tv_sec = Milliseconds / 1000;
#endif /* WIN32 */
    if( Immediate )
    {
#ifdef WIN32
        i.LeftTime = 0;
#else /* WIN32 */
        i.LeftTime.tv_sec = 0;
        i.LeftTime.tv_usec = 0;
#endif /* WIN32 */
    } else {
        i.LeftTime = i.TimeOut;
    }

#ifdef WIN32
    if( MsgQue.Post(&MsgQue, &i) != 0 )
    {
        return -212;
    }
#else /* WIN32 */
    if( WRITE_PIPE(WriteTo, &i, sizeof(TaskInfo)) < 0 )
    {
        return -53;
    }
#endif /* WIN32 */

    return 0;
}

static int Compare(const void *One, const void *Two)
{
    const TaskInfo *o = One, *t = Two;
#ifdef WIN32
    return o->LeftTime - t->LeftTime;
#else /* WIN32 */
    return Tv_Comapre(&(o->LeftTime), &(t->LeftTime));
#endif /* WIN32 */
}

int TimedTask_Init(void)
{
    ThreadHandle t;

    if( LinkedQueue_Init(&TimeQueue,
                         sizeof(TaskInfo),
                         Compare
                         ) != 0
       )
    {
        return -20;
    }

#ifdef WIN32
    if( WinMsgQue_Init(&MsgQue, sizeof(TaskInfo)) != 0 )
    {
        return -247;
    }
#else /* WIN32 */
    if( !CREATE_PIPE_SUCCEEDED(CREATE_PIPE(&ReadFrom, &WriteTo)) )
    {
        return -25;
    }
#endif /* WIN32 */

    CREATE_THREAD(TimeTask_Work, NULL, t);
    DETACH_THREAD(t);

    return 0;
}
