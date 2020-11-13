#ifndef MMGR_H_INCLUDED
#define MMGR_H_INCLUDED
/** Module manager */

#include "udpm.h"
#include "tcpm.h"

int MMgr_Init(ConfigFileInfo *ConfigInfo);

int MMgr_Send(IHeader *h, int BufferLength);

#endif // MMGR_H_INCLUDED
