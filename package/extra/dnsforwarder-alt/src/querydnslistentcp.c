#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "querydnslistentcp.h"
#include "dnsrelated.h"
#include "dnsparser.h"
#include "dnsgenerator.h"
#include "common.h"
#include "utils.h"
#include "stringlist.h"
#include "filter.h"
#include "addresslist.h"

/* Variables */
static BOOL			Inited = FALSE;

static SOCKET		TCPIncomeSocket;

static SOCKET		TCPOutcomeSocket;
static Address_Type	TCPOutcomeAddress;

static int			RefusingResponseCode = 0;

static QueryContext	Context;

/* Functions */
int QueryDNSListenTCPInit(ConfigFileInfo *ConfigInfo)
{
	RefusingResponseCode = ConfigGetInt32(ConfigInfo, "RefusingResponseCode");

	TCPIncomeSocket = InternalInterface_OpenTCP(MAIN_WORKING_ADDRESS, INTERNAL_INTERFACE_TCP_INCOME, MAIN_WORKING_PORT);
	if( TCPIncomeSocket == INVALID_SOCKET )
	{
		ShowFatalMessage("Opening TCP socket failed.", GET_LAST_ERROR());
		return -1;
	} else {
		INFO("TCP socket %s:%d created.\n", MAIN_WORKING_ADDRESS, MAIN_WORKING_PORT);
	}

	InternalInterface_GetAddress(INTERNAL_INTERFACE_TCP_INCOME, NULL);

	TCPOutcomeSocket = InternalInterface_TryBindLocal(10000, &TCPOutcomeAddress);

	InternalInterface_InitQueryContext(&Context);

	Inited = TRUE;

	return 0;
}

static int Query(char *Content, int ContentLength, int BufferLength, SOCKET ThisSocket)
{
	int State;
	char *RequestEntity = Content + sizeof(ControlHeader);
	ControlHeader	*Header = (ControlHeader *)Content;
	int RequestLength = ContentLength - sizeof(ControlHeader);
	uint16_t TCPLength = htons(RequestLength);

	Header->RequestingDomain[0] = '\0';

	if( DNSGetHostName(RequestEntity,
						RequestLength,
						DNSJumpHeader(RequestEntity),
						Header->RequestingDomain,
						sizeof(Header->RequestingDomain)
						)
		< 0 )
	{
		return -1;
	}

	StrToLower(Header->RequestingDomain);

	Header->RequestingType =
		(DNSRecordType)DNSGetRecordType(DNSJumpHeader(RequestEntity));

	Header->RequestingDomainHashValue = ELFHash(Header->RequestingDomain, 0);

	State = QueryBase(Content, ContentLength, BufferLength, TCPOutcomeSocket);

	switch( State )
	{
		case QUERY_RESULT_SUCCESS:
			InternalInterface_QueryContextAddTCP(&Context, Header, ThisSocket);
			return 0;
			break;

		case QUERY_RESULT_DISABLE:
			((DNSHeader *)(RequestEntity))->Flags.Direction = 1;
			((DNSHeader *)(RequestEntity))->Flags.RecursionAvailable = 1;
			((DNSHeader *)(RequestEntity))->Flags.ResponseCode = RefusingResponseCode;
			send(ThisSocket, (const char *)&TCPLength, 2, 0);
			send(ThisSocket, RequestEntity, RequestLength, 0);
			return -1;
			break;

		case QUERY_RESULT_ERROR:
			return -1;
			break;

		default: /* Cache */
			{
				uint16_t NewTCPLength = htons(State);

				send(ThisSocket, (const char *)&NewTCPLength, 2, 0);
				send(ThisSocket, RequestEntity, State, 0);
				return 0;
			}
			break;
	}
}

typedef struct _SocketInfo {
	SOCKET	Socket;
	char	Address[LENGTH_OF_IPV6_ADDRESS_ASCII + 1];
	time_t	TimeAdd;
} SocketInfo;

static Bst	si;

static int SocketInfoCompare(const SocketInfo *_1, const SocketInfo *_2)
{
	return (int)(_1->Socket) - (int)(_2->Socket);
}

static int InitSocketInfo(void)
{
	return Bst_Init(&si, NULL, sizeof(SocketInfo), (int (*)(const void *, const void *))SocketInfoCompare);
}

