// %%cpp bitmask_asm.c

#include <xmmintrin.h>

void bit_and_asm(const int* restrict a, const int* restrict b, int* restrict c);
__asm__(R"(
bit_and_asm:
    movaps (%rdi), %xmm0
    pand (%rsi), %xmm0
    movaps %xmm0, (%rdx)
    ret
)");

