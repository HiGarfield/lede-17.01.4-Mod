#include <stdio.h>


#ifdef WIN32
#include <windows.h>

int main(void)
{
    HANDLE rh,wh;

    BOOL s;

    char w[] = "qwerqwermqweioruoweurcxqmor9q24589q3m4ytiwox1111111";
    char r[1024];

    s = CreatePipe(&rh, &wh, NULL, 0);

    WriteFile(wh, w, sizeof(w)-1, NULL, NULL);
    WriteFile(wh, w, sizeof(w), NULL, NULL);

    ReadFile(rh, r, sizeof(r), NULL, NULL);

    printf("%s\n", r);
}


#else

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

int main(void)
{
    int fd[2];
    char w[] = "qwerqwermqweioruoweurcxqmor9q24589q3m4ytiwox1111111";
    char r[1024];

    pipe(fd);

    write(fd[1], w, sizeof(w)-1);
    write(fd[1], w, sizeof(w)-1);
    write(fd[1], w, sizeof(w)-1);
    write(fd[1], w, sizeof(w)-1);
    write(fd[1], w, sizeof(w)-1);
    write(fd[1], w, sizeof(w));

    read(fd[0], r, sizeof(r));

    printf("%s\n", r);

    return 0;
}

#endif
