// %%cpp intel_example.c
// %run gcc -m32 -masm=intel -O3 intel_example.c -S -o intel_example.S
// %run cat intel_example.S | ./asm_filter_useless

#include <stdint.h>
    
int32_t sum(int32_t a, int32_t b) {
    return a + b;
}

