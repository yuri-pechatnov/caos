#include <stdio.h>
#include <math.h>

    #include <sys/types.h>
    #include <sys/stat.h>
    #include <fcntl.h>
    #include <unistd.h>

extern int a; int fd;
void lib_func_31_9() {
    freopen("lib_func_31_9.err", "w", stderr);
    freopen("lib_func_31_9.out", "w", stdout);
    fd = open("./a.txt", O_WRONLY | O_CREAT, 0644);
    fflush(stderr);
    fflush(stdout);
}
