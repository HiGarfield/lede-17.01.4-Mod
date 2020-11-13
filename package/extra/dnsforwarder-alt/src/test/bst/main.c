#include "../../bst.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>

int f(int *o, int *t)
{
    return *o - *t;
}

int print(Bst *t, const void *Data, void *Arg)
{
    printf("int : %d\n", *(int *)Data);
    return 0;
}

int testify(Bst *t, const void *Data, void *Arg)
{
    Bst_NodeHead *n = ((Bst_NodeHead *)Data) - 1;
    Bst_NodeHead *l = n->Left;
    Bst_NodeHead *r = n->Right;

    int in;
    int il = -1;
    int ir = INT_MAX;

    in = *(int *)(n + 1);
    if( l != NULL ) il = *(int *)(l + 1);
    if( r != NULL ) ir = *(int *)(r + 1);

    /*printf("Node %#010x %d\n", (unsigned int)n, in);*/

    if( il <= in && ir >= in ){}
    else
        printf("Condition ussatisfied.\n");

    if( (l == NULL && r == NULL) ||
           (l == NULL && r != NULL && r->Parent == n) ||
           (l != NULL && r == NULL && l->Parent == n) ||
           (l != NULL && r != NULL && r->Parent == n && l->Parent == n) ){}
    else
        printf("Parent incorrect.\n");

    return 0;
}

int add(Bst *t, int n, int *min, int *max)
{
    void *added = t->Add(t, &n);
    int a = *(int *)added;
    if( a > *max ) *max = a;
    if( a < *min ) *min = a;
    printf("Add :%#010x %d\n", ((Bst_NodeHead *)added) - 1,a);
    return a;
}

void del(Bst *t, int n)
{
    const void *Node = t->Search(t, &n, NULL);

    if( Node != NULL )
    {
        t->Delete(t, Node);
    }
}

int main(void)
{
    Bst t;
    int loop;

    int max = -1,min = INT_MAX;

    srand(time(NULL));

    Bst_Init(&t, sizeof(int), f);

    printf("==>>> Delete a node with no child\n");

    add(&t, INT_MAX - 1, &min, &max);
    add(&t, INT_MAX - 2, &min, &max);
    add(&t, INT_MAX - 3, &min, &max);
    add(&t, INT_MAX - 4, &min, &max);
    add(&t, INT_MAX - 5, &min, &max);
    del(&t, INT_MAX - 5);
    t.Enum(&t, testify, NULL);
    t.Reset(&t);
    max = -1;min = INT_MAX;

    printf("==>>> Delete a node with one child\n");

    add(&t, INT_MAX - 1, &min, &max);
    add(&t, INT_MAX - 2, &min, &max);
    add(&t, INT_MAX - 3, &min, &max);
    add(&t, INT_MAX - 4, &min, &max);
    add(&t, INT_MAX - 5, &min, &max);
    del(&t, INT_MAX - 4);
    t.Enum(&t, testify, NULL);
    t.Reset(&t);
    max = -1;min = INT_MAX;

    printf("==>>> Delete a node with two children\n");

    add(&t, INT_MAX - 3, &min, &max);
    add(&t, INT_MAX - 2, &min, &max);
    add(&t, INT_MAX - 1, &min, &max);
    add(&t, INT_MAX - 4, &min, &max);
    add(&t, INT_MAX - 5, &min, &max);
    del(&t, INT_MAX - 3);
    t.Enum(&t, testify, NULL);
    t.Reset(&t);
    max = -1;min = INT_MAX;

    printf("==>>> Minimum\n");

    for( loop = 0; loop != 100; ++loop )
    {
        add(&t, rand(), &min, &max);
    }

    if( min != *(int *)t.Minimum(&t, NULL) )
    {
        printf("Test failed.\n");
    }

    t.Enum(&t, testify, NULL);
    t.Reset(&t);
    max = -1;min = INT_MAX;

    printf("==>>> Del and add\n");

    add(&t, INT_MAX - 1, &min, &max);
    add(&t, INT_MAX - 2, &min, &max);
    add(&t, INT_MAX - 3, &min, &max);
    add(&t, INT_MAX - 4, &min, &max);
    add(&t, INT_MAX - 5, &min, &max);
    del(&t, INT_MAX - 5);
    del(&t, INT_MAX - 4);
    del(&t, INT_MAX - 3);
    t.Enum(&t, testify, NULL);
    add(&t, INT_MAX - 5, &min, &max);
    add(&t, INT_MAX - 4, &min, &max);
    add(&t, INT_MAX - 3, &min, &max);
    add(&t, INT_MAX - 6, &min, &max);
    t.Reset(&t);
    max = -1;min = INT_MAX;

    printf("==>>> Del root and add\n");
    add(&t, 0, &min, &max);
    del(&t, 0);
    t.Enum(&t, testify, NULL);
    add(&t, 1, &min, &max);
    t.Enum(&t, testify, NULL);
    t.Reset(&t);
    max = -1;min = INT_MAX;

    printf("==>>> Del\n");
    add(&t, 10, &min, &max);
    add(&t, 5, &min, &max);
    add(&t, 18, &min, &max);
    add(&t, 6, &min, &max);
    add(&t, 15, &min, &max);
    t.Enum(&t, print, NULL);
    del(&t, 5);
    del(&t, 15);
    del(&t, 10);
    printf("> After Del\n");
    t.Enum(&t, print, NULL);
    t.Enum(&t, testify, NULL);
    t.Reset(&t);
    max = -1;min = INT_MAX;


    return 0;
}
