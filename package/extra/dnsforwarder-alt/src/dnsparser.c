#include "dnsparser.h"
#include "dnsgenerator.h"
#include "utils.h"

char *DNSJumpOverName(char *NameStart)
{
	return NameStart + DNSGetHostName(NULL, INT_MAX, NameStart, NULL, 0);
}

/* Labels length returned */
int DNSGetHostName(const char *DNSBody, int DNSBodyLength, const char *NameStart, char *buffer, int BufferLength)
{
	char *BufferItr = buffer;
	const char *NameItr = NameStart;
	int LabelsLength = 0;
	BOOL Redirected = FALSE;
	int LabelCount = GET_8_BIT_U_INT(NameItr); /* The amount of characters of the next label */
	while( LabelCount != 0 )
	{
		if( DNSIsLabelPointerStart(LabelCount) )
		{
			int LabelPointer = 0;
			if( Redirected == FALSE )
            {
                LabelsLength += 2;
                Redirected = TRUE;
            }
			if( buffer == NULL )
			{
				break;
			}
			LabelPointer = DNSLabelGetPointer(NameItr);
			if( LabelPointer > DNSBodyLength )
			{
				return -1;
			}
			NameItr = DNSBody + DNSLabelGetPointer(NameItr);
		} else {
			if( DNSBody != NULL &&
                NameItr + LabelCount > DNSBody + DNSBodyLength
                )
			{
				return -1;
			}

			if( buffer != NULL )
			{
				if( BufferItr + LabelCount + 1 - buffer <= BufferLength )
				{
					memcpy(BufferItr, NameItr + 1, LabelCount);
				} else {
					if( BufferItr == buffer )
					{
						if( BufferLength > 0 )
						{
							*BufferItr = '\0';
						}
					} else {
						*(BufferItr - 1) = '\0';
					}
					return -1;
				}
			}

			if( Redirected == FALSE )
			{
				LabelsLength += (LabelCount + 1);
			}
			NameItr += (1 + LabelCount);
			if( buffer != NULL )
			{
				BufferItr += LabelCount;
				*BufferItr = '.';
				++BufferItr;
			}
		}

		LabelCount = GET_8_BIT_U_INT(NameItr);
	}

	if( buffer != NULL )
	{
		if( BufferItr == buffer )
		{
			if( BufferLength > 0 )
			{
				*BufferItr = '\0';
			} else {
				return -1;
			}
		} else {
			*(BufferItr - 1) = '\0';
		}
	}

	if( Redirected == FALSE )
	{
		++LabelsLength;
	}

	return LabelsLength;
}

/* including terminated-zero */
int DNSGetHostNameLength(const char *DNSBody, int DNSBodyLength, const char *NameStart)
{
	const char *NameItr = NameStart;
	int NameLength = 0;
	int LabelCount = GET_8_BIT_U_INT(NameItr); /* The amount of characters of the next label */
	while( LabelCount != 0 )
	{
		if( DNSIsLabelPointerStart(LabelCount) )
		{
			if( DNSLabelGetPointer(NameItr) > DNSBodyLength )
			{
				return INT_MAX; /* Error detected */
			}
			NameItr = DNSBody + DNSLabelGetPointer(NameItr);
		} else {
			if( NameItr + LabelCount > DNSBody + DNSBodyLength )
			{
				return INT_MAX; /* Error detected */
			}
			NameLength += (LabelCount + 1);
			NameItr += (1 + LabelCount);
		}

		LabelCount = GET_8_BIT_U_INT(NameItr);

		if( NameLength > DNSBodyLength )
        {
            return INT_MAX; /* Error detected */
        }
	}

	if( NameLength == 0 )
	{
		return 1;
	} else {
		return NameLength;
	}
}

char *GetAllAnswers(char *DNSBody, int DNSBodyLength, char *Buffer, int BufferLength)
{
    DnsSimpleParser p;
    DnsSimpleParserIterator i;
    int ANACount;

    static const char *Tail = "   And       More ...\n";
    char *BufferItr = Buffer;
    int BufferLeft = BufferLength - strlen(Tail);

    if( BufferLeft <= 0 )
    {
        return NULL;
    }

    if( DnsSimpleParser_Init(&p, DNSBody, DNSBodyLength, FALSE) != 0 )
    {
        return NULL;
    }

    if( DnsSimpleParserIterator_Init(&i, &p) != 0 )
    {
        return NULL;
    }

    ANACount = p.AnswerCount(&p) + p.NameServerCount(&p) + p.AdditionalCount(&p);

    if( ANACount == 0 )
    {
        strcpy(BufferItr, "   Nothing.\n");
    }

    i.GotoAnswers(&i);

    while( i.Next(&i) != NULL &&
           i.Purpose != DNS_RECORD_PURPOSE_QUESTION &&
           i.Purpose != DNS_RECORD_PURPOSE_UNKNOWN
         )
    {
        if( i.TextifyData(&i, "   %t:%v\n", BufferItr, BufferLeft) == 0 )
        {
            sprintf(BufferItr, "   And %d More ...\n", ANACount);

            break;
        } else {
            int StageLength = strlen(BufferItr);

            BufferItr += StageLength;
            BufferLeft -= StageLength;

            --ANACount;
        }
    }

    return Buffer;
}

