#ifndef DOMAINSTATISTIC_H_INCLUDED
#define DOMAINSTATISTIC_H_INCLUDED

#include "readconfig.h"
#include "iheader.h"

typedef enum _StatisticType{
	STATISTIC_TYPE_REFUSED = 0,
	STATISTIC_TYPE_HOSTS,
	STATISTIC_TYPE_CACHE,
	STATISTIC_TYPE_UDP,
	STATISTIC_TYPE_TCP,

	STATISTIC_TYPE_BLOCKEDMSG
} StatisticType;

int DomainStatistic_Init(ConfigFileInfo *ConfigInfo);

int DomainStatistic_Add(IHeader *h, StatisticType Type);

#endif // DOMAINSTATISTIC_H_INCLUDED
