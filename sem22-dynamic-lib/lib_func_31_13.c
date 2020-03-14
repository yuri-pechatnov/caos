#include <stdio.h>
#include <math.h>

    #include <sys/types.h>
    #include <sys/stat.h>
    #include <fcntl.h>
    #include <unistd.h>


    #include <sys/types.h>
    #include <sys/stat.h>
    //#include <fcntl.h>
    #include <unistd.h>

extern int a; extern int fd; extern int fd; ;
void lib_func_31_13() {
    freopen("lib_func_31_13.err", "w", stderr);
    freopen("lib_func_31_13.out", "w", stdout);
    
    dprintf(fd, "Hello students! a = %d", a);
    close(fd);
    printf("a.txt written and closed!");
;
    fflush(stderr);
    fflush(stdout);
}
