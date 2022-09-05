// %%cpp example.c
// %run gcc -m64 -masm=intel -O3 example.c -S -o example.S # -mavx
// %run cat example.S | ./asm_filter_useless

#include <stdio.h>

double add(double a, double b) {
    return a + b;
}

double mult(double a, double b) {
    return a * b;
}

int cmp(double a) {
    return a > 0 ? 42 : 0;
}

double max(double a, double b) {
    return a > b ? a : b;
}

double muldi(double a, int b) {
    return a * b;
}

