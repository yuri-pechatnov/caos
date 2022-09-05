// %%cpp att_example.c
// %run gcc -fcf-protection=none -m64 -masm=att -O3 att_example.c -S -o att_example.S
// %run cat att_example.S | ./asm_filter_useless

#include <stdint.h>

//  rdi, rsi, rdx, rcx, r8, r9
int64_t sum(int32_t a, int32_t b, int32_t c, int32_t d, int32_t e, int32_t f, int32_t g, int64_t h) {
    return a + b + c + d + e + f + g + h;
}

