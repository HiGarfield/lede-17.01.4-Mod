#include <stdio.h>
#include <stdlib.h>
#include "../../linkedqueue.h"

int cf(const void *o, const void *t)
{
    return *(int *)o - *(int *)t;
}

void AddBatch(LinkedQueue *l)
{
    int n = 10;
    int d;

    while( n-- != 0 )
    {
        d = rand();
        printf("Added : %d\n", d);
        l->Add(l, &d);
    }
}

void Print(LinkedQueue *l)
{
    int *d;

    LinkedQueueIterator i;

    LinkedQueueIterator_Init(&i, l);

    while( (d = i.Next(&i)) != NULL  )
    {
        printf("Data: %d\n", *d);
    }

    printf("\n\n\n");
}

int main(void)
{
    LinkedQueue l;
    int *d;

    srand(time(NULL));
    LinkedQueue_Init(&l, sizeof(int), cf);

    AddBatch(&l);

    Print(&l);

    LinkedQueue_FreeNode(l.Get(&l));

    Print(&l);

    LinkedQueue_FreeNode(l.Get(&l));

    Print(&l);

    AddBatch(&l);

    Print(&l);

    while( (d = l.Get(&l)) != NULL )
    {
        LinkedQueue_FreeNode(d);
    }

    Print(&l);

    AddBatch(&l);

    LinkedQueue_FreeNode(l.Get(&l));

    Print(&l);

    return 0;

}
