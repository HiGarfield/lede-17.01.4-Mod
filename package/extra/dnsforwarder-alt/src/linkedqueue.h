#ifndef LINKEDLIST_H_INCLUDED
#define LINKEDLIST_H_INCLUDED

#include "utils.h"

typedef struct _ListHead ListHead;

struct _ListHead{;
	ListHead *Next;
};

typedef struct _LinkedQueue LinkedQueue;

struct _LinkedQueue{
    /* private */
    ListHead *First;
    int DataLength;
    int (*Compare)(const void *One, const void *Two);

    /* public */
    int (*Add)(LinkedQueue *l, const void *Data);
    void *(*Get)(LinkedQueue *l);
    void (*Free)(LinkedQueue *l);
};

int LinkedQueue_Init(LinkedQueue *l,
                     int DataLength,
                     int (*CompareFunc)(const void *One, const void *Two)
                     );

#define LinkedQueue_FreeNode(ptr)   SafeFree((ptr) == NULL ? NULL : ((ListHead *)(ptr)) - 1)

/** Iterator Implementation */
typedef struct _LinkedQueueIterator LinkedQueueIterator;

struct _LinkedQueueIterator{
    /* private */
    ListHead *Current;
    LinkedQueue *l;

    /* public */
    void *(*Next)(LinkedQueueIterator *i);
};

int LinkedQueueIterator_Init(LinkedQueueIterator *i, LinkedQueue *l);

#endif /* LINKEDLIST */
