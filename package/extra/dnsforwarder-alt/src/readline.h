#ifndef READLINE_H_INCLUDED
#define READLINE_H_INCLUDED

#include <stdio.h>

typedef enum _ReadLineStatus
{
	READ_FAILED_OR_END	=	-1,
	READ_DONE	=	0,
	READ_TRUNCATED,
} ReadLineStatus;

ReadLineStatus ReadLine(FILE *fp, char *Buffer, int BufferSize);

ReadLineStatus ReadLine_GoToNextLine(FILE *fp);

#endif // READLINE_H_INCLUDED
