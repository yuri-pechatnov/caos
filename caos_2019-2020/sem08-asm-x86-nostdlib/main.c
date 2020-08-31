// %%cpp main.c
// %run gcc -m32 -masm=intel -fno-asynchronous-unwind-tables -O3 main.c -S -o main.S
// %run gcc -m32 -masm=intel -O3 main.c -o main.exe
// %run ls -la main.exe
// %run ldd main.exe  # Выводим зависимости по динамическим библиотекам
// %run cat main.S
// %run objdump -M intel -d main.exe

int main() {
    return 0;
}

