#include <stdio.h>
#include "dnsgenerator.h"
#include "utils.h"
#include "dnsparser.h"

/* Other Codes */
char *DNSLabelizedName(__inout char *Origin, __in size_t OriginSpaceLength){
	unsigned char *LabelLength = (unsigned char *)Origin;

	if( *Origin == '\0' )
		return Origin + 1;

	if( OriginSpaceLength < strlen(Origin) + 2 )
		return NULL;

	memmove(Origin + 1, Origin, strlen(Origin) + 1);

	*LabelLength = 0;
	while(1){
		++Origin;

		if(*Origin == 0) break;
		if(*Origin != '.') ++(*LabelLength);
		else {
			LabelLength = (unsigned char *)Origin;
			*LabelLength = 0;
		}
	}

	return Origin + 1;
}

/* Compressed length returned */
int DNSCompress(__inout char *DNSBody, __in int DNSBodyLength)
{
    DnsSimpleParser p;
    DnsSimpleParserIterator i;

    char *LastName;
    char *CurrentName;

    if( DnsSimpleParser_Init(&p, DNSBody, DNSBodyLength, FALSE) != 0 )
    {
        return -1;
    }

    if( DnsSimpleParserIterator_Init(&i, &p) != 0 )
    {
        return -2;
    }

    LastName = i.Next(&i);
    if( LastName == NULL || i.Purpose != DNS_RECORD_PURPOSE_QUESTION )
    {
        return -3;
    }

    i.GotoAnswers(&i);
    while( (CurrentName = i.Next(&i)) != NULL &&
           i.Purpose == DNS_RECORD_PURPOSE_ANSWER
           )
    {
        int LengthDifference;

        if( i.Klass != DNS_CLASS_IN )
        {
            continue;
        }

        /* Compressing stage */
        LengthDifference = i.GetName(&i, NULL, 0) -
                           2 /* 2 is the pointer length */
                           ;

        if( LengthDifference < 0 )
        {
            return -4;
        }

        memmove(CurrentName,
                CurrentName + LengthDifference,
                p.RawDnsLength - ((CurrentName + LengthDifference) - p.RawDns)
                );

        DNSLabelMakePointer(CurrentName, LastName - p.RawDns);

        /* Yeah, changed that */
        p.RawDnsLength -= LengthDifference;

        /* Whether to set `LastName' */
        if( i.Type == DNS_TYPE_CNAME )
        {
            LastName = i.RowData(&i);
        }
    }

    return p.RawDnsLength;
}

/**
  New Implementation
*/

static DnsRecordPurpose DnsGenerator_CurrentPurpose(DnsGenerator *g)
{
    switch( (char *)(g->NumberOfRecords) - g->Buffer )
    {
    case 4:
        return DNS_RECORD_PURPOSE_QUESTION;
        break;

    case 6:
        return DNS_RECORD_PURPOSE_ANSWER;
        break;

    case 8:
        return DNS_RECORD_PURPOSE_NAME_SERVER;
        break;

    case 10:
        return DNS_RECORD_PURPOSE_ADDITIONAL;
        break;

    default:
        return DNS_RECORD_PURPOSE_UNKNOWN;
        break;
    }
}

static DnsRecordPurpose DnsGenerator_NextPurpose(DnsGenerator *g)
{
    if( DnsGenerator_CurrentPurpose(g) == DNS_RECORD_PURPOSE_UNKNOWN )
    {
        return DNS_RECORD_PURPOSE_UNKNOWN;
    }

    g->NumberOfRecords = (char *)(g->NumberOfRecords) + 2;

    return DnsGenerator_CurrentPurpose(g);
}

#define LEFT_LENGTH(g_ptr)  ((g_ptr)->BufferLength - ((g_ptr)->Itr - (g_ptr)->Buffer))
#define LABEL_LENGTH(name)  (*(name) == '\0' ? 1 : strlen(name) + 2)

static int DnsGenerator_NamePart(DnsGenerator *g, const char *Name)
{
	if( Name == NULL || *Name == '\0' )
    {
        /* Root domain */
        if( LEFT_LENGTH(g) < 1 )
        {
            return -1;
        }

        *(g->Itr) = 0;
        g->Itr += 1;

    } else {

        int LabelLen = LABEL_LENGTH(Name);

        if( LEFT_LENGTH(g) < LabelLen )
        {
            return -2;
        }

        strcpy(g->Itr, Name);

        if( DNSLabelizedName(g->Itr, LabelLen) == NULL )
        {
            return -3;
        }

        g->Itr += LabelLen;
    }

    return 0;
}

