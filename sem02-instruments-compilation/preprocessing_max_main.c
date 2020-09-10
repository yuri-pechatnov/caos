// %%cpp preprocessing_max_main.c
// %run gcc preprocessing_max_main.c preprocessing_max_E.c -o preprocessing_max.exe
// %run ./preprocessing_max.exe

#include "preprocessing_max.h"

#include <stdio.h>

int main() {
    printf("max(5, 7) = %d\n", f(5, 7));
    return 0;
}

