#ifndef IHEADER_H_INCLUDED
#define IHEADER_H_INCLUDED

#include "dnsrelated.h"
#include "utils.h"

typedef struct _IHeader IHeader;

struct _IHeader{
	int32_t _Pad; /* Must be 0 */

	Address_Type    BackAddress;
	SOCKET          SendBackSocket;

	char	        Domain[256];
	int             HashValue;
	DNSRecordType   Type;

	BOOL            ReturnHeader;
	BOOL		    EDNSEnabled;

	int             EntityLength;

	char            Agent[ROUND_UP(LENGTH_OF_IPV6_ADDRESS_ASCII + 1,
                                   sizeof(void *)
                                   )
                          ];

    void            *TcpLengthPadding;
};

#define IHEADER_TAIL(ptr)   (void *)((IHeader *)(ptr) + 1)

#define IHEADER_CONTAINING_HEADER(ptr)  (((IHeader *)(ptr))->_Pad == 0)

int IHeader_Init(BOOL _ap);

void IHeader_Reset(IHeader *h);

int IHeader_Fill(IHeader *h,
                 BOOL ReturnHeader,
                 char *DnsEntity,
                 int EntityLength,
                 struct sockaddr *BackAddress,
                 SOCKET SendBackSocket,
                 sa_family_t Family,
                 const char *Agent
                 );

int IHeader_AddFakeEdns(IHeader *h, int BufferLength);

BOOL IHeader_Blocked(IHeader *h /* Entity followed */);

int IHeader_SendBack(IHeader *h /* Entity followed */);

int IHeader_SendBackRefusedMessage(IHeader *h);

#endif // IHEADER_H_INCLUDED
