// %%cpp preprocessing_max.c
// %run gcc -E preprocessing_max.c -o preprocessing_max_E.c
// %run cat preprocessing_max_E.c

#include "preprocessing_max.h"

// it's comment
#define max(a, b) ((a) > (b) ? (a) : (b))

int f(int a, int b) {
    return max(a, b);
}

