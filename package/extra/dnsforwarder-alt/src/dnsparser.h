#ifndef _DNS_PARSER_H_
#define _DNS_PARSER_H_

#include <limits.h>
#include "common.h"
#include "dnsrelated.h"

#define GET_16_BIT_U_INT(ptr)	(ntohs(*(int16_t *)(ptr)))
#define GET_32_BIT_U_INT(ptr)		(ntohl(*(int32_t *)(ptr)))
#define GET_8_BIT_U_INT(ptr)		(*(unsigned char*)(ptr))

#define DNS_HEADER_LENGTH	12

/* Handle DNS header*/
#define DNSGetTCPLength(dns_over_tcp_ptr)	GET_16_BIT_U_INT(dns_over_tcp_ptr)

#define DNSGetQueryIdentifier(dns_body)		GET_16_BIT_U_INT((char *)(dns_body))

#define DNSGetFlags(dns_body)				GET_16_BIT_U_INT((char *)(dns_body) + 2)

#define DNSGetQuestionCount(dns_body)		GET_16_BIT_U_INT((char *)(dns_body) + 4)

#define DNSGetAnswerCount(dns_body)			GET_16_BIT_U_INT((char *)(dns_body) + 6)

#define DNSGetNameServerCount(dns_body)		GET_16_BIT_U_INT((char *)(dns_body) + 8)

#define DNSGetAdditionalCount(dns_body)		GET_16_BIT_U_INT((char *)(dns_body) + 10)

#define DNSJumpHeader(dns_body)				((char *)(dns_body) + DNS_HEADER_LENGTH)

#define DNSGetTTL(ans_start_ptr)				GET_32_BIT_U_INT(DNSJumpOverName(ans_start_ptr) + 4)

#define DNSGetResourceDataLength(ans_start_ptr)	GET_16_BIT_U_INT(DNSJumpOverName(ans_start_ptr) + 8)

#define DNSGetResourceDataPos(ans_start_ptr)	(DNSJumpOverName((char *)(ans_start_ptr)) + 10)

/* Common */
char *DNSJumpOverName(char *NameStart);

int DNSGetHostName(const char *DNSBody, int DNSBodyLength, const char *NameStart, char *buffer, int BufferLength);

int DNSGetHostNameLength(const char *DNSBody, int DNSBodyLength, const char *NameStart);

#define DNSGetRecordType(rec_start_ptr)		((rec_start_ptr) == NULL ? DNS_TYPE_UNKNOWN : GET_16_BIT_U_INT(DNSJumpOverName(rec_start_ptr)))

#define DNSIsLabelPointerStart(num)			(((num) & 0xC0) == 0xC0)

#define DNSLabelGetPointer(rec_start_ptr)	((rec_start_ptr) == NULL ? 0 : (int)((unsigned char *)(rec_start_ptr))[1] + (int)(((unsigned char *)(rec_start_ptr))[0] - 192) * 256)

#define DNSGetRecordClass(rec_start_ptr)	((rec_start_ptr) == NULL ? DNS_CLASS_UNKNOWN : GET_16_BIT_U_INT(DNSJumpOverName(rec_start_ptr) + 2))

#ifdef HOST_BIG_ENDIAN
/* DNSMessageFlags, at 2-byte offset of a DNS header, is 2 bytes length.
 * https://tools.ietf.org/html/rfc6895
 */
typedef struct _DNSMessageProperties{
	uint16_t	Direction	:	1; /* query (0), or response (1) */

	/* Type:
	 * 0	a standard query (QUERY).
	 * 1	an inverse query (IQUERY).
	 * 2	a server status request (STATUS).
	 * 3-15	reserved for future use  */
	uint16_t Type			:	4;

	uint16_t	AuthoritativeAnswer:1;

	uint16_t	TrunCation		:	1;

	uint16_t	RecursionDesired:	1; /* 0 no, 1 yes */

	uint16_t	RecursionAvailable:	1; /* 0 no, 1 yes */

	uint16_t	Unused			:	1;

	uint16_t AuthenticData	:	1;

	uint16_t CheckingDisabled:	1;

	/* ResponseCode:
	 * 0	No error condition.
	 * 1	Format error - The name server was unable to interpret the query.
	 * 2	Server failure - The name server was unable to process this query due to a problem with the name server.
	 * 3	Name Error - Meaningful only for responses from an authoritative name server, this code signifies that the domain name referenced in the query does not exist.
	 * 4	Not Implemented - The name server does not support the requested kind of query.
	 * 5	Refused - The name server refuses to perform the specified operation for policy reasons. For example, a name server may not wish to provide the information to the particular requester, or a name server may not wish to perform a particular operation (e.g., zone transfer) for particular data.
	 * 6-15	Reserved for future use. */
	uint16_t	ResponseCode	:	4;

}DNSFlags;
#else
typedef struct _DNSMessageProperties{
	uint16_t	RecursionDesired:	1; /* 0 no, 1 yes */

	uint16_t	TrunCation		:	1;

	uint16_t	AuthoritativeAnswer:1;

	/* Type:
	 * 0	a standard query (QUERY).
	 * 1	an inverse query (IQUERY).
	 * 2	a server status request (STATUS).
	 * 3-15	reserved for future use  */
	uint16_t Type			:	4;

	uint16_t	Direction	:	1; /* query (0), or response (1) */


	/* ResponseCode:
	 * 0	No error condition.
	 * 1	Format error - The name server was unable to interpret the query.
	 * 2	Server failure - The name server was unable to process this query due to a problem with the name server.
	 * 3	Name Error - Meaningful only for responses from an authoritative name server, this code signifies that the domain name referenced in the query does not exist.
	 * 4	Not Implemented - The name server does not support the requested kind of query.
	 * 5	Refused - The name server refuses to perform the specified operation for policy reasons. For example, a name server may not wish to provide the information to the particular requester, or a name server may not wish to perform a particular operation (e.g., zone transfer) for particular data.
	 * 6-15	Reserved for future use. */
	uint16_t	ResponseCode	:	4;

	uint16_t CheckingDisabled:	1;

	uint16_t AuthenticData	:	1;

	uint16_t	Unused			:	1;

	uint16_t	RecursionAvailable:	1; /* 0 no, 1 yes */

}DNSFlags;
#endif

