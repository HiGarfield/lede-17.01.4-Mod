#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#ifdef WIN32
int Execute(const char *Cmd)
{
    int	ret;

    ret = system(Cmd);

    return ret;
}
#else
#include <sys/wait.h>
int Execute(const char *Cmd)
{
	int	ret;

	ret = system(Cmd);

	if( ret != -1 && WIFEXITED(ret) )
	{
		if( WEXITSTATUS(ret) == 0 )
		{
			return 0;
		}
	}

	return -1;
}
#endif /* WIN32 */

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "common.h"
#include "utils.h"
#include "dnsgenerator.h"
#include "addresslist.h"

#ifdef WIN32
	#ifdef MASKED
	#include <wincrypt.h>
	#ifndef CryptStringToBinary
		BOOL WINAPI CryptStringToBinaryA(const BYTE *,DWORD,DWORD,LPTSTR,DWORD *,DWORD *,DWORD *);
		#define	CryptStringToBinary CryptStringToBinaryA
	#endif /* CryptStringToBinary */
	#endif /* MASKED */
#else /* WIN32 */

	#ifdef MASKED
	#ifdef BASE64_DECODER_OPENSSL
		#include <openssl/bio.h>
		#include <openssl/evp.h>
	#endif /* BASE64_DECODER_OPENSSL */
	#ifdef BASE64_DECODER_UUDECODE
	#endif /* BASE64_DECODER_UUDECODE */
	#ifdef BASE64_DECODER_COREUTILS
	#endif /* BASE64_DECODER_COREUTILS */
	#endif /* MASKED */

	#ifdef HAVE_WORDEXP
		#include <wordexp.h>
	#endif

#endif /* WIN32 */


int SafeRealloc(void **Memory_ptr, size_t NewBytes)
{
	void *New;

	New = realloc(*Memory_ptr, NewBytes);

	if(New != NULL)
	{
		*Memory_ptr = New;
		return 0;
	} else {
		return -1;
	}
}

char *StrToLower(char *str)
{
	while( *str != '\0' )
	{
		*str = tolower(*str);
		++str;
	}
	return str;
}

char *BoolToYesNo(BOOL value)
{
	return value == FALSE ? "No" : "Yes";
}

int GetModulePath(char *Buffer, int BufferLength)
{
#ifdef WIN32
	int		ModuleNameLength = 0;
	char	ModuleName[320];
	char	*SlashPosition;

	if( BufferLength < 0 )
		return 0;

	ModuleNameLength = GetModuleFileName(NULL, ModuleName, sizeof(ModuleName) - 1);

	if( ModuleNameLength == 0 )
		return 0;

	SlashPosition = strrchr(ModuleName, '\\');

	if( SlashPosition == NULL )
		return 0;

	*SlashPosition = '\0';

	strncpy(Buffer, ModuleName, BufferLength - 1);
	Buffer[BufferLength - 1] = '\0';

	return strlen(Buffer);
#else
#warning Implement this
#endif
}

int GetErrorMsg(int Code, char *Buffer, int BufferLength)
{

	if( BufferLength < 0 || Buffer == NULL )
	{
		return 0;
	}

#ifdef WIN32
	return FormatMessage(	FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM,
							NULL,
							Code,
							MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
							Buffer,
							BufferLength,
							NULL);
#else
	strncpy(Buffer, strerror(Code), BufferLength - 1);
	Buffer[BufferLength - 1] ='\0';
	return strlen(Buffer);

#endif
}

