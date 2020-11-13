#ifndef HCONTEXT_H_INCLUDED
#define HCONTEXT_H_INCLUDED

#include "bst.h"
#include "iheader.h"
#include "oo.h"

typedef struct _HostsContext HostsContext;

struct _HostsContext
{
    Bst t;

    int (*Add)(HostsContext *c,
               IHeader      *Original, /* Entity followed */
               const char   *RecursedDomain,
               uint16_t     NewIdentifier
               );

    int (*FindAndRemove)(HostsContext *c,

                         /* Entity followed */
                         IHeader      *Input,

                         /* Entity followed */
                         IHeader      *Output
                         );

    void (*Swep)(HostsContext *c);

};

int HostsContext_Init(HostsContext *c);

#endif // HCONTEXT_H_INCLUDED
