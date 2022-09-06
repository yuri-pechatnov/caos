// %%cpp no_main_func.c
// %run gcc -std=gnu11 -m32 -masm=intel -nostdlib -Os -s no_main_func.c -o no_main_func.exe 
// %run ./no_main_func.exe

#include <sys/syscall.h>

    
// Универсальная функция для совершения системных вызовов
int syscall(int code, ...);
__asm__(R"(
syscall:
    push ebx
    push ebp
    push esi
    push edi
    mov eax, DWORD PTR [esp + 20] 
    mov ebx, DWORD PTR [esp + 24] 
    mov ecx, DWORD PTR [esp + 28] 
    mov edx, DWORD PTR [esp + 32]
    mov esi, DWORD PTR [esp + 36]
    mov edi, DWORD PTR [esp + 40]
    int 0x80
    pop edi
    pop esi
    pop ebp
    pop ebx
    ret
)");

// запись строки в файловый дескриптор
int print_s(int fd, const char* s) {
    int len = 0;
    while (s[len]) ++len;
    return syscall(SYS_write, fd, s, len);
}

void _start() {
    print_s(1, "Hello world from 'syscall'!\n");
    syscall(SYS_exit, 0);
}

