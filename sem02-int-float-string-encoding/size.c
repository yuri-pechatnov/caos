// %%cpp size.c
// %run gcc size.c -o size.exe && ./size.exe # Компилируем обычным образом
// %run gcc -m32 size.c -o size.exe && ./size.exe # Под 32-битную архитектуру
// %run arm-linux-gnueabi-gcc -marm size.c -o size.exe && qemu-arm -L ~/arm/gcc-linaro-7.3.1-2018.05-i686_arm-linux-gnueabi/arm-linux-gnueabi/libc ./size.exe # Под ARM

#include <stdio.h>

int main() {
    printf("is char signed = %d, ", (int)((char)(-1) > 0));
    printf("sizeof(long int) = %d\n", (int)sizeof(long int));
}