typedef struct _DNSHeader{
	uint16_t		Identifier;
	DNSFlags		Flags;
	uint16_t		QuestionCount;
	uint16_t		AnswerCount;
	uint16_t		NameServerCount;
	uint16_t		AdditionalCount;
}DNSHeader;

#define DNSGetHeader(dns_body_ptr)	((DNSHeader *)(dns_body_ptr))

/* Convert a DNS message to text */
char *GetAllAnswers(char *DNSBody, int DNSBodyLength, char *Buffer, int BufferLength);

int DNSCopyLable(const char *DNSBody, char *here, const char *src);

/**
  New Implementation
*/

typedef enum _DnsDirection{
    DNS_DIRECTION_QUERY = 0,
    DNS_DIRECTION_RESPONSE = 1,
} DnsDirection;

typedef enum _DnsOperation{
    DNS_OPERATION_QUERY = 0,
    DNS_OPERATION_IQUERY = 1,
    DNS_OPERATION_STATUS = 2,
} DnsOperation;

typedef enum _ResponseCode{
    RESPONSE_CODE_NO_ERROR = 0,
    RESPONSE_CODE_FORMAT_ERROR = 1,
    RESPONSE_CODE_SERVER_FAILURE = 2,
    RESPONSE_CODE_NAME_ERROR = 3,
    RESPONSE_CODE_NOT_IMPLEMENTED = 4,
    RESPONSE_CODE_REFUSED = 5,
} ResponseCode;

typedef enum _DnsRecordPurpose{
    /* Their values are not important */
    DNS_RECORD_PURPOSE_UNKNOWN = 0,
    DNS_RECORD_PURPOSE_QUESTION,
    DNS_RECORD_PURPOSE_ANSWER,
    DNS_RECORD_PURPOSE_NAME_SERVER,
    DNS_RECORD_PURPOSE_ADDITIONAL,
} DnsRecordPurpose;

typedef struct _DnsSimpleParser DnsSimpleParser;

struct _DnsSimpleParser{
    /* public, but don't modify outside, unless you know what are you doing */
    char *RawDns;
    int  RawDnsLength;

    struct {
        /* private */
        const DNSFlags    *Flags;

        /* public */
        DnsDirection    (*Direction)(DnsSimpleParser *p);
        DnsOperation    (*Operation)(DnsSimpleParser *p);
        BOOL            (*IsAuthoritative)(DnsSimpleParser *p);
        BOOL            (*Truncated)(DnsSimpleParser *p);
        BOOL            (*RecursionDesired)(DnsSimpleParser *p);
        BOOL            (*RecursionAvailable)(DnsSimpleParser *p);
        ResponseCode    (*ResponseCode)(DnsSimpleParser *p);
    } _Flags;

    /* public */
    uint16_t    (*QueryIdentifier)(DnsSimpleParser *p);
    int         (*QuestionCount)(DnsSimpleParser *p);
    int         (*AnswerCount)(DnsSimpleParser *p);
    int         (*NameServerCount)(DnsSimpleParser *p);
    int         (*AdditionalCount)(DnsSimpleParser *p);

    BOOL        (*HasType)(DnsSimpleParser *p,
                           DnsRecordPurpose Purpose,
                           DNSRecordClass Klass,
                           DNSRecordType Type
                           );
};

int DnsSimpleParser_Init(DnsSimpleParser *p,
                         char *RawDns,
                         int Length,
                         BOOL IsTcp);

/**
  Iterator
*/
typedef struct _DnsSimpleParserIterator DnsSimpleParserIterator;

struct _DnsSimpleParserIterator{
    /* public, but don't modify outside */
    DnsSimpleParser *Parser;

    /* private */
    char        *CurrentPosition;
    int         RecordPosition; /* Starts at 1 */

    int         QuestionFirst; /* Starts at 1; 0 means no such record */
    int         QuestionLast;

    int         AnswerFirst; /* Starts at 1; 0 means no such record */
    int         AnswerLast;

    int         NameServerFirst; /* Starts at 1; 0 means no such record */
    int         NameServerLast;

    int         AdditionalFirst; /* Starts at 1; 0 means no such record */
    int         AdditionalLast;

    int         AllRecordCount;

    /* public, but don't modify outside */
    /* Current record informations */
    DnsRecordPurpose    Purpose;
    DNSRecordType       Type;
    DNSRecordClass      Klass;
    int                 DataLength;

    char *(*Next)(DnsSimpleParserIterator *i);
    void (*GotoAnswers)(DnsSimpleParserIterator *i);
    int (*GetName)(DnsSimpleParserIterator *i,
                   char *Buffer, /* Could be NULL */
                   int BufferLength
                   );
    int (*GetNameLength)(DnsSimpleParserIterator *i);
    char *(*RowData)(DnsSimpleParserIterator *i);
    int (*TextifyData)(DnsSimpleParserIterator *i,
                       const char *Format, /* "%t:%v\n" */
                       char *Buffer,
                       int BufferLength
                       );

    uint32_t (*GetTTL)(DnsSimpleParserIterator *i);
};

int DnsSimpleParserIterator_Init(DnsSimpleParserIterator *i,
                                 DnsSimpleParser *p
                                 );

#endif /* _DNS_PARSER_H_ */
