// %%cpp intel_example.c
// %run gcc -m64 -masm=intel -O3 intel_example.c -S -o intel_example.S
// %run cat intel_example.S | ./asm_filter_useless

#include <stdint.h>
    
//  rdi, rsi, rdx, rcx, r8, r9
int64_t sum(int32_t a, int32_t b, int32_t c, int32_t d, int32_t e, int32_t f, int32_t g, int64_t h) {
    return a + b + c + d + e + f + g +  h;
}

