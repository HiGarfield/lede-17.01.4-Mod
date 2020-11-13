#include "hostsutils.h"
#include "dnsgenerator.h"
#include "mmgr.h"
#include "goodiplist.h"

static int HostsUtils_GetCName_Callback(int Number,
                                        HostsRecordType Type,
                                        const char *Data,
                                        char *Buffer
                                        )
{
    strcpy(Buffer, Data);
    return 0;
}

int HostsUtils_GetCName(const char *Domain,
                        char *Buffer,
                        HostsContainer *Container
                        )
{
    return Container->Find(Container,
                           Domain,
                           HOSTS_TYPE_CNAME,
                           (HostsFindFunc)HostsUtils_GetCName_Callback,
                           Buffer
                           )
            == NULL;
}

BOOL HostsUtils_TypeExisting(HostsContainer *Container,
                             const char *Domain,
                             HostsRecordType Type
                             )
{
    return Container->Find(Container,
                           Domain,
                           Type,
                           NULL,
                           NULL
                           )
            != NULL;
}

static int HostsUtils_Generate(int              Number,
                               HostsRecordType  Type,
                               const void       *Data,
                               DnsGenerator     *g /* Inited */
                               )
{
    switch( Type )
    {
    case HOSTS_TYPE_CNAME:
        if( g->CName(g, "a", Data, 60) != 0 )
        {
            return -26;
        }
        break;

    case HOSTS_TYPE_A:
        if( g->RawData(g,
                       "a",
                       DNS_TYPE_A,
                       DNS_CLASS_IN,
                       Data,
                       4,
                       60
                       )
            != 0 )
        {
            return -41;
        }
        break;

    case HOSTS_TYPE_AAAA:
        if( g->RawData(g,
                       "a",
                       DNS_TYPE_AAAA,
                       DNS_CLASS_IN,
                       Data,
                       16,
                       60
                       )
            != 0 )
        {
            return -56;
        }
        break;

    case HOSTS_TYPE_GOOD_IP_LIST:
        {
            const char *ActuallData;

            ActuallData = GoodIpList_Get(Data);
            if( ActuallData == NULL )
            {
                return -96;
            }

            if( g->RawData(g,
                           "a",
                           DNS_TYPE_A,
                           DNS_CLASS_IN,
                           ActuallData,
                           4,
                           60
                           )
                != 0 )
            {
                return -109;
            }
        }
        break;

    default:
        return -61;
        break;
    }

    return 0;
}

HostsUtilsTryResult HostsUtils_Try(IHeader *Header,
                                   int BufferLength,
                                   HostsContainer *Container
                                   )
{
	char *RequestEntity = (char *)(Header + 1);
	const char	*MatchState;
	HostsRecordType Type;

    if( Header->Type != DNS_TYPE_CNAME &&
        HostsUtils_TypeExisting(Container, Header->Domain, HOSTS_TYPE_CNAME)
        )
    {
        return HOSTSUTILS_TRY_RECURSED;
    }

    switch( Header->Type )
    {
    case DNS_TYPE_CNAME:
        Type = HOSTS_TYPE_CNAME;
        MatchState = Container->Find(Container,
                                     Header->Domain,
                                     HOSTS_TYPE_CNAME,
                                     NULL,
                                     NULL
                                     );
        break;

    case DNS_TYPE_A:
        MatchState = Container->Find(Container,
                                     Header->Domain,
                                     HOSTS_TYPE_A,
                                     NULL,
                                     NULL
                                     );
        if( MatchState != NULL )
        {
            Type = HOSTS_TYPE_A;
        } else {
            Type = HOSTS_TYPE_GOOD_IP_LIST;
            MatchState = Container->Find(Container,
                                         Header->Domain,
                                         HOSTS_TYPE_GOOD_IP_LIST,
                                         NULL,
                                         NULL
                                         );
        }
        break;

    case DNS_TYPE_AAAA:
        Type = HOSTS_TYPE_AAAA;
        MatchState = Container->Find(Container,
                                     Header->Domain,
                                     HOSTS_TYPE_AAAA,
                                     NULL,
                                     NULL
                                     );
        break;

    default:
        return HOSTSUTILS_TRY_NONE;
        break;
    }

	if( MatchState != NULL )
	{
        DnsGenerator g;

        char *HereToGenerate = RequestEntity + Header->EntityLength;
        int LeftBufferLength =
                          BufferLength - sizeof(IHeader) - Header->EntityLength;

        int ResultLength;

        if( DnsGenerator_Init(&g,
                              HereToGenerate,
                              LeftBufferLength,
                              RequestEntity,
                              Header->EntityLength,
                              TRUE
                              )
           != 0)
        {
            return HOSTSUTILS_TRY_NONE;
        }

        g.Header->Flags.Direction = 1;
        g.Header->Flags.AuthoritativeAnswer = 0;
        g.Header->Flags.RecursionAvailable = 1;
        g.Header->Flags.ResponseCode = 0;
        g.Header->Flags.Type = 0;

        if( g.NextPurpose(&g) != DNS_RECORD_PURPOSE_ANSWER )
        {
            return HOSTSUTILS_TRY_NONE;
        }

        if( Container->Find(Container,
                            Header->Domain,
                            Type,
                            (HostsFindFunc)HostsUtils_Generate,
                            &g
                            )
            == NULL )
        {
            return HOSTSUTILS_TRY_NONE;
        }

        if( Header->EDNSEnabled )
        {
            while( g.NextPurpose(&g) != DNS_RECORD_PURPOSE_ADDITIONAL );
            if( g.EDns(&g, 1280) != 0 )
            {
                return HOSTSUTILS_TRY_NONE;
            }
        }

        /* g will no longer be needed, and can be crapped */
        ResultLength = DNSCompress(HereToGenerate, g.Length(&g));
        if( ResultLength < 0 )
        {
            return HOSTSUTILS_TRY_NONE;
        }

        Header->EntityLength = ResultLength;
        memmove(RequestEntity, HereToGenerate, ResultLength);

        IHeader_SendBack(Header);

        return HOSTSUTILS_TRY_OK;
	} else {
	    return HOSTSUTILS_TRY_NONE;
	}
}