static SOCKET SocketInfoMatch(fd_set *ReadySet, fd_set *ReadSet, char *ClientAddress, int32_t *Number)
{
	int32_t Start = -1;
	SocketInfo *Info;

	time_t	Now = time(NULL);

	Info = Bst_Enum(&si, &Start);
	while( Info != NULL )
	{
		if( FD_ISSET(Info->Socket, ReadySet) )
		{
			Info->TimeAdd = Now;
			strcpy(ClientAddress, Info->Address);
			if( Number != NULL )
			{
				*Number = Start;
			}
			return Info->Socket;
		}

		if( Now - Info->TimeAdd > 2 )
		{
			CLOSE_SOCKET(Info->Socket);
			FD_CLR(Info->Socket, ReadSet);
			Bst_Delete_ByNumber(&si, Start);
		}

		Info = Bst_Enum(&si, &Start);
	}

	return INVALID_SOCKET;
}

static int SocketInfoAdd(SOCKET Socket, const char *Address)
{
	SocketInfo New;

	New.Socket = Socket;
	strcpy(New.Address, Address);
	New.TimeAdd = time(NULL);

	return Bst_Add(&si, &New);
}

static BOOL SocketInfoSwep(fd_set *ReadSet)
{
	int32_t Start = -1;
	SocketInfo *Info;

	time_t	Now = time(NULL);

	Info = Bst_Enum(&si, &Start);
	while( Info != NULL )
	{
		if( Now - Info->TimeAdd > 2 )
		{
			CLOSE_SOCKET(Info->Socket);
			FD_CLR(Info->Socket, ReadSet);

			INFO("TCP connection to client %s closed.\n", Info->Address);

			Bst_Delete_ByNumber(&si, Start);
		}

		Info = Bst_Enum(&si, &Start);
	}

	return Bst_IsEmpty(&si);
}

static void SendBack(char *Result, int Length)
{
	ControlHeader	*Header = (ControlHeader *)Result;
	char			*RequestEntity = Result + sizeof(ControlHeader);
	uint16_t		Identifier = *(uint16_t *)RequestEntity;
	int32_t			Number;
	QueryContextEntry	*Entry;
	uint16_t	 	EntityLength = Length - sizeof(ControlHeader);
	uint16_t	 	EntityLength_n = htons(EntityLength);

	Number = InternalInterface_QueryContextFind(&Context, Identifier, Header->RequestingDomainHashValue);
	if( Number < 0 )
	{
		return;
	}

	Entry = Bst_GetDataByNumber(&Context, Number);

	send(Entry->Context.Socket, (const char *)&EntityLength_n, 2, 0);
	send(Entry->Context.Socket, RequestEntity, EntityLength, 0);

	InternalInterface_QueryContextRemoveByNumber(&Context, Number);

}

