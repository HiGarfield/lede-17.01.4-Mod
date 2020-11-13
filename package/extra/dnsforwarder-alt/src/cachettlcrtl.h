#ifndef CACHETTLCRTL_H_INCLUDED
#define CACHETTLCRTL_H_INCLUDED

#include "stringchunk.h"

#define TTL_STATE_NO_CACHE	0
#define	TTL_STATE_ORIGINAL	(-1)
#define	TTL_STATE_FIXED		(1)
#define	TTL_STATE_VARIABLE	(2)

#define	TTL_CTRL_INFECTION_AGGRESSIVLY	0
#define	TTL_CTRL_INFECTION_PASSIVLY		(1)
#define	TTL_CTRL_INFECTION_NONE			(2)

typedef struct _CtrlContent {
	int			State;
	uint32_t	Coefficient;
	uint32_t	Increment;
	int			Infection;
} CtrlContent;
/* Final TTL = Coefficient * State + Increment, if State is positive. */

typedef StringChunk CacheTtlCtrl;

int CacheTtlCrtl_Init(CacheTtlCtrl *c);

int CacheTtlCrtl_Add(CacheTtlCtrl *c, const char *Domain, int State, uint32_t Coefficient, uint32_t Increment, int Infection);

int CacheTtlCrtl_Add_From_String(CacheTtlCtrl *c, const char *Rule);

int CacheTtlCrtl_Add_From_StringList(CacheTtlCtrl *c, StringList *sl);

const CtrlContent *CacheTtlCrtl_Get(CacheTtlCtrl *c, const char *Domain);

#endif // CACHETTLCRTL_H_INCLUDED
