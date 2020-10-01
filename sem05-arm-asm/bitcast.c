// %%cpp bitcast.c
// %run gcc -O2 -Wall bitcast.c -o bitcast.exe
// %run ./bitcast.exe

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>

uint64_t bit_cast_memcpy(double d) {
    uint64_t result;
    memcpy(&result, &d, sizeof(result)); // Железобетонный способ, но чуть сложнее для оптимизатора
    return result;
}

typedef union {
    double double_val;
    uint64_t ui64_val;
} converter_t;

uint64_t bit_cast_union(double d) {
    converter_t conv;
    conv.double_val = d;
    return conv.ui64_val;
    //return ((converter_t){.double_val = d}).ui64_val; // Вроде (?) хорошее решение
}

uint64_t bit_cast_ptr(double d) {
    return *(uint64_t*)(void*)&d; // Простое, но неоднозначное решение из-за алиасинга
}

int main() {
    double d = 3.15;
    printf("%" PRId64 "\n", bit_cast_memcpy(d));
    printf("%" PRId64 "\n", bit_cast_union(d));
    printf("%" PRId64 "\n", bit_cast_memcpy(d));
}

