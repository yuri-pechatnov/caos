// %%cpp size.c
// %run arm-linux-gnueabi-gcc -marm size.c -o size.exe
// %run qemu-arm ./size.exe

#include <stdio.h>

int main() {
    printf("is char signed = %d, ", (int)((char)(-1) > 0));
    printf("sizeof(long int) = %d\n", (int)sizeof(long int));
}

