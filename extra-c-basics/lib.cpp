// %%cpp lib.cpp

#include "lib.h"
// #include <stdio.h>

// void print42() {
//     printf("42\n");
// }

template <typename T>
T min(T a, T b) {
    return (a > b) ? b : a;
}

// int f(int a, int b) {
//     return min(a, b);
// }

template 
int min<int> (int a, int b);

