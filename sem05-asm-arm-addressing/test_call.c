// %%cpp test_call.c
// %run arm-linux-gnueabi-gcc -marm test_call.c -O2 -o test_call.exe
// %run echo "123 124 125" | qemu-arm -L ~/Downloads/sysroot-glibc-linaro-2.25-2018.05-arm-linux-gnueabi ./test_call.exe

#include <stdio.h>

int ret_eof();
__asm__(R"(
#include <stdio.h>
    .text
    .global ret_eof
ret_eof:
    mov r0, =EOF
    bx lr
)");

int main() {
    printf("%d\n", ret_eof());
}

