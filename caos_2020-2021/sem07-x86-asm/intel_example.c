// %%cpp intel_example.c
// %run gcc -m64 -masm=intel -Os intel_example.c -S -o intel_example.S
// %run cat intel_example.S 

#include <stdio.h>
    
int hello(int a, int b) {
    printf("Hello %d and %d\n", a, b);
    return a;
}

