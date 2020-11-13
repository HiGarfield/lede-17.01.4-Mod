#include <stdio.h>
#include <string.h>
#include "cachettlcrtl.h"
#include "logs.h"

int CacheTtlCrtl_Init(CacheTtlCtrl *c)
{
	return StringChunk_Init((StringChunk *)c, NULL);
}

int CacheTtlCrtl_Add(CacheTtlCtrl *c, const char *Domain, int State, uint32_t Coefficient, uint32_t Increment, int Infection)
{
	CtrlContent cc = {State, Coefficient, Increment, Infection};

	if( State == TTL_STATE_FIXED )
	{
		cc.Coefficient = 0;
	}

	return StringChunk_Add_Domain((StringChunk *)c, Domain, (const char *)&cc, sizeof(CtrlContent));
}

int CacheTtlCrtl_Add_From_String(CacheTtlCtrl *c, const char *Rule)
{
	char	Domain[128], Cmd[16], Arg[64];
	int		State;
	uint32_t Coefficient = 0;
	uint32_t Increment = 0;
	int		Infection = TTL_CTRL_INFECTION_AGGRESSIVLY;

	sscanf(Rule, "%127s%15s%64s", Domain, Cmd, Arg);

	if( Cmd[0] == '$' )
	{
		if( Cmd[1] == '$' )
		{
			Infection = TTL_CTRL_INFECTION_NONE;
			memmove(Cmd, Cmd + 2, sizeof(Cmd) - 2);
		} else {
			Infection = TTL_CTRL_INFECTION_PASSIVLY;
			memmove(Cmd, Cmd + 1, sizeof(Cmd) - 1);
		}
	} else {
		Infection = TTL_CTRL_INFECTION_AGGRESSIVLY;
	}

	#define IS_STATE(s)	(strncmp(Cmd, (s), strlen(s)) == 0)
	if( IS_STATE("orig") )
	{
		State = TTL_STATE_ORIGINAL;
	} else if( IS_STATE("nocache") )
	{
		State = TTL_STATE_NO_CACHE;
	} else if( IS_STATE("fixed") )
	{
		State = TTL_STATE_FIXED;
		Coefficient = 0;
		sscanf(Arg, "%u", &Increment);
	} else if( IS_STATE("vari") )
	{
		State = TTL_STATE_VARIABLE;
		sscanf(Arg, "%ux+%u", &Coefficient, &Increment);
	} else {
		ERRORMSG("Invalid `CacheControl' option : %s\n", Rule);
		return -1;
	}

	return CacheTtlCrtl_Add(c, Domain, State, Coefficient, Increment, Infection);
}

int CacheTtlCrtl_Add_From_StringList(CacheTtlCtrl *c, StringList *sl)
{
	const char *Itr = NULL;
	StringListIterator  sli;

	if( sl == NULL )
    {
        return 0;
    }

	if( StringListIterator_Init(&sli, sl) != 0 )
    {
        return -1;
    }

	Itr = sli.Next(&sli);
	while( Itr != NULL )
	{
		CacheTtlCrtl_Add_From_String(c, Itr);

		Itr = sli.Next(&sli);
	}

	return 0;
}

const CtrlContent *CacheTtlCrtl_Get(CacheTtlCtrl *c, const char *Domain)
{
	CtrlContent *ret = NULL;
	if( StringChunk_Domain_Match((StringChunk *)c, Domain, NULL, (void **)&(ret)) == TRUE )
	{
		return ret;
	} else {
		return NULL;
	}
}
