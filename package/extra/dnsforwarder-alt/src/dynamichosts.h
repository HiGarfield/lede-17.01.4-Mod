#ifndef DYNAMICHOSTS_H_INCLUDED
#define DYNAMICHOSTS_H_INCLUDED

#include "hostsutils.h"
#include "readconfig.h"

int DynamicHosts_Init(ConfigFileInfo *ConfigInfo);

int DynamicHosts_GetCName(const char *Domain, char *Buffer);

BOOL DynamicHosts_TypeExisting(const char *Domain, HostsRecordType Type);

HostsUtilsTryResult DynamicHosts_Try(IHeader *Header, int BufferLength);

#endif // DYNAMICHOSTS_H_INCLUDED
