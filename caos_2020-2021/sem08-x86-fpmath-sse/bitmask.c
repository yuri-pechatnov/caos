// %%cpp bitmask.c
// %run gcc -m64 -masm=intel -msse4 -O3 bitmask.c -S -o bitmask.S # SSE, 64bit
// %run cat bitmask.S | grep -v "^\s*\."

#include <xmmintrin.h>
    
#define N 1
   
    
void bit_and_0(const int* a, const int* b, int* c) {
    for (int i = 0; i < 4 * N; ++i) {
        c[i] = a[i] & b[i];
    }
}
    
void bit_and_1(const int* restrict a, const int* restrict b, int* restrict c) {
    for (int i = 0; i < 4 * N; ++i) {
        c[i] = a[i] & b[i];
    }
}

void bit_and_2(const int* restrict a_, const int* restrict b_, int* restrict c_) {
    const int* a = __builtin_assume_aligned(a_, 16);
    const int* b = __builtin_assume_aligned(b_, 16);
    int* c = __builtin_assume_aligned(c_, 16);
    for (int i = 0; i < 4 * N; ++i) {
        c[i] = a[i] & b[i];
    }
}


void bit_and_intr(const int* restrict a, const int* restrict b, int* restrict c) {
    for (int i = 0; i < N; i += 1) {
        ((__m128i*)c)[i] = _mm_and_si128(((__m128i*)a)[i], ((__m128i*)b)[i]);
    }
}

