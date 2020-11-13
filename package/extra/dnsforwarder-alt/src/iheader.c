#include <string.h>
#include "iheader.h"
#include "dnsparser.h"
#include "dnsgenerator.h"
#include "common.h"
#include "logs.h"

static BOOL ap = FALSE;

int IHeader_Init(BOOL _ap)
{
    ap = _ap;

    return 0;
}

void IHeader_Reset(IHeader *h)
{
    h->_Pad = 0;
    h->Agent[0] = '\0';
    h->BackAddress.family = AF_UNSPEC;
    h->Domain[0] = '\0';
    h->EDNSEnabled = FALSE;
}

int IHeader_Fill(IHeader *h,
                 BOOL ReturnHeader, /* For tcp, this will be ignored */
                 char *DnsEntity,
                 int EntityLength,
                 struct sockaddr *BackAddress, /* NULL for tcp */
                 SOCKET SendBackSocket,
                 sa_family_t Family, /* For tcp, this will be ignored */
                 const char *Agent
                 )
{
    DnsSimpleParser p;
    DnsSimpleParserIterator i;

    h->_Pad = 0;
    h->EDNSEnabled = FALSE;

    if( DnsSimpleParser_Init(&p, DnsEntity, EntityLength, FALSE) != 0 )
    {
        return -31;
    }

    if( DnsSimpleParserIterator_Init(&i, &p) != 0 )
    {
        return -36;
    }

    while( i.Next(&i) != NULL )
    {
        switch( i.Purpose )
        {
        case DNS_RECORD_PURPOSE_QUESTION:
            if( i.Klass != DNS_CLASS_IN )
            {
                return -48;
            }

            if( i.GetName(&i, h->Domain, sizeof(h->Domain)) < 0 )
            {
                return -46;
            }

            StrToLower(h->Domain);
            h->HashValue = ELFHash(h->Domain, 0);
            h->Type = (DNSRecordType)DNSGetRecordType(DNSJumpHeader(DnsEntity));
            break;

        case DNS_RECORD_PURPOSE_ADDITIONAL:
            if( i.Type == DNS_TYPE_OPT )
            {
                h->EDNSEnabled = TRUE;
            }
            break;

        default:
            break;
        }
    }

    h->ReturnHeader = ReturnHeader;

    if( BackAddress != NULL )
    {
        memcpy(&(h->BackAddress.Addr), BackAddress, GetAddressLength(Family));
        h->BackAddress.family = Family;
    } else {
        h->BackAddress.family = AF_UNSPEC;
    }

    h->SendBackSocket = SendBackSocket;

    if( Agent != NULL )
    {
        strncpy(h->Agent, Agent, sizeof(h->Agent));
        h->Agent[sizeof(h->Agent) - 1] = '\0';
    } else {
        h->Agent[0] = '\0';
    }

    h->EntityLength = EntityLength;

    return 0;
}

int IHeader_AddFakeEdns(IHeader *h, int BufferLength)
{
    DnsGenerator g;

    if( ap == FALSE || h->EDNSEnabled )
    {
        return 0;
    }

    if( DnsGenerator_Init(&g,
                          IHEADER_TAIL(h),
                          BufferLength - sizeof(IHeader),
                          IHEADER_TAIL(h),
                          h->EntityLength,
                          FALSE
                          )
        != 0 )
    {
        return -125;
    }

    while( g.NextPurpose(&g) != DNS_RECORD_PURPOSE_ADDITIONAL );

    g.EDns(&g, 1280);

    h->EntityLength = g.Length(&g);
    h->EDNSEnabled = TRUE;

    return 0;
}

BOOL IHeader_Blocked(IHeader *h /* Entity followed */)
{
    return (ap && !(h->EDNSEnabled));
}

int IHeader_SendBack(IHeader *h /* Entity followed */)
{
    if( h->BackAddress.family == AF_UNSPEC )
    {
        /* TCP */
        uint16_t TcpLength = htons(h->EntityLength);

        memcpy((char *)(IHEADER_TAIL(h)) - 2, &TcpLength, 2);

        if( send(h->SendBackSocket,
                 (char *)(IHEADER_TAIL(h)) - 2,
                 h->EntityLength + 2,
                 MSG_NOSIGNAL
                 )
            != h->EntityLength )
        {
            /** TODO: Show error */
            return -112;
        }
    } else {
        /* UDP */
        const char *Content;
        int Length;

        if( h->ReturnHeader )
        {
            Content = (const char *)h;
            Length = h->EntityLength + sizeof(IHeader);
        } else {
            Content = IHEADER_TAIL(h);
            Length = h->EntityLength;
        }

        if( sendto(h->SendBackSocket,
                   Content,
                   Length,
                   MSG_NOSIGNAL,
                   (const struct sockaddr *)&(h->BackAddress.Addr),
                   GetAddressLength(h->BackAddress.family)
                   )
           != Length )
        {
            /** TODO: Show error */
            return -138;
        }
    }

    return 0;
}

int IHeader_SendBackRefusedMessage(IHeader *h)
{
    DNSHeader *RequestContent = IHEADER_TAIL(h);

    RequestContent->Flags.Direction = 1;
    RequestContent->Flags.RecursionAvailable = 1;
    RequestContent->Flags.ResponseCode = 0;

    return IHeader_SendBack(h);

}
