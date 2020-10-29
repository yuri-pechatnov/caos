// %%cpp main2.c --under-spoiler-threshold 5
// %run gcc -m64 -masm=intel -fno-asynchronous-unwind-tables -static -flto -O3 main2.c -S -o main2.S
// %run gcc -m64 -masm=intel -static -flto -O3 main2.c -o main2.exe
// %run ls -la main2.exe  # Заметьте, что размер стал сильно больше
// %run ldd main2.exe || echo "fails with code=$?"
// %run objdump -M intel -d main2.exe | grep "<_start>" -A 10 # Вот она функция с которой все начинается
// %run objdump -M intel -d main2.exe | grep "<main>" -A 10 # Вот она функция main
// %run objdump -M intel -d main2.exe | grep "<__libc_write>:" -A 8 | head -n 100 # А тут мы найдем syscall
// %run strace ./main2.exe

#include <unistd.h>

int main() {
    int w = write(1, "X", 1);
    return 0;
}