static int QueryDNSListenTCP(void)
{
	int		MaxFd = TCPIncomeSocket > TCPOutcomeSocket ? TCPIncomeSocket : TCPOutcomeSocket;

	static fd_set	ReadSet, ReadySet;

	int		NumberOfQueryBeforeSwep = 0;

	static const struct timeval	LongTime = {3600, 0};
	static const struct timeval	ShortTime = {10, 0};

	struct timeval	TimeLimit = LongTime;

	static char		RequestEntity[2048];
	ControlHeader	*Header = (ControlHeader *)RequestEntity;

	InternalInterface_InitControlHeader(Header);

	memcpy(&(Header->BackAddress), &TCPOutcomeAddress, sizeof(Address_Type));
	Header->NeededHeader = TRUE;

	InitSocketInfo();

	FD_ZERO(&ReadSet);
	FD_ZERO(&ReadySet);

	FD_SET(TCPIncomeSocket, &ReadSet);
	FD_SET(TCPOutcomeSocket, &ReadSet);

	while( TRUE )
	{
		ReadySet = ReadSet;

		switch( select(MaxFd + 1, &ReadySet, NULL, NULL, &TimeLimit) )
		{
			case SOCKET_ERROR:
				{
					int LastError = GET_LAST_ERROR();
					ERRORMSG("SOCKET_ERROR Reached, 1.\n");
					if( FatalErrorDecideding(LastError) != 0 )
					{
						ERRORMSG("\n\n\n\n\n\n\n\n\n\n");
						ERRORMSG(" !!!!! Something bad happend, please restart this program. %d\n", LastError);
						while( TRUE ) SLEEP(100000);
					}
				}
				break;

			case 0:
				if( SocketInfoSwep(&ReadSet) == TRUE )
				{
					Bst_Reset(&Context);
					TimeLimit = LongTime;
				} else {
					InternalInterface_QueryContextSwep(&Context, 10, NULL);
					TimeLimit = ShortTime;
				}

				NumberOfQueryBeforeSwep = 0;
				break;

			default:
				TimeLimit = ShortTime;

				++NumberOfQueryBeforeSwep;
				if( NumberOfQueryBeforeSwep > 1024 )
				{
					InternalInterface_QueryContextSwep(&Context, 2, NULL);
					SocketInfoSwep(&ReadSet);
					NumberOfQueryBeforeSwep = 0;
				}

				if( FD_ISSET(TCPIncomeSocket, &ReadySet) )
				{
					SOCKET			NewSocket;
					Address_Type	Address;
					socklen_t		AddrLen;

					char	AddressString[LENGTH_OF_IPV6_ADDRESS_ASCII + 1];

					if( MAIN_FAMILY == AF_INET )
					{
						AddrLen = sizeof(struct sockaddr);
						NewSocket = accept(TCPIncomeSocket,
										  (struct sockaddr *)&(Address.Addr.Addr4),
										  (socklen_t *)&AddrLen
										  );
					} else {
						AddrLen = sizeof(struct sockaddr_in6);
						NewSocket = accept(TCPIncomeSocket,
										  (struct sockaddr *)&(Address.Addr.Addr6),
										  (socklen_t *)&AddrLen
										  );
					}

					if( NewSocket != INVALID_SOCKET )
					{
						FD_SET(NewSocket, &ReadSet);

						if( MAIN_FAMILY == AF_INET )
						{
							strcpy(AddressString, inet_ntoa(Address.Addr.Addr4.sin_addr));
						} else {
							IPv6AddressToAsc(&(Address.Addr.Addr6.sin6_addr), AddressString);
						}

						if( NewSocket > MaxFd )
						{
							MaxFd = NewSocket;
						}

						SocketInfoAdd(NewSocket, AddressString);
						INFO("TCP connection to client %s established.\n", AddressString);
					}

				} else if( FD_ISSET(TCPOutcomeSocket, &ReadySet) )
				{
					static char Result[2048];
					int	State;

					State = recvfrom(TCPOutcomeSocket, Result, sizeof(Result), 0, NULL, NULL);
					if( State > 0 )
					{
						SendBack(Result, State);
					}
				} else {
					SOCKET	Socket;
					int		State;

					int32_t	Number;

					Socket = SocketInfoMatch(&ReadySet, &ReadSet, Header->Agent, &Number);

					if( Socket != INVALID_SOCKET )
					{
						uint16_t TCPLength;

						if( recv(Socket, (char *)&TCPLength, 2, MSG_NOSIGNAL) < 2 )
						{
							Bst_Delete_ByNumber(&si, Number);
							FD_CLR(Socket, &ReadSet);
							CLOSE_SOCKET(Socket);
							break;
						}

						TCPLength = ntohs(TCPLength);

						State = recv(Socket, RequestEntity + sizeof(ControlHeader), sizeof(RequestEntity) - sizeof(ControlHeader), MSG_NOSIGNAL);
						if( State < 1 )
						{
							Bst_Delete_ByNumber(&si, Number);
							FD_CLR(Socket, &ReadSet);
							CLOSE_SOCKET(Socket);
							INFO("Lost TCP connection to client %s.\n", Header->Agent);
							break;
						}

						Query(RequestEntity, State + sizeof(ControlHeader), sizeof(RequestEntity), Socket);
					}
				}
			break;
		}
	}

	return 0;
}

void QueryDNSListenTCPStart(void)
{
	ThreadHandle t;

	CREATE_THREAD(QueryDNSListenTCP, NULL, t);
	DETACH_THREAD(t);
}
