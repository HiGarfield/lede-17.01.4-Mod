#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h> /* exit() */

#ifndef NODOWNLOAD
	#ifndef WIN32
		#include <sys/types.h>
		#include <sys/stat.h>
		#ifdef DOWNLOAD_LIBCURL
			#include <curl/curl.h>
		#endif /* DOWNLOAD_LIBCURL */
	#endif /* WIN32 */
#endif /* NODOWNLOAD */

#include "common.h"
#include "utils.h"
#include "readconfig.h"
#include "logs.h"
#include "mmgr.h"
#include "udpfrontend.h"
#include "timedtask.h"
#include "domainstatistic.h"

#define VERSION__ "6.1.15"

static char		*ConfigFile;
static BOOL		DeamonMode;

static BOOL     ShowMessages = TRUE;
static BOOL     DebugOn = FALSE;

static ConfigFileInfo	ConfigInfo;

static int EnvironmentInit(void)
{
	VType	TmpTypeDescriptor;
	char	TmpStr[1024];

	/* Setting env */
	GetFileDirectory(TmpStr);
	strcat(TmpStr, PATH_SLASH_STR);

	SetProgramEnvironment("PROGRAMDIRECTORY", TmpStr);

	strncpy(TmpStr, ConfigFile, sizeof(TmpStr));
	TmpStr[sizeof(TmpStr) - 1] = '\0';

    if( GetPathPart(TmpStr) == NULL )
    {
        snprintf(TmpStr, sizeof(TmpStr), ".%s", PATH_SLASH_STR);
        TmpStr[sizeof(TmpStr) - 1] = '\0';
    }

	SetProgramEnvironment("CONFIGFILEDIRECTORY", TmpStr);

    /* Initializing configure options */
	ConfigInitInfo(&ConfigInfo);

    TmpTypeDescriptor.boolean = FALSE;
    ConfigAddOption(&ConfigInfo, "LogOn", STRATEGY_DEFAULT, TYPE_BOOLEAN, TmpTypeDescriptor);

    TmpTypeDescriptor.INT32 = 102400;
    ConfigAddOption(&ConfigInfo, "LogFileThresholdLength", STRATEGY_DEFAULT, TYPE_INT32, TmpTypeDescriptor);

	GetFileDirectory(TmpStr);
	strcat(TmpStr, PATH_SLASH_STR);
    TmpTypeDescriptor.str = TmpStr;
    ConfigAddOption(&ConfigInfo, "LogFileFolder", STRATEGY_REPLACE, TYPE_PATH, TmpTypeDescriptor);

    TmpTypeDescriptor.str = NULL;
    ConfigAddOption(&ConfigInfo, "UDPLocal", STRATEGY_APPEND_DISCARD_DEFAULT, TYPE_STRING, TmpTypeDescriptor);
    ConfigSetStringDelimiters(&ConfigInfo, "UDPLocal", ",");
    TmpTypeDescriptor.str = "127.0.0.1:53";
    ConfigSetDefaultValue(&ConfigInfo, TmpTypeDescriptor, "UDPLocal");

    TmpTypeDescriptor.str = NULL;
    ConfigAddOption(&ConfigInfo, "ServerGroup", STRATEGY_APPEND_DISCARD_DEFAULT, TYPE_STRING, TmpTypeDescriptor);
    ConfigSetStringDelimiters(&ConfigInfo, "ServerGroup", "\t ");
    TmpTypeDescriptor.str = "UDP 1.2.4.8,114.114.114.114 * on";
    ConfigSetDefaultValue(&ConfigInfo, TmpTypeDescriptor, "ServerGroup");

    ConfigAddAlias(&ConfigInfo, "ServerGroup", "GroupFile", "FILE", "");

    /* UDPGroup 1.2.4.8,114.114.114.114 * on */
    ConfigAddAlias(&ConfigInfo, "ServerGroup", "UDPGroup", "UDP", NULL);

    /* TCPGroup 1.2.4.8,114.114.114.114 example.com 192.168.50.5:8080, 192.168.50.6:8080*/
    /* TCPGroup 1.2.4.8,114.114.114.114 * no*/
    ConfigAddAlias(&ConfigInfo, "ServerGroup", "TCPGroup", "TCP", NULL);

    /* TLSGroup getdnsapi.net:853:185.49.141.38|foxZRnIh9gZpWnl+zEiKa0EJ2rdCGroMWm02gaxSc9S= example.com */
    ConfigAddAlias(&ConfigInfo, "ServerGroup", "TLSGroup", "TLS", NULL);

    TmpTypeDescriptor.str = NULL;
    ConfigAddOption(&ConfigInfo, "BlockIP", STRATEGY_APPEND, TYPE_STRING, TmpTypeDescriptor);

    TmpTypeDescriptor.boolean = FALSE;
    ConfigAddOption(&ConfigInfo, "AP", STRATEGY_DEFAULT, TYPE_BOOLEAN, TmpTypeDescriptor);

    TmpTypeDescriptor.str = NULL;
    ConfigAddOption(&ConfigInfo, "IPSubstituting", STRATEGY_APPEND, TYPE_STRING, TmpTypeDescriptor);
    ConfigSetStringDelimiters(&ConfigInfo, "IPSubstituting", "\t ,");

    TmpTypeDescriptor.boolean = FALSE;
    ConfigAddOption(&ConfigInfo, "BlockNegativeResponse", STRATEGY_DEFAULT, TYPE_BOOLEAN, TmpTypeDescriptor);

    TmpTypeDescriptor.boolean = FALSE;
    ConfigAddOption(&ConfigInfo, "DomainStatistic", STRATEGY_DEFAULT, TYPE_BOOLEAN, TmpTypeDescriptor);

	GetFileDirectory(TmpStr);
	strcat(TmpStr, PATH_SLASH_STR);
	strcat(TmpStr, "StatisticTemplate.html");
    TmpTypeDescriptor.str = TmpStr;
    ConfigAddOption(&ConfigInfo, "DomainStatisticTempletFile", STRATEGY_REPLACE, TYPE_PATH, TmpTypeDescriptor);

	TmpTypeDescriptor.str = "<!-- INSERT HERE -->";
	ConfigAddOption(&ConfigInfo, "StatisticInsertionPosition", STRATEGY_DEFAULT, TYPE_STRING, TmpTypeDescriptor);

	TmpTypeDescriptor.INT32 = 60;
	ConfigAddOption(&ConfigInfo, "StatisticUpdateInterval", STRATEGY_DEFAULT, TYPE_INT32, TmpTypeDescriptor);

    TmpTypeDescriptor.str = NULL;
    ConfigAddOption(&ConfigInfo, "Hosts", STRATEGY_APPEND, TYPE_PATH, TmpTypeDescriptor);

    TmpTypeDescriptor.INT32 = 18000;
    ConfigAddOption(&ConfigInfo, "HostsUpdateInterval", STRATEGY_DEFAULT, TYPE_INT32, TmpTypeDescriptor);

    TmpTypeDescriptor.INT32 = 30;
    ConfigAddOption(&ConfigInfo, "HostsRetryInterval", STRATEGY_DEFAULT, TYPE_INT32, TmpTypeDescriptor);

	GetFileDirectory(TmpStr);
	strcat(TmpStr, PATH_SLASH_STR);
	strcat(TmpStr, "hosts.txt");
    TmpTypeDescriptor.str = TmpStr;
    ConfigAddOption(&ConfigInfo, "HostsDownloadPath", STRATEGY_REPLACE, TYPE_PATH, TmpTypeDescriptor);

    TmpTypeDescriptor.str = NULL;
    ConfigAddOption(&ConfigInfo, "HostsScript", STRATEGY_REPLACE, TYPE_STRING, TmpTypeDescriptor);

    TmpTypeDescriptor.boolean = FALSE;
    ConfigAddOption(&ConfigInfo, "BlockIpv6WhenIpv4Exists", STRATEGY_DEFAULT, TYPE_BOOLEAN, TmpTypeDescriptor);

    TmpTypeDescriptor.str = NULL;
    ConfigAddOption(&ConfigInfo, "AppendHosts", STRATEGY_APPEND, TYPE_STRING, TmpTypeDescriptor);

	TmpTypeDescriptor.boolean = TRUE;
    ConfigAddOption(&ConfigInfo, "UseCache", STRATEGY_DEFAULT, TYPE_BOOLEAN, TmpTypeDescriptor);

    TmpTypeDescriptor.INT32 = 1048576;
    ConfigAddOption(&ConfigInfo, "CacheSize", STRATEGY_DEFAULT, TYPE_INT32, TmpTypeDescriptor);

    TmpTypeDescriptor.boolean = TRUE;
    ConfigAddOption(&ConfigInfo, "MemoryCache", STRATEGY_DEFAULT, TYPE_BOOLEAN, TmpTypeDescriptor);

	GetFileDirectory(TmpStr);
	strcat(TmpStr, PATH_SLASH_STR);
	strcat(TmpStr, "cache");
    TmpTypeDescriptor.str = TmpStr;
    ConfigAddOption(&ConfigInfo, "CacheFile", STRATEGY_REPLACE, TYPE_PATH, TmpTypeDescriptor);

    TmpTypeDescriptor.boolean = FALSE;
    ConfigAddOption(&ConfigInfo, "IgnoreTTL", STRATEGY_DEFAULT, TYPE_BOOLEAN, TmpTypeDescriptor);

    TmpTypeDescriptor.INT32 = -1;
    ConfigAddOption(&ConfigInfo, "OverrideTTL", STRATEGY_DEFAULT, TYPE_INT32, TmpTypeDescriptor);

    TmpTypeDescriptor.INT32 = 1;
    ConfigAddOption(&ConfigInfo, "MultipleTTL", STRATEGY_DEFAULT, TYPE_INT32, TmpTypeDescriptor);

    TmpTypeDescriptor.str = NULL;
    ConfigAddOption(&ConfigInfo, "CacheControl", STRATEGY_APPEND, TYPE_STRING, TmpTypeDescriptor);

	TmpTypeDescriptor.boolean = FALSE;
    ConfigAddOption(&ConfigInfo, "ReloadCache", STRATEGY_DEFAULT, TYPE_BOOLEAN, TmpTypeDescriptor);

	TmpTypeDescriptor.boolean = FALSE;
	ConfigAddOption(&ConfigInfo, "OverwriteCache", STRATEGY_DEFAULT, TYPE_BOOLEAN, TmpTypeDescriptor);

    TmpTypeDescriptor.str = NULL;
    ConfigAddOption(&ConfigInfo, "DisabledType", STRATEGY_APPEND, TYPE_STRING, TmpTypeDescriptor);

    TmpTypeDescriptor.str = NULL;
    ConfigAddOption(&ConfigInfo, "DisabledDomain", STRATEGY_APPEND, TYPE_STRING, TmpTypeDescriptor);

    TmpTypeDescriptor.str = NULL;
    ConfigAddOption(&ConfigInfo, "DisabledList", STRATEGY_APPEND, TYPE_PATH, TmpTypeDescriptor);

    TmpTypeDescriptor.str = NULL;
    ConfigAddOption(&ConfigInfo, "GoodIPList", STRATEGY_APPEND, TYPE_STRING, TmpTypeDescriptor);

    TmpTypeDescriptor.str = NULL;
    ConfigAddOption(&ConfigInfo, "GoodIPListAddIP", STRATEGY_APPEND, TYPE_STRING, TmpTypeDescriptor);

	if( ConfigOpenFile(&ConfigInfo, ConfigFile) != 0 )
    {
        printf("WARNING: Cannot load configuration file : %s. Default options will be used. Or use `-f' to specify other configure file.\n", ConfigFile);
        return 0;
    }

    ConfigRead(&ConfigInfo);
    ConfigCloseFile(&ConfigInfo);

	return 0;
}

