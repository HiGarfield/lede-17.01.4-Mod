#ifndef SOCKETPOOL_H_INCLUDED
#define SOCKETPOOL_H_INCLUDED

#include <time.h>
#include "bst.h"
#include "stablebuffer.h"
#include "common.h"

typedef struct _SocketUnit {
	SOCKET      Sock;
	const char *Data;
} SocketUnit;

typedef struct _SocketPool SocketPool;

struct _SocketPool{
    /* private */
    Bst t;
    StableBuffer d;

	/* public */
    int (*Add)(SocketPool *sp,
               SOCKET Sock,
               const void *Data,
               int DataLength
               );

    int (*Del)(SocketPool *sp, SOCKET Sock);

    SOCKET (*FetchOnSet)(SocketPool *sp,
                         fd_set *fs,
                         void **Data
                         );

    void (*CloseAll)(SocketPool *sp, SOCKET ExceptFor);

    void (*Free)(SocketPool *sp, BOOL CloseAllSocket);
};

int SocketPool_Init(SocketPool *sp);

#endif // SOCKETPOOL_H_INCLUDED
