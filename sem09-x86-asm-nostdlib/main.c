// %%cpp main.c --under-spoiler-threshold 5
// %run gcc -m64 -masm=intel -fno-asynchronous-unwind-tables -O3 main.c -S -o main.S
// %run gcc -m64 -masm=intel -O3 main.c -o main.exe
// %run ls -la main.exe
// %run ldd main.exe  # Выводим зависимости по динамическим библиотекам
// %run cat main.S
// %run objdump -M intel -d main.exe | grep main

#include <unistd.h>

int main() {
    int w = write(1, "X", 1);
    return 0;
}

