#ifndef BST_H_INCLUDED
#define BST_H_INCLUDED

#include "stablebuffer.h"
#include "utils.h"
#include "oo.h"

typedef struct _Bst_NodeHead Bst_NodeHead;

struct _Bst_NodeHead{
	Bst_NodeHead    *Parent;
	Bst_NodeHead    *Left;
	Bst_NodeHead    *Right;
};

typedef struct _Bst Bst;

/* 0 : not to stop, other than 0 : to stop */
typedef int (*Bst_Enum_Callback)(Bst *t, const void *Data, void *Arg);

struct _Bst {
	PRIMEMB StableBuffer    Nodes;
	PRIMEMB int             ElementLength;
	PRIMEMB Bst_NodeHead    *Root;
	PRIMEMB Bst_NodeHead    *FreeList;
	PRIMEMB CompareFunc     Compare;

	PUBMEMB const void *(*Add)(Bst *t, const void *Data);
	PUBMEMB const void *(*Search)(Bst *t, const void *Key, const void *Last);
	PUBMEMB void (*Enum)(Bst *t, Bst_Enum_Callback cb, void *Arg);
	PUBMEMB const void *(*Minimum)(Bst *t, const void *Subtree);
    PUBMEMB const void *(*Successor)(Bst *t, const void *Last);
    PUBMEMB void (*Delete)(Bst *t, const void *Node);
    PUBMEMB void (*Reset)(Bst *t);
    PUBMEMB void (*Free)(Bst *t);
};

int Bst_Init(Bst *t, int ElementLength, CompareFunc Compare);

#endif // BST_H_INCLUDED