char *GetCurDateAndTime(char *Buffer, int BufferLength)
{
	time_t				rawtime;
	struct tm			*timeinfo;

	*Buffer = '\0';
	*(Buffer + BufferLength - 1) = '\0';

	time(&rawtime);

	timeinfo = localtime(&rawtime);

	strftime(Buffer, BufferLength - 1 ,"%b %d %X", timeinfo);

	return Buffer;
}
#ifdef MASKED
int	Base64Decode(const char *File)
{
#ifdef WIN32
	FILE *fp = fopen(File, "rb");
	long FileSize;
	DWORD OutFileSize = 0;
	char *FileContent;
	char *ResultContent;

	if( fp == NULL )
	{
		return -1;
	}

	if( fseek(fp, 0L, SEEK_END) != 0 )
	{
		fclose(fp);
		return -2;
	}

	FileSize = ftell(fp);

	if( FileSize < 0 )
	{
		fclose(fp);
		return -3;
	}

	if( fseek(fp, 0L, SEEK_SET) != 0 )
	{
		fclose(fp);
		return -4;
	}

	FileContent = SafeMalloc(FileSize);
	if( FileContent == NULL )
	{
		fclose(fp);
		return -5;
	}

	if( fread(FileContent, 1, FileSize, fp) != FileSize )
	{
		SafeFree(FileContent);
		fclose(fp);
		return -6;
	}

	fclose(fp);

	fp = fopen(File, "wb");
	if( fp == NULL )
	{
		SafeFree(FileContent);
		return -7;
	}

	if( CryptStringToBinary((LPCSTR)FileContent, FileSize, 0x00000001, NULL, &OutFileSize, NULL, NULL) != TRUE )
	{
		SafeFree(FileContent);
		fclose(fp);
		return -8;
	}

	ResultContent = SafeMalloc(OutFileSize);
	if( ResultContent == NULL )
	{
		SafeFree(FileContent);
		fclose(fp);
		return -9;
	}


	if( CryptStringToBinary((LPCSTR)FileContent, FileSize, 0x00000001, (BYTE *)ResultContent, &OutFileSize, NULL, NULL) != TRUE )
	{
		SafeFree(ResultContent);
		SafeFree(FileContent);
		fclose(fp);
		return -9;
	}

	fwrite(ResultContent, 1, OutFileSize, fp);

	SafeFree(ResultContent);
	SafeFree(FileContent);
	fclose(fp);
	return 0;

#else /* WIN32 */
#ifdef BASE64_DECODER_OPENSSL
	BIO *ub64, *bmem;

	FILE *fp = fopen(File, "rb");
	long FileSize;
	int	OutputSize = 0;
	char *FileContent;
	char *ResultContent;

	if( fp == NULL )
	{
		return -1;
	}

	if( fseek(fp, 0L, SEEK_END) != 0 )
	{
		fclose(fp);
		return -2;
	}

	FileSize = ftell(fp);

	if( FileSize < 0 )
	{
		fclose(fp);
		return -3;
	}

	if( fseek(fp, 0L, SEEK_SET) != 0 )
	{
		fclose(fp);
		return -4;
	}

	FileContent = SafeMalloc(FileSize);
	if( FileContent == NULL )
	{
		fclose(fp);
		return -5;
	}

	if( fread(FileContent, 1, FileSize, fp) != FileSize )
	{
		SafeFree(FileContent);
		fclose(fp);
		return -6;
	}

	fclose(fp);

	ub64 = BIO_new(BIO_f_base64());
	if( ub64 == NULL )
	{
		SafeFree(FileContent);
		return -7;
	}

	bmem = BIO_new_mem_buf(FileContent, FileSize);
	if( ub64 == NULL )
	{
		SafeFree(FileContent);
		return -8;
	}

	fp = fopen(File, "wb");
	if( fp == NULL )
	{
		BIO_free_all(bmem);
		SafeFree(FileContent);
		return -9;
	}

	bmem = BIO_push(ub64, bmem);
	if( bmem== NULL )
	{
		SafeFree(FileContent);
		fclose(fp);
		return -10;
	}

	ResultContent = SafeMalloc(FileSize);
	if( ResultContent == NULL )
	{
		BIO_free_all(bmem);
		SafeFree(FileContent);
		fclose(fp);
		return -11;
	}

	OutputSize = BIO_read(bmem, ResultContent, FileSize);
	if( OutputSize < 1 )
	{
		BIO_free_all(bmem);
		SafeFree(ResultContent);
		SafeFree(FileContent);
		fclose(fp);
		return -12;
	}

	fwrite(ResultContent, 1, OutputSize, fp);

	BIO_free_all(bmem);
	SafeFree(ResultContent);
	SafeFree(FileContent);
	fclose(fp);
	return 0;
#endif /* BASE64_DECODER_OPENSSL */
#ifdef BASE64_DECODER_UUDECODE
	char Cmd[2048];
	FILE *fp;

	sprintf(Cmd, "%s.base64", File);

	fp = fopen(Cmd, "w");
	if( fp == NULL )
	{
		return -1;
	}

	fputs("begin-base64 775 \xA7\x0A", fp);

	fclose(fp);

	sprintf(Cmd, "cat %s >> %s.base64", File, File);

	if( Execute(Cmd) != 0 )
	{
		return -1;
	}

	sprintf(Cmd, "rm %s", File);

	if( Execute(Cmd) != 0 )
	{
		return -1;
	}

	sprintf(Cmd, "uudecode -o %s %s.base64", File, File);

	Execute(Cmd);

	return 0;
#endif /* BASE64_DECODER_UUDECODE */
#ifdef BASE64_DECODER_COREUTILS
	char Cmd[2048];
	FILE *fp;

	sprintf(Cmd, "%s.base64", File);

	fp = fopen(Cmd, "w");
	if( fp == NULL )
	{
		return -1;
	}

	fclose(fp);

	sprintf(Cmd, "cat %s >> %s.base64", File, File);

	if( Execute(Cmd) != 0 )
	{
		return -1;
	}

	sprintf(Cmd, "rm %s", File);

	if( Execute(Cmd) != 0 )
	{
		return -1;
	}

	sprintf(Cmd, "base64 -d %s.base64 > %s", File, File);

	Execute(Cmd);

	return 0;
#endif /* BASE64_DECODER_COREUTILS */
#endif /* WIN32 */
}
#endif /* MASKED */
int IPv6AddressToNum(const char *asc, void *Buffer)
{
	int16_t	*buf_s	=	(int16_t *)Buffer;
	const char	*itr;

	memset(Buffer, 0, 16);

	for(; isspace(*asc); ++asc);

	if( strstr(asc, "::") == NULL )
	{	/* full format */
		int a[8];
		sscanf(asc, "%x:%x:%x:%x:%x:%x:%x:%x",
				a, a + 1, a + 2, a + 3, a + 4, a + 5, a + 6, a + 7
				);
		SET_16_BIT_U_INT(buf_s, a[0]);
		SET_16_BIT_U_INT(buf_s + 1, a[1]);
		SET_16_BIT_U_INT(buf_s + 2, a[2]);
		SET_16_BIT_U_INT(buf_s + 3, a[3]);
		SET_16_BIT_U_INT(buf_s + 4, a[4]);
		SET_16_BIT_U_INT(buf_s + 5, a[5]);
		SET_16_BIT_U_INT(buf_s + 6, a[6]);
		SET_16_BIT_U_INT(buf_s + 7, a[7]);
	} else {
		/* not full*/

		if( asc[2] == '\0' || isspace(asc[2]) )
		{
			memset(Buffer, 0, 16);
			return 0;
		}

		while(1)
		{
			int a;
			itr = asc;
			asc = strchr(asc, ':');
			if( asc == NULL )
				return 0;

			if( itr == asc )
			{
				break;
			}

			sscanf(itr, "%x:", &a);
			SET_16_BIT_U_INT(buf_s, a);
			++buf_s;
			++asc;
		}
		buf_s = (int16_t *)Buffer + 7;
		for(; *asc != '\0'; ++asc);
		while(1)
		{
			int a;
			for(itr = asc; *itr != ':'; --itr);

			if( *(itr + 1) == '\0' )
				break;

			sscanf(itr + 1, "%x", &a);
			SET_16_BIT_U_INT(buf_s, a);
			--buf_s;
			asc = itr - 1;

			if( *(itr - 1) == ':' )
				break;
		}
	}
	return 0;
}

