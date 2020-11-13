#include <string.h>
#include "mmgr.h"
#include "stringchunk.h"
#include "utils.h"
#include "filter.h"
#include "hosts.h"
#include "dnscache.h"
#include "logs.h"
#include "ipmisc.h"
#include "readline.h"

typedef int (*SendFunc)(void *Module,
                        IHeader *h, /* Entity followed */
                        int BufferLength
                        );

typedef struct _ModuleInterface {
    union {
        UdpM    Udp;
        TcpM    Tcp;
    } ModuleUnion;

    SendFunc    Send;

    const char *ModuleName;

} ModuleInterface;

static StableBuffer Modules; /* Storing ModuleInterfaces */
static Array        ModuleArray; /* ModuleInterfaces' references */
static StringChunk  Distributor; /* Domain-to-ModuleInterface mapping */

static int MappingAModule(ModuleInterface *Stored, StringList *DomainList)
{
    StringListIterator  i;
    const char *OneDomain;

    DomainList->TrimAll(DomainList, "\t .");
    DomainList->LowercaseAll(DomainList);

    if( StringListIterator_Init(&i, DomainList) != 0 )
    {
        return -46;
    }

    while( (OneDomain = i.Next(&i)) != NULL )
    {
        StringChunk_Add_Domain(&Distributor,
                               OneDomain,
                               &Stored,
                               sizeof(ModuleInterface *)
                               );
    }

    return 0;
}

static ModuleInterface *StoreAModule(void)
{
    ModuleInterface *Added;

    Added = Modules.Add(&Modules, NULL, sizeof(ModuleInterface), TRUE);
    if( Added == NULL )
    {
        return NULL;
    }

    if( Array_PushBack(&ModuleArray, &Added, NULL) < 0 )
    {
        return NULL;
    }

    Added->ModuleName = "Unknown";

    return Added;
}

static int Udp_Init_Core(const char *Services,
                         StringList *DomainList,
                         const char *Parallel
                         )
{
    ModuleInterface *NewM;

    char ParallelOnOff[8];
    BOOL ParallelQuery;

    if( Services == NULL || DomainList == NULL || Parallel == NULL )
    {
        return -99;
    }

    NewM = StoreAModule();
    if( NewM == NULL )
    {
        return -101;
    }

    NewM->ModuleName = "UDP";

    strncpy(ParallelOnOff, Parallel, sizeof(ParallelOnOff));
    ParallelOnOff[sizeof(ParallelOnOff) - 1] = '\0';
    StrToLower(ParallelOnOff);

    if( strcmp(ParallelOnOff, "on") == 0 )
    {
        ParallelQuery = TRUE;
    } else {
        ParallelQuery = FALSE;
    }

    /* Initializing module */
    if( UdpM_Init(&(NewM->ModuleUnion.Udp), Services, ParallelQuery) != 0 )
    {
        return -128;
    }

    NewM->Send = (SendFunc)(NewM->ModuleUnion.Udp.Send);

    if( MappingAModule(NewM, DomainList) != 0 )
    {
        ERRORMSG("Mapping UDP module of %s failed.\n", Services);
    }

    return 0;
}

static int Udp_Init(StringListIterator  *i)
{
    const char *Services;
    const char *Domains;
    const char *Parallel;

    StringList DomainList;

    /* Initializing parameters */
    Services = i->Next(i);
    Domains = i->Next(i);
    Parallel = i->Next(i);

    if( Domains == NULL )
    {
        return -143;
    }

    if( StringList_Init(&DomainList, Domains, ",") != 0 )
    {
        return -148;
    }

    if( Udp_Init_Core(Services, &DomainList, Parallel) != 0 )
    {
        return -153;
    }

    DomainList.Free(&DomainList);

    return 0;
}

