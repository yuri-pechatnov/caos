// %%cpp call.c
// %run arm-linux-gnueabi-gcc -marm call.c -O2 -S -o call.s
// %run cat call.s

#include <stdio.h>

int scan_a(int* a) {
    fscanf(stdin, "%d", a);
    return 42;
}

