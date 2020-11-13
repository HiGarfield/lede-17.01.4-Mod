#include <stdlib.h>
#include "dnsrelated.h"

/* http://en.wikipedia.org/wiki/List_of_DNS_record_types */
const DNSTypeAndName DNSTypeList[] = {
	{1,		"IPv4 Address"},
	{5,		"Canonical Name"},
	{28,	"IPv6 Address"},
	{6,		"start of authority"},
	{12,	"Domain pointer"},
	{2,		"Name Server"},
	{15,	"MX"},
	{16,	"TXT"},
	{41,	"OPT"},
	{255,	"*"},

	{18,	"AFSDB"},
	{42,	"APL"},
	{37,	"CERT"},
	{49,	"DHCID"},
	{32769,	"DLV"},
	{39,	"DNAME"},
	{48,	"DNSKEY"},
	{43,	"DS"},
	{55,	"HIP"},
	{45,	"IPSECKEY"},
	{25,	"KEY"},
	{36,	"KX"},
	{29,	"LOC"},
	{35,	"NAPTR"},
	{47,	"NSEC"},
	{50,	"NSEC3"},
	{51,	"NSEC3PARAM"},
	{46,	"RRSIG"},
	{17,	"RP"},
	{24,	"SIG"},
	{99,	"SPF"},
	{33,	"SRV"},
	{44,	"SSHFP"},
	{32768,	"TA"},
	{249,	"TKEY"},
	{250,	"TSIG"},
	{252,	"AXFR"},
	{251,	"IXFR"},
	{13,	"Host Information"},
	{0,		NULL}
};

/* http://www.iana.org/assignments/dns-sec-alg-numbers/dns-sec-alg-numbers.xhtml */
const DNSSECAlgorithm DNSSECAlgorithmList[] = {
	{0,		"(reserved)"},
	{1,		"RSA/MD5"},
	{2,		"Diffie-Hellman"},
	{3,		"DSA/SHA-1"},
	{4,		"Elliptic Curve"},
	{5,		"RSA/SHA-1"},
	{6,		"DSA-NSEC3-SHA1"},
	{7,		"RSASHA1-NSEC3-SHA1"},
	{8,		"RSA/SHA-256"},
	{9,		"(reserved)"},
	{10,	"RSA/SHA-512"},
	{11,	"(reserved)"},
	{12,	"GOST R 34.10-2001"},
	{13,	"ECDSA Curve P-256 with SHA-256"},
	{14,	"ECDSA Curve P-384 with SHA-384"},

	{252,	"Indirect"},
	{253,	"Private"},
	{254,	"Private"},
	{255,	"(reserved)"}
};

const char *DNSGetTypeName(uint16_t Num)
{
	int loop;

	for(loop = 0; loop != sizeof(DNSTypeList) / sizeof(DNSTypeAndName); ++loop)
	{
		if( DNSTypeList[loop].Num == Num )
		{
			return DNSTypeList[loop].Name;
		}
	}

	return "UNKNOWN";
}

static int DNSSECAlgorithm_Compare(DNSSECAlgorithm *Key, DNSSECAlgorithm *Element)
{
	return Key->Num - Element->Num;
}

const char *DNSSECGetAlgorithmName(int Num)
{
	DNSSECAlgorithm Key = {Num, NULL};

	DNSSECAlgorithm *Result = NULL;

	Result = bsearch(&Key, DNSSECAlgorithmList, sizeof(DNSSECAlgorithmList) / sizeof(DNSSECAlgorithm), sizeof(DNSSECAlgorithm), (int (*)(const void *, const void *))DNSSECAlgorithm_Compare);

	if( Result == NULL )
	{
		return "UNKNOWN";
	} else {
		return Result->Name;
	}
}
