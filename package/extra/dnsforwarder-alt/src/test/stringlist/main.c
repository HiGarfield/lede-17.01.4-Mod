#include <stdio.h>
#include <stdlib.h>
#include "../../stringlist.h"
#include "../testutils.h"

int main(void)
{
    StringList l;
    StringListIterator i;

    char str[128];
    const char *ci;

    int n;

    srand(time(NULL));
    StringList_Init(&l, "          asd          ,      facryhty,,    ,  ,00000", ",");

    l.Add(&l, ",,,,,,asd          ,      facryhty,,    ,  ,00000,,,,,,,", ",");
    l.Add(&l, ",,,,,,asd          ,      facryhty,,    ,  ,00000,,,,,,,", ",");
    l.Add(&l, ",,,,,,asd          ,      facryhty,,    ,  ,00000,,,,,,,", ",");
    l.Add(&l, ",,,,,,asd          ,      facryhty,,    ,  ,00000,,,,,,,", ",");
    l.Add(&l, ",,,,,,asd          ,      facryhty,,    ,  ,00000,,,,,,,", ",");
    l.Add(&l, ",,,,,,asd          ,      facryhty,,    ,  ,00000,,,,,,,", ",");
    l.Add(&l, ",,,,,,asd          ,      facryhty,,    ,  ,00000,,,,,,,", ",");
    l.Add(&l, ",,,,,,asd          ,      facryhty,,    ,  ,00000,,,,,,,", ",");
    l.Add(&l, ",,,,,,asd          ,      facryhty,,    ,  ,00000,,,,,,,", ",");
    l.Add(&l, ",,,,,,asd          ,      facryhty,,    ,  ,00000,,,,,,,", ",");
    l.Add(&l, ",,,,,,asd          ,      facryhty,,    ,  ,00000,,,,,,,", ",");
    l.Add(&l, ",,,,,,asd          ,      facryhty,,    ,  ,00000,,,,,,,", ",");
    l.Add(&l, ",,,,,,asd          ,      facryhty,,    ,  ,00000,,,,,,,", ",");
    l.Add(&l, ",,,,,,asd          ,      facryhty,,    ,  ,00000,,,,,,,", ",");
    l.Add(&l, ",,,,,,asd          ,      facryhty,,    ,  ,00000,,,,,,,", ",");
    l.Add(&l, ",,,,,,asd          ,      facryhty,,    ,  ,00000,,,,,,,", ",");
    l.Add(&l, ",,,,,,asd          ,      facryhty,,    ,  ,00000,,,,,,,", ",");
    l.Add(&l, ",,,,,,asd          ,      facryhty,,    ,  ,00000,,,,,,,", ",");
    l.Add(&l, ",,,,,,asd          ,      facryhty,,    ,  ,00000,,,,,,,", ",");
    l.Add(&l, ",,,,,,asd          ,      facryhty,,    ,  ,00000,,,,,,,", ",");
    l.Add(&l, ",,,,,,asd          ,      facryhty,,    ,  ,00000,,,,,,,", ",");
    l.Add(&l, ",,,,,,asd          ,      facryhty,,    ,  ,00000,,,,,,,", ",");
    l.Add(&l, ",,,,,,asd          ,      facryhty,,    ,  ,00000,,,,,,,", ",");
    l.Add(&l, ",,,,,,asd          ,      facryhty,,    ,  ,00000,,,,,,,", ",");
    l.Add(&l, ",,,,,,asd          ,      facryhty,,    ,  ,00000,,,,,,,", ",");
    l.Add(&l, ",,,,,,asd          ,      facryhty,,    ,  ,00000,,,,,,,", ",");
    l.Add(&l, ",,,,,,asd          ,      facryhty,,    ,  ,00000,,,,,,,", ",");
    l.Add(&l, ",,,,,,asd          ,      facryhty,,    ,  ,00000,,,,,,,", ",");
    l.Add(&l, ",,,,,,asd          ,      facryhty,,    ,  ,00000,,,,,,,", ",");
    l.Add(&l, ",,,,,,asd          ,      facryhty,,    ,  ,00000,,,,,,,", ",");
    l.Add(&l, ",,,,,,asd          ,      facryhty,,    ,  ,00000,,,,,,,", ",");
    l.Add(&l, ",,,,,,asd          ,      facryhty,,    ,  ,00000,,,,,,,", ",");
    l.Add(&l, ",,,,,,asd          ,      facryhty,,    ,  ,00000,,,,,,,", ",");
    l.Add(&l, ",,,,,,asd          ,      facryhty,,    ,  ,00000,,,,,,,", ",");
    l.Add(&l, ",,,,,,asd          ,      facryhty,,    ,  ,00000,,,,,,,", ",");

    l.TrimAll(&l);

/*
    for( n = 2; n < sizeof(str); ++n )
    {
        l.Add(&l, RandomString(str, n), "");
    }
*/
    printf("Count : %d\n", l.Count(&l));

    StringListIterator_Init(&i, &l);

    ci = i.Next(&i);
    n = 0;
    while( ci != NULL )
    {
        printf("%s\n\n", ci);
        ci = i.Next(&i);
        ++n;
    }

    l.Free(&l);

    printf("Count : %d\n", n);

    return 0;
}