static int Tcp_Init_Core(const char *Services,
                         StringList *DomainList,
                         const char *Proxies
                         )
{
    ModuleInterface *NewM;

    char ProxyString[8];

    if( Services == NULL || DomainList == NULL || Proxies == NULL )
    {
        return -157;
    }

    NewM = StoreAModule();
    if( NewM == NULL )
    {
        return -192;
    }

    NewM->ModuleName = "TCP";

    strncpy(ProxyString, Proxies, sizeof(ProxyString));
    ProxyString[sizeof(ProxyString) - 1] = '\0';
    StrToLower(ProxyString);

    if( strcmp(ProxyString, "no") == 0 )
    {
        Proxies = NULL;
    }

    /* Initializing module */
    if( TcpM_Init(&(NewM->ModuleUnion.Tcp), Services, Proxies) != 0 )
    {
        return -180;
    }

    NewM->Send = (SendFunc)(NewM->ModuleUnion.Tcp.Send);

    if( MappingAModule(NewM, DomainList) != 0 )
    {
        ERRORMSG("Mapping TCP module of %s failed.\n", Services);
    }

    return 0;
}

static int Tcp_Init(StringListIterator  *i)
{
    const char *Services;
    const char *Domains;
    const char *Proxies;

    StringList DomainList;

    /* Initializing parameters */
    Services = i->Next(i);
    Domains = i->Next(i);
    Proxies = i->Next(i);

    if( Domains == NULL )
    {
        return -143;
    }

    if( StringList_Init(&DomainList, Domains, ",") != 0 )
    {
        return -148;
    }

    if( Tcp_Init_Core(Services, &DomainList, Proxies) != 0 )
    {
        return -233;
    }

    DomainList.Free(&DomainList);

    return 0;
}

/*
# UDP
PROTOCOL UDP
SERVER 1.2.4.8,127.0.0.1
PARALLEL ON

example.com

#############################
# TCP
PROTOCOL TCP
SERVER 1.2.4.8,127.0.0.1
PROXY NO

example.com

#############################
# TCP
PROTOCOL TCP
SERVER 1.2.4.8,127.0.0.1
PROXY 192.168.1.1:8080,192.168.1.1:8081

example.com

*/
static int Modules_InitFromFile(StringListIterator  *i)
{
    #define MAX_PATH_BUFFER     384

    StringChunk Args;
    StringList  Domains;

    const char *FileOri;
    char File[MAX_PATH_BUFFER];
    FILE *fp;

    ReadLineStatus  Status;

    const char *Protocol = NULL;

    FileOri = i->Next(i);

    if( FileOri == NULL )
    {
        return -201;
    }

    strncpy(File, FileOri, sizeof(File));
    File[sizeof(File) - 1] = '\0';

    ReplaceStr(File, "\"", "");

    ExpandPath(File, sizeof(File));

    fp = fopen(File, "r");
    if( fp == NULL )
    {
        WARNING("Cannot open group file \"%s\".\n", File);
        return 0;
    }

    if( StringChunk_Init(&Args, NULL) != 0 )
    {
        return -230;
    }

    if( StringList_Init(&Domains, NULL, NULL) != 0 )
    {
        return -235;
    }

    do {
        char Buffer[384];
        const char *Value;

        Status = ReadLine(fp, Buffer, sizeof(Buffer));

        if( Status == READ_TRUNCATED )
        {
            WARNING("Line is too long %s, file \"%s\".\n", Buffer, File);
            Status = ReadLine_GoToNextLine(fp);
            continue;
        }

        if( Status == READ_FAILED_OR_END )
        {
            break;
        }

        StrToLower(Buffer);

        Value = SplitNameAndValue(Buffer, " \t=");
        if( Value != NULL )
        {
            StringChunk_Add(&Args, Buffer, Value, strlen(Value) + 1);
        } else {
            Domains.Add(&Domains, Buffer, NULL);
        }

    } while( TRUE );

    fclose(fp);

    if( !StringChunk_Match_NoWildCard(&Args,
                                      "protocol",
                                      NULL,
                                      (void **)&Protocol
                                      ) ||
        Protocol == NULL
        )
    {
        ERRORMSG("No protocol specified, file \"%s\".\n", File);
        return -270;
    }

    if( strcmp(Protocol, "udp") == 0 )
    {
        const char *Services = NULL;
        const char *Parallel = "on";

        StringChunk_Match_NoWildCard(&Args, "server", NULL, (void **)&Services);
        StringChunk_Match_NoWildCard(&Args,
                                     "parallel",
                                     NULL,
                                     (void **)&Parallel
                                     );

        if( Udp_Init_Core(Services, &Domains, Parallel) != 0 )
        {
            ERRORMSG("Loading group file \"%s\" failed.\n", File);
            return -337;
        }

    } else if( strcmp(Protocol, "tcp") == 0 )
    {
        const char *Services = NULL;
        const char *Proxies = "no";

        StringChunk_Match_NoWildCard(&Args, "server", NULL, (void **)&Services);
        StringChunk_Match_NoWildCard(&Args, "proxy", NULL, (void **)&Proxies);

        if( Tcp_Init_Core(Services, &Domains, Proxies) != 0 )
        {
            ERRORMSG("Loading group file \"%s\" failed.\n", File);
            return -233;
        }

    } else {
        ERRORMSG("Unknown protocol %s, file \"%s\".\n", Protocol, File);
        return -281;
    }

    StringChunk_Free(&Args, TRUE);
    Domains.Free(&Domains);

    return 0;
}

