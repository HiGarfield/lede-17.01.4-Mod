#include <string.h>
#include "stringlist.h"
#include "utils.h"

static int Divide(char *Str, const char *Delimiters)
{
	int		Count = 0;
	char	*Itr;

	if( Delimiters == NULL )
    {
        return 1;
    }

	Itr = strpbrk(Str, Delimiters);
	while( Itr != NULL )
    {
        *Itr = '\0';

        ++Itr;
		++Count;

		Itr = strpbrk(Itr, Delimiters);
    }

	return Count + 1;
}

static int StringList_Count(StringList *s)
{
    StringListIterator    i;
    const char            *b;
    int                   ret = 0;

    if( StringListIterator_Init(&i, s) != 0 )
    {
        return -1;
    }

    b = i.Next(&i);
    while( b != NULL )
    {
        ++ret;

        b = i.Next(&i);
    }

    return ret;
}

static void *StringList_Add(StringList *s,
                            const char *str,
                            const char *Delimiters
                            )
{
    StableBuffer *sb;

    sb = &(s->Buffer);

    void *Here = sb->Add(sb, str, strlen(str) + 1, FALSE);
    if( Here == NULL )
    {
        return NULL;
    }

    Divide(Here, Delimiters);

    return Here;
}

/* Unsafe operation, it may change strings' positions */
static int StringList_AppendLast(StringList *s,
                                 const char *str,
                                 const char *Delimiters
                                 )
{
    StableBuffer            *sb;
    StableBufferIterator    i;
    char                    *b;

    char *l;
    int Used;

    int StrLength; /* Including terminated-0 */
    int LastHalfLength;
    char *NewStr;
    char *NewlyAdded;

    if( s == NULL )
    {
        return -1;
    }

    sb = &(s->Buffer);

    if( StableBufferIterator_Init(&i, sb) != 0 )
    {
        return -2;
    }

    b = i.ToLast(&i);
    if( b == NULL )
    {
        return -3;
    }

    Used = i.CurrentBlockUsed(&i);
    for( l = b + Used - 2; l > b; --l )
    {
        if( *l == '\0' )
        {
            ++l;
            break;
        }
    }

    if( l <= b )
    {
        l = b;
    }

    StrLength = strlen(str) + 1; /* Including terminated-0 */
    LastHalfLength = Used - (l - b); /* Including terminated-0 */
    NewStr = SafeMalloc(StrLength + LastHalfLength - 1);
    if( NewStr == NULL )
    {
        return -4;
    }

    strcpy(NewStr, l);
    strcat(NewStr, str);

    i.RemoveLastNBytesOfCurrentBlock(&i, LastHalfLength);

    NewlyAdded = sb->Add(sb, NewStr, StrLength + LastHalfLength - 1, FALSE);

    SafeFree(NewStr);

    if( NewlyAdded == NULL )
    {
        return -5;
    }

    return Divide(NewlyAdded, Delimiters);
}

/* free the return value and all the strings */
static const char **StringList_ToCharPtrArray(StringList *s)
{
    const char  **ret;
    int         Index = 0;

    StringListIterator    i;
    const char  *ci;

    if( StringListIterator_Init(&i, s) != 0 )
    {
        return NULL;
    }

    ret = SafeMalloc((StringList_Count(s) + 1) * sizeof(const char *));
    if( ret == NULL )
    {
        return NULL;
    }

    ci = i.Next(&i);
    while( ci != NULL )
    {
        ret[Index] = StringDup(ci);
        if( ret[Index] == NULL )
        {
            /** WARNING: Memory leak occured */
            return NULL;
        }

        ++Index;

        ci = i.Next(&i);
    }

    ret[Index] = NULL;

    return ret;
}

