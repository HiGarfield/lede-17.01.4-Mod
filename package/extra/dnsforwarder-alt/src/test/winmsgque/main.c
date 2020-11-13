#define WIN32
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "../../winmsgque.h"

void t(void *a)
{
    WinMsgQue *q = (WinMsgQue *)a;
    srand(time(NULL));
    while( TRUE )
    {
        int i = rand();
        q->Post(q, &i);
        printf("***Posted number %d.\n", i);
        SLEEP(1000);
    }
}

void p(WinMsgQue *q)
{
    int n = 10;
    while( n-- != 0 )
    {
        int i = rand();
        q->Post(q, &i);
    }
}

int main(void)
{
    WinMsgQue q;
    ThreadHandle th;

    WinMsgQue_Init(&q, sizeof(int));

    CREATE_THREAD(t, &q, th);
    CREATE_THREAD(t, &q, th);
    CREATE_THREAD(t, &q, th);
    CREATE_THREAD(t, &q, th);
    CREATE_THREAD(t, &q, th);
    CREATE_THREAD(t, &q, th);

    while( TRUE )
    {
        int *i;
        DWORD t = 1000;
        i = q.Wait(&q, &t);
        if( i == NULL )
        {
            printf("-->Didn't get anything.\n");
        } else {
            printf("-->Get number %d.\n", *i);
        }
    }

    return 0;
}
