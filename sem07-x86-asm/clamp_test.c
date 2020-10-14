// %%cpp clamp_test.c
// compile and test using all three asm clamp implementations
// %run gcc -m32 -masm=intel -O2 clamp_disasm.S clamp_test.c -o clamp_test.exe
// %run ./clamp_test.exe
// %run gcc -m32 -masm=intel -O2 clamp_if.S clamp_test.c -o clamp_if_test.exe
// %run ./clamp_if_test.exe
// %run gcc -m32 -masm=intel -O2 clamp_cmov.S clamp_test.c -o clamp_cmov_test.exe
// %run ./clamp_cmov_test.exe

#include <stdint.h>
#include <stdio.h>
#include <assert.h>
    
int32_t clamp(int32_t a, int32_t b, int32_t c);

int main() {
    assert(clamp(1, 10, 20) == 10);
    assert(clamp(100, 10, 20) == 20);
    assert(clamp(15, 10, 20) == 15);
    fprintf(stderr, "All is OK");
    return 0;
}

