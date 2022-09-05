// %%cpp example.c
// %run gcc -m64 -masm=intel -Os example.c -S -o example.S
// %run cat example.S 

#include <stdio.h>
    
int hello(int a, int b) {
    printf("Hello %d and %d\n", a, b);
    return a;
}

