/*************************************************************************
    > File Name: shellsync.c
    > Author: GuoGuo
    > Mail: gch981213@gmail.com
    > Created Time: 2014年11月06日 星期四 19时15分30秒
 ************************************************************************/
#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1 /* for struct timespec */
#include <time.h>
#undef _GNU_SOURCE
#else
#include <time.h>
#endif
#include <stdio.h>
#include <semaphore.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>

#define SEM_BLOCK_NAME "SYNCSHELL_block"
#define SEM_COUNT_NAME "SYNCSHELL_count"

static int sync_wait(int nproc, int wait_timeout)
{
    int flags;
    int value;
    sem_t *block;
    sem_t *count;
    struct timespec ts;

    if (nproc <= 1)
    {
        printf("sync_wait: number of processes should be larger than 1.\n");
        return -1;
    }

    if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
    {
        printf("clock_gettime error.\n");
        return -1;
    }
    ts.tv_sec += wait_timeout;

    flags = O_RDWR | O_CREAT;
    block = sem_open(SEM_BLOCK_NAME, flags, 0644, 0);
    count = sem_open(SEM_COUNT_NAME, flags, 0644, 0);
    if (block == SEM_FAILED || count == SEM_FAILED)
    {
        printf("sync_wait: sem_open failed.\n");
        return -1;
    }

    if (sem_post(count) < 0)
    {
        printf("sync_wait: sem_post failed.\n");
        return -1;
    }
    if (sem_getvalue(count, &value) < 0)
    {
        printf("sync_wait: sem_getvalue failed.\n");
        return -1;
    }
    printf("%d processes have arrived, waiting for the left %d.\n",
           value, nproc - value);

    if (value >= nproc)
    {
        while (nproc - 1 > 0)
        {
            if (sem_post(block) < 0)
            {
                printf("sync_wait: sem_post failed.\n");
                return -1;
            }
            nproc--;
        }
    }
    else
    {
        if (sem_timedwait(block, &ts) < 0)
        {
            if (errno == ETIMEDOUT)
            {
                printf("sync_wait: sem_timewait time out.\n");
            }
            else
            {
                printf("sync_wait: sem_timewait error.\n");
            }
            return -1;
        }
    }

    sem_close(count);
    sem_close(block);

    sem_unlink(SEM_COUNT_NAME);
    sem_unlink(SEM_BLOCK_NAME);

    return 0;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Usage: shellsync <process_num> <wait_timeout>.\n");
        return 1;
    }
    else
    {
        int proc_num, wait_timeout;
        if ((proc_num = atoi(argv[1])) <= 0)
        {
            printf("Wrong <process_num>.\n");
            return 1;
        }
        if ((wait_timeout = atoi(argv[2])) <= 0)
        {
            printf("Wrong <wait_timeout>.\n");
            return 1;
        }
        if (sync_wait(proc_num, wait_timeout) < 0)
        {
            printf("Processes sync failed.\n");
            sem_unlink(SEM_COUNT_NAME);
            sem_unlink(SEM_BLOCK_NAME);
            return 1;
        }
        else
        {
            printf("Processes sync succeed.\n");
            return 0;
        }
    }
    return 0;
}
