// %%cpp preprocessing_max_main.c
// %run gcc preprocessing_max_main.c preprocessing_max_E.c -o preprocessing_max.exe
// %run ./preprocessing_max.exe

#include <stdio.h>

int f(int a, int b);

int main() {
    printf("min(5, 7) = %d\n", f(5, 7));
    return 0;
}

