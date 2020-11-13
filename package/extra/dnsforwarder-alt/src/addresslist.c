#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "addresslist.h"
#include "common.h"
#include "utils.h"

int AddressList_Init(AddressList *a)
{
	if( a == NULL )
	{
		return 0;
	}

	if( Array_Init(&(a->AddressList), sizeof(Address_Type), 8, FALSE, NULL) != 0 )
	{
		return -1;
	}

	a->Counter = 0;
	return 0;
}


int AddressList_Add(AddressList *a, Address_Type	*Addr)
{
	if( a == NULL )
	{
		return -1;
	}

	if( Array_PushBack(&(a->AddressList), Addr, NULL) < 0 )
	{
		return -1;
	} else {
		return 0;
	}

}

sa_family_t AddressList_ConvertFromString(Address_Type *Out, const char *Addr_Port, int DefaultPort)
{
	sa_family_t	Family;

	memset(Out, 0, sizeof(Address_Type));

	Family = GetAddressFamily(Addr_Port);
	Out->family = Family;

	switch( Family )
	{
		case AF_INET6:
			{
				char		Addr[LENGTH_OF_IPV6_ADDRESS_ASCII] = {0};
				in_port_t	Port;
				const char	*PortPos;

				memset(Addr, 0, sizeof(Addr));

				PortPos = strchr(Addr_Port, ']');
				if( PortPos == NULL )
				{
					return AF_UNSPEC;
				}

				PortPos = strchr(PortPos, ':');
				if( PortPos == NULL )
				{
					sscanf(Addr_Port + 1, "%[^]]", Addr);
					Port = DefaultPort;
				} else {
					int	Port_warpper;

					sscanf(Addr_Port + 1, "%[^]]", Addr);
					sscanf(PortPos + 1, "%d", &Port_warpper);
					Port = Port_warpper;
				}

				Out->Addr.Addr6.sin6_family = Family;
				Out->Addr.Addr6.sin6_port = htons(Port);

				IPv6AddressToNum(Addr, &(Out->Addr.Addr6.sin6_addr));

				return AF_INET6;
			}
			break;

		case AF_INET:
			{
				char		Addr[] = "xxx.xxx.xxx.xxx";
				in_port_t	Port;
				const char	*PortPos;

				memset(Addr, 0, sizeof(Addr));

				PortPos = strchr(Addr_Port, ':');
				if( PortPos == NULL )
				{
					sscanf(Addr_Port, "%s", Addr);
					Port = DefaultPort;
				} else {
					int Port_warpper;
					sscanf(Addr_Port, "%[^:]", Addr);
					sscanf(PortPos + 1, "%d", &Port_warpper);
					Port = Port_warpper;
				}
				FILL_ADDR4(Out->Addr.Addr4, Family, Addr, Port);

				return AF_INET;
			}
			break;

		default:
			return AF_UNSPEC;
			break;
	}
}

int AddressList_Add_From_String(AddressList *a, const char *Addr_Port, int DefaultPort)
{
	Address_Type	Tmp;

	if( AddressList_ConvertFromString(&Tmp, Addr_Port, DefaultPort) == AF_UNSPEC )
	{
		return -1;
	}

	return AddressList_Add(a, &Tmp);

}

int AddressList_Advance(AddressList *a)
{
	if( a == NULL )
	{
		return 0;
	}

	return (a->Counter)++;
}

struct sockaddr *AddressList_GetOneBySubscript(AddressList *a, sa_family_t *family, int Subscript)
{
	Address_Type *Result;

	if( a == NULL )
	{
		return 0;
	}

	Result = (Address_Type *)Array_GetBySubscript(&(a->AddressList), Subscript);
	if( Result == NULL )
	{
		return NULL;
	} else {
		if( family != NULL )
		{
			*family = Result->family;
		}
		return (struct sockaddr *)&(Result->Addr);
	}
}

struct sockaddr *AddressList_GetOne(AddressList *a, sa_family_t *family)
{
	return AddressList_GetOneBySubscript(a, family, a->Counter % Array_GetUsed(&(a->AddressList)));
}

struct sockaddr **AddressList_GetPtrListOfFamily(AddressList *a, sa_family_t family)
{
	int Itr;
	int NumberOfAddresses = AddressList_GetNumberOfAddresses(a);
	struct sockaddr **AddrList, **AddrList_Ori;
	struct sockaddr *OneAddr;
	sa_family_t OneFamily = AF_UNSPEC;

	AddrList = SafeMalloc(sizeof(struct sockaddr *) * (NumberOfAddresses + 1));
	if( AddrList == NULL )
	{
		return NULL;
	}

	AddrList_Ori = AddrList;
	Itr = 0;
	while( Itr != NumberOfAddresses )
	{
		OneAddr = AddressList_GetOneBySubscript(a, &OneFamily, Itr);
		if( OneFamily == family )
		{
			*AddrList = OneAddr;
			++AddrList;
		}
		++Itr;
	}

	*AddrList = NULL;
	return AddrList_Ori;
}

struct sockaddr **AddressList_GetPtrList(AddressList *a, sa_family_t **families)
{
	int Itr;

	int NumberOfAddresses = AddressList_GetNumberOfAddresses(a);
	struct sockaddr **AddrList;

	AddrList = SafeMalloc(sizeof(struct sockaddr *) * (NumberOfAddresses + 1));
	if( AddrList == NULL )
	{
		return NULL;
	}

	*families = SafeMalloc(sizeof(sa_family_t) * (NumberOfAddresses + 1));
    if( *families == NULL )
    {
		SafeFree(AddrList);
		return NULL;
    }

	Itr = 0;
	while( Itr != NumberOfAddresses )
	{
		AddrList[Itr] = AddressList_GetOneBySubscript(a, &((*families)[Itr]), Itr);

		++Itr;
	}

	AddrList[Itr] = NULL;
	(*families)[Itr] = AF_UNSPEC;
	return AddrList;
}
