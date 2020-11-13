#include <string.h>
#include "linkedqueue.h"

static int LinkedQueue_Add(LinkedQueue *l, const void *Data)
{
    ListHead *d;

    /* Fill the new node */
    if( l == NULL || Data == NULL )
    {
        return -10;
    }

    d = SafeMalloc(sizeof(ListHead) + l->DataLength);
    if( d == NULL )
    {
        return -16;
    }

    memcpy((void *)(d + 1), Data, l->DataLength);

    /* Find the place to add */
    if( l->First == NULL || l->Compare(Data, (void *)(l->First + 1)) <= 0 )
    {
        d->Next = l->First;
        l->First = d;
    } else {
        ListHead *n;

        n = l->First;
        while( n->Next != NULL && l->Compare(Data, (void *)(n->Next + 1)) > 0 )
        {
            n = n->Next;
        }

        d->Next = n->Next;
        n->Next = d;
    }

    return 0;
}

static void *LinkedQueue_Get(LinkedQueue *l)
{
    ListHead *h;

    if( l == NULL )
    {
        return NULL;
    }

    if( l->First == NULL )
    {
        return NULL;
    }

    h = l->First;
    l->First = h->Next;

    return (void *)(h + 1);
}

static void LinkedQueue_Free(LinkedQueue *l)
{
    void *d;

    while( (d = LinkedQueue_Get(l)) != NULL )
    {
        LinkedQueue_FreeNode(d);
    }
}

int LinkedQueue_Init(LinkedQueue *l,
                     int DataLength,
                     int (*CompareFunc)(const void *One, const void *Two)
                     )
{
    l->First = NULL;
    l->DataLength = DataLength;
    l->Compare = CompareFunc;

    l->Add = LinkedQueue_Add;
    l->Get = LinkedQueue_Get;
    l->Free = LinkedQueue_Free;
    return 0;
}

/** Iterator Implementation */
static void *LinkedQueueIterator_Next(LinkedQueueIterator *i)
{
    if( i->Current == NULL )
    {
        i->Current = i->l->First;
    } else {
        i->Current = i->Current->Next;
    }

    if( i->Current == NULL )
    {
        return NULL;
    } else {
        return (void *)(i->Current + 1);
    }
}

int LinkedQueueIterator_Init(LinkedQueueIterator *i, LinkedQueue *l)
{
    if( i == NULL )
    {
        return -96;
    }

    i->l = l;
    i->Current = NULL;

    i->Next = LinkedQueueIterator_Next;
    return 0;
}
