#ifndef _COMMON_H_
#define _COMMON_H_

#include <limits.h>
#ifdef HAVE_CONFIG_H
	#include "config.h"
#endif /* HAVE_CONFIG_H */

/* There are many differeces between Linux and Windows.
 * And we defined things here to unify interfaces,
 * but it doesn't seem to be very good. */

#ifdef WIN32 /* For Windows below. */

    #ifdef WIN64
        #ifdef _WIN32_WINNT
            #undef _WIN32_WINNT
        #endif
        #define _WIN32_WINNT 0x0600
    #endif

	#include <stdlib.h>
	#include <winsock2.h> /* fd_set, struct sockaddr_in,  */
	#include <windows.h> /* For many things */
	#include <wininet.h> /* Some internet API, include InternetOpen(), InternetOpenUrl(), etc. */
	#include <Shlwapi.h> /* PathMatchSpec() */
	#include <ws2tcpip.h> /* struct sockaddr_in6 */

	/* In Linux, the last prarmeter of 'send' is mostly MSG_NOSIGNAL(0x4000) (defined in linux headers),
	 * but in Windows, no this macro. And this prarmeter is zero, mostly.
	 * So we define this macro for Windows.
	 */

	typedef	int		socklen_t;

	/* In Windows, the indetifer of a thread is just a 'HANDLE'. */
	typedef	HANDLE	ThreadHandle;
	/* And Mutex */
	typedef	HANDLE	MutexHandle;

	/* Files */
	typedef	HANDLE	FileHandle;
	#define INVALID_FILE	((FileHandle)NULL)
	typedef	HANDLE	MappingHandle;
	#define INVALID_MAP		((MappingHandle)NULL)
	#define INVALID_MAPPING_FILE	(NULL)

    /* TCP_TIME_OUT, used as a return value */
	#define TCP_TIME_OUT	WSAETIMEDOUT

	#define GET_LAST_ERROR()	(WSAGetLastError())
	#define SET_LAST_ERROR(i)	(WSASetLastError(i))

	/* Close a socket */
	#define	CLOSE_SOCKET(s)	(closesocket(s))

	/* Threading */
	#define CREATE_THREAD(func_ptr, para_ptr, result_holder)	(result_holder) = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)(func_ptr), (para_ptr), 0, NULL);
	#define EXIT_THREAD(r)	return (r)
	#define DETACH_THREAD(t)	CloseHandle(t)

	/* Mutex */
	#define CREATE_MUTEX(m)		((m) = CreateMutex(NULL, FALSE, NULL))
	#define GET_MUTEX(m)		(WaitForSingleObject((m), INFINITE))
	#define GET_MUTEX_TRY(m)	(WaitForSingleObject((m), 0))
	#define RELEASE_MUTEX(m)	(ReleaseMutex(m))
	#define DESTROY_MUTEX(m)	(CloseHandle(m))
	#define GET_MUTEX_FAILED	WAIT_TIMEOUT /* Used as return value */

	/* CRITICAL_SECTION */
	#define CRITICAL_SECTION_INIT(c, spin_count)	(InitializeCriticalSectionAndSpinCount(&(c), (spin_count)))
	#define ENTER_CRITICAL_SECTION(c)				(EnterCriticalSection(&(c)))
	#define ENTER_CRITICAL_SECTION_TRY(c)			(TryEnterCriticalSection(&(c)))
	#define CRITICAL_SECTION_TRY_SUCCEED(ret)       ((ret) != 0)
	#define LEAVE_CRITICAL_SECTION(c)				(LeaveCriticalSection(&(c)))
	#define DELETE_CRITICAL_SECTION(c)				(DeleteCriticalSection(&(c)))

    /* File and mapping handles*/
	#define OPEN_FILE(file)			CreateFile((file), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)
	#define CREATE_FILE_MAPPING(handle, size)	CreateFileMapping((handle), NULL, PAGE_READWRITE, 0, size, NULL);
	#define MPA_FILE(handle, size)	MapViewOfFile((handle), FILE_MAP_WRITE, 0, 0, 0)

	#define UNMAP_FILE(start, size)	UnmapViewOfFile(start)
	#define DESTROY_MAPPING(handle)	CloseHandle(handle)
	#define CLOSE_FILE(handle)		CloseHandle(handle)

	#define PATH_SLASH_CH	'\\'
	#define PATH_SLASH_STR	"\\"

    /* Fill Address  */
	#define FILL_ADDR4(addr_struct, family, address_string, port)	(addr_struct).sin_family = (family); \
																	(addr_struct).sin_addr.S_un.S_addr = inet_addr(address_string); \
																	(addr_struct).sin_port = htons(port);
	/* Suspend current thread for some milliseconds */
	#define	SLEEP(i)	(Sleep(i))

	#define GET_TEMP_DIR()	getenv("TEMP")

	#define GET_THREAD_ID()	((int)GetCurrentThreadId())

	/* Wildcard match function */
	#define WILDCARD_MATCH(p, s)	PathMatchSpec((s), (p))
	#define WILDCARD_MATCHED		TRUE	/* Used as return value */

	#define	CONNECT_FUNCTION_BLOCKED	WSAEWOULDBLOCK

	typedef short	sa_family_t;

	#define ExitThisThread()    (ExitThread(0))

