#ifndef WINMSGQUE_H_INCLUDED
#define WINMSGQUE_H_INCLUDED
/** First-in last-out message queue, used only on Windows */
#ifdef WIN32
#include "linkedqueue.h"
#include "common.h"

typedef struct _WinMsgQue WinMsgQue;

struct _WinMsgQue{
    /* private */
    LinkedQueue q;
    HANDLE  e;
    EFFECTIVE_LOCK  l;

    /* public */
    int (*Post)(WinMsgQue *q, const void *Data);
    void *(*Wait)(WinMsgQue *q, DWORD *Milliseconds);
};

int WinMsgQue_Init(WinMsgQue *q, int MsgSize);

#define WinMsgQue_FreeMsg(m_ptr)    LinkedQueue_FreeNode(m_ptr)

#endif /* WIN32 */
#endif // WINMSGQUE_H_INCLUDED
