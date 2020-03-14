#include <stdio.h>

; ;
void lib_func_32_0() {
    freopen("lib_func_32_0.err", "w", stderr);
    freopen("lib_func_32_0.out", "w", stdout);
    
    printf("%d", 40 + 2); 
    dprintf(2, "Hello world!");
;
    fflush(stderr);
    fflush(stdout);
}
