#ifndef __MEMRCHR_C__
#define __MEMRCHR_C__

#include <string.h>

void *memrchr(const void *m, int c, size_t n)
{
    const unsigned char *s = m;
    unsigned char uc = (unsigned char)c;
    while (n--)
        if (s[n] == uc)
            return (void *)(s + n);
    return NULL;
}

#endif
