// %%cpp bitmask_test.c
// %run gcc -m64 -msse4 -O3 bitmask_test.c bitmask.c -o bitmask_test.exe # SSE, 64bit
// %run ./bitmask_test.exe
 
#include <stdio.h>
#include <assert.h>
#include <xmmintrin.h>
    
void bit_and(const int* __restrict__ a, 
             const int* __restrict__ b, 
             int* __restrict__ c);
void bit_and_2(const int* __restrict__ a, 
               const int* __restrict__ b, 
               int* __restrict__ c);
    

int main() {
    char __attribute__((aligned(128))) ac[128 * 10 + 3] = "ahjlvbshrvkbvjknkjsnbjskndsenckjwncskjncvsjckjsncksjncskjdnckjsncjksdncndkcnsdkjcnsdcjksndcjksdncjksdnjkdcnjknckdjcahjlvbshrvkbvjknkjsnbjskndsenckjwncskjncvsjckjsncksjncskjdnckjsncjksdncndkcnsdkjcnsdcjksndcjksdncjksdnjkdcnjknckdjcahjlvbshrvkbvjknkjsnbjskndsenckjwncskjncvsjckjsncksjncskjdnckjsncjksdncndkcnsdkjcnsdcjksndcjksdncjksdnjkdcnjknckdjcahjlvbshrvkbvjknkjsnbjskndsenckjwncskjncvsjckjsncksjncskjdnckjsncjksdncndkcnsdkjcnsdcjksndcjksdncjksdnjkdcnjknckdjcahjlvbshrvkbvjknkjsnbjskndsenckjwncskjncvsjckjsncksjncskjdnckjsncjksdncndkcnsdkjcnsdcjksndcjksdncjksdnjkdcnjknckdjcahjlvbshrvkbvjknkjsnbjskndsenckjwncskjncvsjckjsncksjncskjdnckjsncjksdncndkcnsdkjcnsdcjksndcjksdncjksdnjkdcnjknckdjcahjlvbshrvkbvjknkjsnbjskndsenckjwncskjncvsjckjsncksjncskjdnckjsncjksdncndkcnsdkjcnsdcjksndcjksdncjksdnjkdcnjknckdjcahjlvbshrvkbvjknkjsnbjskndsenckjwncskjncvsjckjsncksjncskjdnckjsncjksdncndkcnsdkjcnsdcjksndcjksdncjksdnjkdcnjknckdjcahjlvbshrvkbvjknkjsnbjskndsenckjwncskjncvsjckjsncksjncskjdnckjsncjksdncndkcnsdkjcnsdcjksndcjksdncjksdnjkdcnjknckdjcahjlvbshrvkbvjknkjsnbjskndsenckjwncskjncvsjckjsncksjncskjdnckjsncjksdncndkcnsdkjcnsdcjksndcjksdncjksdnjkdcnjknckdjc";
    char __attribute__((aligned(128))) bc[128 * 10 + 3] = "ahjlascscsdafbshrvkbvjknkjsnbjskndsenckjwncskjncvsjckjsncksjncskjdnckjsncjksdncndkcnsdkjcnsdcjksndcjksdncjksdnjkdcnjknckdjcahjlvbshrvkbvjknkjsnbjskndsenckjwncskjncvsjckjsncksjncskjdnckjsncjksdncndkcnsdkjcnsdcjksndcjksdncjksdnjkdcnjknckdjcahjlvbshrvkbvjknkjsnbjskndsenckjwncskjncvsjckjsncksjncskjdnckjsncjksdncndkcnsdkjcnsdcjksndcjksdncjksdnjkdcnjknckdjcahjlvbshrvkbvjknkjsnbjskndsenckjwncskjncvsjckjsncksjncskjdnckjsncjksdncndkcnsdkjcnsdcjksndcjksdncjksdnjkdcnjknckdjcahjlvbshrvkbvjknkjsnbjskndsenckjwncskjncvsjckjsncksjncskjdnckjsncjksdncndkcnsdkjcnsdcjksndcjksdncjksdnjkdcnjknckdjcahjlvbshrvkbvjknkjsnbjskndsenckjwncskjncvsjckjsncksjncskjdnckjsncjksdncndkcnsdkjcnsdcjksndcjksdncjksdnjkdcnjknckdjcahjlvbshrvkbvjknkjsnbjskndsenckjwncskjncvsjckjsncksjncskjdnckjsncjksdncndkcnsdkjcnsdcjksndcjksdncjksdnjkdcnjknckdjcahjlvbshrvkbvjknkjsnbjskndsenckjwncskjncvsjckjsncksjncskjdnckjsncjksdncndkcnsdkjcnsdcjksndcjksdncjksdnjkdcnjknckdjcahjlvbshrvkbvjknkjsnbjskndsenckjwncskjncvsjckjsncksjncskjdnckjsncjksdncndkcnsdkjcnsdcjksndcjksdncjksdnjkdcnjknckdjcahjlvbshrvkbvjknkjsnbjskndsenckjwncskjncvsjckjsncksjncskjdnckjsncjksdncndkcnsdkjcnsdcjksndcjksdncjksdnjkdcnjknckdjc";
    char __attribute__((aligned(128))) c1c[128 * 10 + 3];
    char __attribute__((aligned(128))) c2c[128 * 10 + 3];
    bit_and((int*)ac, (int*)bc, (int*)c1c);
    bit_and_2((int*)ac, (int*)bc, (int*)c2c);
    printf("%p %p %p %p\n", ac, bc, c1c, c2c);
    for (int i = 0; i < 4 * 4 * 10; ++i) {
        assert(c1c[i] == c2c[i]);
    }
    return 0;
}

