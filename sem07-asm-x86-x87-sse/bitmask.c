// %%cpp bitmask.c
// %run gcc -m32 -msse4 -O3 bitmask.c -S -o bitmask.S # SSE, 64bit
// %run cat bitmask.S | grep -v "^\s*\."
  
    
#include <xmmintrin.h>
    
void bit_and(const int* __restrict__ a, 
             const int* __restrict__ b, 
             int* __restrict__ c) {
    for (int i = 0; i < 4 * 10; ++i) {
        c[i] = a[i] & b[i];
    }
}

void bit_and_2(const int* __restrict__ a, 
               const int* __restrict__ b, 
               int* __restrict__ c) {
    for (int i = 0; i < 10; i += 1) {
        ((__m128*)c)[i] = _mm_and_ps(((__m128*)a)[i], ((__m128*)b)[i]);
    }
}
    

