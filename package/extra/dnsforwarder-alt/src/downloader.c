#ifndef	WIN32
#ifndef DOWNLOAD_LIBCURL
#ifndef DOWNLOAD_WGET
#ifndef NODOWNLOAD
#define NODOWNLOAD
#endif /* NODOWNLOAD */
#endif /* DOWNLOAD_WGET */
#endif /*  DOWNLOAD_LIBCURL */
#endif /* WIN32 */

#ifndef NODOWNLOAD
#ifdef WIN32
#else
#include <limits.h>
#ifdef DOWNLOAD_LIBCURL
#include <curl/curl.h>
#endif /* DOWNLOAD_LIBCURL */
#ifdef DOWNLOAD_WGET
#include <stdlib.h>
#include <sys/wait.h>
#endif /* DOWNLOAD_WGET */
#endif
#include "common.h"
#include "utils.h"
#endif /* NODOWNLOAD */

#include <string.h>
#include "downloader.h"
#include "logs.h"

int GetFromInternet_MultiFiles(const char	**URLs,
							   const char	*File,
							   int			RetryInterval,
							   int			RetryTimes,
							   void			(*ErrorCallBack)(int ErrorCode, const char *URL, const char *File),
							   void			(*SuccessCallBack)(const char *URL, const char *File)
							   )
{
	int State = FALSE;
	FILE *fp;
	char *TempFile;

	TempFile = SafeMalloc(strlen(File) + sizeof(".tmp") + 1);
	if( TempFile == NULL )
	{
		ERRORMSG("Cannot create temp file %s\n", TempFile);
		return -1;
	}

	strcpy(TempFile, File);
	strcat(TempFile, ".tmp");

	fp = fopen(TempFile, "w");
	if( fp != NULL )
	{
		fclose(fp);
	} else {
		ERRORMSG("Cannot create temp file %s\n", TempFile);
		SafeFree(TempFile);
		return -2;
	}

	while( *URLs != NULL )
	{
		State |= !GetFromInternet_SingleFile(*URLs, TempFile, TRUE, RetryInterval, RetryTimes, ErrorCallBack, SuccessCallBack);

		fp = fopen(TempFile, "a+");
		if( fp != NULL )
		{
			fputc('\n', fp);
			fclose(fp);
		} else {
			break;
		}

		++URLs;
	}

	if( State && TRUE )
	{
		remove(File);
		rename(TempFile, File);
	}

	SafeFree(TempFile);
	return !State;
}

int GetFromInternet_SingleFile(const char	*URL,
							   const char	*File,
							   BOOL			Append,
							   int			RetryInterval,
							   int			RetryTimes,
							   void			(*ErrorCallBack)(int ErrorCode, const char *URL, const char *File),
							   void			(*SuccessCallBack)(const char *URL, const char *File)
							   )
{
	int DownloadState = 0;

	if( strncmp(URL, "file", 4) == 0 )
	{
		char LocalPath[384];

		if( GetLocalPathFromURL(URL, LocalPath, sizeof(LocalPath)) == NULL )
		{
			if( ErrorCallBack != NULL )
			{
				ErrorCallBack(0, URL, File);
			}

			return -1;
		}

		if( CopyAFile(LocalPath, File, Append) != 0 )
		{
			if( ErrorCallBack != NULL )
			{
				ErrorCallBack(0, URL, File);
			}

			return -1;
		}

		if( SuccessCallBack != NULL )
		{
			SuccessCallBack(URL, File);
		}

		return 0;
	} else {
		int Ret = -1;
		char *TempFile;
		TempFile = SafeMalloc(strlen(File) + sizeof(".tmp") + 1);
		if( TempFile == NULL )
		{
			return -1;
		}
		strcpy(TempFile, File);
		strcat(TempFile, ".tmp");

		while( RetryTimes != 0 )
		{
			DownloadState = GetFromInternet_Base(URL, TempFile);
			if( DownloadState == 0 )
			{
				if( SuccessCallBack != NULL )
				{
					SuccessCallBack(URL, File);
				}

				if( CopyAFile(TempFile, File, Append) != 0 )
				{
					if( ErrorCallBack != NULL )
					{
						ErrorCallBack(0, URL, File);
					}

					Ret = -1;
					break;
				} else {
					Ret = 0;
					break;
				}
			} else {
				if( RetryTimes > 0 )
				{
					--RetryTimes;
				}

				if( ErrorCallBack != NULL )
				{
					ErrorCallBack((-1) * DownloadState, URL, File);
				}

				SLEEP(RetryInterval * 1000);
			}
		}

		remove(TempFile);
		SafeFree(TempFile);
		return Ret;
	}
}