static int Modules_Init(ConfigFileInfo *ConfigInfo)
{
    StringList  *ServerGroups;
    StringListIterator  i;

    const char *Type;

    ServerGroups = ConfigGetStringList(ConfigInfo, "ServerGroup");
    if( ServerGroups == NULL )
    {
        ERRORMSG("Please set at least one server group.\n");
        return -202;
    }

    if( StringListIterator_Init(&i, ServerGroups) != 0 )
    {
        return -207;
    }

    while( (Type = i.Next(&i)) != NULL )
    {
        if( strcmp(Type, "UDP") == 0 )
        {
            if( Udp_Init(&i) != 0 )
            {
                ERRORMSG("Initializing UDPGroups failed.\n");
                return -218;
            }
        } else if( strcmp(Type, "TCP") == 0 )
        {
            if( Tcp_Init(&i) != 0 )
            {
                ERRORMSG("Initializing TCPGroups failed.\n");
                return -226;
            }
        } else if( strcmp(Type, "FILE") == 0 )
        {
            if( Modules_InitFromFile(&i) != 0 )
            {
                ERRORMSG("Initializing group files failed.\n");
                return -318;
            }
        } else {
            ERRORMSG("Initializing server groups failed, near %s.\n", Type);
            return -230;
        }
    }

    INFO("Server groups initialized.\n", Type);
    return 0;
}

int MMgr_Init(ConfigFileInfo *ConfigInfo)
{
    if( Filter_Init(ConfigInfo) != 0 )
    {
        return -159;
    }

    /* Hosts & Cache */
    if( Hosts_Init(ConfigInfo) != 0 )
    {
        return -165;
    }

    if( DNSCache_Init(ConfigInfo) != 0 )
    {
        return -164;
    }

    if( IpMiscSingleton_Init(ConfigInfo) != 0 )
    {
        return -176;
    }

    /* Ordinary modeles */
    if( StringChunk_Init(&Distributor, NULL) != 0 )
    {
        return -10;
    }

    if( StableBuffer_Init(&Modules) != 0 )
    {
        return -27;
    }

    if( Array_Init(&ModuleArray,
                   sizeof(ModuleInterface *),
                   0,
                   FALSE,
                   NULL
                   )
       != 0 )
    {
        return -98;
    }

    return Modules_Init(ConfigInfo);
}

int MMgr_Send(IHeader *h, int BufferLength)
{
    ModuleInterface **i;
    ModuleInterface *TheModule;

    /* Determine whether to discard the query */
    if( Filter_Out(h) )
    {
        return 0;
    }

    /* Hosts & Cache */
    if( Hosts_Get(h, BufferLength) == 0 )
    {
        return 0;
    }

    if( DNSCache_FetchFromCache(h, BufferLength) == 0 )
    {
        return 0;
    }

    /* Ordinary modeles */
    if( StringChunk_Domain_Match(&Distributor,
                                 h->Domain,
                                 &(h->HashValue),
                                 (void **)&i
                                 )
       )
    {
    } else if( Array_GetUsed(&ModuleArray) > 0 ){
        i = Array_GetBySubscript(&ModuleArray,
                                 (int)(*(uint16_t *)IHEADER_TAIL(h)) %
                                     Array_GetUsed(&ModuleArray)
                                 );
    } else {
        i = NULL;
    }

    if( i == NULL || *i == NULL )
    {
        return -190;
    }

    TheModule = *i;

    return TheModule->Send(&(TheModule->ModuleUnion), h, BufferLength);
}
