#include <string.h>
#include "stablebuffer.h"
#include "utils.h"

static BOOL NeedRealloc(Array *MetaInfo, int DataLength)
{
    StableBuffer_MetaInfo   *lm;

    if( Array_GetUsed(MetaInfo) <= 0 )
    {
        return TRUE;
    }

    lm = Array_GetBySubscript(MetaInfo, Array_GetUsed(MetaInfo) - 1);
    if( lm == NULL )
    {
        return FALSE;
    }

    if( lm->Amount - lm->Used < DataLength )
    {
        return TRUE;
    } else {
        return FALSE;
    }
}

static StableBuffer_MetaInfo *Realloc(Array *MetaInfo, int DataLength)
{
    int s;
    StableBuffer_MetaInfo   m;

    int BLOCK_ORDER = Array_GetUsed(MetaInfo) + 1;

    m.Amount = ROUND_UP(DataLength * BLOCK_ORDER, sizeof(void *));
    m.Used = 0;
    m.Start = SafeMalloc(m.Amount);
    if( m.Start == NULL )
    {
        return NULL;
    }

    s = Array_PushBack(MetaInfo, &m, NULL);
    if( s < 0 )
    {
        return NULL;
    }

    return Array_GetBySubscript(MetaInfo, s);

}

static void *WriteHere(Array *MetaInfo, int DataLength)
{
    if( NeedRealloc(MetaInfo, DataLength) )
    {
        StableBuffer_MetaInfo   *lm = Realloc(MetaInfo, DataLength);
        if( lm != NULL)
        {
            lm->Used = DataLength;
            return lm->Start;
        } else {
            return NULL;
        }
    } else {
        int l;
        StableBuffer_MetaInfo   *lm;

        l = Array_GetUsed(MetaInfo) - 1;
        lm = Array_GetBySubscript(MetaInfo, l);
        if( lm == NULL )
        {
            return NULL;
        } else {
            void *ret = lm->Start + lm->Used;
            lm->Used += DataLength;
            return ret;
        }
    }
}

static void *Add(StableBuffer *s, const void *Data, int Length, BOOL Align)
{
    void *wh = WriteHere(&(s->MetaInfo),
                         Align ? ROUND_UP(Length, sizeof(void *)) : Length
                         );

    if( wh == NULL )
    {
        return NULL;
    }

    if( Data != NULL )
    {
        memcpy(wh, Data, Length);
    }

    return wh;
}

static void Clear(StableBuffer *s)
{
    int i;
    for( i = 0; i < Array_GetUsed(&(s->MetaInfo)); ++i )
    {
        StableBuffer_MetaInfo *m = Array_GetBySubscript(&(s->MetaInfo), i);

        if( m != NULL )
        {
            SafeFree(m->Start);
        }
    }

    Array_Clear(&(s->MetaInfo));
}

static void Free(StableBuffer *s)
{
    Clear(s);
    Array_Free(&(s->MetaInfo));
}

int StableBuffer_Init(StableBuffer *s)
{
    s->Add = Add;
    s->Clear = Clear;
    s->Free = Free;

    return Array_Init(&(s->MetaInfo),
                      sizeof(StableBuffer_MetaInfo), 0, FALSE, NULL);
}

/**
  Iterator Implementation
*/
static void *StableBufferIterator_NextBlock(StableBufferIterator *i)
{
    StableBuffer *s = i->Buffer;
    StableBuffer_MetaInfo *m;

    if( i->Current < 0 )
    {
        i->Current = 0;
    } else {
        i->Current += 1;
    }

    m = Array_GetBySubscript(&(s->MetaInfo), i->Current);
    if( m == NULL )
    {
        i->Current = -1;
        return NULL;
    } else {
        return m->Start;
    }
}

static void StableBufferIterator_Reset(StableBufferIterator *i)
{
    i->Current = -1;
}

static void *StableBufferIterator_ToLast(StableBufferIterator *i)
{
    i->Current = Array_GetUsed(&(i->Buffer->MetaInfo)) - 2;

    return StableBufferIterator_NextBlock(i);
}

static StableBuffer_MetaInfo *StableBufferIterator_CurrentMeta(StableBufferIterator *i)
{
    StableBuffer *s = i->Buffer;

    if( i->Current < 0 )
    {
        return NULL;
    }

    return Array_GetBySubscript(&(s->MetaInfo), i->Current);
}

static BOOL StableBufferIterator_IsInCurrentBlock(StableBufferIterator *i,
                                                  const void *Position)
{
    const char              *pos = Position;
    StableBuffer_MetaInfo   *m = StableBufferIterator_CurrentMeta(i);

    if( m == NULL )
    {
        return FALSE;
    } else {
        long s = pos - m->Start;

        if( s >= 0 && s < m->Used )
        {
            return TRUE;
        } else {
            return FALSE;
        }
    }
}

static int32_t StableBufferIterator_CurrentBlockSize(StableBufferIterator *i)
{
    StableBuffer_MetaInfo   *m = StableBufferIterator_CurrentMeta(i);

    return m->Amount;
}

static int32_t StableBufferIterator_CurrentBlockUsed(StableBufferIterator *i)
{
    StableBuffer_MetaInfo   *m = StableBufferIterator_CurrentMeta(i);

    return m->Used;
}

static void StableBufferIterator_RemoveLastNBytesOfCurrentBlock(
                                                    StableBufferIterator *i,
                                                    int n)
{
    StableBuffer_MetaInfo   *m = StableBufferIterator_CurrentMeta(i);

    if( m->Used < n )
    {
        m->Used = 0;
    } else {
        m->Used -= n;
    }
}

static void StableBufferIterator_RemoveNBytesOfCurrentBlock(
                                                    StableBufferIterator *i,
                                                    char *Here,
                                                    int n)
{
    StableBuffer_MetaInfo   *m = StableBufferIterator_CurrentMeta(i);

    if( n < 0 ||
       !StableBufferIterator_IsInCurrentBlock(i, Here) ||
       Here + n - m->Start > m->Used
       )
    {
        return;
    }

    if( m->Used < n )
    {
        m->Used = 0;
    } else {
        memmove(Here, Here + n, m->Used - (Here - m->Start) - n);
        m->Used -= n;
    }
}

static void StableBufferIterator_Free(StableBufferIterator *i)
{

}

int StableBufferIterator_Init(StableBufferIterator *i, StableBuffer *s)
{
    if( i == NULL || s == NULL )
    {
        return -1;
    }

    i->Buffer = s;
    i->Current = -1;

    i->NextBlock = StableBufferIterator_NextBlock;
    i->Reset = StableBufferIterator_Reset;
    i->ToLast = StableBufferIterator_ToLast;

    i->IsInCurrentBlock = StableBufferIterator_IsInCurrentBlock;
    i->CurrentBlockSize = StableBufferIterator_CurrentBlockSize;
    i->CurrentBlockUsed = StableBufferIterator_CurrentBlockUsed;
    i->RemoveLastNBytesOfCurrentBlock =
                            StableBufferIterator_RemoveLastNBytesOfCurrentBlock;
    i->RemoveNBytesOfCurrentBlock =
                                StableBufferIterator_RemoveNBytesOfCurrentBlock;

    i->Free = StableBufferIterator_Free;

    return 0;
}