int HostsUtils_Query(SOCKET Socket, /* Both for sending and receiving */
                     Address_Type *BackAddress,
                     int Identifier,
                     const char *Name,
                     DNSRecordType Type
                     )
{
    static const char DNSHeader[DNS_HEADER_LENGTH] = {
        00, 00, /* QueryIdentifier */
        01, 00, /* Flags */
        00, 00, /* QuestionCount */
        00, 00, /* AnswerCount */
        00, 00, /* NameServerCount */
        00, 00, /* AdditionalCount */
    };

    char RequestBuffer[2048];
    IHeader *Header = (IHeader *)RequestBuffer;
    char *RequestEntity = RequestBuffer + sizeof(IHeader);

	DnsGenerator g;

	if( DnsGenerator_Init(&g,
                          RequestEntity,
                          sizeof(RequestBuffer) - sizeof(IHeader),
                          DNSHeader,
                          DNS_HEADER_LENGTH,
                          FALSE
                          )
        != 0 )
    {
        return -323;
    }

    g.CopyIdentifier(&g, Identifier);

    if( g.Question(&g, Name, Type, DNS_CLASS_IN) != 0 )
    {
        return -328;
    }

    if( IHeader_Fill(Header,
                     TRUE,
                     RequestEntity,
                     g.Length(&g),
                     (struct sockaddr *)&(BackAddress->Addr),
                     Socket,
                     BackAddress->family,
                     "CNameRedirect"
                     )
        != 0 )
    {
        return -309;
    }

    return MMgr_Send(Header, sizeof(RequestBuffer));
}

/* Error code returned */
int HostsUtils_CombineRecursedResponse(void       *Buffer, /* Include IHeader */
                                       int          Bufferlength,
                                       char         *RecursedEntity,
                                       int          EntityLength,
                                       const char   *RecursedDomain
                                       )
{
    IHeader *NewHeader = (IHeader *)Buffer;
    char *NewEntity = Buffer + sizeof(IHeader);

    DnsSimpleParser p;
    DnsSimpleParserIterator i;
    DnsGenerator g;

    uint16_t OriginalIdentifier = *(uint16_t *)NewEntity;

    int CompressedLength;

    if( DnsSimpleParser_Init(&p,
                             RecursedEntity,
                             EntityLength,
                             FALSE
                             ) != 0
        )
    {
        return -268;
    }

    if( DnsSimpleParserIterator_Init(&i, &p) != 0 )
    {
        return -273;
    }

    if( DnsGenerator_Init(&g,
                          NewEntity,
                          Bufferlength - sizeof(IHeader),
                          NULL,
                          0,
                          FALSE
                          )
       != 0 )
    {
        return -285;
    }

    g.CopyHeader(&g, RecursedEntity, FALSE);
    g.CopyIdentifier(&g, OriginalIdentifier);

    if( g.Question(&g,
                   NewHeader->Domain,
                   NewHeader->Type,
                   DNS_CLASS_IN
                   )
       != 0 )
    {
        return -298;
    }

    if( g.NextPurpose(&g) != DNS_RECORD_PURPOSE_ANSWER )
    {
        return -303;
    }

    if( g.CName(&g, NewHeader->Domain, RecursedDomain, 60) != 0 )
    {
        return -309;
    }

    i.GotoAnswers(&i);

    while( i.Next(&i) != NULL && i.Purpose == DNS_RECORD_PURPOSE_ANSWER )
    {
        switch( i.Type )
        {
        case DNS_TYPE_CNAME:
            if( g.CopyCName(&g, &i) != 0 )
            {
                return -321;
            }
            break;

        case DNS_TYPE_A:
            if( g.CopyA(&g, &i) != 0 )
            {
                return -328;
            }
            break;

        case DNS_TYPE_AAAA:
            if( g.CopyAAAA(&g, &i) != 0 )
            {
                return -335;
            }
            break;

        default:
            break;
        }
    }

    if( NewHeader->EDNSEnabled )
    {
        while( g.NextPurpose(&g) != DNS_RECORD_PURPOSE_ADDITIONAL );

        if( g.EDns(&g, 1280) != 0 )
        {
            return -351;
        }
    }

    CompressedLength = DNSCompress(g.Buffer, g.Length(&g));
    if( CompressedLength < 0 )
    {
        return -343;
    }

    NewHeader->EntityLength = CompressedLength;

    return 0;
}