#ifdef DOWNLOAD_LIBCURL
static size_t WriteFileCallback(void *Contents,
                                size_t Size,
                                size_t nmemb,
                                void *FileDes
                                )
{
	FILE *fp = (FILE *)FileDes;
	fwrite(Contents, Size, nmemb, fp);
	return Size * nmemb;
}
#endif /* DOWNLOAD_LIBCURL */

int GetFromInternet_Base(const char *URL, const char *File)
{
#ifndef NODOWNLOAD
#	ifdef WIN32
	FILE		*fp;
	HINTERNET	webopen		=	NULL,
				webopenurl	=	NULL;
	BOOL		ReadFlag;
	DWORD		ReadedLength;
	DWORD		TotalLength = 0;
	char		Buffer[4096];
	int			ret = -1;
	int			TimeOut = 30000;

	webopen = InternetOpen("dnsforwarder", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	if( webopen == NULL ){
		ret = -1 * GetLastError();
		InternetCloseHandle(webopen);
		return ret;
	}

	webopenurl = InternetOpenUrl(webopen, URL, NULL, 0, INTERNET_FLAG_RELOAD, 0);
	if( webopenurl == NULL ){
		ret = -1 * GetLastError();
		InternetCloseHandle(webopenurl);
		InternetCloseHandle(webopen);
		return ret;
	}

	InternetSetOption(webopenurl, INTERNET_OPTION_CONNECT_TIMEOUT, &TimeOut, sizeof(TimeOut));

	fp = fopen(File, "wb" );
	if( fp == NULL )
	{
		ret = -1 * GetLastError();
		InternetCloseHandle(webopenurl);
		InternetCloseHandle(webopen);
		return ret;
	}

	while(1)
	{
		ReadedLength = 0;
		ReadFlag = InternetReadFile(webopenurl, Buffer, sizeof(Buffer), &ReadedLength);

		if( ReadFlag == FALSE ){
			ret = -1 * GetLastError();
			InternetCloseHandle(webopenurl);
			InternetCloseHandle(webopen);
			fclose(fp);
			return ret;
		}

		if( ReadedLength == 0 )
			break;

		fwrite(Buffer, 1, ReadedLength, fp);

		TotalLength += ReadedLength;
	}

	InternetCloseHandle(webopenurl);
	InternetCloseHandle(webopen);
	fclose(fp);

	return 0;
#	else /* WIN32 */

#		ifdef DOWNLOAD_LIBCURL
	CURL *curl;
	CURLcode res;

	FILE *fp;

	fp = fopen(File, "w");
	if( fp == NULL )
	{
		return -1;
	}

	curl = curl_easy_init();
	if( curl == NULL )
	{
		fclose(fp);
		return -2;
	}

	curl_easy_setopt(curl, CURLOPT_URL, URL);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1l);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteFileCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

	res = curl_easy_perform(curl);
	if( res != CURLE_OK )
	{
		curl_easy_cleanup(curl);
		fclose(fp);
		return -3;
	} else {
		curl_easy_cleanup(curl);
		fclose(fp);
		return 0;
	}
#		endif /* DOWNLOAD_LIBCURL */
#		ifdef DOWNLOAD_WGET
	char Cmd[2048];

	sprintf(Cmd, "wget -t 2 -T 60 -q --no-check-certificate %s -O %s ", URL, File);

	return Execute(Cmd);
#		endif /* DOWNLOAD_WGET */
#	endif /* WIN32 */
#else /* NODOWNLOAD */
    WARNING("No downloader implemented.\n");
	return -1;
#endif /* NODOWNLOAD */
}