static int DnsGenerator_16Uint(DnsGenerator *g, int Value)
{
	if( LEFT_LENGTH(g) < 2 )
    {
        return -1;
    }

    SET_16_BIT_U_INT(g->Itr, Value);

    g->Itr += 2;

    return 0;
}

static int DnsGenerator_32Uint(DnsGenerator *g, int Value)
{
	if( LEFT_LENGTH(g) < 4 )
    {
        return -1;
    }

    SET_32_BIT_U_INT(g->Itr, Value);

    g->Itr += 4;

    return 0;
}

static int DnsGenerator_IPv4(DnsGenerator *g, const char *ip)
{
	if( LEFT_LENGTH(g) < 4 )
    {
        return -1;
    }

    IPv4AddressToNum(ip, g->Itr);

    g->Itr += 4;

    return 0;
}

static int DnsGenerator_IPv6(DnsGenerator *g, const char *ip)
{
	if( LEFT_LENGTH(g) < 16 )
    {
        return -1;
    }

    IPv6AddressToNum(ip, g->Itr);

    g->Itr += 16;

    return 0;
}

/* Record generations */
static int DnsGenerator_Question(DnsGenerator *g,
                                 const char *Name,
                                 DNSRecordType Type,
                                 DNSRecordClass Klass
                                 )
{
    if( DnsGenerator_CurrentPurpose(g) != DNS_RECORD_PURPOSE_QUESTION )
    {
        return 1;
    }

	if( DnsGenerator_NamePart(g, Name) != 0 )
    {
        return -1;
    }

	if( DnsGenerator_16Uint(g, Type) != 0 )
    {
        return -2;
    }

	if( DnsGenerator_16Uint(g, Klass) != 0 )
    {
        return -3;
    }

    SET_16_BIT_U_INT(g->NumberOfRecords,
                     GET_16_BIT_U_INT(g->NumberOfRecords) + 1
                     );

	return 0;
}

static int DnsGenerator_CName(DnsGenerator *g,
                              const char *Name,
                              const char *CName,
                              int Ttl
                              )
{
    DnsRecordPurpose p = DnsGenerator_CurrentPurpose(g);

    if( p != DNS_RECORD_PURPOSE_ANSWER &&
        p != DNS_RECORD_PURPOSE_NAME_SERVER &&
        p != DNS_RECORD_PURPOSE_ADDITIONAL
        )
    {
        return 1;
    }

	if( DnsGenerator_NamePart(g, Name) != 0 )
    {
        return -1;
    }

	if( DnsGenerator_16Uint(g, DNS_TYPE_CNAME) != 0 )
    {
        return -2;
    }

	if( DnsGenerator_16Uint(g, DNS_CLASS_IN) != 0 )
    {
        return -3;
    }

	if( DnsGenerator_32Uint(g, Ttl) != 0 )
    {
        return -4;
    }

	if( DnsGenerator_16Uint(g, LABEL_LENGTH(CName)) != 0 )
    {
        return -5;
    }

	if( DnsGenerator_NamePart(g, CName) != 0 )
    {
        return -6;
    }

    SET_16_BIT_U_INT(g->NumberOfRecords,
                     GET_16_BIT_U_INT(g->NumberOfRecords) + 1
                     );

    return 0;
}

static int DnsGenerator_A(DnsGenerator *g,
                          const char *Name,
                          const char *ip,
                          int Ttl
                          )
{
    DnsRecordPurpose p = DnsGenerator_CurrentPurpose(g);

    if( p != DNS_RECORD_PURPOSE_ANSWER &&
        p != DNS_RECORD_PURPOSE_NAME_SERVER &&
        p != DNS_RECORD_PURPOSE_ADDITIONAL
        )
    {
        return 1;
    }

	if( DnsGenerator_NamePart(g, Name) != 0 )
    {
        return -1;
    }

	if( DnsGenerator_16Uint(g, DNS_TYPE_A) != 0 )
    {
        return -2;
    }

	if( DnsGenerator_16Uint(g, DNS_CLASS_IN) != 0 )
    {
        return -3;
    }

	if( DnsGenerator_32Uint(g, Ttl) != 0 )
    {
        return -4;
    }

	if( DnsGenerator_16Uint(g, 4) != 0 )
    {
        return -5;
    }

	if( DnsGenerator_IPv4(g, ip) != 0 )
    {
        return -6;
    }

    SET_16_BIT_U_INT(g->NumberOfRecords,
                     GET_16_BIT_U_INT(g->NumberOfRecords) + 1
                     );

    return 0;
}

