#ifndef LOGS_H_INCLUDED
#define LOGS_H_INCLUDED

#include "readconfig.h"
#include "common.h"
#include "iheader.h"

#define PRINTON		Log_Inited()

#define DEBUGSECTION	if( PRINTON && Log_DebugOn() )

int Log_Init(ConfigFileInfo *ConfigInfo, BOOL PrintScreen, BOOL Debug);

BOOL Log_Inited(void);

BOOL Log_DebugOn(void);

void Log_Print(const char *Type, const char *format, ...);

#define	WARNING(...)    Log_Print("WARN", __VA_ARGS__)
#define	INFO(...)       Log_Print("INFO", __VA_ARGS__)
#define	ERRORMSG(...)   Log_Print("ERROR", __VA_ARGS__)
#define	DEBUG(...)      DEBUGSECTION \
                            Log_Print("DEBUG", __VA_ARGS__);

void ShowRefusingMessage(IHeader *h, const char *Message);

void ShowTimeOutMessage(const IHeader *h, char Protocol);

void ShowErrorMessage(IHeader *h, char Protocol);

void ShowNormalMessage(IHeader *h, char Protocol);

void ShowBlockedMessage(IHeader *h, const char *Message);

void ShowSocketError(const char *Prompts, int ErrorNum);

#endif // LOGS_H_INCLUDED