#else /* For Linux below */

	#include <netinet/in.h>	/* For struct 'sockaddr_in' */

	/* For function 'socket', 'bind', 'connect', 'send', 'recv',
	 * 'sendto', 'recvfrom', 'setsockopt', 'shutdown'. */
	#include <sys/socket.h>

	#include <unistd.h>		/* For function 'close' , 'sleep' */
	#include <errno.h>		/* For extern variable 'errno'. */
	#include <arpa/inet.h>	/* For function 'inet_addr'. */
	#include <pthread.h>	/* Multithread support. */

	#include <sys/types.h>	/* struct stat */
	#include <sys/stat.h>	/* stat() */

	#include <sys/mman.h>	/* mmap */
	#include <fcntl.h>

	#ifdef HAVE_SYS_SYSCALL_H
		#include <sys/syscall.h> /* syscall */
	#endif /* HAVE_SYS_SYSCALL_H */

	#include <pwd.h>	/* struct passwd */

	#include <fnmatch.h> /* fnmatch() */

	/* In Linux, the type of socket is 'int'. */
	typedef	int			SOCKET;

	/* We use pthread to implement multi threads */
	/* The indetifer of pthread is 'pthread_t'. */
	typedef	pthread_t			ThreadHandle;
	/* And mutex */
	typedef	pthread_mutex_t		MutexHandle;
	/* spin lock */
#ifdef HAVE_PTHREAD_SPIN_INIT
	typedef	pthread_spinlock_t	SpinHandle;
#else
	typedef	pthread_mutex_t		SpinHandle;
#endif

	/* There are so many HANDLEs are just ints in Linux. */
	typedef	int	FileHandle;	/* The type of return value of open() */
	#define INVALID_FILE	((FileHandle)(-1))

	typedef	int	MappingHandle;
	#define INVALID_MAP		((MappingHandle)(-1))
	#define INVALID_MAPPING_FILE	((void *)(-1))

    /* TCP_TIME_OUT, used as a return value */
	#define TCP_TIME_OUT	EAGAIN

	#define GET_LAST_ERROR()	errno
	#define SET_LAST_ERROR(i)	(errno = (i))

	/* These are defined in 'windows.h'. */
	#define	INVALID_SOCKET	((SOCKET)(~0))
	#define	SOCKET_ERROR	(-1)

	/* Close a socket */
	#define	CLOSE_SOCKET(s)	(close(s))

	/* Boolean */
	#define	BOOL	int
	#define	FALSE	0
	#define	TRUE	(!0)


#ifndef SO_DONTLINGER
	#define SO_DONTLINGER   ((unsigned int) (~SO_LINGER))
