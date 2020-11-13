#include <time.h>
#include <stdlib.h>

char *RandomString(char *out, int len)
{
    int i;

    for( i = 0; i < len - 1; ++i )
    {
        out[i] = rand() % ('~' - '!' + 1) + '!';
    }
    out[len - 1] = '\0';
    return out;
}

char *RandomAlpha(char *out, int len)
{
    int i;

    for( i = 0; i < len - 1; ++i )
    {
        out[i] = rand() % ('Z' - 'A' + 1) + 'A';
    }
    out[len - 1] = '\0';
    return out;
}