static int DaemonInit(void)
{
#ifdef WIN32
	char		*CmdLine = GetCommandLine();
	char		*NewArguments;

	BOOL		StartUpStatus;
	STARTUPINFO	StartUpInfo;
	PROCESS_INFORMATION ProcessInfo;

	CmdLine = GoToNextNonSpace(CmdLine);

	if( CmdLine[0] != '\"' && CmdLine[1] != ':' )
    {
        /* CmdLine doesn't contain module name portion */

        char ModuleName[320];

        if( GetModuleFileName(NULL, ModuleName, sizeof(ModuleName) - 1) == 0 )
        {
            return -255;
        }

        ModuleName[sizeof(ModuleName) - 1] = '\0';

        NewArguments = SafeMalloc(strlen(ModuleName) + strlen(CmdLine) + 32);
        if( NewArguments == NULL )
        {
            return -283;
        }

        sprintf(NewArguments, "\"%s\" %s", ModuleName, CmdLine);
    } else {
        NewArguments = SafeMalloc(strlen(CmdLine) + 32);
        if( NewArguments == NULL )
        {
            return -292;
        }

        strcpy(NewArguments, CmdLine);
    }

	StartUpInfo.cb = sizeof(StartUpInfo);
	StartUpInfo.lpReserved = NULL;
	StartUpInfo.lpDesktop = NULL;
	StartUpInfo.lpTitle = NULL;
	StartUpInfo.dwFlags = STARTF_USESHOWWINDOW;
	StartUpInfo.wShowWindow = SW_HIDE;
	StartUpInfo.cbReserved2 = 0;
	StartUpInfo.lpReserved2 = NULL;

	StartUpStatus = CreateProcess(NULL,
                                  NewArguments,
                                  NULL,
                                  NULL,
                                  FALSE,
                                  CREATE_NO_WINDOW,
                                  NULL,
                                  NULL,
                                  &StartUpInfo,
                                  &ProcessInfo
                                  );

	SafeFree(NewArguments);

	if( StartUpStatus != FALSE )
	{
		printf("deamon process pid : %lu\n", ProcessInfo.dwProcessId);
		exit(0);
	} else {
		return 1;
	}
#else /* WIN32 */

    pid_t	pid;
    if( (pid = fork()) < 0 )
    {
        return 1;
    }
    else
    {
        if(pid != 0)
        {
            printf("deamon process pid : %lu\n", (unsigned long)pid);
            exit(0);
        }
        setsid();
        umask(0); /* clear file mode creation mask */
        close(0);
        close(1);
        close(2);
        return 0;
    }
#endif /* WIN32 */
}

