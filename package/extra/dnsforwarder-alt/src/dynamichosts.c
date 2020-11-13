#include <string.h>
#include "dynamichosts.h"
#include "common.h"
#include "downloader.h"
#include "readline.h"
#include "goodiplist.h"
#include "timedtask.h"
#include "rwlock.h"
#include "logs.h"

static const char   *File = NULL;
static RWLock		HostsLock;
static volatile HostsContainer	*MainDynamicContainer = NULL;

static int DynamicHosts_Load(void)
{
	FILE			*fp;
	char			Buffer[320];
	ReadLineStatus	Status;

	HostsContainer *TempContainer;

	fp = fopen(File, "r");
	if( fp == NULL )
	{
		return -1;
	}

	TempContainer = (HostsContainer *)SafeMalloc(sizeof(HostsContainer));
	if( TempContainer == NULL )
	{
		fclose(fp);
		return -1;
	}

	if( HostsContainer_Init(TempContainer) != 0 )
	{
		fclose(fp);

		SafeFree(TempContainer);
		return -1;
	}

	while( TRUE )
	{
		Status = ReadLine(fp, Buffer, sizeof(Buffer));
		if( Status == READ_FAILED_OR_END )
        {
            break;
        }

		if( Status == READ_TRUNCATED )
		{
			ERRORMSG("Hosts is too long : %s\n", Buffer);
			ReadLine_GoToNextLine(fp);
			continue;
		}

        TempContainer->Load(TempContainer, Buffer);
	}

	RWLock_WrLock(HostsLock);
	if( MainDynamicContainer != NULL )
	{
	    MainDynamicContainer->Free((HostsContainer *)MainDynamicContainer);
		SafeFree((void *)MainDynamicContainer);
	}
	MainDynamicContainer = TempContainer;

	RWLock_UnWLock(HostsLock);

	INFO("Loading hosts completed.\n");

	fclose(fp);
	return 0;
}

/* Arguments for updating  */
static int          HostsRetryInterval;
static char         *Script = NULL; /* malloced */
static const char	**HostsURLs = NULL; /* malloced */

static void GetHostsFromInternet_Failed(int ErrorCode, const char *URL, const char *File1)
{
	ERRORMSG("Getting Hosts %s failed. Waiting %d second(s) to try again.\n",
             URL,
             HostsRetryInterval
             );
}

static void GetHostsFromInternet_Succeed(const char *URL, const char *File1)
{
	INFO("Hosts %s saved.\n", URL);
}

static void GetHostsFromInternet_Thread(void *Unused1, void *Unused2)
{
	int			DownloadState;

    if( HostsURLs[1] == NULL )
    {
        INFO("Getting hosts from %s ...\n", HostsURLs[0]);
    } else {
        INFO("Getting hosts from various places ...\n");
    }

    DownloadState = GetFromInternet_MultiFiles(HostsURLs,
                                               File,
                                               HostsRetryInterval,
                                               -1,
                                               GetHostsFromInternet_Failed,
                                               GetHostsFromInternet_Succeed
                                               );

    if( DownloadState == 0 )
    {
        INFO("Hosts saved at %s.\n", File);

        if( Script != NULL )
        {
            INFO("Running hosts script \"%s\"...\n", Script);

            if( Execute(Script) < 0 )
            {
                ERRORMSG("Hosts script running failed.\n");
            }
        }

        DynamicHosts_Load();
    } else {
        ERRORMSG("Getting hosts file(s) failed.\n");
    }
}

int DynamicHosts_Init(ConfigFileInfo *ConfigInfo)
{
	StringList  *Hosts;
	int          UpdateInterval;
	const char  *RawScript;

	Hosts = ConfigGetStringList(ConfigInfo, "Hosts");
	if( Hosts == NULL )
	{
		File = NULL;
		return -151;
	}

	Hosts->TrimAll(Hosts, "\"\t ");

    HostsURLs = Hosts->ToCharPtrArray(Hosts);
	UpdateInterval = ConfigGetInt32(ConfigInfo, "HostsUpdateInterval");
	HostsRetryInterval = ConfigGetInt32(ConfigInfo, "HostsRetryInterval");

	RawScript = ConfigGetRawString(ConfigInfo, "HostsScript");
	if( RawScript != NULL )
    {
        static const int SIZE_OF_PATH_BUFFER = 1024;

        Script = SafeMalloc(SIZE_OF_PATH_BUFFER);
        if( Script == NULL )
        {
            ERRORMSG("No enough memory.\n");
            return -160;
        }

        strcpy(Script, RawScript);

        if( ExpandPath(Script, SIZE_OF_PATH_BUFFER) != 0 )
        {
            ERRORMSG("Path is too long : .\n", Script);
            return -170;
        }
    } else {
        Script = NULL;
    }

	RWLock_Init(HostsLock);

	File = ConfigGetRawString(ConfigInfo, "HostsDownloadPath");

	if( HostsRetryInterval < 0 )
	{
		ERRORMSG("`HostsRetryInterval' is too small (< 0).\n");
		File = NULL;
		return -167;
	}

	INFO("Local hosts file : \"%s\"\n", File);

	if( FileIsReadable(File) )
	{
		INFO("Loading the existing hosts file ...\n");
		DynamicHosts_Load();
	} else {
		INFO("Hosts file is unreadable, this may cause some failures.\n");
	}

	if( UpdateInterval <= 0 )
    {
        TimedTask_Add(FALSE,
                      TRUE,
                      0,
                      (TaskFunc)GetHostsFromInternet_Thread,
                      NULL,
                      NULL,
                      TRUE);
    } else {
        TimedTask_Add(TRUE,
                      TRUE,
                      UpdateInterval * 1000,
                      (TaskFunc)GetHostsFromInternet_Thread,
                      NULL,
                      NULL,
                      TRUE);
    }

	return 0;
}

int DynamicHosts_GetCName(const char *Domain, char *Buffer)
{
    int ret;

    if( MainDynamicContainer == NULL )
    {
        return -198;
    }

    RWLock_RdLock(HostsLock);

    ret = HostsUtils_GetCName(Domain,
                              Buffer,
                              (HostsContainer *)MainDynamicContainer
                              );

    RWLock_UnRLock(HostsLock);

    return ret;
}

BOOL DynamicHosts_TypeExisting(const char *Domain, HostsRecordType Type)
{
    BOOL ret;

    if( MainDynamicContainer == NULL )
    {
        return FALSE;
    }

    RWLock_RdLock(HostsLock);

    ret = HostsUtils_TypeExisting((HostsContainer *)MainDynamicContainer,
                                  Domain,
                                  Type
                                  );

    RWLock_UnRLock(HostsLock);

    return ret;
}

HostsUtilsTryResult DynamicHosts_Try(IHeader *Header, int BufferLength)
{
    HostsUtilsTryResult ret;

    if( MainDynamicContainer == NULL )
    {
        return HOSTSUTILS_TRY_NONE;
    }

    RWLock_RdLock(HostsLock);

    ret = HostsUtils_Try(Header,
                         BufferLength,
                         (HostsContainer *)MainDynamicContainer
                         );

    RWLock_UnRLock(HostsLock);

    return ret;
}
