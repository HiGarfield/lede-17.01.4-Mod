#ifndef RWLOCK_H_INCLUDED
#define RWLOCK_H_INCLUDED

#include "common.h"

#ifdef WIN32
	#ifdef WIN64
    /*
		#ifndef InitializeSRWLock
		typedef struct _RWLock {
			PVOID Ptr;
		} RWLock;
		#else
		typedef SRWLOCK RWLock;
		#endif
    */
        typedef SRWLOCK RWLock;
	#else /* WIN64 */
		typedef CRITICAL_SECTION RWLock;
	#endif /* WIN64 */
#else /* WIN32 */

#ifdef HAVE_PTHREAD_RWLOCK_INIT
	typedef pthread_rwlock_t  RWLock;
#else /* HAVE_PTHREAD_RWLOCK_INIT */
	typedef pthread_mutex_t  RWLock;
#endif /* HAVE_PTHREAD_RWLOCK_INIT */

#endif /* WIN32 */

#ifdef WIN32

	#ifdef WIN64

        /*
		#ifndef InitializeSRWLock

		VOID WINAPI InitializeSRWLock(RWLock *SRWLock);
		VOID WINAPI AcquireSRWLockShared(RWLock *SRWLock);
		VOID WINAPI AcquireSRWLockExclusive(RWLock *SRWLock);
		VOID WINAPI ReleaseSRWLockShared(RWLock *SRWLock);
		VOID WINAPI ReleaseSRWLockExclusive(RWLock *SRWLock);

		#endif
		*/

		#define	RWLock_Init(l)		InitializeSRWLock(&(l))
		#define RWLock_RdLock(l)	AcquireSRWLockShared(&(l))
		#define RWLock_WrLock(l)	AcquireSRWLockExclusive(&(l))
		#define RWLock_UnRLock(l)	ReleaseSRWLockShared(&(l))
		#define RWLock_UnWLock(l)	ReleaseSRWLockExclusive(&(l))
		#define RWLock_Destroy(l)

	#else /* WIN64 */

		#define	RWLock_Init(l)		CRITICAL_SECTION_INIT((l), 1024)
		#define RWLock_RdLock(l)	ENTER_CRITICAL_SECTION(l)
		#define RWLock_WrLock(l)	ENTER_CRITICAL_SECTION(l)
		#define RWLock_UnRLock(l)	LEAVE_CRITICAL_SECTION(l)
		#define RWLock_UnWLock(l)	LEAVE_CRITICAL_SECTION(l)
		#define RWLock_Destroy(l)	DELETE_CRITICAL_SECTION(l)

	#endif /* WIN64 */
#else /* WIN32 */

	#ifdef HAVE_PTHREAD_RWLOCK_INIT
		#define RWLock_Init(l)		pthread_rwlock_init(&(l), NULL)
		#define RWLock_RdLock(l)	pthread_rwlock_rdlock(&(l))
		#define RWLock_WrLock(l)	pthread_rwlock_wrlock(&(l))
		#define RWLock_UnRLock(l)	pthread_rwlock_unlock(&(l))
		#define RWLock_UnWLock(l)	pthread_rwlock_unlock(&(l))
		#define RWLock_Destroy(l)	pthread_rwlock_destroy(&(l))
	#else
		#define RWLock_Init(l)		(pthread_mutex_init(&(l), NULL))
		#define RWLock_RdLock(l)	(pthread_mutex_lock(&(l)))
		#define RWLock_WrLock(l)	(pthread_mutex_lock(&(l)))
		#define RWLock_UnRLock(l)	(pthread_mutex_unlock(&(l)))
		#define RWLock_UnWLock(l)	(pthread_mutex_unlock(&(l)))
		#define RWLock_Destroy(l)	(pthread_mutex_destroy(&(l)))
	#endif
#endif /* WIN32 */

#endif // RWLOCK_H_INCLUDED
