#ifndef STATICHOSTS_H_INCLUDED
#define STATICHOSTS_H_INCLUDED

#include "hostsutils.h"
#include "readconfig.h"

int StaticHosts_Init(ConfigFileInfo *ConfigInfo);

int StaticHosts_GetCName(const char *Domain, char *Buffer);

BOOL StaticHosts_TypeExisting(const char *Domain, HostsRecordType Type);

HostsUtilsTryResult StaticHosts_Try(IHeader *Header, int BufferLength);

#endif // STATICHOSTS_H_INCLUDED
