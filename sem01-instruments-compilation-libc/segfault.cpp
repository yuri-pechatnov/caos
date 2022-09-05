// %%cpp segfault.cpp

#include<stdio.h>

int access(int* a, int i) { 
    return a[i]; 
}

int main() {
    int a[2] = {41, 42};
    printf("%d\n", access(a, 100500 + 1)); // проезд по памяти
}