/* Unsafe operation, it may change strings' positions */
static void StringList_TrimAll(StringList *s, const char *Garbage)
{
    StringListIterator i;
    char *Str;

    if( StringListIterator_Init(&i, s) != 0 )
    {
        return;
    }

    if( Garbage == NULL )
    {
        Garbage = "\t \r\n";
    }

    Str = (char *)i.Next(&i);
    while( Str != NULL )
    {
        char *HardContent;
        char *HardContentTail;

        HardContent = StrNpbrk(Str, Garbage);
        if( HardContent != NULL )
        {
            i.BufferIterator.RemoveNBytesOfCurrentBlock(&(i.BufferIterator),
                                                        Str,
                                                        HardContent - Str
                                                        );
        } else {
            /* Str is full of whitespaces or just empty, remove it */
            Str = (char *)i.Remove(&i);
            continue;
        }

        HardContentTail = StrRNpbrk(Str, Garbage);
        if( HardContentTail != NULL )
        {
            ++HardContentTail;
            i.BufferIterator.RemoveNBytesOfCurrentBlock(&(i.BufferIterator),
                                                        HardContentTail,
                                           strlen(Str) - (HardContentTail - Str)
                                                        );
        }

        Str = (char *)i.Next(&i);
    }
}

static void StringList_LowercaseAll(StringList *s)
{
    StringListIterator i;
    char *Str;

    if( StringListIterator_Init(&i, s) != 0 )
    {
        return;
    }

    while( (Str = (char *)i.Next(&i)) != NULL )
    {
        StrToLower(Str);
    }
}

static void StringList_Clear(StringList *s)
{
    s->Buffer.Clear(&(s->Buffer));
}

static void StringList_Free(StringList *s)
{
    s->Buffer.Free(&(s->Buffer));
}

int StringList_Init(__in StringList *s,
                    __in const char *ori,
                    __in const char *Delimiters
                    )
{
    StableBuffer *sb;

	if( s == NULL )
    {
        return -1;
    }

    sb = &(s->Buffer);

    if( StableBuffer_Init(sb) != 0 )
    {
        return -2;
    }

    s->Count = StringList_Count;
    s->Add = StringList_Add;
    s->AppendLast = StringList_AppendLast;
    s->ToCharPtrArray = StringList_ToCharPtrArray;
    s->TrimAll = StringList_TrimAll;
    s->LowercaseAll = StringList_LowercaseAll;
    s->Clear = StringList_Clear;
    s->Free = StringList_Free;

	if( ori != NULL )
	{
        void *Here = sb ->Add(sb, ori, strlen(ori) + 1, FALSE);
        if( Here == NULL )
        {
            sb->Free(sb);
            return -3;
        }

		Divide(Here, Delimiters);
	}

    return 0;
}

/**
  Iterator Implementation
*/

static const char *StringListIterator_Next(StringListIterator *i)
{
    if( i->CurrentPosition == NULL )
    {
        i->BufferIterator.Reset(&(i->BufferIterator));
        i->CurrentPosition = i->BufferIterator.NextBlock(&(i->BufferIterator));
    } else {
        i->CurrentPosition += strlen(i->CurrentPosition) + 1;
    }

    while( TRUE )
    {
        if( i->CurrentPosition == NULL )
        {
            return NULL;
        } else if( i->BufferIterator.IsInCurrentBlock(&(i->BufferIterator),
                                                      i->CurrentPosition
                                                      )
                 )
        {
            return i->CurrentPosition;
        }

        i->CurrentPosition = i->BufferIterator.NextBlock(&(i->BufferIterator));
    }
}

static const char *StringListIterator_Remove(StringListIterator *i)
{
    if( i->CurrentPosition == NULL )
    {
        return NULL;
    }

    i->BufferIterator.RemoveNBytesOfCurrentBlock(&(i->BufferIterator),
                                                 i->CurrentPosition,
                                                 strlen(i->CurrentPosition) + 1
                                                 );

    while( TRUE )
    {
        if( i->CurrentPosition == NULL )
        {
            return NULL;
        } else if( i->BufferIterator.IsInCurrentBlock(&(i->BufferIterator),
                                                      i->CurrentPosition
                                                      )
                 )
        {
            return i->CurrentPosition;
        }

        i->CurrentPosition = i->BufferIterator.NextBlock(&(i->BufferIterator));
    }
}

static void StringListIterator_Reset(StringListIterator *i)
{
    i->CurrentPosition = NULL;
}

int StringListIterator_Init(StringListIterator *i, StringList *l)
{
    if( i == NULL || l == NULL )
    {
        return -1;
    }

    if( StableBufferIterator_Init(&(i->BufferIterator), &(l->Buffer)) != 0 )
    {
        return -2;
    }

    i->CurrentPosition = NULL;

    i->Next = StringListIterator_Next;
    i->Reset = StringListIterator_Reset;
    i->Remove = StringListIterator_Remove;

    return 0;
}
