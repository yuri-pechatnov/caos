// %%cpp macro_example.c
// %run gcc macro_example.c -o macro_example.exe
// %run ./macro_example.exe
// %run gcc -DDEBUG macro_example.c -o macro_example.exe
// %run ./macro_example.exe

#include <stdio.h>

#define CONST_A 123

// #VAR_NAME разворачивается в строковый литерал "VAR_NAME"
#define print_int(i) printf(#i " = %d\n", (i));

#define mult(a, b) ((a) * (b))

#define mult_bad(a, b) (a * b)

// Склеивание имен
#define add_prefix_aba_(w) aba_##w

int main() {
    printf("START\n");
    #ifdef DEBUG
        const char* file_name = "001.txt";
        printf("Read from '%s'\n", file_name);
        freopen(file_name, "rt", stdin);
    #endif

    printf("CONST_A %d\n", CONST_A);
    printf("mult(4, 6) = %d\n", mult(2 + 2, 3 + 3));
    printf("mult_bad(4, 6) = %d\n", mult_bad(2 + 2, 3 + 3));

    int aba_x = 42;
    int x = 420;
    printf("aba_x ? x = %d\n", add_prefix_aba_(x));

    return 0;
}