/* Full label length returned, including terminated-zero */
int DNSCopyLable(const char *DNSBody, char *here, const char *src)
{
    int FullLength = 0;

	while( TRUE )
	{
		if( DNSIsLabelPointerStart(GET_8_BIT_U_INT(src)) )
		{
			src = DNSBody + DNSLabelGetPointer(src);
		} else {
		    ++FullLength;

		    if( here != NULL )
            {
                *here = *src;
                ++here;
            }

			if( *src == '\0' )
			{
				break;
			}

			++src;
		}
	}

	return FullLength;
}

/**
  New Implementation
*/

/* Converted to host byte order */
static uint16_t DnsSimpleParser_QueryIdentifier(DnsSimpleParser *p)
{
    return DNSGetQueryIdentifier(p->RawDns);
}

static DnsDirection DnsSimpleParser_Flags_Direction(DnsSimpleParser *p)
{
    return (DnsDirection)(p->_Flags.Flags->Direction);
}

static DnsOperation DnsSimpleParser_Flags_Operation(DnsSimpleParser *p)
{
    return (DnsOperation)(p->_Flags.Flags->Type);
}

static BOOL DnsSimpleParser_Flags_IsAuthoritative(DnsSimpleParser *p)
{
    return !!(p->_Flags.Flags->AuthoritativeAnswer);
}

static BOOL DnsSimpleParser_Flags_Truncated(DnsSimpleParser *p)
{
    return !!(p->_Flags.Flags->TrunCation);
}

static BOOL DnsSimpleParser_Flags_RecursionDesired(DnsSimpleParser *p)
{
    return !!(p->_Flags.Flags->RecursionDesired);
}

static BOOL DnsSimpleParser_Flags_RecursionAvailable(DnsSimpleParser *p)
{
    return !!(p->_Flags.Flags->RecursionAvailable);
}

static ResponseCode DnsSimpleParser_Flags_ResponseCode(DnsSimpleParser *p)
{
    return (ResponseCode)(p->_Flags.Flags->ResponseCode);
}

static int DnsSimpleParser_QuestionCount(DnsSimpleParser *p)
{
    return DNSGetQuestionCount(p->RawDns);
}

static int DnsSimpleParser_AnswerCount(DnsSimpleParser *p)
{
    return DNSGetAnswerCount(p->RawDns);
}

static int DnsSimpleParser_NameServerCount(DnsSimpleParser *p)
{
    return DNSGetNameServerCount(p->RawDns);
}

static int DnsSimpleParser_AdditionalCount(DnsSimpleParser *p)
{
    return DNSGetAdditionalCount(p->RawDns);
}

static BOOL DnsSimpleParser_HasType(DnsSimpleParser *p,
                                    DnsRecordPurpose Purpose,
                                    DNSRecordClass Klass,
                                    DNSRecordType Type
                                    )
{
    DnsSimpleParserIterator i;

    if( DnsSimpleParserIterator_Init(&i, p) != 0 )
    {
        return FALSE;
    }

    while( i.Next(&i) != NULL )
    {
        if( (Purpose == DNS_RECORD_PURPOSE_UNKNOWN || i.Purpose == Purpose) &&
            (Klass == DNS_CLASS_UNKNOWN || i.Klass == Klass) &&
             i.Type == Type
             )
        {
            return TRUE;
        }
    }

    return FALSE;
}

