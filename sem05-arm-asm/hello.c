// %%cpp hello.c
// %run arm-linux-gnueabi-gcc -S -Os -fno-asynchronous-unwind-tables -marm hello.c -o /dev/stdout

// Скомпилируем под arm и запустим hello_world 

#include <stdio.h>

int main() {
    printf("hello world!\n");
    return 0;
}

