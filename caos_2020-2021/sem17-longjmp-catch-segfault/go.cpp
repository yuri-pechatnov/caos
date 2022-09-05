// %%cpp go.cpp
// %run gcc go.cpp -o go.exe
// %run ./go.exe

#include <stdio.h>

int main() {
    int i = 0;
    start:
    printf("i = %d\n", i);
    if (++i < 3) {
        goto start;
    }
    return 0;
}

