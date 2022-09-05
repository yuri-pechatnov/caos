// %%cpp size.c
// %// Компилируем обычным образом
// %run gcc size.c -o size.exe
// %run ./size.exe
// %// Под 32-битную архитектуру
// %run gcc -m32 size.c -o size.exe
// %run ./size.exe
// %// Под ARM
// %run arm-linux-gnueabi-gcc -marm size.c -o size.exe
// %run qemu-arm -L ~/arm/gcc-linaro-7.3.1-2018.05-i686_arm-linux-gnueabi/arm-linux-gnueabi/libc ./size.exe


#include <stdio.h>

int main() {
    printf("is char signed = %d, ", (int)((char)(-1) > 0));
    printf("sizeof(long int) = %d\n", (int)sizeof(long int));
}

