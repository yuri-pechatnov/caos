// %%cpp bitmask_test.c
// %run gcc -m64 -masm=intel -msse4 -O3 bitmask_test.c bitmask.c -o bitmask_test.exe # SSE, 64bit
// %run ./bitmask_test.exe
 
#include <stdio.h>
#include <assert.h>
#include <xmmintrin.h>
    
typedef void (*and_func_t)(const int* a_, const int* b_, int* c_);
    
void bit_and_0(const int* a, const int* b, int* c);   
void bit_and_1(const int* restrict a, const int* restrict b, int* restrict c);
void bit_and_2(const int* restrict a_, const int* restrict b_, int* restrict c_);
void bit_and_intr(const int* restrict a, const int* restrict b, int* restrict c);

void bit_and_asm(const int* restrict a, const int* restrict b, int* restrict c);
__asm__(R"(
bit_and_asm:
    movaps xmm0, XMMWORD PTR [rsi]
    pand xmm0, XMMWORD PTR [rdi]
    movaps XMMWORD PTR [rdx], xmm0
    ret
)");

int main() {
    char __attribute__((aligned(16))) ac[16] = "ahjlvbshrvkbv";
    char __attribute__((aligned(16))) bc[16] = "ahjlascscsdaf";
    and_func_t funcs[] = {bit_and_0, bit_and_1, bit_and_2, bit_and_intr, bit_and_asm};
    char __attribute__((aligned(16))) cc[sizeof(funcs) / sizeof(funcs[0])][16];
    int M = sizeof(funcs) / sizeof(funcs[0]);
    for (int i = 0; i < M; ++i) {
        funcs[i]((int*)ac, (int*)bc, (int*)cc[i]);
    }
    printf("%p %p %p %p\n", ac, bc, cc[0], cc[1]);
    for (int j = 0; j + 1 < M; ++j) {
        for (int i = 0; i < 16; ++i) {
            assert(cc[j][i] == cc[j + 1][i]);
        }
    }
    printf("OK\n");
    return 0;
}