static int GetDefaultConfigureFile(char *out, int OutLength)
{
#ifdef WIN32
	GetModulePath(out, OutLength);
	strcat(out, "\\dnsforwarder.config");
#else /* WIN32 */
	GetConfigDirectory(out);
	strcat(out, "/config");
#endif /* WIN32 */
	return 0;
}

#ifndef WIN32
static void PrepareEnvironment(void)
{
	char ConfigDirectory[2048];

	GetConfigDirectory(ConfigDirectory);

	if( mkdir(ConfigDirectory, S_IRWXU | S_IRGRP | S_IROTH) != 0 )
	{
		int		ErrorNum = GET_LAST_ERROR();
		char	ErrorMessage[320];
		ErrorMessage[0] = '\0';

		GetErrorMsg(ErrorNum, ErrorMessage, sizeof(ErrorMessage));

		printf("mkdir : %s failed : %s\n", ConfigDirectory, ErrorMessage);
	}

	printf("Please put configure file into `%s' and rename it to `config'.\n", ConfigDirectory);
}
#endif /* WIN32 */

static int ArgParse(int argc, char *argv_ori[])
{
	char **argv = argv_ori;
	++argv;
    while(*argv != NULL)
    {
    	if(strcmp("-h", *argv) == 0)
		{
			printf("DNSforwarder by several people. Version "VERSION__" . License : GPL v3.\n Time of compilation : %s %s.\n\n", __DATE__, __TIME__);
			printf("https://github.com/holmium/dnsforwarder\n\n");
			printf("Usage : %s [args].\n", strrchr(argv_ori[0], PATH_SLASH_CH) == NULL ? argv_ori[0] : strrchr(argv_ori[0], PATH_SLASH_CH) + 1);
			printf(" [args] is case sensitivity and can be zero or more (in any order) of:\n"
				  "  -f <FILE>  Use configuration <FILE> instead of the default one.\n"
				  "  -q         Quiet mode. Do not print any information.\n"
				  "  -D         Show debug messages.\n"
				  "  -d         Daemon mode. Running at background.\n"
#ifndef WIN32
				  "\n"
				  "  -p         Prepare needed environment.\n"
#endif /* WIN32 */
				  "\n"
				  "  -h         Show this help.\n"
				  "\n"
				  "Output format:\n"
				  " Date & Time [Udp|Tcp|Cache|Hosts|Refused|Blocked][Client IP][Queried type][Queried domain] : Message size\n"
				  "    Results\n"
				  );
			exit(0);
		}
        if(strcmp("-q", *argv) == 0)
        {
            ShowMessages = FALSE;
            ++argv;
            continue;
        }

        if(strcmp("-D", *argv) == 0)
        {
			DebugOn = TRUE;
            ++argv;
            continue;
        }

        if(strcmp("-d", *argv) == 0)
        {
			DeamonMode = TRUE;
            ++argv;
            continue;
        }

        if(strcmp("-f", *argv) == 0)
        {
            ConfigFile = *(++argv);
            ++argv;
            continue;
        }

#ifndef WIN32
		if( strcmp("-p", *argv) == 0 )
		{
			PrepareEnvironment();
			exit(0);

			++argv;
            continue;
		}
#endif /* WIN32 */

		printf("Unrecognisable arg `%s'. Try `-h'.\n", *argv);
        ++argv;
    }

    return 0;
}

