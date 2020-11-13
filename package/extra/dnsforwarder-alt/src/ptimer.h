#ifndef PTIMER_H_INCLUDED
#define PTIMER_H_INCLUDED

#ifdef WIN32
    #ifdef WIN64
        #ifdef _WIN32_WINNT
            #undef _WIN32_WINNT
        #endif
        #define _WIN32_WINNT 0x0600
    #endif
    #include <windows.h>
#else
    #include <time.h>
#endif /* WIN32 */

typedef struct _PTimer PTimer;

struct _PTimer{
#ifdef WIN32
    #ifdef WIN64
    unsigned long long   c;
    #else
    unsigned long   c;
    #endif /* WIN64 */
#else
    struct timespec c;
#endif /* WIN32 */
};

int PTimer_Start(PTimer *t);

unsigned long PTimer_End(PTimer *t);

#endif // PTIMER_H_INCLUDED
