#include <string.h>
#include <time.h>
#include "mcontext.h"
#include "common.h"

typedef struct _ModuleContextItem{
    IHeader     h;
    uint32_t    i; /* Query identifier */
    time_t      t; /* Time of addition */
} ModuleContextItem;

static int ModuleContext_Swep_Collect(Bst *t,
                                      const ModuleContextItem *Context,
                                      Array *Pending
                                      )
{
    time_t	Now = time(NULL);

    if( Now - Context->t > 2 )
    {
        Array_PushBack(Pending, &Context, NULL);
    }

    return 0;
}

static void ModuleContext_Swep(ModuleContext *c, SwepCallback cb, void *Arg)
{
    Array Pending;
    int i;

    if( Array_Init(&Pending,
                   sizeof(const ModuleContextItem *),
                   4,
                   FALSE,
                   NULL
                   )
       != 0
       )
    {
        return;
    }

    c->d.Enum(&(c->d),
              (Bst_Enum_Callback)ModuleContext_Swep_Collect,
              &Pending
              );

    for( i = 0; i < Array_GetUsed(&Pending); ++i )
    {
        const ModuleContextItem **Context;

        Context = Array_GetBySubscript(&Pending, i);

        if( cb != NULL )
        {
            cb(&((**Context).h), i + 1, Arg);
        }

        c->d.Delete(&(c->d), *Context);
    }

    Array_Free(&Pending);
}

static int ModuleContext_Add(ModuleContext *c,
                             IHeader *h /* Entity followed */
                             )
{
    ModuleContextItem n;
    const char *e = (const char *)(h + 1);

    if( h == NULL )
    {
        return -21;
    }

    memcpy(&(n.h), h, sizeof(IHeader));
    n.i = *(uint16_t *)e;
    n.t = time(NULL);

    if( c->d.Add(&(c->d), &n) != NULL )
    {
        return 0;
    } else {
        return -83;
    }
}

static int ModuleContext_FindAndRemove(ModuleContext *c,
                                       IHeader *Input, /* Entity followed */
                                       IHeader *Output
                                       )
{
    ModuleContextItem k;
    const char *e = (const char *)(Input + 1);

    const ModuleContextItem *ri;

    int EntityLength;
    BOOL EDNSEnabled;

    k.i = *(uint16_t *)e;
    k.h.HashValue = Input->HashValue;

    ri = c->d.Search(&(c->d), &k, NULL);
    if( ri == NULL )
    {
        return -60;
    }

    EntityLength = Input->EntityLength;
    EDNSEnabled = Input->EDNSEnabled;

    memcpy(Output, &(ri->h), sizeof(IHeader));

    Output->EntityLength = EntityLength;
    Output->EDNSEnabled = EDNSEnabled;

    c->d.Delete(&(c->d), ri);

    return 0;
}

static int ModuleContextCompare(const void *_1, const void *_2)
{
    const ModuleContextItem *One = (ModuleContextItem *)_1;
    const ModuleContextItem *Two = (ModuleContextItem *)_2;

	if( One->i != Two->i )
	{
		return (int)(One->i) - (int)(Two->i);
	} else {
		return One->h.HashValue - Two->h.HashValue;
	}
}

int ModuleContext_Init(ModuleContext *c)
{
    if( c == NULL )
    {
        return -86;
    }

    if( Bst_Init(&(c->d),
                 sizeof(ModuleContextItem),
                 ModuleContextCompare
                 )
       != 0
       )
    {
        return -106;
    }

    c->Add = ModuleContext_Add;
    c->FindAndRemove = ModuleContext_FindAndRemove;
    c->Swep = ModuleContext_Swep;

    return 0;
}