int IPv4AddressToNum(const char *asc, void *Buffer)
{
	int ret = 0;

	unsigned char *BufferInByte = (unsigned char *)Buffer;

	int Components[4];

	ret = sscanf(asc, "%d.%d.%d.%d", Components, Components + 1, Components + 2, Components + 3);
	BufferInByte[0] = Components[0];
	BufferInByte[1] = Components[1];
	BufferInByte[2] = Components[2];
	BufferInByte[3] = Components[3];

	return ret;
}

sa_family_t GetAddressFamily(const char *Addr)
{
	char Buffer[8];

	if( strchr(Addr, '[') != NULL )
	{
		return AF_INET6;
	}

	if( IPv4AddressToNum(Addr, Buffer) == 4 )
	{
		return AF_INET;
	}

	return AF_UNSPEC;
}

int IPv6AddressToAsc(const void *Address, void *Buffer)
{
	sprintf((char *)Buffer, "%x:%x:%x:%x:%x:%x:%x:%x",
		GET_16_BIT_U_INT((const char *)Address),
		GET_16_BIT_U_INT((const char *)Address + 2),
		GET_16_BIT_U_INT((const char *)Address + 4),
		GET_16_BIT_U_INT((const char *)Address + 6),
		GET_16_BIT_U_INT((const char *)Address + 8),
		GET_16_BIT_U_INT((const char *)Address + 10),
		GET_16_BIT_U_INT((const char *)Address + 12),
		GET_16_BIT_U_INT((const char *)Address + 14)

	);

	return 0;
}

