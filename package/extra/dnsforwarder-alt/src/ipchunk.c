#include <string.h>
#include "utils.h"
#include "ipchunk.h"

static int Compare(IpElement *_1, IpElement *_2)
{
	if( _1->IpLength != _2->IpLength )
	{
		return  _1->IpLength - _2->IpLength;
	} else {
		if( _1->IpLength == 4 )
		{
			return _1->Ip.Ipv4 - _2->Ip.Ipv4;
		} else {
			return memcmp(_1->Ip.Ipv6, _2->Ip.Ipv6, _1->IpLength);
		}
	}
}

int IpChunk_Init(IpChunk *ic)
{
	IpElement	Root;
	Root.IpLength = 10; /* 4 < 10 < 16 */

	if( Bst_Init(&(ic->Chunk), sizeof(IpElement), (CompareFunc)Compare) != 0 )
	{
		return -1;
	}

	if( StableBuffer_Init(&(ic->Datas)) != 0 )
	{
		return -1;
	}

	if( ic->Chunk.Add(&(ic->Chunk), &Root) == NULL )
    {
        return -37;
    }

	return 0;
}

int IpChunk_Add(IpChunk *ic,
                uint32_t Ip,
                int Type,
                const void *Data,
                uint32_t DataLength
                )
{
    StableBuffer *sb = &(ic->Datas);

	IpElement	New;
	New.IpLength = 4;
	New.Ip.Ipv4 = Ip;
	New.Type = Type;
	New.Data = NULL;

	if( Data != NULL )
	{
		New.Data = sb->Add(sb, Data, DataLength, TRUE);
	}

	return ic->Chunk.Add(&(ic->Chunk), &New) == NULL;
}

int IpChunk_AddFromString(IpChunk *ic,
                          const char *Ip,
                          int Type,
                          const char *Data,
                          uint32_t DataLength
                          )
{
    uint32_t    IpNum = 0;

    IPv4AddressToNum(Ip, &IpNum);

    return IpChunk_Add(ic, IpNum, Type, Data, DataLength);
}

int IpChunk_Add6(IpChunk *ic, const char *Ipv6, int Type, const char *Data, uint32_t DataLength)
{
    StableBuffer *sb = &(ic->Datas);

	IpElement	New;
	New.IpLength = 16;
	memcpy(New.Ip.Ipv6, Ipv6, 16);
	New.Type = Type;
	New.Data = NULL;

	if( Data != NULL )
	{
		New.Data = sb->Add(sb, Data, DataLength, TRUE);
	}

	return ic->Chunk.Add(&(ic->Chunk), &New) == NULL;
}

int IpChunk_Add6FromString(IpChunk *ic,
                           const char *Ip,
                           int Type,
                           const char *Data,
                           uint32_t DataLength
                           )
{
    char	IpNum[16];

    IPv6AddressToNum(Ip, IpNum);

    return IpChunk_Add6(ic, IpNum, Type, Data, DataLength);
}

int IpChunk_AddAnyFromString(IpChunk *ic,
                             const char *Ip,
                             int Type,
                             const char *Data,
                             uint32_t DataLength
                             )
{
    if( strchr(Ip, ':') != NULL )
    {
        return IpChunk_Add6FromString(ic, Ip, Type, Data, DataLength);
    } else {
        return IpChunk_AddFromString(ic, Ip, Type, Data, DataLength);
    }
}

BOOL IpChunk_Find(IpChunk *ic, uint32_t Ip, int *Type, const char **Data)
{
	IpElement	Key;
	const IpElement	*Result;

	if( ic == NULL )
	{
		return FALSE;
	}

	Key.IpLength = 4;
	Key.Ip.Ipv4 = Ip;
	Key.Type = 0;
	Key.Data = NULL;

	Result = ic->Chunk.Search(&(ic->Chunk), &Key, NULL);

	if( Result == NULL )
	{
		return FALSE;
	} else {
        if( Type != NULL )
        {
            *Type = Result->Type;
        }

        if( Data != NULL )
        {
            *Data = Result->Data;
        }

		return TRUE;
	}
}

BOOL IpChunk_Find6(IpChunk *ic, const char *Ipv6, int *Type, const char **Data)
{
	IpElement	Key;
	const IpElement	*Result;

	if( ic == NULL )
	{
		return FALSE;
	}

	Key.IpLength = 16;
	memcpy(Key.Ip.Ipv6, Ipv6, 16);
	Key.Type = 0;
	Key.Data = NULL;

	Result = ic->Chunk.Search(&(ic->Chunk), &Key, NULL);

	if( Result == NULL )
	{
		return FALSE;
	} else {
        if( Type != NULL )
        {
            *Type = Result->Type;
        }

        if( Data != NULL )
        {
            *Data = Result->Data;
        }

		return TRUE;
	}
}
