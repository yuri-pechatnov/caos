// %%cpp main2.c
// %run gcc -m32 -masm=intel -fno-asynchronous-unwind-tables -static -flto -O3  main2.c -S -o main2.S
// %run gcc -m32 -masm=intel -static -flto -O3 main2.c -o main2.exe
// %run ls -la main2.exe  # Заметьте, что размер стал сильно больше
// %run ldd main2.exe
//%run objdump -M intel -d main2.exe
// %run ./main2.exe

int main() {
    return 0;
}

