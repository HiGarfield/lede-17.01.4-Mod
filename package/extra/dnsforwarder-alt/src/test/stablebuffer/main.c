#include <stdio.h>
#include <stdlib.h>
#include "../../stablebuffer.h"
#include "../testutils.h"

int main(void)
{
    StableBuffer b;
    StableBufferIterator i;
    char str[10240];

    int n;
    char *bl;

    srand(time(NULL));

    StableBuffer_Init(&b);

    for( n = 2; n < 50; ++n )
    {
        b.Add(&b, RandomString(str, n), n);
    }

    StableBufferIterator_Init(&i, &b);

    bl = i.NextBlock(&i);
    while( bl != NULL )
    {
        printf("____________Block %d/%d\n", i.CurrentBlockUsed(&i), i.CurrentBlockSize(&i));
        printf("%s\n", (char *)bl);
        i.RemoveNBytesOfCurrentBlock(&i, bl + 1, 10);
        printf("____________Block %d/%d\n", i.CurrentBlockUsed(&i), i.CurrentBlockSize(&i));
        printf("After removing:\n%s\n\n", (char *)bl);


        bl = i.NextBlock(&i);
    }

    b.Free(&b);

    return 0;
}
