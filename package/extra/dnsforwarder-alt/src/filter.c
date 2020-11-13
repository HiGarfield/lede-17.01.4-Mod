#include <stdio.h>
#include <stdlib.h>
#include "filter.h"
#include "stringchunk.h"
#include "bst.h"
#include "common.h"
#include "logs.h"
#include "readline.h"
#include "domainstatistic.h"

static Bst			*DisabledTypes = NULL;

static StringChunk	*DisabledDomain = NULL;

static int TypeCompare(const int *_1, const int *_2)
{
	return *_1 - *_2;
}

static int InitChunk(StringChunk **List)
{
    if( *List != NULL )
    {
        return 0;
    }

	*List = malloc(sizeof(StringChunk));
	if( *List == NULL )
	{
		return -77;
	}

	if( StringChunk_Init(*List, NULL) < 0 )
	{
		return -82;
	}

	return 0;
}

static int InitBst(Bst **t, int (*CompareFunc)(const void *, const void *))
{
	*t = malloc(sizeof(Bst));
	if( *t == NULL )
	{
		return -93;
	}

	if( Bst_Init(*t, sizeof(int), CompareFunc) != 0 )
	{
		return -102;
	}

	return 0;
}

static int LoadDomainsFromList(StringChunk *List, StringList *Domains)
{
	const char *Str;

	StringListIterator  sli;

	if( List == NULL || Domains == NULL )
	{
		return 0;
	}

	if( StringListIterator_Init(&sli, Domains) != 0 )
    {
        return -1;
    }

	Str = sli.Next(&sli);
	while( Str != NULL )
	{
		StringChunk_Add_Domain(List, Str, NULL, 0);
		Str = sli.Next(&sli);
	}

	return 0;
}

static int FilterDomain_Init(ConfigFileInfo *ConfigInfo)
{
    StringList *dd = ConfigGetStringList(ConfigInfo, "DisabledDomain");

    if( dd == NULL )
    {
        return 0;
    }

	if( InitChunk(&DisabledDomain) != 0 )
	{
        return -120;
	}

    LoadDomainsFromList(DisabledDomain, dd);
    dd->Free(dd);

    return 0;
}

static int FilterDomain_InitFromFile(ConfigFileInfo *ConfigInfo)
{
    const char *FilePath = ConfigGetRawString(ConfigInfo, "DisabledList");
    FILE *fp;
    ReadLineStatus	Status;
    char	Domain[512];

    if( FilePath == NULL )
    {
        return 0;
    }

    fp = fopen(FilePath, "r");
    if( fp == NULL )
    {
        return -118;
    }

    if( InitChunk(&DisabledDomain) != 0 )
	{
	    fclose(fp);
        return -117;
	}

    Status = ReadLine(fp, Domain, sizeof(Domain));
	while( Status != READ_FAILED_OR_END )
	{
		if( Status == READ_DONE )
		{
			StringChunk_Add_Domain(DisabledDomain, Domain, NULL, 0);
		} else {
			ReadLine_GoToNextLine(fp);
		}

		Status = ReadLine(fp, Domain, sizeof(Domain));
	}

    fclose(fp);

    return 0;
}

static int FilterType_Init(ConfigFileInfo *ConfigInfo)
{
	StringList *DisableType_Str =
        ConfigGetStringList(ConfigInfo, "DisabledType");

	const char *OneTypePendingToAdd_Str;
	int OneTypePendingToAdd;

    StringListIterator  sli;

	if( DisableType_Str == NULL )
	{
		return 0;
	}

	if( InitBst(&DisabledTypes,
                (int (*)(const void *, const void *))TypeCompare
             ) != 0 )
    {
        return -146;
    }

	if( StringListIterator_Init(&sli, DisableType_Str) != 0 )
    {
        return -2;
    }

	OneTypePendingToAdd_Str = sli.Next(&sli);
	while( OneTypePendingToAdd_Str != NULL )
	{
		sscanf(OneTypePendingToAdd_Str, "%d", &OneTypePendingToAdd);
		DisabledTypes->Add(DisabledTypes, &OneTypePendingToAdd);

		OneTypePendingToAdd_Str = sli.Next(&sli);
	}

	DisableType_Str->Free(DisableType_Str);

	return 0;
}

int Filter_Init(ConfigFileInfo *ConfigInfo)
{
    if( FilterDomain_Init(ConfigInfo) != 0 )
    {
        INFO("Disabled domains was not initialized.\n");
    } else {
        INFO("Disabled domains initialized.\n");
    }

    if( FilterType_Init(ConfigInfo) != 0 )
    {
        INFO("Disabled types was not initialized.\n");
    } else {
        INFO("Disabled types initialized.\n");
    }

    if( FilterDomain_InitFromFile(ConfigInfo) != 0 )
    {
        INFO("Disabled list was not initialized.\n");
    } else {
        INFO("Disabled list initialized.\n");
    }

	return 0;
}

static BOOL IsDisabledType(int Type)
{
	if( DisabledTypes != NULL &&
        DisabledTypes->Search(DisabledTypes, &Type, NULL) != NULL )
	{
		return TRUE;
	} else {
		return FALSE;
	}
}

static BOOL IsDisabledDomain(const char *Domain, int HashValue)
{
	return DisabledDomain != NULL &&
	       StringChunk_Domain_Match(DisabledDomain, Domain, &HashValue, NULL);
}

BOOL Filter_Out(IHeader *h)
{
    if(IsDisabledType(h->Type) || IsDisabledDomain(h->Domain, h->HashValue) )
    {
        IHeader_SendBackRefusedMessage(h);
        ShowRefusingMessage(h, "Disabled type or domain");
        DomainStatistic_Add(h, STATISTIC_TYPE_REFUSED);

        return TRUE;
    } else {
        return FALSE;
    }
}