int IPv4AddressToAsc(const void *Address, void *Buffer)
{
    const char  *a = (const char *)Address;

    sprintf(Buffer, "%u.%u.%u.%u",	GET_8_BIT_U_INT(a),
                                    GET_8_BIT_U_INT(a + 1),
                                    GET_8_BIT_U_INT(a + 2),
                                    GET_8_BIT_U_INT(a + 3)
        );

	return 0;
}

int	GetConfigDirectory(char *out)
{
#ifdef WIN32
	return -1;
#else /* WIN32 */
#ifndef ANDROID
	struct passwd *pw = getpwuid(getuid());
	char *home = pw->pw_dir;
	*out = '\0';
	if( home == NULL )
		return 1;

	strcpy(out, home);
	strcat(out, "/.dnsforwarder");

	return 0;
#else /* ANDROID */
	strcpy(out, "/system/root/.dnsforwarder");
#endif /* ANDROID */
#endif /* WIN32 */
}

BOOL FileIsReadable(const char *File)
{
	FILE *fp = fopen(File, "r");

	if( fp == NULL )
	{
		return FALSE;
	} else {
		fclose(fp);
		return TRUE;
	}
}

int GetFileSizePortable(const char *File)
{
	int s = 0;
	FILE *fp = fopen(File, "rb");

	if( fp == NULL )
	{
		return 0;
	}

	if( fseek(fp, 0, SEEK_END) == 0 )
	{
		s = ftell(fp);
	}

	fclose(fp);
	return s;
}

int GetTextFileContent(const char *File, char *Content)
{
	FILE *fp = fopen(File, "rb");
	int	c = 0;

	if( fp == NULL )
	{
		return -1;
	}

	while( (c = fgetc(fp)) != EOF )
	{
		*Content++ = c;
	}

	fclose(fp);

	return 0;
}

BOOL IsPrime(int n)
{
	int i;

	if( n < 2 )
	{
		return FALSE;
	}

	if( n == 2 )
	{
		return TRUE;
	}

	if( n % 2 == 0 )
	{
		return FALSE;
	}

	for(i = 3; i < sqrt(n) + 1; i += 2)
	{
		if( n % i == 0 )
		{
			return FALSE;
		}
	}

	return TRUE;
}

int FindNextPrime(int Current)
{
	if( IsPrime(Current) )
	{
		return Current;
	}

	Current = ROUND_UP(Current, 2) + 1;

	do
	{
		if( IsPrime(Current) )
		{
			return Current;
		} else {
			Current += 2;
		}

	} while( TRUE );
}

