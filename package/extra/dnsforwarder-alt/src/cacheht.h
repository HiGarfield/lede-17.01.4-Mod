#ifndef HASHTABLE_H_INCLUDED
#define HASHTABLE_H_INCLUDED

#include <time.h>
#include "array.h"

typedef struct _Cht_Node{
	int32_t		Slot;
	int32_t		Next;
	int32_t		Offset;
	uint32_t	TTL;
	time_t		TimeAdded;
	uint32_t	Length;
} Cht_Node;

typedef struct _HashTable{
	Array	NodeChunk;
	Array	Slots;
	int32_t	FreeList;
}CacheHT;

int CacheHT_Init(CacheHT *h, char *BaseAddr, int CacheSize);

int CacheHT_ReInit(CacheHT *h, char *BaseAddr, int CacheSize);

int32_t CacheHT_FindUnusedNode(CacheHT		*h,
								uint32_t	ChunkSize,
								Cht_Node	**Out,
								void		*Boundary,
								BOOL		*NewCreated
								);

int CacheHT_InsertToSlot(CacheHT	*h,
						 const char	*Key,
						 int		Node_index,
						 Cht_Node	*Node,
						 int		*HashValue
						 );

int CacheHT_RemoveFromSlot(CacheHT *h, int32_t SubScriptOfNode, Cht_Node *Node);

Cht_Node *CacheHT_Get(CacheHT *h, const char *Key, Cht_Node *Start, int *HashValue);

void CacheHT_Free(CacheHT *h);

#endif // HASHTABLE_H_INCLUDED
