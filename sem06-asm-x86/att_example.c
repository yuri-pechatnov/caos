// %%cpp att_example.c
// %run gcc -m32 -masm=intel -O3 att_example.c -S -o att_example.S
// %run cat att_example.S | grep -v "^\s*\."

#include <stdint.h>
    
int32_t sum(int32_t a, int32_t b) {
    return a + b;
}