int DnsSimpleParser_Init(DnsSimpleParser *p,
                         char *RawDns,
                         int Length,
                         BOOL IsTcp)
{
    if( RawDns == NULL || Length < DNS_HEADER_LENGTH )
    {
        return -1;
    }

    if( IsTcp )
    {
        p->RawDns = RawDns + 2;
        p->RawDnsLength = Length - 2;
    } else {
        p->RawDns = RawDns;
        p->RawDnsLength = Length;
    }

    p->_Flags.Flags = (DNSFlags *)(p->RawDns + 2);

    p->QueryIdentifier = DnsSimpleParser_QueryIdentifier;

    p->_Flags.Direction = DnsSimpleParser_Flags_Direction;
    p->_Flags.Operation = DnsSimpleParser_Flags_Operation;
    p->_Flags.IsAuthoritative = DnsSimpleParser_Flags_IsAuthoritative;
    p->_Flags.Truncated = DnsSimpleParser_Flags_Truncated;
    p->_Flags.RecursionDesired = DnsSimpleParser_Flags_RecursionDesired;
    p->_Flags.RecursionAvailable = DnsSimpleParser_Flags_RecursionAvailable;
    p->_Flags.ResponseCode = DnsSimpleParser_Flags_ResponseCode;

    p->QuestionCount = DnsSimpleParser_QuestionCount;
    p->AnswerCount = DnsSimpleParser_AnswerCount;
    p->NameServerCount = DnsSimpleParser_NameServerCount;
    p->AdditionalCount = DnsSimpleParser_AdditionalCount;
    p->HasType = DnsSimpleParser_HasType;

    return 0;
}

/**
  Iterator
*/
static DnsRecordPurpose DnsSimpleParserIterator_DeterminePurpose(
                                                    DnsSimpleParserIterator *i,
                                                    int RecordPosition)
{
    if( i->QuestionFirst != 0 &&
        RecordPosition >= i->QuestionFirst &&
        RecordPosition <= i->QuestionLast
      )
    {
        return DNS_RECORD_PURPOSE_QUESTION;
    }

    if( i->AnswerFirst != 0 &&
        RecordPosition >= i->AnswerFirst &&
        RecordPosition <= i->AnswerLast
      )
    {
        return DNS_RECORD_PURPOSE_ANSWER;
    }

    if( i->NameServerFirst != 0 &&
        RecordPosition >= i->NameServerFirst &&
        RecordPosition <= i->NameServerLast
      )
    {
        return DNS_RECORD_PURPOSE_NAME_SERVER;
    }

    if( i->AdditionalFirst != 0 &&
        RecordPosition >= i->AdditionalFirst &&
        RecordPosition <= i->AdditionalLast
      )
    {
        return DNS_RECORD_PURPOSE_ADDITIONAL;
    }

    return DNS_RECORD_PURPOSE_UNKNOWN;
}

static char *DnsSimpleParserIterator_Next(DnsSimpleParserIterator *i)
{
    if( i->CurrentPosition == NULL )
    {
        i->CurrentPosition = i->Parser->RawDns + DNS_HEADER_LENGTH;
        i->RecordPosition = 1;
    } else if( i->RecordPosition < i->AllRecordCount ){
        /* The record length excluding its labeled name at the beginning. */
        int ExLength = i->Purpose == DNS_RECORD_PURPOSE_QUESTION ?
                       /* For a question record, there are only 4 bytes */
                       4 :
                       /* For an other types of record, there are many things */
                       10 + i->DataLength;

        /* The length of all labels in the beginning of current record
           plus `ExLength'
         */

        int FullLength = DNSGetHostName(NULL,
                                            INT_MAX,
                                            i->CurrentPosition,
                                            NULL,
                                            0)
                         + ExLength;

        if( FullLength < ExLength )
        {
            return NULL;
        }

        i->CurrentPosition += FullLength;

        i->RecordPosition += 1;
    } else {
        i->CurrentPosition = NULL;
        i->RecordPosition = 0;
        return NULL;
    }

    if( (i->RecordPosition > i->AllRecordCount) ||
        (i->CurrentPosition - i->Parser->RawDns > i->Parser->RawDnsLength)
      )
    {
        i->CurrentPosition = NULL;
        i->RecordPosition = 0;
        return NULL;
    }

    /* Update record informations */
    i->Purpose =  DnsSimpleParserIterator_DeterminePurpose(i, i->RecordPosition);
    i->Type = DNSGetRecordType(i->CurrentPosition);
    i->Klass = DNSGetRecordClass(i->CurrentPosition);

    if( i->Purpose != DNS_RECORD_PURPOSE_UNKNOWN &&
        i->Type != DNS_TYPE_UNKNOWN &&
        i->Klass != DNS_CLASS_UNKNOWN
      )
    {
        if( i->Purpose != DNS_RECORD_PURPOSE_QUESTION )
        {
            i->DataLength = DNSGetResourceDataLength(i->CurrentPosition);
        }

        return i->CurrentPosition;
    } else {
        i->CurrentPosition = NULL;
        i->RecordPosition = 0;
        return NULL;
    }
}

