#include <string.h>
#include <ctype.h>
#include "hostscontainer.h"
#include "logs.h"

typedef struct _TableNode TableNode;

struct _TableNode{
    const TableNode *Next;
    HostsRecordType Type;
    const void      *Data;
};

PRIFUNC HostsRecordType HostsContainer_DetermineType(const char *IPOrCName)
{
	if( IPOrCName == NULL )
	{
		return HOSTS_TYPE_UNKNOWN;
	}

	/* Good IP List */
	if( *IPOrCName == '<' && IPOrCName[strlen(IPOrCName) - 1] == '>' )
	{
		return HOSTS_TYPE_GOOD_IP_LIST;
	}

	/* A host IPOrCName started with "@@ " is excluded */
	if( *IPOrCName == '@' && *(IPOrCName + 1) == '@' )
	{
		return HOSTS_TYPE_EXCLUEDE;
	}

	if( isxdigit(*IPOrCName) )
	{
		const char *Itr;
		/* Check if it is IPv6 */
		if( strchr(IPOrCName, ':') != NULL )
		{
			return HOSTS_TYPE_AAAA;
		}

		/* Check if it is CNAME */
		for(Itr = IPOrCName; *Itr != '\0'; ++Itr)
		{
			if( isalpha(*Itr) || *Itr == '-' )
			{
				return HOSTS_TYPE_CNAME;
			}
		}

		for(Itr = IPOrCName; *Itr != '\0'; ++Itr)
		{
			if( isdigit(*Itr) || *Itr == '.' )
			{
				return HOSTS_TYPE_A;
			}
		}

		return HOSTS_TYPE_UNKNOWN;

	} else {

		if( *IPOrCName == ':' )
		{
			return HOSTS_TYPE_AAAA;
		}

		for(; *IPOrCName != '\0'; ++IPOrCName)
		{
			if( !isalnum(*IPOrCName) && *IPOrCName != '-' && *IPOrCName != '.' )
			{
				return HOSTS_TYPE_UNKNOWN;
			}
		}

		return HOSTS_TYPE_CNAME;
	}
}

/*
 If Func == NULL:
    Return NULL : No match found;
    Otherwise : A match found;

 If Func != NULL:
    Return NULL : Func returned non-zero at a call or no match found;
    Otherwise : Func returned zero at every call and at least one match found;
*/
PUBFUNC const void *HostsContainer_Find(HostsContainer  *Container,
                                        const char      *Name,
                                        HostsRecordType Type,
                                        HostsFindFunc   Func,
                                        void            *Arg
                                        )
{
    int Number = 1;

	const TableNode **Matched = NULL;
	const TableNode *IP = NULL;

	if( !StringChunk_Match(&(Container->Mappings), Name, NULL, (void **)&Matched) )
	{
        return NULL;
	}

	if( Matched != NULL )
    {
        IP = *Matched;
    }

    if( Func != NULL )
    {
        const TableNode *ret = NULL;

        while( IP != NULL )
        {
            if( Type == HOSTS_TYPE_UNKNOWN || IP->Type == Type )
            {
                /* Found */
                if( Func(Number++, IP->Type, IP->Data, Arg) != 0 )
                {
                    return NULL;
                }

                ret = IP;
            }

            IP = IP->Next;
        }

        return ret;
    } else {
        while( IP != NULL )
        {
            if( Type == HOSTS_TYPE_UNKNOWN || IP->Type == Type )
            {
                return IP;
            }

            IP = IP->Next;
        }

        return NULL;
    }
}

PRIFUNC const void *HostsContainer_FindExist(HostsContainer  *Container,
                                            const char      *Name
                                            )
{
	const TableNode **Matched = NULL;
	const TableNode *IP = NULL;

	if( !StringChunk_Match_Exacly(&(Container->Mappings), Name, NULL, (void **)&Matched) )
	{
        return NULL;
	}

	if( Matched != NULL )
    {
        IP = *Matched;
    }

    if( IP != NULL )
    {
        return IP;
    } else {
        return NULL;
    }
}

PRIFUNC int HostsContainer_AddNode(HostsContainer   *Container,
                                   const char       *Name,
                                   HostsRecordType  Type,
                                   const void       *Data,
                                   int              DataLength
                                   )
{
    TableNode   n, *s = NULL, *Exist = NULL;

    if( Data != NULL )
    {
        n.Data = Container->Table.Add(&(Container->Table),
                                      Data,
                                      DataLength,
                                      TRUE
                                      );
        if( n.Data == NULL )
        {
            return -171;
        }
    } else {
        n.Data = NULL;
    }

    n.Type = Type;

    Exist = (TableNode *)HostsContainer_FindExist(Container, Name);
    if( Exist == NULL )
    {
        n.Next = NULL;
        s = Container->Table.Add(&(Container->Table),
                                 &n,
                                 sizeof(TableNode),
                                 TRUE
                                 );
        if( s == NULL )
        {
            return -201;
        }

        if( StringChunk_Add(&(Container->Mappings),
                            Name,
                            &s,
                            sizeof(TableNode *)
                            )
            != 0 )
        {
            return -212;
        }
    } else {
        n.Next = Exist->Next;
        s = Container->Table.Add(&(Container->Table),
                                 &n,
                                 sizeof(TableNode),
                                 TRUE
                                 );
        if( s == NULL )
        {
            return -213;
        }

        Exist->Next = s;
    }

    return 0;
}

