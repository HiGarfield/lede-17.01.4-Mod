#ifndef HOSTS_H_INCLUDED
#define HOSTS_H_INCLUDED

#include "readconfig.h"
#include "iheader.h"
#include "statichosts.h"
#include "dynamichosts.h"

int Hosts_Init(ConfigFileInfo *ConfigInfo);

BOOL Hosts_TypeExisting(const char *Domain, HostsRecordType Type);

HostsUtilsTryResult Hosts_Try(IHeader *Header, int BufferLength);

int Hosts_Get(IHeader *Header, int BufferLength);

#endif // HOSTS_H_INCLUDED
