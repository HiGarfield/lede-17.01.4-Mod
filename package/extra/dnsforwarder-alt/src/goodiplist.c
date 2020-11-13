#include <string.h>
#include <limits.h>
#include "readconfig.h"
#include "goodiplist.h"
#include "utils.h"
#include "timedtask.h"
#include "socketpuller.h"
#include "logs.h"

typedef struct _ListInfo{
	int     Interval;
	Array   List;
} ListInfo;

static StringChunk	*GoodIpList = NULL;

/* The fastest returned */
static struct sockaddr_in *CheckAList(struct sockaddr_in *Ips, int Count)
{
    SocketPuller    p;
    int i;
    struct timeval	Time	=	{5, 0};
    struct sockaddr_in **Fastest = NULL;

    struct sockaddr_in *ret = NULL;

    if( SocketPuller_Init(&p) != 0 )
    {
        return NULL;
    }

	for( i = 0; i != Count; ++i )
    {
		SOCKET	skt;
		struct sockaddr *a;

		skt = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if( skt == INVALID_SOCKET )
		{
			continue;
		}
		SetSocketNonBlock(skt, TRUE);

		a = (struct sockaddr *)&(Ips[i]);

		if( connect(skt, a, sizeof(struct sockaddr_in)) != 0 &&
            FatalErrorDecideding(GET_LAST_ERROR()) != 0
            )
		{
			CLOSE_SOCKET(skt);
			continue;
		}

		p.Add(&p, skt, &a, sizeof(struct sockaddr *));
    }

    if( p.Select(&p, &Time, (void **)&Fastest, FALSE, TRUE) == INVALID_SOCKET )
    {
        ret = NULL;
    }

    if( Fastest == NULL )
    {
        ret = NULL;
    } else {
        ret = *Fastest;
    }

    p.Free(&p);

    return ret;
}

static int ThreadJod(const char *Domain, ListInfo *inf)
{
    struct sockaddr_in *Fastest, *First;

    if( inf == NULL )
    {
        return -159;
    }

    Fastest = CheckAList((struct sockaddr_in *)Array_GetRawArray(&(inf->List)),
                         Array_GetUsed(&(inf->List))
                         );

    if( Fastest != NULL )
    {
        struct sockaddr_in t;

        INFO("The fastest ip for `%s' is %s\n",
             Domain,
             inet_ntoa(Fastest->sin_addr)
             );

        First = Array_GetBySubscript(&(inf->List), 0);
        if( First == NULL )
        {
            return -178;
        }

        memcpy(&t, Fastest, sizeof(struct sockaddr_in));
        memcpy(Fastest, First, sizeof(struct sockaddr_in));
        memcpy(First, &t, sizeof(struct sockaddr_in));
    } else {
        INFO("Checking list `%s' timeout.\n", Domain);
    }

    return 0;
}

/* GoodIPList list1 1000 */
static int InitListsAndTimes(ConfigFileInfo *ConfigInfo)
{
	StringList	*l	=	ConfigGetStringList(ConfigInfo, "GoodIPList");
	StringListIterator  sli;

	const char	*Itr	=	NULL;

	if( l == NULL )
	{
		return -1;
	}

	if( StringListIterator_Init(&sli, l) != 0 )
    {
        return -2;
    }

	GoodIpList = SafeMalloc(sizeof(StringChunk));
	if( GoodIpList != NULL && StringChunk_Init(GoodIpList, NULL) != 0 )
	{
		return -3;
	}

    while( (Itr = sli.Next(&sli)) != NULL )
    {
		ListInfo	m = {0, Array_Init_Static(sizeof(struct sockaddr_in))};
		char n[128];

		sscanf(Itr, "%127s%d", n, &(m.Interval));

		if( m.Interval <= 0 )
		{
			ERRORMSG("GoodIpList is invalid : %s\n", Itr);
			continue;
		}

		StringChunk_Add(GoodIpList, n, (const char *)&m, sizeof(ListInfo));
    }

    return 0;
}

/* GoodIPListAddIP list1 ip:port */
static int AddToLists(ConfigFileInfo *ConfigInfo)
{
	StringList	*l	=	ConfigGetStringList(ConfigInfo, "GoodIPListAddIP");
	StringListIterator  sli;

	const char	*Itr	=	NULL;

	if( l == NULL )
	{
		return -1;
	}

	if( StringListIterator_Init(&sli, l) != 0 )
    {
        return -2;
    }

    while( (Itr = sli.Next(&sli)) != NULL )
    {
		ListInfo	*m = NULL;
		char n[128], ip_str[LENGTH_OF_IPV4_ADDRESS_ASCII];
		int Port;
		struct sockaddr_in	ip;

		sscanf(Itr, "%127s%*[^0123456789]%15[^:]:%d", n, ip_str, &Port);
		ip.sin_port = htons(Port);
		ip.sin_family = AF_INET; /* IPv4 only */

		IPv4AddressToNum(ip_str, &(ip.sin_addr));

		if( StringChunk_Match_NoWildCard(GoodIpList,
                                         n,
                                         NULL,
                                         (void **)&m)
            == FALSE
            )
		{
			ERRORMSG("GoodIpList is not found : %s\n", Itr);
			continue;
		}

		Array_PushBack(&(m->List), &ip, NULL);
    }

    return 0;
}

static int AddTask(void)
{
    ListInfo *m;
    const char *Domain;

    int32_t Start = 0;

    Domain = StringChunk_Enum_NoWildCard(GoodIpList, &Start, (void **)&m);
    while( Domain != NULL )
    {
        if( m != NULL )
        {
            TimedTask_Add(TRUE,
                          FALSE,
                          m->Interval,
                          (TaskFunc)ThreadJod,
                          (void *)Domain,
                          (void *)m,
                          TRUE
                          );
        }

        Domain = StringChunk_Enum_NoWildCard(GoodIpList, &Start, (void **)&m);
    }

    return 0;
}

int GoodIpList_Init(ConfigFileInfo *ConfigInfo)
{
	if( InitListsAndTimes(ConfigInfo) != 0 )
	{
		return -1;
	}

	if( AddToLists(ConfigInfo) != 0 )
	{
		return -2;
	}

	if( AddTask() != 0 )
	{
		return -270;
	}

	return 0;
}

const char *GoodIpList_Get(const char *List)
{
    ListInfo   *m;
    if( StringChunk_Match_NoWildCard(GoodIpList,
                                     List,
                                     NULL,
                                     (void **)&m
                                     )
       == TRUE )
    {
        if( Array_GetUsed(&(m->List)) <= 0 )
        {
            return NULL;
        } else {
            return (const char *)&(((const struct sockaddr_in *)Array_GetBySubscript(&(m->List), 0))->sin_addr);
        }
    } else {
        return NULL;
    }
}
