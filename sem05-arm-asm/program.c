// %%cpp program.c
// %run arm-linux-gnueabi-gcc -marm program.c lib.c -o program.exe
// %run qemu-arm ./program.exe

#include <stdio.h>
#include <assert.h>

int strange_function(int n, int a);

int main() {
    printf("%d\n", strange_function(4, 0));
    assert(strange_function(4, 0) == 1);
    printf("%d\n", strange_function(4, 2));
    assert(strange_function(4, 2) == 1);
    printf("%d\n", strange_function(5, 0));
    assert(strange_function(5, 0) == 41000000);
    printf("%d\n", strange_function(5, 2));
    assert(strange_function(5, 2) == 41000010);
    return 0;
}

