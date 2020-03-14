#include <stdio.h>
#include <math.h>
extern int a; ;
void lib_func_32_4() {
    freopen("lib_func_32_4.err", "w", stderr);
    freopen("lib_func_32_4.out", "w", stdout);
    printf("1) %d", a);;
    fflush(stderr);
    fflush(stdout);
}
