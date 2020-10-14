// %%cpp mul.c
// %run gcc -m32 -masm=intel -O3 mul.c -S -o mul.S
// %run cat mul.S | ./asm_filter_useless

#include <stdint.h>
    
int32_t mul(int32_t a) { 
    return a * 2;
}

