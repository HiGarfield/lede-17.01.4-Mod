#ifndef EXCLUDEDLIST_H_INCLUDED
#define EXCLUDEDLIST_H_INCLUDED

#include "stringlist.h"
#include "stringchunk.h"
#include "readconfig.h"
#include "iheader.h"

int Filter_Init(ConfigFileInfo *ConfigInfo);

BOOL Filter_Out(IHeader *h);

#endif // EXCLUDEDLIST_H_INCLUDED
