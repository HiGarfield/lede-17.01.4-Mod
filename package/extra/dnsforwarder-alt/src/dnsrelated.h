#ifndef _DNS_RELATED_
#define _DNS_RELATED_

#include "common.h"

typedef enum _DNSRecordType{
	DNS_TYPE_UNKNOWN	=	0,
	DNS_TYPE_A		=	1,
	DNS_TYPE_AAAA	=	28,
	DNS_TYPE_APL	=	42,
	DNS_TYPE_CERT	=	37,
	DNS_TYPE_CNAME	=	5,
	DNS_TYPE_DHCID	=	49,
	DNS_TYPE_DLV	=	32769,
	DNS_TYPE_DNAME	=	39,
	DNS_TYPE_DNSKEY	=	48,
	DNS_TYPE_DS		=	43,
	DNS_TYPE_HIP	=	55,
	DNS_TYPE_IPSECKEY	=	45,
	DNS_TYPE_KEY	=	25,
	DNS_TYPE_KX		=	36,
	DNS_TYPE_LOC	=	29,
	DNS_TYPE_MX		=	15,
	DNS_TYPE_NAPTR	=	35,
	DNS_TYPE_NS		=	2,
	DNS_TYPE_NSEC	=	47,
	DNS_TYPE_NSEC3	=	50,
	DNS_TYPE_NSEC3PARAM	=	51,
	DNS_TYPE_PTR	=	12,
	DNS_TYPE_RRSIG	=	46,
	DNS_TYPE_RP		=	17,
	DNS_TYPE_SIG	=	24,
	DNS_TYPE_SOA	=	6,
	DNS_TYPE_SPF	=	99,
	DNS_TYPE_SRV	=	33,
	DNS_TYPE_SSHFP	=	44,
	DNS_TYPE_TA		=	32768,
	DNS_TYPE_TKEY	=	249,
	DNS_TYPE_TSIG	=	250,
	DNS_TYPE_TXT	=	16,
	DNS_TYPE_ANY	=	255,
	DNS_TYPE_AXFR	=	252,
	DNS_TYPE_IXFR	=	251,
	DNS_TYPE_HINFO	=	13,
	DNS_TYPE_OPT	=	41,
}DNSRecordType;

typedef enum _DNSRecordClass{
	DNS_CLASS_UNKNOWN	=	0,
	DNS_CLASS_IN		=	1,
	DNS_CLASS_CH		=	3,
	DNS_CLASS_ANY		=	255,
}DNSRecordClass;

typedef struct _DNSTypeName{
	DNSRecordType	Num;
	const char		*Name;
}DNSTypeAndName;

typedef struct _DNSSECAlgorithm {
	int Num;
	const char *Name;
} DNSSECAlgorithm;

extern const DNSTypeAndName DNSTypeList[];

extern const DNSSECAlgorithm DNSSECAlgorithmList[];

const char *DNSGetTypeName(uint16_t Num);

const char *DNSSECGetAlgorithmName(int Num);

#endif /* _DNS_RELATED_ */
