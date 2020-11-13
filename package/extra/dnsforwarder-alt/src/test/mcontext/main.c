#include "../../common.h"
#include "../../mcontext.h"
#include <stdlib.h>
#include <time.h>

typedef struct {
    IHeader h;
    uint16_t    i;
} HH;

IHeader *MakeOne(void)
{
    static int s = 0;

    static HH h;

    h.h.HashValue = s++;

    h.i = s++;

    return (IHeader *)&h;
}

int main(void)
{
    HH a, b;

    ModuleContext   c;

    ModuleContext_Init(&c);

    a = *(HH *)MakeOne();

    c.Add(&c, MakeOne());
    c.Add(&c, MakeOne());
    c.Add(&c, MakeOne());
    c.Add(&c, MakeOne());
    c.Add(&c, MakeOne());
    c.Add(&c, MakeOne());
    c.Add(&c, MakeOne());
    c.Add(&c, MakeOne());
    c.Add(&c, MakeOne());
    c.Add(&c, MakeOne());
    c.Add(&c, MakeOne());
    c.Add(&c, MakeOne());
    c.Add(&c, (IHeader *)&a);
    c.Add(&c, MakeOne());
    c.Add(&c, MakeOne());
    c.Add(&c, MakeOne());
    c.Add(&c, MakeOne());
    c.Add(&c, MakeOne());

    c.FindAndRemove(&c, (IHeader *)&a, (IHeader *)&b);

    return 0;
}
