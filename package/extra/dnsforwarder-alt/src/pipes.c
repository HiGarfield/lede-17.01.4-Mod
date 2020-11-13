#include "pipes.h"

#ifdef WIN32

#else

int CREATE_PIPE(PIPE_HANDLE *rh, PIPE_HANDLE *wh)
{
    int pipefd[2];

    if( pipe(pipefd) != 0 )
    {
        return -1;
    }

    *rh = pipefd[0];
    *wh = pipefd[1];

    return 0;
}

#endif /* WIN32 */
