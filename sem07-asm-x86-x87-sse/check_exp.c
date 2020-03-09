// %%cpp check_exp.c
// %run gcc -g3 -m32 -masm=intel check_exp.c exp.c -o check_exp.exe
// %run ./check_exp.exe
// %run gcc -g3 -m32 -masm=intel check_exp.c exp2.S -o check_exp2.exe
// %run ./check_exp2.exe

#include <stdio.h>
#include <assert.h>

double my_exp(double x);

int main() {
    printf("exp(1) = %0.9lf\n", my_exp(1));
    return 0;
}