static int DnsGenerator_AAAA(DnsGenerator *g,
                             const char *Name,
                             const char *ip,
                             int Ttl
                             )
{
    DnsRecordPurpose p = DnsGenerator_CurrentPurpose(g);

    if( p != DNS_RECORD_PURPOSE_ANSWER &&
        p != DNS_RECORD_PURPOSE_NAME_SERVER &&
        p != DNS_RECORD_PURPOSE_ADDITIONAL
        )
    {
        return 1;
    }

	if( DnsGenerator_NamePart(g, Name) != 0 )
    {
        return -1;
    }

	if( DnsGenerator_16Uint(g, DNS_TYPE_AAAA) != 0 )
    {
        return -2;
    }

	if( DnsGenerator_16Uint(g, DNS_CLASS_IN) != 0 )
    {
        return -3;
    }

	if( DnsGenerator_32Uint(g, Ttl) != 0 )
    {
        return -4;
    }

	if( DnsGenerator_16Uint(g, 16) != 0 )
    {
        return -5;
    }

	if( DnsGenerator_IPv6(g, ip) != 0 )
    {
        return -6;
    }

    SET_16_BIT_U_INT(g->NumberOfRecords,
                     GET_16_BIT_U_INT(g->NumberOfRecords) + 1
                     );

    return 0;
}

static int DnsGenerator_EDns(DnsGenerator *g, int UdpPayloadSize)
{
    DnsRecordPurpose p = DnsGenerator_CurrentPurpose(g);

    if( p != DNS_RECORD_PURPOSE_ADDITIONAL )
    {
        return 1;
    }

	if( DnsGenerator_NamePart(g, NULL) != 0 )
    {
        return -1;
    }

	if( DnsGenerator_16Uint(g, DNS_TYPE_OPT) != 0 )
    {
        return -2;
    }

	if( DnsGenerator_16Uint(g, UdpPayloadSize) != 0 )
    {
        return -3;
    }

	if( DnsGenerator_32Uint(g, 0) != 0 )
    {
        return -4;
    }

	if( DnsGenerator_16Uint(g, 0) != 0 )
    {
        return -5;
    }

    SET_16_BIT_U_INT(g->NumberOfRecords,
                     GET_16_BIT_U_INT(g->NumberOfRecords) + 1
                     );

    return 0;
}

static int DnsGenerator_RawData(DnsGenerator *g,
                                const char *Name,
                                DNSRecordType Type,
                                DNSRecordClass Klass,
                                const char *Data,
                                int DataLength,
                                int Ttl
                                )
{
    DnsRecordPurpose p = DnsGenerator_CurrentPurpose(g);

    if( p != DNS_RECORD_PURPOSE_ANSWER &&
        p != DNS_RECORD_PURPOSE_NAME_SERVER &&
        p != DNS_RECORD_PURPOSE_ADDITIONAL
        )
    {
        return 1;
    }

    if( Data == NULL || DataLength <= 0 )
    {
        return 2;
    }

	if( DnsGenerator_NamePart(g, Name) != 0 )
    {
        return -1;
    }

	if( DnsGenerator_16Uint(g, Type) != 0 )
    {
        return -2;
    }

	if( DnsGenerator_16Uint(g, Klass) != 0 )
    {
        return -3;
    }

	if( DnsGenerator_32Uint(g, Ttl) != 0 )
    {
        return -4;
    }

	if( DnsGenerator_16Uint(g, DataLength) != 0 )
    {
        return -5;
    }

    memcpy(g->Itr, Data, DataLength);
    g->Itr += DataLength;

    SET_16_BIT_U_INT(g->NumberOfRecords,
                     GET_16_BIT_U_INT(g->NumberOfRecords) + 1
                     );

    return 0;
}

static void DnsGenerator_CopyHeader(DnsGenerator *g,
                                   const char *Source,
                                   BOOL IncludeRecordCounts
                                   )
{
    memcpy(g->Buffer, Source, IncludeRecordCounts ? DNS_HEADER_LENGTH : 4);
}

static void DnsGenerator_CopyIdentifier(DnsGenerator *g, uint16_t Value)
{
    *(uint16_t *)(g->Buffer) = Value;
}