#endif

	/* pthread */
	#define CREATE_THREAD(func_ptr, para_ptr, return_value) (pthread_create(&return_value, NULL, (void *(*)())(func_ptr), (para_ptr)))
	#define EXIT_THREAD(r)	pthread_exit(r)
	#define DETACH_THREAD(t)	pthread_detach(t)

    /* mutex */
	#define CREATE_MUTEX(m)		(pthread_mutex_init(&(m), NULL))
	#define GET_MUTEX(m)		(pthread_mutex_lock(&(m)))
	#define GET_MUTEX_TRY(m)	(pthread_mutex_trylock(&(m)))
	#define RELEASE_MUTEX(m)	(pthread_mutex_unlock(&(m)))
	#define DESTROY_MUTEX(m)	(pthread_mutex_destroy(&(m)))
	#define GET_MUTEX_FAILED	(!0)

	/* spin lock */
#ifdef HAVE_PTHREAD_SPIN_INIT
	#define CREATE_SPIN(s)		(pthread_spin_init(&(s), PTHREAD_PROCESS_PRIVATE))
	#define LOCK_SPIN(s)		(pthread_spin_lock(&(s)))
	#define LOCK_SPIN_TRY(s)	(pthread_spin_trylock(&(s)))
	#define SPIN_TRY_SUCCEED(ret) ((ret) == 0)
	#define UNLOCK_SPIN(s)		(pthread_spin_unlock(&(s)))
	#define DESTROY_SPIN(s)		(pthread_spin_destroy(&(s)))
#else /*HAVE_PTHREAD_SPIN_INIT  */
	#define CREATE_SPIN(s)		(pthread_mutex_init(&(s), NULL))
	#define LOCK_SPIN(s)		(pthread_mutex_lock(&(s)))
	#define LOCK_SPIN_TRY(s)	(pthread_mutex_trylock(&(s)))
	#define SPIN_TRY_SUCCEED(ret) ((ret) == 0)
	#define UNLOCK_SPIN(s)		(pthread_mutex_unlock(&(s)))
	#define DESTROY_SPIN(s)		(pthread_mutex_destroy(&(s)))
#endif /*HAVE_PTHREAD_SPIN_INIT  */

    /* File and Mapping */
    /* In Linux, there is no a long process to map a file like Windows. */
	#define OPEN_FILE(file)						(open((file), O_RDWR | O_CREAT, S_IRWXU))
	#define CREATE_FILE_MAPPING(handle, size)	(lseek((handle), size, SEEK_SET), write((handle), "\0", 1), (handle))
	#define MPA_FILE(handle, size)				(mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, (handle), 0))
	#define UNMAP_FILE(start, size)				(munmap(start, size))
	#define DESTROY_MAPPING(handle)				/* Nothing */
	#define CLOSE_FILE(handle)					(close(handle))

	#define PATH_SLASH_CH	'/'
	#define PATH_SLASH_STR	"/"

	#define FILL_ADDR4(addr_struct, family, address_string, port)	(addr_struct).sin_family = (family); \
																	(addr_struct).sin_addr.s_addr = inet_addr(address_string); \
																	(addr_struct).sin_port = htons(port);

    /* Suspend current thread for some milliseconds */
	#define	SLEEP(i)	do \
						{ \
							int	Count = 1000; \
							do \
							{ \
								usleep(i); \
								--Count; \
							} while( Count > 0 ); \
						} while( 0 )

    /* As the name suggests */
	#define GET_TEMP_DIR()	"/tmp"

#ifdef HAVE_SYS_SYSCALL_H
	#define GET_THREAD_ID()	syscall(__NR_gettid)
#else /* HAVE_SYS_SYSCALL_H */
	#define GET_THREAD_ID()	(-1)
#endif /* HAVE_SYS_SYSCALL_H */

	#define WILDCARD_MATCH(p, s)	fnmatch((p), (s), FNM_NOESCAPE)
	#define WILDCARD_MATCHED	0

	#define	CONNECT_FUNCTION_BLOCKED	EINPROGRESS

	#define ExitThisThread()    (pthread_exit(NULL))

#endif /* WIN32 */