BOOL ContainWildCard(const char *item)
{
	if( strchr(item, '?') != NULL || strchr(item, '*') != NULL )
	{
		return TRUE;
	} else {
		return FALSE;
	}
}

int ELFHash(const char *str, int Unused)
{
	uint32_t h = 0;
	uint32_t x = 0;

	while( *str != '\0' )
	{
		h += *str;
		h <<= 4;

		x = h & 0xF0000000;
		if( x != 0 )
		{
			h ^= (x >> 24);

		}
		h &= ~x;
		str++;
	}
	return (h & 0x7FFFFFFF);
}

void HexDump(const char *Data, int Length)
{
	int Itr;

	for( Itr = 0; Itr != Length; ++Itr )
	{
		printf("%x ", (unsigned char)Data[Itr]);
	}

	putchar('\n');
}

char *BinaryOutput(const char *Origin, int OriginLength, char *Buffer)
{
	int loop;

	while( OriginLength != 0 )
	{
		for( loop = 7; loop <= 0; --loop )
		{
			if( (((int)*Origin) & (1 << loop)) == 0 )
			{
				*Buffer = '0';
			} else {
				*Buffer = '1';
			}

			++Buffer;
		}

		--OriginLength;
		++Origin;
	}

	*Buffer = '\0';
	return Buffer + 1;
}

char *StringDup(const char *Str)
{
	char *New;

	if( Str == NULL )
	{
		return NULL;
	}

	New = malloc(strlen(Str) + 1);
	if( New != NULL )
	{
		strcpy(New, Str);
	}

	return New;
}

char *StrNpbrk(char *Str, const char *Ch)
{
	if( Str == NULL || Ch == NULL )
	{
		return Str;
	}

	while( *Str != '\0' && strchr(Ch, *Str) != NULL )
	{
		++Str;
	}

	if( *Str == '\0' )
	{
		return NULL;
	} else {
		return Str;
	}

}

char *StrRNpbrk(char *Str, const char *Ch)
{
	char *LastCharacter;

	if( Str == NULL || Ch == NULL )
	{
		return Str;
	}

	LastCharacter = Str + strlen(Str) - 1;

	while( LastCharacter >= Str && strchr(Ch, *LastCharacter) != NULL )
	{
		--LastCharacter;
	}

	if( LastCharacter <  Str )
	{
		return NULL;
	} else {
		return LastCharacter;
	}

}

char *GoToNextNonSpace(const char *Here)
{
	return (char *)StrNpbrk((char *)Here, "\t ");
}

char *GoToPrevNonSpace(char *Here)
{
	for( ; isspace(*Here); --Here );

	return Here;
}

int GetAddressLength(sa_family_t Family)
{
	switch( Family )
	{
		case AF_INET:
			return sizeof(struct sockaddr);
			break;

		case AF_INET6:
			return sizeof(struct sockaddr_in6);
			break;

		default:
			return -1;
			break;
	}
}

int SetProgramEnvironment(const char *Name, const char *Value)
{
#ifdef WIN32
	return !SetEnvironmentVariable(Name, Value);
#else
#ifdef HAVE_SETENV
	return setenv(Name, Value, 1);
#else
	return 0;
#endif
#endif
}

int ExpandPath(char *String, int BufferLength)
{
#ifdef WIN32
	char	TempStr[2048];
	int		State;

	State = ExpandEnvironmentStrings(String, TempStr, sizeof(TempStr) - 1);

	if( State == 0 || State >= sizeof(TempStr) - 1 )
	{
		return -1;
	}

	TempStr[sizeof(TempStr) - 1] = '\0';

	if( strlen(TempStr) + 1 > BufferLength )
	{
		return -1;
	}

	strcpy(String, TempStr);

	return 0;
#else
#ifdef HAVE_WORDEXP
	wordexp_t Result;

	if( wordexp(String, &Result, 0) != 0 )
	{
		wordfree(&Result);
		return -1;
	}

	if( strlen(Result.we_wordv[0]) + 1 <= BufferLength )
	{
		strcpy(String, Result.we_wordv[0]);
	}

	wordfree(&Result);
	return 0;
#else
	return 0;
#endif
#endif
}

