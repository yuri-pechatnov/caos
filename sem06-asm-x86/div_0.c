// %%cpp div_0.c
// %run gcc -m64 -masm=intel -O3 div_0.c -S -o div_0.S
// %run cat div_0.S | grep -v "^\s*\."

#include <stdint.h>
    
uint32_t div(uint32_t a) { 
    return a / 11;
}

uint32_t div2(uint32_t a, uint32_t b) { 
    return a / b;
}

