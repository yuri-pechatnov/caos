// %%cpp main.cpp
// %run gcc main.cpp lib.o -o main.exe
// %run ./main.exe

#include <stdio.h>

int sum(int a, int b);

int main() {
    printf("%d", sum(1, 2));
    return 0;
}