static void DnsSimpleParserIterator_GotoAnswers(DnsSimpleParserIterator *i)
{
    i->CurrentPosition = NULL;

    if( i->QuestionFirst > 0 )
    {
        while( DnsSimpleParserIterator_Next(i) != NULL )
        {
            if( i->RecordPosition == i->QuestionLast )
            {
                break;
            }
        }
    }
}

static int DnsSimpleParserIterator_GetName(DnsSimpleParserIterator *i,
                                       char *Buffer, /* Could be NULL */
                                       int BufferLength
                                       )
{
    return DNSGetHostName(i->Parser->RawDns,
                          i->Parser->RawDnsLength,
                          i->CurrentPosition,
                          Buffer,
                          BufferLength
                          );
}

static int DnsSimpleParserIterator_GetNameLength(DnsSimpleParserIterator *i)
{
    return DNSGetHostNameLength(i->Parser->RawDns,
                                i->Parser->RawDnsLength,
                                i->CurrentPosition
                                );
}

static char *DnsSimpleParserIterator_RowData(DnsSimpleParserIterator *i)
{
    if( i->Purpose != DNS_RECORD_PURPOSE_QUESTION )
    {
        return DNSGetResourceDataPos(i->CurrentPosition);
    } else {
        return NULL;
    }
}

/* Number of items generated returned */
static int DnsSimpleParserIterator_ParseIPv4(DnsSimpleParserIterator *i,
                                          const char *Data,
                                          int DataLength,
                                          const char *Format, /* "%t:%v\n" */
                                          char *Buffer,
                                          int BufferLength,
                                          const char *Text,
                                          int *AcutalDataLength
                                          )
{
    char Example[LENGTH_OF_IPV4_ADDRESS_ASCII];

    if( DataLength < 4 || strlen(Format) + 1 > BufferLength )
    {
        return 0;
    }

    if( Text == NULL )
    {
        Text = "";
    }

    strcpy(Buffer, Format);

    if( ReplaceStr_WithLengthChecking(Buffer,
                                      "%t",
                                      Text,
                                      BufferLength
                                      )
       == NULL )
    {
        *Buffer = '\0';
        return 0;
    }

    IPv4AddressToAsc(Data, Example);

    if( ReplaceStr_WithLengthChecking(Buffer,
                                      "%v",
                                      Example,
                                      BufferLength
                                      )
       == NULL )
    {
        *Buffer = '\0';
        return 0;
    }

    if( AcutalDataLength != NULL )
    {
        *AcutalDataLength = 4;
    }

    return 1;
}

static int DnsSimpleParserIterator_ParseA(DnsSimpleParserIterator *i,
                                          const char *Data,
                                          int DataLength,
                                          const char *Format, /* "%t:%v\n" */
                                          char *Buffer,
                                          int BufferLength
                                          )
{
    return DnsSimpleParserIterator_ParseIPv4(i,
                                             Data,
                                             DataLength,
                                             Format,
                                             Buffer,
                                             BufferLength,
                                             "IPv4 Address",
                                             NULL
                                             );
}

static int DnsSimpleParserIterator_ParseIPv6(DnsSimpleParserIterator *i,
                                          const char *Data,
                                          int DataLength,
                                          const char *Format, /* "%t:%v\n" */
                                          char *Buffer,
                                          int BufferLength,
                                          const char *Text,
                                          int *AcutalDataLength
                                          )
{
    char Example[LENGTH_OF_IPV6_ADDRESS_ASCII];

    if( DataLength < 16 || strlen(Format) + 1 > BufferLength )
    {
        return 0;
    }

    if( Text == NULL )
    {
        Text = "";
    }

    strcpy(Buffer, Format);

    if( ReplaceStr_WithLengthChecking(Buffer,
                                      "%t",
                                      Text,
                                      BufferLength
                                      )
       == NULL )
    {
        *Buffer = '\0';
        return 0;
    }

    IPv6AddressToAsc(Data, Example);

    if( ReplaceStr_WithLengthChecking(Buffer,
                                      "%v",
                                      Example,
                                      BufferLength
                                      )
       == NULL )
    {
        *Buffer = '\0';
        return 0;
    }

    if( AcutalDataLength != NULL )
    {
        *AcutalDataLength = 16;
    }

    return 1;
}

