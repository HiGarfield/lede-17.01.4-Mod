#include <stdio.h>
#include <time.h>
#include "../../timedtask.h"

void p(const char *t, void *u)
{
    printf("%lu : %s\n", time(NULL), t);
}

void j2(void *a, void *b)
{
    printf("%lu : %s\n", time(NULL), "Job 2 start .");
    SLEEP(4000);
    printf("%lu : %s\n", time(NULL), "Job 2 end .");
}

int main(void)
{
    TimeTask_Init();

    printf("-->Job 1, Executing every 2 seconds.\n");
    TimeTask_Add(TRUE, FALSE, 2000, p, "Job 1 .", NULL, FALSE);

    printf("-->Job 2, Executing every 3 seconds.\n");
    TimeTask_Add(TRUE, TRUE, 3000, j2, NULL, NULL, FALSE);

    SLEEP(10000);

    printf("-->Job 3, Executing after 5 seconds.\n");
    TimeTask_Add(TRUE, FALSE, 5000, p, "Job 3 .", NULL, TRUE);

    SLEEP(9999999);
    return 0;
}
