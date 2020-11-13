#ifndef _DNS_CACHE_
#define _DNS_CACHE_

#include "dnsrelated.h"
#include "readconfig.h"
#include "iheader.h"

int DNSCache_Init(ConfigFileInfo *ConfigInfo);

BOOL Cache_IsInited(void);

int DNSCache_AddItemsToCache(IHeader *Header);

int DNSCache_FetchFromCache(IHeader *h /* Entity followed */, int BufferLength);

#endif /* _DNS_CACHE_ */
