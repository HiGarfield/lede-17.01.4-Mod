#include <string.h>
#include "stringchunk.h"
#include "utils.h"

typedef struct _EntryForString{
	const char	*str;
	void  *Data;
} EntryForString;

int StringChunk_Init(StringChunk *dl, StringList *List)
{
	if( dl == NULL )
	{
		return 0;
	}

	if( SimpleHT_Init(&(dl->List_Pos), sizeof(EntryForString), 5, ELFHash) != 0 )
	{
		return -1;
	}

	if( Array_Init(&(dl->List_W_Pos), sizeof(EntryForString), 0, FALSE, NULL) != 0 )
	{
		SimpleHT_Free(&(dl->List_Pos));
		return -2;
	}

	if( StableBuffer_Init(&(dl->AdditionalDataChunk)) != 0 )
	{
		SimpleHT_Free(&(dl->List_Pos));
		Array_Free(&(dl->List_W_Pos));
		return -3;
	}

	/* Whether to use external `StringList' to store strings. */
	if( List == NULL )
	{
		dl->List = SafeMalloc(sizeof(StringList));
		if( dl->List == NULL )
		{
			return -4;
		}

		if( StringList_Init(dl->List, NULL, NULL) != 0 )
		{
			return -5;
		}
	} else {
		dl->List = List;
	}

	return 0;
}

int StringChunk_Add(StringChunk	*dl,
					const char	*Str,
					const void	*AdditionalData,
					int			LengthOfAdditionalData /* The length will not be stored. */
					)
{
    StableBuffer    *sb;
    StringList      *sl;

    SimpleHT        *nl;
    Array           *wl;

	EntryForString NewEntry;

	if( dl == NULL )
	{
		return FALSE;
	}

    sb = &(dl->AdditionalDataChunk);
    sl = dl->List;
    nl = &(dl->List_Pos);
    wl = &(dl->List_W_Pos);

	if( AdditionalData != NULL && LengthOfAdditionalData > 0 )
	{
	    NewEntry.Data = sb->Add(sb,
                             AdditionalData,
                             LengthOfAdditionalData,
                             TRUE
                             );
	    if( NewEntry.Data == NULL )
        {
            return -1;
        }
	} else {
		NewEntry.Data = NULL;
	}

	NewEntry.str = sl->Add(sl, Str, NULL);
	if( NewEntry.str == NULL )
	{
		return -2;
	}

	if( ContainWildCard(Str) )
	{
		if( Array_PushBack(wl, &NewEntry, NULL) < 0 )
        {
            return -3;
        }
	} else {
		if( SimpleHT_Add(nl, Str, 0, (const char *)&NewEntry, NULL) == NULL )
        {
            return -4;
        }
	}

	return 0;
}

int StringChunk_Add_Domain(StringChunk	*dl,
							const char	*Domain,
							const void	*AdditionalData,
							int			LengthOfAdditionalData /* The length will not be stored. */
							)
{
	if( *Domain == '.' )
	{
		++Domain;
	}

	return StringChunk_Add(dl, Domain, AdditionalData, LengthOfAdditionalData);
}

BOOL StringChunk_Match_NoWildCard(StringChunk	*dl,
								  const char	*Str,
								  int			*HashValue,
								  void			**Data
								  )
{
    SimpleHT        *nl;

	EntryForString *FoundEntry;

	const char *FoundString;

	if( dl == NULL )
	{
		return FALSE;
	}

    nl = &(dl->List_Pos);

	FoundEntry = (EntryForString *)SimpleHT_Find(nl, Str, 0, HashValue, NULL);
	while( FoundEntry != NULL )
	{
		FoundString = FoundEntry->str;
		if( strcmp(FoundString, Str) == 0 )
		{
			if( Data != NULL )
			{
				*Data = (void *)(FoundEntry->Data);
			}

			return TRUE;
		}

		FoundEntry = (EntryForString *)SimpleHT_Find(nl, Str, 0, HashValue, (const char *)FoundEntry);
	}

	return FALSE;

}

