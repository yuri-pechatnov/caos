// %%cpp simdiv.c
// %run gcc -m64 -masm=intel -O3 simdiv.c -o simdiv.exe
// %run ./simdiv.exe

#include <stdio.h>
#include <stdint.h>
#include <assert.h>
    
int32_t simdiv(int32_t a) { 
    uint32_t eax = ((uint32_t)a >> 31) + a;
    __asm__("sar %0" : "=a"(eax) : "a"(eax));
    return eax;
}

int main() {
    assert(simdiv(1) == 0);
    assert(simdiv(5) == 2);
    assert(simdiv(-1) == 0);
    assert(simdiv(-5) == -2);
    printf("SUCCESS\n");
}

