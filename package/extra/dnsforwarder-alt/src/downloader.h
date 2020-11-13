#ifndef DOWNLOADER_H_INCLUDED
#define DOWNLOADER_H_INCLUDED

#include "common.h"

int GetFromInternet_MultiFiles(const char	**URLs,
							   const char	*File,
							   int			RetryInterval,
							   int			RetryTimes,
							   void			(*ErrorCallBack)(int ErrorCode, const char *URL, const char *File),
							   void			(*SuccessCallBack)(const char *URL, const char *File)
							   );

int GetFromInternet_SingleFile(const char	*URL,
							   const char	*File,
							   BOOL			Append,
							   int			RetryInterval,
							   int			RetryTimes,
							   void			(*ErrorCallBack)(int ErrorCode, const char *URL, const char *File),
							   void			(*SuccessCallBack)(const char *URL, const char *File)
							   );

int GetFromInternet_Base(const char *URL, const char *File);

#endif // DOWNLOADER_H_INCLUDED