char *GetLocalPathFromURL(const char *URL, char *Buffer, int BufferLength)
{
	const char *Itr;
#ifdef WIN32
	char *Itr_Buffer;
#endif

	Itr = strstr(URL, "://");
	if( Itr == NULL )
	{
		return NULL;
	}

	++Itr;
	for( ; *Itr == '/'; ++Itr );

#ifndef WIN32
	--Itr;
#endif

	if( strlen(Itr) + 1 > BufferLength )
	{
		return NULL;
	}

	strcpy(Buffer, Itr);

#ifdef WIN32
	for( Itr_Buffer = Buffer; *Itr_Buffer != '\0'; ++Itr_Buffer )
	{
		if( *Itr_Buffer == '/' )
		{
			*Itr_Buffer = '\\';
		}
	}
#endif

	if( ExpandPath(Buffer, BufferLength) == 0 )
	{
		return Buffer;
	} else {
		return NULL;
	}

}

int CopyAFile(const char *Src, const char *Dst, BOOL Append)
{
	FILE *Src_Fp, *Dst_Fp;
	int ch;

	Src_Fp = fopen(Src, "r");
	if( Src_Fp == NULL )
	{
		return -1;
	}

	Dst_Fp = fopen(Dst, Append == TRUE ? "a+" : "w");
	if( Dst_Fp == NULL )
	{
		fclose(Src_Fp);
		return -2;
	}

	do{
		ch = fgetc(Src_Fp);
		if( ch != EOF && !feof(Src_Fp) )
		{
			fputc(ch, Dst_Fp);
		} else {
			break;
		}

	} while( TRUE );

	fclose(Src_Fp);
	fclose(Dst_Fp);

	return 0;
}

int FatalErrorDecideding(int LastError)
{
#ifdef WIN32
	if( LastError == WSAEWOULDBLOCK || LastError == WSAEINVAL || LastError == WSAEINTR || LastError == WSAEINPROGRESS )
	{
		return 0;
	}
#else
	if( LastError == EINTR || LastError == EAGAIN || LastError == EINPROGRESS )
	{
		return 0;
	}
#endif

	return -1;
}

int CountSubStr(const char *Src, const char *SubStr)
{
    int ret = 0;
    int SubStrLen = strlen(SubStr);
    const char *Itr;

    Itr = strstr(Src, SubStr);
    while( Itr != NULL )
    {
        ++ret;

        Itr = strstr(Itr + SubStrLen, SubStr);
    }

    return ret;
}

char *ReplaceStr(char *Src, const char *OriSubstr, const char *DesSubstr)
{
    int DesLen = strlen(DesSubstr);
    int OriLen = strlen(OriSubstr);
    //int Difference = DesLen - OriLen;

    char *Itr;

    Itr = strstr(Src, OriSubstr);
    while( Itr != NULL )
    {
        memmove(Itr + DesLen, Itr + OriLen, strlen(Itr + OriLen) + 1);

        memcpy(Itr, DesSubstr, DesLen);

        Itr = strstr(Itr + DesLen, OriSubstr);
    }

    return Src;
}

char *ReplaceStr_WithLengthChecking(char *Src,
                                    const char *OriSubstr,
                                    const char *DesSubstr,
                                    int SrcBufferLength
                                    )
{
    int TotalSpaceNeeded = TOTAL_SPACE_NEEDED(Src,
                                              strlen(OriSubstr),
                                              strlen(DesSubstr),
                                              CountSubStr(Src, OriSubstr)
                                              );

    if( TotalSpaceNeeded > SrcBufferLength )
    {
        return NULL;
    } else {
        return ReplaceStr(Src, OriSubstr, DesSubstr);
    }
}

