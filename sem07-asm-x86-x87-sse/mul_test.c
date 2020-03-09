// %%cpp mul_test.c
// %run gcc -g3 -m32 -masm=intel mul_test.c mul.S -o mul_test.exe
// %run ./mul_test.exe

#include <stdio.h>
#include <assert.h>

double mul(double a);

int main() {
    printf("mul(2) = %0.9lf\n", mul(2));
    return 0;
}