#ifdef WIN32
	typedef	CRITICAL_SECTION	EFFECTIVE_LOCK;
	#define EFFECTIVE_LOCK_INIT(l)		CRITICAL_SECTION_INIT((l), 1024)
	#define EFFECTIVE_LOCK_GET(l)		ENTER_CRITICAL_SECTION(l)
	#define EFFECTIVE_LOCK_TRY_GET(l)	ENTER_CRITICAL_SECTION_TRY(l)
	#define EFFECTIVE_LOCK_TRY_SUCCEED(ret) CRITICAL_SECTION_TRY_SUCCEED(ret)
	#define EFFECTIVE_LOCK_RELEASE(l)	LEAVE_CRITICAL_SECTION(l)
	#define EFFECTIVE_LOCK_DESTROY(l)	DELETE_CRITICAL_SECTION(l)
#else /* WIN32 */
	typedef	SpinHandle	EFFECTIVE_LOCK;
	#define EFFECTIVE_LOCK_INIT(l)		CREATE_SPIN(l)
	#define EFFECTIVE_LOCK_GET(l)		LOCK_SPIN(l)
	#define EFFECTIVE_LOCK_TRY_GET(l)	LOCK_SPIN_TRY(l)
	#define EFFECTIVE_LOCK_TRY_SUCCEED(ret) SPIN_TRY_SUCCEED(ret)
	#define EFFECTIVE_LOCK_RELEASE(l)	UNLOCK_SPIN(l)
	#define EFFECTIVE_LOCK_DESTROY(l)	DESTROY_SPIN(l)
#endif /* WIN32 */

#ifdef WIN32
	#define GetFileDirectory(out)	(GetModulePath(out, sizeof(out)))
#else /* WIN32 */
	#define GetFileDirectory(out)	(GetConfigDirectory(out))
#endif /* WIN32 */

#define INVALID_THREAD	((ThreadHandle)NULL)

#ifndef MSG_NOSIGNAL
#define	MSG_NOSIGNAL	0
#endif /* MSG_NOSIGNAL */

#ifndef MSG_MORE
#define	MSG_MORE	0
#endif /* MSG_MORE */

#ifndef MSG_WAITALL
#define	MSG_WAITALL	0
#endif /* MSG_WAITALL */

/* Unified interfaces end */

/* something is STILL on some state */
#define __STILL

#ifdef HAVE_STDINT_H
	#include <stdint.h>
#else
#ifndef HAVE_CONFIG_H
	#ifndef __USE_MISC

		#if (INT_MAX == 2147483647)
			#define int32_t		int
			#define uint32_t		unsigned int
			#define UINT32_T_MAX	0xFFFFFFFF
		#endif

		#if (SHRT_MAX == 32767)
			#define int16_t	short
			#define uint16_t	unsigned short
		#endif

	#endif
#endif
#endif

#ifndef HAVE_IN_PORT_T
	typedef uint16_t	in_port_t;
#endif

/* Parameters' tag */
#ifndef __in
	#define __in
#endif /* __in */

#ifndef __in_opt
	#define __in_opt
#endif /* __in_opt */

#ifndef __out
	#define __out
#endif /* __out */

#ifndef __out_opt
	#define __out_opt
#endif /* __out_opt */

#ifndef __inout
	#define __inout
#endif /* __inout */

#ifndef __inout_opt
	#define __inout_opt
#endif /* __inout_opt */

#define LENGTH_OF_IPV6_ADDRESS_ASCII	(sizeof("XXXX:XXXX:XXXX:XXXX:XXXX:XXXX:xxx.xxx.xxx.xxx"))
#define LENGTH_OF_IPV4_ADDRESS_ASCII	(sizeof("xxx.xxx.xxx.xxx"))

typedef struct _Address_Type{

	/* Union of address of IPv4 and IPv6 */
	union {
		struct sockaddr_in	Addr4;
		struct sockaddr_in6	Addr6;
	}		Addr;

	/* Although there is a `family' field in both `struct sockaddr_in' and
	 * `struct sockaddr_in6', we also add it out here.
	 */
	sa_family_t	family;

} Address_Type;

#endif /* _COMMON_H_ */