int SetSocketNonBlock(SOCKET sock, BOOL NonBlocked)
{
#ifdef WIN32
	unsigned long NonBlock = 1;

	if( ioctlsocket(sock, FIONBIO, &NonBlock) != 0 )
	{
		return -1;
	} else {
		return 0;
	}
#else
	int Flags;
	int BlockFlag;

	Flags = fcntl(sock, F_GETFL, 0);
	if( Flags < 0 )
	{
		return -1;
	}

	if( NonBlocked == TRUE )
	{
        BlockFlag = O_NONBLOCK;
	} else {
        BlockFlag = ~O_NONBLOCK;
	}

	if( fcntl(sock, F_SETFL, Flags | BlockFlag) < 0 )
	{
		return -1;
	}

	return 0;
#endif
}

BOOL SocketIsWritable(SOCKET sock, int Timeout)
{
	struct timeval TimeLimit = {Timeout / 1000, (Timeout % 1000) * 1000};
	fd_set rfd;

	if( sock == INVALID_SOCKET )
	{
		return FALSE;
	}

	FD_ZERO(&rfd);
	FD_SET(sock, &rfd);

	switch(select(sock + 1, NULL, &rfd, NULL, &TimeLimit))
	{
		case 0:
		case SOCKET_ERROR:
			return FALSE;
			break;

		default:
			return TRUE;
			break;
	}
}

BOOL SocketIsStillReadable(SOCKET Sock, int timeout)
{
	fd_set rfd;
	struct timeval TimeLimit = {timeout / 1000, (timeout % 1000) * 1000};

	FD_ZERO(&rfd);
	FD_SET(Sock, &rfd);

	switch(select(Sock + 1, &rfd, NULL, NULL, &TimeLimit))
	{
		case SOCKET_ERROR:
		case 0:
			return FALSE;
			break;
		case 1:
			return TRUE;
			break;
		default:
			return FALSE;
			break;
	}
}

void ClearTCPSocketBuffer(SOCKET Sock, int Length)
{
	char BlackHole[128];

	while( Length > 0 )
	{
	    int UnitLength;

	    UnitLength = recv(Sock,
                          BlackHole,
                          sizeof(BlackHole) < Length ? sizeof(BlackHole) : Length,
                          0
                          );

        if( UnitLength <= 0 )
        {
            return;
        }

		Length -= UnitLength;
	}
}

SOCKET TryBindLocal(BOOL Ipv6, int StartPort, Address_Type *Address)
{
    const char *Loopback = Ipv6 ? "[::1]" : "127.0.0.1";

	int MaxTime = 10000;

	Address_Type Address1;
	SOCKET ret = INVALID_SOCKET;

	do {
		AddressList_ConvertFromString(&Address1, Loopback, StartPort);

		ret = socket(Address1.family, SOCK_DGRAM, IPPROTO_UDP);
		if( ret == INVALID_SOCKET )
        {
            continue;
        }

        if( bind(ret,
                 (struct sockaddr *)&(Address1.Addr),
                 GetAddressLength(Address1.family)
                 )
           != 0 )
        {
            CLOSE_SOCKET(ret);
            ret = INVALID_SOCKET;
            continue;
        }

	} while( ret == INVALID_SOCKET && --MaxTime > 0 && ++StartPort > 0 );

	if( ret != INVALID_SOCKET && Address != NULL )
    {
        memcpy(Address, &Address1, sizeof(Address_Type));
    }

	return ret;
}

char *SplitNameAndValue(char *Line, const char *Delimiters)
{
	char *Delimiter = strpbrk(Line, Delimiters);

	if( Delimiter == NULL )
	{
		return NULL;
	}

	*Delimiter = '\0';

	return GoToNextNonSpace(Delimiter + 1);
}

char *GetPathPart(char *FullPath)
{
    char *SlashPos;

    SlashPos = strrchr(FullPath, PATH_SLASH_CH);

    if( SlashPos == NULL )
    {
        return NULL;
    }

    *(SlashPos + 1) = '\0';

    return FullPath;
}
