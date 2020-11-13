#include <stdio.h>
#include <stdlib.h>
#include "../../stringlist.h"
#include "../testutils.h"
#include "../../stringchunk.h"

int main(void)
{
    StringChunk c;

    StringList l;

    char str[128];
    char Data[64];

    char *si;
    char *di;

    int n;

    int Start = 0;

    srand(time(NULL));

    StringList_Init(&l, NULL, NULL);

    StringChunk_Init(&c, &l);

    for( n = 70; n < sizeof(str); ++n )
    {
        StringChunk_Add(&c, RandomAlpha(str, sizeof(str)), RandomAlpha(Data, sizeof(Data)), sizeof(Data));
    }

    StringChunk_Add(&c, "a??.exe", "12345", 6);

    si = StringChunk_Enum_NoWildCard(&c, &Start, &di);
    while( si != NULL )
    {
        int h = StringChunk_Match(&c, si, NULL, &di);

        if( h )
        {
            printf("STRING : %s\nDATA: %s\n\n", si, di);
        } else {
            printf("STRING : %s\nNOT MATCHED\n\n", si);
        }

        si = StringChunk_Enum_NoWildCard(&c, &Start, &di);
    }

    int h = StringChunk_Match(&c, "asd1.exe", NULL, &di);
    if( h )
    {
        printf("STRING : %s\nDATA: %s\n\n", "asd1.exe", di);
    } else {
        printf("STRING : %s\nNOT MATCHED\n\n", "asd1.exe");
    }

    StringChunk_Free(&c, FALSE);

    return 0;
}
