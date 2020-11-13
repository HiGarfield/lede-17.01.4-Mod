#ifndef HOSTSCONTAINER_H_INCLUDED
#define HOSTSCONTAINER_H_INCLUDED

#define DOMAIN_NAME_LENGTH_MAX 128

#include "stringchunk.h"
#include "oo.h"

typedef enum _HostsRecordType{
	HOSTS_TYPE_TOO_LONG = -1,
	HOSTS_TYPE_UNKNOWN = 0,
	HOSTS_TYPE_A = 1 << 1,
	HOSTS_TYPE_AAAA = 1 << 2,
	HOSTS_TYPE_CNAME = 1 << 3,
	HOSTS_TYPE_EXCLUEDE = 1 << 4,
	HOSTS_TYPE_GOOD_IP_LIST = 1 << 5,

} HostsRecordType;

typedef struct _HostsContainer HostsContainer;

/* Return 0 : continue; Otherwise : break;  */
typedef int (*HostsFindFunc)(int                Number, /* Start with 1 */
                             HostsRecordType    Type,
                             const void         *Data,
                             void               *Arg
                             );

struct _HostsContainer{
	PRIMEMB StringChunk     Mappings;
	PRIMEMB StableBuffer    Table;

	PUBMEMB HostsRecordType (*Load)(HostsContainer *Container,
                                    const char *MetaLine
                                    );

    PUBMEMB const void *(*Find)(HostsContainer  *Container,
                                const char      *Name,
                                HostsRecordType Type,
                                HostsFindFunc   Func,
                                void            *Arg
                                );

    PUBMEMB void (*Free)(HostsContainer *Container);

};

int HostsContainer_Init(HostsContainer *Container);

#endif // HOSTSCONTAINER_H_INCLUDED
