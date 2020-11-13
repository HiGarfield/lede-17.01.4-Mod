#ifndef HOSTSUTILS_H_INCLUDED
#define HOSTSUTILS_H_INCLUDED

#include "iheader.h"
#include "hostscontainer.h"

typedef enum _HostsUtilsTryResult{
    HOSTSUTILS_TRY_BLOCKED = -2,
    HOSTSUTILS_TRY_NONE = -1,
    HOSTSUTILS_TRY_OK = 0,
    HOSTSUTILS_TRY_RECURSED = 1
} HostsUtilsTryResult;

int HostsUtils_GetCName(const char *Domain,
                        char *Buffer,
                        HostsContainer *Container
                        );

BOOL HostsUtils_TypeExisting(HostsContainer *Container,
                             const char *Domain,
                             HostsRecordType Type
                             );

HostsUtilsTryResult HostsUtils_Try(IHeader *Header,
                                   int BufferLength,
                                   HostsContainer *Container
                                   );

int HostsUtils_Query(SOCKET Socket, /* Both for sending and receiving */
                     Address_Type *BackAddress,
                     int Identifier,
                     const char *Name,
                     DNSRecordType Type
                     );

/* Error code returned */
int HostsUtils_CombineRecursedResponse(void       *Buffer, /* Include IHeader */
                                       int          Bufferlength,
                                       char         *RecursedEntity,
                                       int          EntityLength,
                                       const char   *RecursedDomain
                                       );

#endif // HOSTSUTILS_H_INCLUDED
