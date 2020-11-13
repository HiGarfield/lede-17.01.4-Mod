#ifndef GOODIPLIST_H_INCLUDED
#define GOODIPLIST_H_INCLUDED

#include "readconfig.h"

int GoodIpList_Init(ConfigFileInfo *ConfigInfo);

const char *GoodIpList_Get(const char *List);

#endif /* GOODIPLIST_H_INCLUDED */