static int DnsGenerator_CopyNamePart(DnsGenerator *g,
                                     DnsSimpleParserIterator *i
                                     )
{
    if( LEFT_LENGTH(g) < DNSCopyLable(i->Parser->RawDns,
                                      NULL,
                                      i->CurrentPosition)
      )
    {
        return -1;
    }

    g->Itr += DNSCopyLable(i->Parser->RawDns, g->Itr, i->CurrentPosition);

    return 0;
}

static int DnsGenerator_CopyCName(DnsGenerator *g, DnsSimpleParserIterator *i)
{
    int CNameLabelLength;

    DnsRecordPurpose p = DnsGenerator_CurrentPurpose(g);

    if( p != DNS_RECORD_PURPOSE_ANSWER &&
        p != DNS_RECORD_PURPOSE_NAME_SERVER &&
        p != DNS_RECORD_PURPOSE_ADDITIONAL
        )
    {
        return 1;
    }

    if( i->Type != DNS_TYPE_CNAME || i->Klass != DNS_CLASS_IN )
    {
        return 2;
    }

    if( DnsGenerator_CopyNamePart(g, i) != 0 )
    {
        return -1;
    }

    if( DnsGenerator_16Uint(g, i->Type) != 0 )
    {
        return -2;
    }

    if( DnsGenerator_16Uint(g, i->Klass) != 0 )
    {
        return -3;
    }

    if( DnsGenerator_32Uint(g, i->GetTTL(i)) != 0 )
    {
        return -4;
    }

    CNameLabelLength = DNSCopyLable(i->Parser->RawDns,
                                    NULL,
                                    i->RowData(i)
                                    );

    if( DnsGenerator_16Uint(g, CNameLabelLength) != 0 )
    {
        return -5;
    }

    if( LEFT_LENGTH(g) < CNameLabelLength )
    {
        return -6;
    }

    g->Itr += DNSCopyLable(i->Parser->RawDns, g->Itr, i->RowData(i));

    SET_16_BIT_U_INT(g->NumberOfRecords,
                     GET_16_BIT_U_INT(g->NumberOfRecords) + 1
                     );

    return 0;
}

static int DnsGenerator_CopyA(DnsGenerator *g, DnsSimpleParserIterator *i)
{
    DnsRecordPurpose p = DnsGenerator_CurrentPurpose(g);

    if( p != DNS_RECORD_PURPOSE_ANSWER &&
        p != DNS_RECORD_PURPOSE_NAME_SERVER &&
        p != DNS_RECORD_PURPOSE_ADDITIONAL
        )
    {
        return 1;
    }

    if( i->Type != DNS_TYPE_A || i->Klass != DNS_CLASS_IN )
    {
        return 2;
    }

    if( DnsGenerator_CopyNamePart(g, i) != 0 )
    {
        return -1;
    }

    if( DnsGenerator_16Uint(g, i->Type) != 0 )
    {
        return -2;
    }

    if( DnsGenerator_16Uint(g, i->Klass) != 0 )
    {
        return -3;
    }

    if( DnsGenerator_32Uint(g, i->GetTTL(i)) != 0 )
    {
        return -4;
    }

    if( DnsGenerator_16Uint(g, 4) != 0 )
    {
        return -5;
    }

    if( LEFT_LENGTH(g) < 4 )
    {
        return -6;
    }

    memcpy(g->Itr, i->RowData(i), 4);

    g->Itr += 4;

    SET_16_BIT_U_INT(g->NumberOfRecords,
                     GET_16_BIT_U_INT(g->NumberOfRecords) + 1
                     );

    return 0;
}

static int DnsGenerator_CopyAAAA(DnsGenerator *g, DnsSimpleParserIterator *i)
{
    DnsRecordPurpose p = DnsGenerator_CurrentPurpose(g);

    if( p != DNS_RECORD_PURPOSE_ANSWER &&
        p != DNS_RECORD_PURPOSE_NAME_SERVER &&
        p != DNS_RECORD_PURPOSE_ADDITIONAL
        )
    {
        return 1;
    }

    if( i->Type != DNS_TYPE_AAAA || i->Klass != DNS_CLASS_IN )
    {
        return 2;
    }

    if( DnsGenerator_CopyNamePart(g, i) != 0 )
    {
        return -1;
    }

    if( DnsGenerator_16Uint(g, i->Type) != 0 )
    {
        return -2;
    }

    if( DnsGenerator_16Uint(g, i->Klass) != 0 )
    {
        return -3;
    }

    if( DnsGenerator_32Uint(g, i->GetTTL(i)) != 0 )
    {
        return -4;
    }

    if( DnsGenerator_16Uint(g, 16) != 0 )
    {
        return -5;
    }

    if( LEFT_LENGTH(g) < 16 )
    {
        return -6;
    }

    memcpy(g->Itr, i->RowData(i), 16);

    g->Itr += 16;

    SET_16_BIT_U_INT(g->NumberOfRecords,
                     GET_16_BIT_U_INT(g->NumberOfRecords) + 1
                     );

    return 0;
}

