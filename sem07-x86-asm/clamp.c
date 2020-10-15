// %%cpp clamp.c
// %run # gcc -S -m64 -masm=intel -O2 clamp.c -o /dev/stdout
#include <stdint.h>

int32_t clamp(int32_t x, int32_t a, int32_t b) {
    if (x < a) return a;
    if (x > b) return b;
    return x;
}

