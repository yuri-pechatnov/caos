// %%cpp clamp_inline_test.c
// %run gcc -m64 -masm=intel -O2 clamp_inline_test.c -o clamp_inline_test.exe
// %run ./clamp_inline_test.exe

#include <stdint.h>
#include <stdio.h>
#include <assert.h>
    
int32_t clamp(int32_t a, int32_t b, int32_t c);
__asm__(R"(
clamp:
    endbr64
    mov eax, esi
    cmp edi, esi
    jl .clamp_return
    cmp edi, edx
    mov eax, edx
    cmovle eax, edi
.clamp_return:
    ret
)");

int main() {
    assert(clamp(1, 10, 20) == 10);
    assert(clamp(100, 10, 20) == 20);
    assert(clamp(15, 10, 20) == 15);
    fprintf(stderr, "All is OK");
    return 0;
}