static int DnsSimpleParserIterator_ParseAAAA(DnsSimpleParserIterator *i,
                                          const char *Data,
                                          int DataLength,
                                          const char *Format, /* "%t:%v\n" */
                                          char *Buffer,
                                          int BufferLength
                                          )
{
    return DnsSimpleParserIterator_ParseIPv6(i,
                                             Data,
                                             DataLength,
                                             Format,
                                             Buffer,
                                             BufferLength,
                                             "IPv6 Address",
                                             NULL
                                             );
}

static int DnsSimpleParserIterator_ParseLabeledName(DnsSimpleParserIterator *i,
                                                   const char *Data,
                                                   int DataLength,
                                             const char *Format, /* "%t:%v\n" */
                                                   char *Buffer,
                                                   int BufferLength,
                                                   const char *Text,
                                                   int *AcutalDataLength
                                                   )
{
    /* Static max length assumed to be 127+1 */
    char Example[128];
    char *Resulting;

    int HostNameLength; /* Including terminated-zero */
    int LabelLength;

    if( strlen(Format) + 1 > BufferLength )
    {
        return 0;
    }

    if( Text == NULL )
    {
        Text = "";
    }

    strcpy(Buffer, Format);

    if( ReplaceStr_WithLengthChecking(Buffer,
                                      "%t",
                                      Text,
                                      BufferLength
                                      )
       == NULL )
    {
        *Buffer = '\0';
        return 0;
    }

    HostNameLength = DNSGetHostNameLength(i->Parser->RawDns,
                                          i->Parser->RawDnsLength,
                                          Data
                                          );

    if( HostNameLength == INT_MAX )
    {
        *Buffer = '\0';
        return 0;
    }

    if( HostNameLength > sizeof(Example) )
    {
        Resulting = SafeMalloc(HostNameLength);
    } else {
        Resulting = Example;
    }

    LabelLength = DNSGetHostName(i->Parser->RawDns,
                                 i->Parser->RawDnsLength,
                                 Data,
                                 Resulting,
                                 HostNameLength
                                 );

    if( LabelLength < 0 )
    {
        if( Resulting != Example )
        {
            SafeFree(Resulting);
        }

        *Buffer = '\0';
        return 0;
    }

    if( ReplaceStr_WithLengthChecking(Buffer,
                                      "%v",
                                      Resulting,
                                      BufferLength
                                      )
       == NULL )
    {
        if( Resulting != Example )
        {
            SafeFree(Resulting);
        }

        *Buffer = '\0';
        return 0;
    }

    if( Resulting != Example )
    {
        SafeFree(Resulting);
    }

    if( AcutalDataLength != NULL )
    {
        *AcutalDataLength = LabelLength;
    }

    return 1;
}

static int DnsSimpleParserIterator_ParseCName(DnsSimpleParserIterator *i,
                                              const char *Data,
                                              int DataLength,
                                             const char *Format, /* "%t:%v\n" */
                                              char *Buffer,
                                              int BufferLength
                                              )
{
    return DnsSimpleParserIterator_ParseLabeledName(i,
                                                    Data,
                                                    DataLength,
                                                    Format,
                                                    Buffer,
                                                    BufferLength,
                                                    "Canonical Name",
                                                    NULL
                                                    );
}

static int DnsSimpleParserIterator_Parse32Uint(DnsSimpleParserIterator *i,
                                               const char *Data,
                                               int DataLength,
                                             const char *Format, /* "%t:%v\n" */
                                               char *Buffer,
                                               int BufferLength,
                                               const char *Text,
                                               int *AcutalDataLength
                                               )
{
    char Example[] = "4294967295";
    uint32_t    u;

    if( DataLength < 4 || strlen(Format) + 1 > BufferLength )
    {
        return 0;
    }

    if( Text == NULL )
    {
        Text = "";
    }

    strcpy(Buffer, Format);

    if( ReplaceStr_WithLengthChecking(Buffer,
                                      "%t",
                                      Text,
                                      BufferLength
                                      )
       == NULL )
    {
        *Buffer = '\0';
        return 0;
    }

    u = GET_32_BIT_U_INT(Data);

    sprintf(Example, "%u", u);

    if( ReplaceStr_WithLengthChecking(Buffer,
                                      "%v",
                                      Example,
                                      BufferLength
                                      )
       == NULL )
    {
        *Buffer = '\0';
        return 0;
    }

    if( AcutalDataLength != NULL )
    {
        *AcutalDataLength = 4;
    }

    return 1;
}

