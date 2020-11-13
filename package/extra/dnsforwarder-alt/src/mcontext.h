#ifndef MCONTEXT_H_INCLUDED
#define MCONTEXT_H_INCLUDED
/** Thread unsafe */

#include "iheader.h"
#include "bst.h"

typedef void (*SwepCallback)(const IHeader *h, int Number, void *Arg);

typedef struct _ModuleContext ModuleContext;

struct _ModuleContext{
    /* private */
    Bst	d;

    /* public */
    int (*Add)(ModuleContext *c,
               IHeader *h /* Entity followed */
               );
    int (*FindAndRemove)(ModuleContext *c,
                         IHeader *Input, /* Entity followed */
                         IHeader *Output
                         );

    void (*Swep)(ModuleContext *c, SwepCallback cb, void *Arg);
};

int ModuleContext_Init(ModuleContext *c);

#endif // MCONTEXT_H_INCLUDED
