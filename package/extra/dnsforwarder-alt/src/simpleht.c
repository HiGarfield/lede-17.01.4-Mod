#include <string.h>
#include "simpleht.h"

typedef struct _Sht_Slot{
	int32_t	Next;
} Sht_Slot;

static const Sht_Slot EmptySlot = {-1};

int SimpleHT_Init(SimpleHT *ht, int DataLength, size_t MaxLoadFactor, int (*HashFunction)(const char *, int))
{
	int loop;

    if( Array_Init(&(ht->Slots), sizeof(Sht_Slot), 7, FALSE, NULL) != 0 )
    {
		return -1;
    }

	for( loop = 0; loop != 7; ++loop )
	{
		Array_PushBack(&(ht->Slots), &EmptySlot, NULL);
	}

    if( Array_Init(&(ht->Nodes), sizeof(Sht_NodeHead) + DataLength, 0, FALSE, NULL) != 0 )
    {
		return -2;
    }

	ht->MaxLoadFactor = MaxLoadFactor;
	ht->LeftSpace = 7 * MaxLoadFactor;
	ht->HashFunction = HashFunction;

	return 0;
}

static Sht_NodeHead *SimpleHT_RemoveFromSlot(SimpleHT *ht, int Slot, int *NodeSubscript)
{
	int NumberOfSlots_New = Array_GetUsed(&(ht->Slots));

	Sht_Slot *TheSlot;
	Sht_NodeHead *FirstNode = NULL;
	Sht_NodeHead *SecondNode = NULL;

	TheSlot = Array_GetBySubscript(&(ht->Slots), Slot);
	if( TheSlot == NULL )
	{
		return NULL;
	}

	FirstNode = Array_GetBySubscript(&(ht->Nodes), TheSlot->Next);
	if( FirstNode == NULL )
	{
		return NULL;
	}

	if( FirstNode->HashValue % NumberOfSlots_New != Slot )
	{
		*NodeSubscript = TheSlot->Next;
		TheSlot->Next = FirstNode->Next;
		return FirstNode;
	} else {
		*NodeSubscript = FirstNode->Next;
		SecondNode = Array_GetBySubscript(&(ht->Nodes), FirstNode->Next);
		while( SecondNode != NULL )
		{
			if( SecondNode->HashValue % NumberOfSlots_New != Slot )
			{
				FirstNode->Next = SecondNode->Next;
				return SecondNode;
			}

			FirstNode = SecondNode;
			*NodeSubscript = SecondNode->Next;
			SecondNode = Array_GetBySubscript(&(ht->Nodes), SecondNode->Next);
		}
		return NULL;
	}
}

static int SimpleHT_AddToSlot(SimpleHT *ht, Sht_NodeHead *Node, int NodeSubscript)
{
	int NumberOfSlots = Array_GetUsed(&(ht->Slots));
	Sht_Slot *TheSlot;

	TheSlot = Array_GetBySubscript(&(ht->Slots), Node->HashValue % NumberOfSlots);
	if( TheSlot == NULL )
	{
		return -1;
	}

	Node->Next = TheSlot->Next;
	TheSlot->Next = NodeSubscript;

	return 0;
}

static int SimpleHT_Expand(SimpleHT *ht)
{
	int NumberOfSlots_Old = Array_GetUsed(&(ht->Slots));
	Sht_NodeHead *Itr = NULL;
	int ItrSubscript = 0;
	int loop;

	for( loop = 0; loop < NumberOfSlots_Old; ++loop )
	{
		if( Array_PushBack(&(ht->Slots), &EmptySlot, NULL) < 0 )
		{
			return -1;
		}
	}

	for( loop = 0; loop < NumberOfSlots_Old; ++loop )
	{
		Itr = SimpleHT_RemoveFromSlot(ht, loop, &ItrSubscript);
		while( Itr != NULL )
		{
			SimpleHT_AddToSlot(ht, Itr, ItrSubscript);
			Itr = SimpleHT_RemoveFromSlot(ht, loop, &ItrSubscript);
		}
	}

	return 0;
}

const char *SimpleHT_Add(SimpleHT *ht, const char *Key, int KeyLength, const char *Data, int *HashValue)
{
	Sht_NodeHead *New;
	int	NewSubscript;

	if( ht->LeftSpace == 0 )
	{
		int NumberOfSlots_Old = Array_GetUsed(&(ht->Slots));

		if( SimpleHT_Expand(ht) != 0 )
		{
			return NULL;
		}

		ht->LeftSpace = NumberOfSlots_Old * ht->MaxLoadFactor;
	}

	NewSubscript = Array_PushBack(&(ht->Nodes), NULL, NULL);
	if( NewSubscript < 0 )
	{
		return NULL;
	}

	New = Array_GetBySubscript(&(ht->Nodes), NewSubscript);

	if( HashValue == NULL )
	{
		New->HashValue = (ht->HashFunction)(Key, KeyLength);
	} else {
		New->HashValue = *HashValue;
	}

	memcpy(New + 1, Data, ht->Nodes.DataLength - sizeof(Sht_NodeHead));

	SimpleHT_AddToSlot(ht, New, NewSubscript);

	--(ht->LeftSpace);

	return (const char *)(New + 1);
}

const char *SimpleHT_Find(SimpleHT *ht, const char *Key, int KeyLength, int *HashValue, const char *Start)
{
	int NumberOfSlots = Array_GetUsed(&(ht->Slots));
	int SlotNumber;
	Sht_Slot *TheSlot;
	Sht_NodeHead *Node;

	if( NumberOfSlots <= 0 )
	{
		return NULL;
	}

	if( Start != NULL )
	{
		Node = Array_GetBySubscript(&(ht->Nodes), (((Sht_NodeHead *)Start) - 1)->Next);
	} else {
		if( HashValue == NULL )
		{
			SlotNumber = (ht->HashFunction)(Key, KeyLength) % NumberOfSlots;
		} else {
			SlotNumber = (*HashValue) % NumberOfSlots;
		}

		TheSlot = Array_GetBySubscript(&(ht->Slots), SlotNumber);
		if( TheSlot == NULL )
		{
			return NULL;
		}

		Node = Array_GetBySubscript(&(ht->Nodes), TheSlot->Next);
	}

	if( Node == NULL )
	{
		return NULL;
	}

	return (const char *)(Node + 1);

}

const char *SimpleHT_Enum(SimpleHT *ht, int32_t *Start)
{
	Array *Nodes = &(ht->Nodes);
	Sht_NodeHead *Node;

	Node = Array_GetBySubscript(Nodes, *Start);

	if( Node != NULL )
	{
		++(*Start);
		return (const char *)(Node + 1);
	} else {
		return NULL;
	}
}

void SimpleHT_Free(SimpleHT *ht)
{
	Array_Free(&(ht->Slots));
	Array_Free(&(ht->Nodes));
}
