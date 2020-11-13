#include <ctype.h>
#include <string.h>
#include "common.h"
#include "readline.h"

static BOOL ClearAnnotation(char *str, char mark)
{
	char *pos = strchr(str, mark);

	if( pos != NULL )
	{
		for(--pos; pos >= str && isspace(*pos); --pos);
		*(pos + 1) = '\0';
		return TRUE;
	} else {
		return FALSE;
	}
}

static BOOL ReachedLineEnd(FILE *fp, char *str)
{
	int len = strlen(str);

	if( len != 0 && (str[len - 1] == '\r' || str[len - 1] == '\n') )
	{
		return TRUE;
	} else {
		if( feof(fp) )
		{
			return TRUE;
		} else {
			return FALSE;
		}
	}
}

static void EliminateCRLF(char *str)
{
	ClearAnnotation(str, '\r');
	ClearAnnotation(str, '\n');
}

static void EliminateHeadSpace(char *str)
{
	char *Home;

	for(Home = str; isspace(*Home); ++Home);
	if( Home != str )
	{
		memmove(str, Home, strlen(Home) + 1);
	}

}

static void EliminateFootSpace(char *str)
{
	char *End = str + strlen(str) - 1;

	if( End >= str )
	{
		for(; isspace(*End) && End >= str; --End);
		*(End + 1) = '\0';
	}

}

ReadLineStatus ReadLine(FILE *fp, char *Buffer, int BufferSize)
{
	BOOL	ReachedEnd;

START:
	ReachedEnd = TRUE;

	if( fgets(Buffer, BufferSize, fp) == NULL )
	{
		return READ_FAILED_OR_END;
	} else {
		ReachedEnd = ReachedLineEnd(fp, Buffer);
	}

	if( (ClearAnnotation(Buffer, '#') || ClearAnnotation(Buffer, ';')) != FALSE )
	{
		if( ReachedEnd == FALSE )
		{
			char BlackHole[128];
			do
			{
				fgets(BlackHole, sizeof(BlackHole), fp);
				ReachedEnd = ReachedLineEnd(fp, BlackHole);
			}while( ReachedEnd == FALSE );
		}

		EliminateFootSpace(Buffer);
		EliminateHeadSpace(Buffer);

		if( *Buffer == '\0' )
		{
			goto START;
		} else {
			return READ_DONE;
		}

	}

	if( ReachedEnd == TRUE )
	{
		EliminateCRLF(Buffer);
		EliminateFootSpace(Buffer);
		EliminateHeadSpace(Buffer);
		if( *Buffer == '\0' )
		{
			goto START;
		} else {
			return READ_DONE;
		}
	} else {
		return READ_TRUNCATED;
	}
}

ReadLineStatus ReadLine_GoToNextLine(FILE *fp)
{
	ReadLineStatus Status;
	char Buffer[128];

	do
	{
		Status = ReadLine(fp, Buffer, sizeof(Buffer));
	}
	while( Status == READ_TRUNCATED );

	return Status;
}
