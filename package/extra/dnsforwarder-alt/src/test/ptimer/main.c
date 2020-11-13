#include <stdio.h>
#include <stdlib.h>
#include "../../common.h"
#include "../../ptimer.h"

int main(void)
{
    PTimer t;

    PTimer_Start(&t);
    SLEEP(2002);

    printf("%lu", PTimer_End(&t));

    return 0;
}