int main(int argc, char *argv[])
{
#ifdef WIN32
    WSADATA wdata;
    HWND ThisWindow = GetConsoleWindow();
    BOOL DeamonInited = ThisWindow == NULL ?
                        TRUE :
                        !IsWindowVisible(ThisWindow);
#else /* WIN32 */
    BOOL DeamonInited = FALSE;
#endif /* WIN32 */

#ifndef NODOWNLOAD
    #ifdef WIN32
    if( WSAStartup(MAKEWORD(2, 2), &wdata) != 0 )
    {
        return -244;
    }
    #else
        #ifdef DOWNLOAD_LIBCURL
	curl_global_init(CURL_GLOBAL_ALL);
        #endif /* DOWNLOAD_LIBCURL */
    #endif /* WIN32 */
#endif /* NODOWNLOAD */

#ifdef WIN32
	SetConsoleTitle("dnsforwarder");
#endif /* WIN32 */

	ArgParse(argc, argv);

	if( ConfigFile == NULL )
	{
		ConfigFile = malloc(320);
		if( ConfigFile == NULL )
		{
			return -264;
		}

		GetDefaultConfigureFile(ConfigFile, 320);
	}

    printf("DNSforwarder mainly by holmium. Version "VERSION__" . License : GPL v3.\nTime of compilation : %s %s.\n\n", __DATE__, __TIME__);

#ifndef WIN32
    printf("Please run `dnsforwarder -p' if something goes wrong.\n\n");
#endif /* WIN32 */

    printf("Configure File : %s\n\n", ConfigFile);

    if( DeamonInited )
    {
        DeamonMode = FALSE;
    }

	if( DeamonMode )
	{
		if( DaemonInit() == 0 )
		{
			DeamonInited = TRUE;
		} else {
			printf("Daemon init failed, continuing on non-daemon mode. Restart recommended.\n");
		}
	}

	if( EnvironmentInit() != 0 )
    {
        return -498;
    }

    putchar('\n');

	if( DeamonInited )
    {
        ShowMessages = FALSE;
    }

	if( Log_Init(&ConfigInfo, ShowMessages, DebugOn) != 0 )
    {
        return -291;
    }

    INFO("New session.\n");

    if( DeamonInited )
    {
        INFO("Running on daemon mode.\n");
    }

	if( TimedTask_Init() != 0 )
    {
        return -505;
    }

    if( DomainStatistic_Init(&ConfigInfo) != 0 )
    {
        return -496;
    }

    IHeader_Init(ConfigGetBoolean(&ConfigInfo, "AP"));

    if( UdpFrontend_Init(&ConfigInfo, FALSE) != 0 )
    {
        return -311;
    }

    if( MMgr_Init(&ConfigInfo) != 0 )
    {
        return -305;
    }

    UdpFrontend_StartWork();

    ConfigFree(&ConfigInfo);

	ExitThisThread();

#ifdef WIN32
    WSACleanup();
#endif /* WIN32 */
    return 0;
}