static int DnsSimpleParserIterator_Parse16Uint(DnsSimpleParserIterator *i,
                                               const char *Data,
                                               int DataLength,
                                             const char *Format, /* "%t:%v\n" */
                                               char *Buffer,
                                               int BufferLength,
                                               const char *Text,
                                               int *AcutalDataLength
                                               )
{
    char Example[] = "4294967295";
    uint32_t    u;

    if( DataLength < 2 || strlen(Format) + 1 > BufferLength )
    {
        return 0;
    }

    if( Text == NULL )
    {
        Text = "";
    }

    strcpy(Buffer, Format);

    if( ReplaceStr_WithLengthChecking(Buffer,
                                      "%t",
                                      Text,
                                      BufferLength
                                      )
       == NULL )
    {
        *Buffer = '\0';
        return 0;
    }

    u = GET_16_BIT_U_INT(Data);

    sprintf(Example, "%d", (int)u);

    if( ReplaceStr_WithLengthChecking(Buffer,
                                      "%v",
                                      Example,
                                      BufferLength
                                      )
       == NULL )
    {
        *Buffer = '\0';
        return 0;
    }

    if( AcutalDataLength != NULL )
    {
        *AcutalDataLength = 2;
    }

    return 1;
}

static int DnsSimpleParserIterator_ParseSingleTxt(DnsSimpleParserIterator *i,
                                                  const char *Data,
                                                  int DataLength,
                                             const char *Format, /* "%t:%v\n" */
                                                  char *Buffer,
                                                  int BufferLength,
                                                  const char *Text,
                                                  int *AcutalDataLength
                                               )
{
    /* Static max length assumed to be 127+1 */
    char Example[128];
    char *Resulting;

    int StringLength = GET_8_BIT_U_INT(Data);

    if( strlen(Format) + 1 > BufferLength )
    {
        return 0;
    }

    if( Text == NULL )
    {
        Text = "";
    }

    strcpy(Buffer, Format);

    if( ReplaceStr_WithLengthChecking(Buffer,
                                      "%t",
                                      Text,
                                      BufferLength
                                      )
       == NULL )
    {
        *Buffer = '\0';
        return 0;
    }

    if( StringLength + 1 > sizeof(Example) )
    {
        Resulting = SafeMalloc(StringLength + 1);
    } else {
        Resulting = Example;
    }

    memcpy(Resulting, Data + 1, StringLength);
    Resulting[StringLength] = '\0';

    if( ReplaceStr_WithLengthChecking(Buffer,
                                      "%v",
                                      Resulting,
                                      BufferLength
                                      )
       == NULL )
    {
        *Buffer = '\0';

        if( Resulting != Example )
        {
            SafeFree(Resulting);
        }
        return 0;
    }

    if( AcutalDataLength != NULL )
    {
        *AcutalDataLength = StringLength + 1;
    }

    if( Resulting != Example )
    {
        SafeFree(Resulting);
    }
    return 1;
}

typedef int (*Parser)(DnsSimpleParserIterator *i,
                      const char *Data,
                      int DataLength,
                      const char *Format, /* "%t:%v\n" */
                      char *Buffer,
                      int BufferLength,
                      const char *Text,
                      int *AcutalDataLength
                      );

typedef struct {
    const char *Text;
    Parser ps;
} ParserProjector;

static int DnsSimpleParserIterator_ParseData(DnsSimpleParserIterator *i,
                                              const char *Data,
                                              int DataLength,
                                             const char *Format, /* "%t:%v\n" */
                                              char *Buffer,
                                              int BufferLength,
                                              const ParserProjector *pp
                                              )
{
    int n = 0;

    const char *DataItr = Data;
    int LeftDataLength = DataLength;

    char *BufferItr = Buffer;
    int LeftBufferLength = BufferLength;

    while( pp[n].Text != NULL )
    {
        int Stage;
        int ActualLength;
        Stage = pp[n].ps(i,
                         DataItr,
                         LeftDataLength,
                         Format,
                         BufferItr,
                         LeftBufferLength,
                         pp[n].Text,
                         &ActualLength
                         );
        if( Stage <= 0 )
        {
            break;
        } else {
            int ResultLength = strlen(BufferItr);

            BufferItr += ResultLength;
            LeftBufferLength -= ResultLength;

            DataItr += ActualLength;
            LeftDataLength -= ActualLength;
            ++n;
        }
    }

    return n;
}

