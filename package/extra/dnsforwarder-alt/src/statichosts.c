#include "hostscontainer.h"
#include "statichosts.h"
#include "logs.h"

static HostsContainer	MainStaticContainer;
static BOOL             Inited = FALSE;

int StaticHosts_Init(ConfigFileInfo *ConfigInfo)
{
	StringList *AppendHosts = ConfigGetStringList(ConfigInfo, "AppendHosts");
	StringListIterator  sli;

	const char *Itr;

	if( HostsContainer_Init(&MainStaticContainer) != 0 )
	{
		return -17;
	}

	if( AppendHosts == NULL )
	{
		return -22;
	}

	if( StringListIterator_Init(&sli, AppendHosts) != 0 )
    {
        return -27;
    }

	Itr = sli.Next(&sli);
	while( Itr != NULL )
	{
        MainStaticContainer.Load(&MainStaticContainer, Itr);

		Itr = sli.Next(&sli);
	}

	Inited = TRUE;

	INFO("Loading Appendhosts completed.\n");

	return 0;
}

int StaticHosts_GetCName(const char *Domain, char *Buffer)
{
    if( !Inited )
    {
        return -49;
    }

    return HostsUtils_GetCName(Domain, Buffer, &MainStaticContainer);
}

BOOL StaticHosts_TypeExisting(const char *Domain, HostsRecordType Type)
{
    if( !Inited )
    {
        return FALSE;
    }

    return HostsUtils_TypeExisting(&MainStaticContainer, Domain, Type);
}

HostsUtilsTryResult StaticHosts_Try(IHeader *Header, int BufferLength)
{
    if( !Inited )
    {
        return HOSTSUTILS_TRY_NONE;
    }

    return HostsUtils_Try(Header, BufferLength, &MainStaticContainer);
}
