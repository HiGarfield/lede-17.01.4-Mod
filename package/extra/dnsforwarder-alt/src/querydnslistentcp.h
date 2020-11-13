#ifndef _QUERY_DNS_TCP_H_
#define _QUERY_DNS_TCP_H_

#include "common.h"
#include "readconfig.h"

int QueryDNSListenTCPInit(ConfigFileInfo *ConfigInfo);

void QueryDNSListenTCPStart(void);

#endif /* _QUERY_DNS_TCP_H_ */
