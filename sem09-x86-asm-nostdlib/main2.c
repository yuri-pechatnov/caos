// %%cpp main2.c --under-spoiler-threshold 20
// %run gcc -m32 -masm=intel -static -flto -O3 main2.c -o main32.exe
// %run gcc -m64 -masm=intel -static -flto -O3 main2.c -o main64.exe
// %run objdump -M intel -d main64.exe | grep "<__libc_write>:" -A 8 | head -n 100 # Тут мы найдем syscall
// %run objdump -M intel -d main32.exe | grep "<__libc_write>:" -A 15 | head -n 100 # А тут мы найдем...
// %// call   DWORD PTR gs:0x10 -- это что-то про сегментные регситры и VDSO и там дальше сложно раскопать
// %run objdump -M intel -d main32.exe | grep "<_exit>:" -A 9 | head -n 1000 # Попробуем покопать другую функцию - exit

#include <unistd.h>

int main() {
    int w = write(1, "X", 1);
    return 0;
}

