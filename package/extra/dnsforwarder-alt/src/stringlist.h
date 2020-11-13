#ifndef STRINGLIST_H_INCLUDED
#define STRINGLIST_H_INCLUDED

#include "common.h"
#include "stablebuffer.h"

/* This class is not thread safe. */

typedef struct _StringList StringList;

struct _StringList{
    StableBuffer    Buffer;

    int (*Count)(StringList *s);
    void *(*Add)(StringList *s, const char *str, const char *Delimiters);
    int (*AppendLast)(StringList *s, const char *str, const char *Delimiters);
    const char **(*ToCharPtrArray)(StringList *s);
    void (*TrimAll)(StringList *s, const char *Garbage);
    void (*LowercaseAll)(StringList *s);
    void (*Clear)(StringList *s);
    void (*Free)(StringList *s);
};

int StringList_Init(__in StringList *s,
                    __in const char *ori,
                    __in const char *Delimiters
                    );

/**
 Iterator
*/
typedef struct _StringListIterator StringListIterator;

struct _StringListIterator{
    StableBufferIterator    BufferIterator;
    char                    *CurrentPosition;

    const char *(*Next)(StringListIterator *i);
    const char *(*Remove)(StringListIterator *i);
    void (*Reset)(StringListIterator *i);
};

int StringListIterator_Init(StringListIterator *i, StringList *l);

#endif // STRINGLIST_H_INCLUDED