static int DnsSimpleParserIterator_ParseSOA(DnsSimpleParserIterator *i,
                                              const char *Data,
                                              int DataLength,
                                             const char *Format, /* "%t:%v\n" */
                                              char *Buffer,
                                              int BufferLength
                                              )
{
    ParserProjector pp[] = {
        {"(SOA)primary name server", DnsSimpleParserIterator_ParseLabeledName},
        {"(SOA)responsible mail addr", DnsSimpleParserIterator_ParseLabeledName},
        {"(SOA)serial", DnsSimpleParserIterator_Parse32Uint},
        {"(SOA)refresh", DnsSimpleParserIterator_Parse32Uint},
        {"(SOA)retry", DnsSimpleParserIterator_Parse32Uint},
        {"(SOA)expire", DnsSimpleParserIterator_Parse32Uint},
        {"(SOA)default TTL", DnsSimpleParserIterator_Parse32Uint},

        {NULL, NULL},
    };

    return DnsSimpleParserIterator_ParseData(i,
                                             Data,
                                             DataLength,
                                             Format,
                                             Buffer,
                                             BufferLength,
                                             pp
                                             ) == 7 ? 7 : 0;
}

static int DnsSimpleParserIterator_ParseDomainPtr(DnsSimpleParserIterator *i,
                                                  const char *Data,
                                                  int DataLength,
                                             const char *Format, /* "%t:%v\n" */
                                                  char *Buffer,
                                                  int BufferLength
                                                  )
{
    return DnsSimpleParserIterator_ParseLabeledName(i,
                                                    Data,
                                                    DataLength,
                                                    Format,
                                                    Buffer,
                                                    BufferLength,
                                                    "name",
                                                    NULL
                                                    );
}

static int DnsSimpleParserIterator_ParseNameServer(DnsSimpleParserIterator *i,
                                                   const char *Data,
                                                   int DataLength,
                                             const char *Format, /* "%t:%v\n" */
                                                   char *Buffer,
                                                   int BufferLength
                                                   )
{
    return DnsSimpleParserIterator_ParseLabeledName(i,
                                                    Data,
                                                    DataLength,
                                                    Format,
                                                    Buffer,
                                                    BufferLength,
                                                    "Name Server",
                                                    NULL
                                                    );
}

static int DnsSimpleParserIterator_ParseMailEx(DnsSimpleParserIterator *i,
                                               const char *Data,
                                               int DataLength,
                                             const char *Format, /* "%t:%v\n" */
                                               char *Buffer,
                                               int BufferLength
                                               )
{
    ParserProjector pp[] = {
        {"preference", DnsSimpleParserIterator_Parse16Uint},
        {"mail exchanger", DnsSimpleParserIterator_ParseLabeledName},

        {NULL, NULL},
    };

    return DnsSimpleParserIterator_ParseData(i,
                                             Data,
                                             DataLength,
                                             Format,
                                             Buffer,
                                             BufferLength,
                                             pp
                                             ) == 2 ? 2 : 0;
}

static int DnsSimpleParserIterator_ParseTxt(DnsSimpleParserIterator *i,
                                            const char *Data,
                                            int DataLength,
                                            const char *Format, /* "%t:%v\n" */
                                            char *Buffer,
                                            int BufferLength
                                            )
{
    const char *DataItr = Data;
    int DataLeft = DataLength;

    char *BufferItr = Buffer;
    int BufferLeft = BufferLength;

    int n = 0;

    while( DataItr < Data + DataLength )
    {
        int Stage;
        int ActualLength;

        Stage = DnsSimpleParserIterator_ParseSingleTxt(i,
                                                       DataItr,
                                                       DataLeft,
                                                       Format,
                                                       BufferItr,
                                                       BufferLeft,
                                                       "TXT",
                                                       &ActualLength
                                                       );

        if( Stage <= 0 )
        {
            break;
        } else {
            int StageLength = strlen(BufferItr);

            DataItr += ActualLength;
            DataLeft -= ActualLength;

            BufferItr += StageLength;
            BufferLeft -= StageLength;

            ++n;
        }
    }

    return n;
}

static int DnsSimpleParserIterator_Unparsable(const char *Format, /* "%t:%v\n" */
                                              char *Buffer,
                                              int BufferLength,
                                              DNSRecordType Type
                                              )
{
    char a[] = "4294967295";

    strcpy(Buffer, Format);

    if( ReplaceStr_WithLengthChecking(Buffer,
                                      "%t",
                                      "Unparsable type",
                                      BufferLength
                                      )
       == NULL )
    {
        return 0;
    }

    sprintf(a, "%d", (int)Type);

    if( ReplaceStr_WithLengthChecking(Buffer,
                                      "%v",
                                      a,
                                      BufferLength
                                      )
       == NULL )
    {
        return 0;
    }

    return 1;
}

