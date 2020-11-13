#ifndef SIMPLEHT_H_INCLUDED
#define SIMPLEHT_H_INCLUDED

#include "array.h"

typedef struct _Sht_NodeHead{
	int32_t	Next;
	int		HashValue;
} Sht_NodeHead;

typedef struct _SimpleHT {
	Array	Slots;
	Array	Nodes;

	size_t	MaxLoadFactor;
	size_t	LeftSpace;

	int		(*HashFunction)(const char *, int);

} SimpleHT;

int SimpleHT_Init(SimpleHT *ht, int DataLength, size_t MaxLoadFactor, int (*HashFunction)(const char *, int));

const char *SimpleHT_Add(SimpleHT *ht, const char *Key, int KeyLength, const char *Data, int *HashValue);

const char *SimpleHT_Find(SimpleHT *ht, const char *Key, int KeyLength, int *HashValue, const char *Start);

const char *SimpleHT_Enum(SimpleHT *ht, int32_t *Start);

void SimpleHT_Free(SimpleHT *ht);

#endif // SIMPLEHT_H_INCLUDED
