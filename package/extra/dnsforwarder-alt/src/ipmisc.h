#ifndef IPMISC_H_INCLUDED
#define IPMISC_H_INCLUDED

#include "ipchunk.h"
#include "dnsparser.h"
#include "readconfig.h"
#include "iheader.h"

typedef enum _MiscType {
    IP_MISC_TYPE_UNKNOWN = 0,
    IP_MISC_TYPE_BLOCK,
    IP_MISC_TYPE_SUBSTITUTE,
} MiscType;

#define IP_MISC_NOTHING 0
#define IP_MISC_FILTERED_IP (-1)
#define IP_MISC_NEGATIVE_RESULT (-2)

typedef struct _IPMisc IPMisc;

struct _IPMisc{
    /* private */
    IpChunk c;

    BOOL BlockNegative;

    int (*AddBlockFromString)(IPMisc *m, const char *Ip);
    int (*AddSubstituteFromString)(IPMisc *m,
                                   const char *Ip,
                                   const char *Substituter
                                   );
    void (*SetBlockNegative)(IPMisc *m, BOOL Value);
    int (*Process)(IPMisc *m,
                   char *DNSPackage, /* Without TCPLength */
                   int PackageLength
                   );
};

int IPMisc_Init(IPMisc *m);

/** Singleton */

int IpMiscSingleton_Init(ConfigFileInfo *ConfigInfo);

int IPMiscSingleton_Process(IHeader *h /* Entity followed */);

#endif // IPMISC_H_INCLUDED