/* Number of items generated returned */
static int DnsSimpleParserIterator_TextifyData(DnsSimpleParserIterator *i,
                                             const char *Format, /* "%t:%v\n" */
                                               char *Buffer,
                                               int BufferLength
                                               )
{
    const char *Data = DNSGetResourceDataPos(i->CurrentPosition);

    int (*RecordParser)(DnsSimpleParserIterator *i,
                        const char *Data,
                        int DataLength,
                        const char *Format, /* "%t:%v\n" */
                        char *Buffer,
                        int BufferLength
                        ) = NULL;

    if( i->Type != DNS_TYPE_OPT &&
        i->Klass != DNS_CLASS_IN
        )
    {
        return 0; /* Unparsable */
    }

    switch( i->Type )
    {
    case DNS_TYPE_A:
        RecordParser = DnsSimpleParserIterator_ParseA;
        break;

    case DNS_TYPE_AAAA:
        RecordParser = DnsSimpleParserIterator_ParseAAAA;
        break;

    case DNS_TYPE_CNAME:
        RecordParser = DnsSimpleParserIterator_ParseCName;
        break;

    case DNS_TYPE_PTR:
        RecordParser = DnsSimpleParserIterator_ParseDomainPtr;
        break;

    case DNS_TYPE_SOA:
        RecordParser = DnsSimpleParserIterator_ParseSOA;
        break;

    case DNS_TYPE_TXT:
        RecordParser = DnsSimpleParserIterator_ParseTxt;
        break;

    case DNS_TYPE_MX:
        RecordParser = DnsSimpleParserIterator_ParseMailEx;
        break;

    case DNS_TYPE_NS:
        RecordParser = DnsSimpleParserIterator_ParseNameServer;
        break;

    default:
        RecordParser = NULL;
        break;
    }

    if( RecordParser != NULL )
    {
        return RecordParser(i,
                            Data,
                            i->DataLength,
                            Format,
                            Buffer,
                            BufferLength
                            );
    } else {
        return DnsSimpleParserIterator_Unparsable(Format,
                                                  Buffer,
                                                  BufferLength,
                                                  i->Type
                                                  );
    }
}

static uint32_t DnsSimpleParserIterator_GetTTL(DnsSimpleParserIterator *i)
{
    return DNSGetTTL(i->CurrentPosition);
}

int DnsSimpleParserIterator_Init(DnsSimpleParserIterator *i, DnsSimpleParser *p)
{
    int QuestionCount, AnswerCount, NameServerCount, AdditionalCount;

    if( i == NULL || p == NULL )
    {
        return -1;
    }

    QuestionCount = p->QuestionCount(p);
    AnswerCount = p->AnswerCount(p);
    NameServerCount = p->NameServerCount(p);
    AdditionalCount = p->AdditionalCount(p);

    i->Parser = p;
    i->CurrentPosition = NULL;
    i->RecordPosition = 0;

    i->AllRecordCount = QuestionCount +
                        AnswerCount +
                        NameServerCount +
                        AdditionalCount;

    i->QuestionFirst = QuestionCount == 0 ? 0 : 1;
    i->QuestionLast = i->QuestionFirst + QuestionCount - 1;

    i->AnswerFirst = AnswerCount == 0 ?
                     0 :
                     i->QuestionFirst + QuestionCount;
    i->AnswerLast = i->AnswerFirst + AnswerCount - 1;

    i->NameServerFirst = NameServerCount == 0 ?
                         0 :
                         i->QuestionFirst +
                             QuestionCount +
                             AnswerCount;
    i->NameServerLast = i->NameServerFirst + NameServerCount - 1;

    i->AdditionalFirst = AdditionalCount == 0 ?
                         0 :
                         i->QuestionFirst +
                             QuestionCount +
                             AnswerCount +
                             NameServerCount;
    i->AdditionalLast = i->AdditionalFirst + AdditionalCount - 1;

    i->Next = DnsSimpleParserIterator_Next;
    i->GotoAnswers = DnsSimpleParserIterator_GotoAnswers;
    i->GetName = DnsSimpleParserIterator_GetName;
    i->GetNameLength = DnsSimpleParserIterator_GetNameLength;
    i->RowData = DnsSimpleParserIterator_RowData;
    i->TextifyData = DnsSimpleParserIterator_TextifyData;
    i->GetTTL = DnsSimpleParserIterator_GetTTL;

    return 0;
}