PRIFUNC int HostsContainer_AddIPV6(HostsContainer   *Container,
                                   const char       *IPOrCName,
                                   const char       *Domain
                                   )
{
	char		NumericIP[16];

	IPv6AddressToNum(IPOrCName, NumericIP);

    return HostsContainer_AddNode(Container,
                                  Domain,
                                  HOSTS_TYPE_AAAA,
                                  NumericIP,
                                  16
                                  );
}

PRIFUNC int HostsContainer_AddIPV4(HostsContainer *Container,
                                   const char *IPOrCName,
                                   const char *Domain
                                   )
{
	char		NumericIP[4];

	IPv4AddressToNum(IPOrCName, NumericIP);

    return HostsContainer_AddNode(Container,
                                  Domain,
                                  HOSTS_TYPE_A,
                                  NumericIP,
                                  4
                                  );
}

PRIFUNC int HostsContainer_AddCName(HostsContainer *Container,
                                    const char *IPOrCName,
                                    const char *Domain
                                    )
{
    return HostsContainer_AddNode(Container,
                                  Domain,
                                  HOSTS_TYPE_CNAME,
                                  IPOrCName,
                                  strlen(IPOrCName) + 1
                                  );
}

PRIFUNC int HostsContainer_AddGoodIpList(HostsContainer *Container,
                                         const char *ListName,
                                         const char *Domain
                                         )
{
	char            Trimed[128];

    sscanf(ListName, "<%127[^>]", Trimed);

    return HostsContainer_AddNode(Container,
                                  Domain,
                                  HOSTS_TYPE_GOOD_IP_LIST,
                                  Trimed,
                                  strlen(Trimed) + 1
                                  );
}

PRIFUNC int HostsContainer_AddExcluded(HostsContainer *Container,
                                       const char *Domain
                                       )
{
    return HostsContainer_AddNode(Container,
                                  Domain,
                                  HOSTS_TYPE_EXCLUEDE,
                                  NULL,
                                  0
                                  );
}

PRIFUNC HostsRecordType HostsContainer_Add(HostsContainer *Container,
                                           const char *IPOrCName,
                                           const char *Domain
                                           )
{
	switch( HostsContainer_DetermineType(IPOrCName) )
	{
		case HOSTS_TYPE_AAAA:
			if( HostsContainer_AddIPV6(Container, IPOrCName, Domain) != 0)
			{
				return HOSTS_TYPE_UNKNOWN;
			} else {
				return HOSTS_TYPE_AAAA;
			}
			break;

		case HOSTS_TYPE_A:
			if( HostsContainer_AddIPV4(Container, IPOrCName, Domain) != 0 )
			{
				return HOSTS_TYPE_UNKNOWN;
			} else {
				return HOSTS_TYPE_A;
			}
			break;

		case HOSTS_TYPE_CNAME:
			if( HostsContainer_AddCName(Container, IPOrCName, Domain) != 0 )
			{
				return HOSTS_TYPE_UNKNOWN;
			} else {
				return HOSTS_TYPE_CNAME;
			}
			break;

		case HOSTS_TYPE_EXCLUEDE:
			if( HostsContainer_AddExcluded(Container, Domain) != 0 )
			{
				return HOSTS_TYPE_UNKNOWN;
			} else {
				return HOSTS_TYPE_EXCLUEDE;
			}
			break;

		case HOSTS_TYPE_GOOD_IP_LIST:
			if( HostsContainer_AddGoodIpList(Container, IPOrCName, Domain) != 0 )
			{
				return HOSTS_TYPE_UNKNOWN;
			} else {
				return HOSTS_TYPE_GOOD_IP_LIST;
			}
			break;

		default:
			INFO("Unrecognisable host : %s %s\n", IPOrCName, Domain);
			return HOSTS_TYPE_UNKNOWN;
			break;
	}
}

PUBFUNC HostsRecordType HostsContainer_Load(HostsContainer *Container,
                                            const char *MetaLine
                                            )
{
	char IPOrCName[DOMAIN_NAME_LENGTH_MAX + 1];
	char Domain[DOMAIN_NAME_LENGTH_MAX + 1];

	if( sscanf(MetaLine,
               "%" STRINGIZINGINT(DOMAIN_NAME_LENGTH_MAX) "s%" STRINGIZINGINT(DOMAIN_NAME_LENGTH_MAX) "s",
               IPOrCName,
               Domain
               )
     != 2 )
    {
		INFO("Unrecognisable host : %s, it may be too long.\n", MetaLine);
		return HOSTS_TYPE_UNKNOWN;
    }

	return HostsContainer_Add(Container, IPOrCName, Domain);
}

PUBFUNC void HostsContainer_Free(HostsContainer *Container)
{
	StringChunk_Free(&(Container->Mappings), TRUE);
	Container->Table.Free(&(Container->Table));
}

int HostsContainer_Init(HostsContainer *Container)
{
	if( StringChunk_Init(&(Container->Mappings), NULL) != 0 )
	{
		return -2;
	}

	if( StableBuffer_Init(&(Container->Table)) != 0 )
	{
		return -6;
	}

	Container->Load = HostsContainer_Load;
	Container->Find = HostsContainer_Find;
	Container->Free = HostsContainer_Free;

	return 0;
}