static int DnsGenerator_Length(DnsGenerator *g)
{
    return g->Itr - g->Buffer;
}

static int StripedLength(const char *Origin, int OriginLength)
{
    DnsSimpleParser p;
    DnsSimpleParserIterator i;

    const char *CurrentPosition;

    if( DnsSimpleParser_Init(&p, (char *)Origin, OriginLength, FALSE) != 0 )
    {
        return -1;
    }

    if( DnsSimpleParserIterator_Init(&i, &p) != 0 )
    {
        return -2;
    }

    while( (CurrentPosition = i.Next(&i)) != NULL &&
           (i.Purpose == DNS_RECORD_PURPOSE_QUESTION ||
                i.Purpose == DNS_RECORD_PURPOSE_ANSWER
                )
           );

    if( CurrentPosition == NULL )
    {
        return OriginLength;
    } else {
        return CurrentPosition - p.RawDns;
    }
}

int DnsGenerator_Init(DnsGenerator *g,
                      char *Buffer,
                      int BufferLength,
                      const char *CopyFrom,
                      int SourceLength,

                      /* Whether to remove every record except question and
                         answer records. Used when `CopyFrom' is not `NULL'.
                      */
                      BOOL Strip
                      )
{
    if( g == NULL || Buffer == NULL )
    {
        return -1;
    }

    g->Buffer = Buffer;
    g->BufferLength = BufferLength;
    g->Header = (DNSHeader *)(g->Buffer);

    if( CopyFrom != NULL && SourceLength > 0 )
    {
        int FourCounts[4] = {
            DNSGetQuestionCount(CopyFrom),
            DNSGetAnswerCount(CopyFrom),
            Strip == TRUE ? 0 : DNSGetNameServerCount(CopyFrom),
            Strip == TRUE ? 0 : DNSGetAdditionalCount(CopyFrom)
        };

        int i;

        if( Strip == TRUE )
        {
            SourceLength = StripedLength(CopyFrom, SourceLength);
            if( SourceLength < 0 )
            {
                return -2;
            }
        }

        memmove(g->Buffer, CopyFrom, SourceLength);
        g->Itr = g->Buffer + SourceLength;

        for( i = 3; i >= 0; --i )
        {
            if( FourCounts[i] > 0 )
            {
                break;
            }
        }

        if( i < 0 )
        {
            g->NumberOfRecords = g->Buffer + 4;
        } else {
            g->NumberOfRecords = g->Buffer + 4 + 2 * i;
        }

        if( Strip == TRUE )
        {
            DNSSetNameServerCount(g->Buffer, 0);
            DNSSetAdditionalCount(g->Buffer, 0);
        }

    } else {
        g->Itr = g->Buffer + DNS_HEADER_LENGTH;
        g->NumberOfRecords = g->Buffer + 4;

        DNSSetQuestionCount(g->Buffer, 0);
        DNSSetAnswerCount(g->Buffer, 0);
        DNSSetNameServerCount(g->Buffer, 0);
        DNSSetAdditionalCount(g->Buffer, 0);
    }

    g->Length = DnsGenerator_Length;
    g->NextPurpose = DnsGenerator_NextPurpose;

    g->CopyHeader = DnsGenerator_CopyHeader;
    g->CopyIdentifier = DnsGenerator_CopyIdentifier;
    g->CopyCName = DnsGenerator_CopyCName;
    g->CopyA = DnsGenerator_CopyA;
    g->CopyAAAA = DnsGenerator_CopyAAAA;

    g->Question = DnsGenerator_Question;
    g->CName = DnsGenerator_CName;
    g->A = DnsGenerator_A;
    g->AAAA = DnsGenerator_AAAA;
    g->EDns = DnsGenerator_EDns;
    g->RawData = DnsGenerator_RawData;

    return 0;
}