BOOL StringChunk_Match_OnlyWildCard(StringChunk	*dl,
									const char	*Str,
									void		**Data
									)
{
    Array           *wl;

	EntryForString *FoundEntry;

	int loop;

	if( dl == NULL )
	{
		return FALSE;
	}

    wl = &(dl->List_W_Pos);

	for( loop = 0; loop != Array_GetUsed(wl); ++loop )
	{
		FoundEntry = (EntryForString *)Array_GetBySubscript(wl, loop);
		if( FoundEntry != NULL )
		{
            const char *FoundString = FoundEntry->str;
			if( WILDCARD_MATCH(FoundString, Str) == WILDCARD_MATCHED )
			{
				if( Data != NULL )
				{
					*Data = (void *)(FoundEntry->Data);
				}
				return TRUE;
			}

		} else {
			return FALSE;
		}
	}

	return FALSE;
}

BOOL StringChunk_Match(StringChunk *dl, const char *Str, int *HashValue, void **Data)
{
	return (StringChunk_Match_NoWildCard(dl, Str, HashValue, Data) ||
		StringChunk_Match_OnlyWildCard(dl, Str, Data));
}

static BOOL StringChunk_Match_WildCard_Exacly(StringChunk	*dl,
                                        const char	*Str,
                                        void		**Data
                                        )
{
    Array           *wl;

	EntryForString *FoundEntry;

	int loop;

	if( dl == NULL )
	{
		return FALSE;
	}

    wl = &(dl->List_W_Pos);

	for( loop = 0; loop != Array_GetUsed(wl); ++loop )
	{
		FoundEntry = (EntryForString *)Array_GetBySubscript(wl, loop);
		if( FoundEntry != NULL )
		{
            const char *FoundString = FoundEntry->str;
			if( strcmp(Str, FoundString) == 0 )
			{
				if( Data != NULL )
				{
					*Data = (void *)(FoundEntry->Data);
				}
				return TRUE;
			}

		} else {
			return FALSE;
		}
	}

	return FALSE;
}

BOOL StringChunk_Match_Exacly(StringChunk *dl, const char *Str, int *HashValue, void **Data)
{
	return (StringChunk_Match_NoWildCard(dl, Str, HashValue, Data) ||
		StringChunk_Match_WildCard_Exacly(dl, Str, Data));
}

BOOL StringChunk_Domain_Match_NoWildCard(StringChunk *dl, const char *Domain, int *HashValue, void **Data)
{
	if( StringChunk_Match_NoWildCard(dl, Domain, HashValue, Data) == TRUE )
	{
		return TRUE;
	}

	Domain = strchr(Domain + 1, '.');

	while( Domain != NULL )
	{
		if( StringChunk_Match_NoWildCard(dl, Domain + 1, NULL, Data) == TRUE )
		{
			return TRUE;
		}

		Domain = strchr(Domain + 1, '.');
	}

	return FALSE;
}

BOOL StringChunk_Domain_Match(StringChunk *dl, const char *Domain, int *HashValue, void **Data)
{
	if( dl == NULL )
	{
		return FALSE;
	}

	return (StringChunk_Domain_Match_NoWildCard(dl, Domain, HashValue, Data) ||
			StringChunk_Match_OnlyWildCard(dl, Domain, Data) );
}

/* Start by 0 */
const char *StringChunk_Enum_NoWildCard(StringChunk *dl, int32_t *Start, void **Data)
{
	EntryForString *Result;

	Result = (EntryForString *)SimpleHT_Enum(&(dl->List_Pos), Start);
	if( Result == NULL )
	{
		if( Data != NULL )
		{
			*Data = NULL;
		}

		return NULL;
	}

	if( Data != NULL )
	{
		*Data = (void *)Result->Data;
	}

	return Result->str;
}

void StringChunk_Free(StringChunk *dl, BOOL FreeStringList)
{
	SimpleHT_Free(&(dl->List_Pos));
	Array_Free(&(dl->List_W_Pos));
	dl->AdditionalDataChunk.Free(&(dl->AdditionalDataChunk));

	if( FreeStringList == TRUE )
	{
		dl->List->Free(dl->List);
		SafeFree(dl->List);
	}
}
