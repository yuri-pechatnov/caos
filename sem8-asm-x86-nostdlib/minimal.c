
//%run cat minimal.S
//%run objdump -d minimal.exe


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


void int_to_s(unsigned int i, char* s, int* len) {
    int clen = 0;
    for (int ic = i; ic; ic /= 10, ++clen);
    clen = clen ?: 1;
    s[clen] = '\0';
    for (int j = 0; j < clen; ++j, i /= 10) {
        s[clen - j - 1] = '0' + i % 10;
    }
    *len = clen;
}

unsigned int s_to_int(char* s) {
    unsigned int res = 0;
    while ('0' <= *s && *s <= '9') {
        res *= 10;
        res += *s - '0';
        ++s;
    }
    return res;
}

int print_int(int fd, unsigned int i) {
    char s[20];
    int len;
    int_to_s(i, s, &len);
    return syscall(SYS_write, fd, s, len);
}

int print_s(int fd, const char* s) {
    int len = 0;
    while (s[len]) ++len;
    return syscall(SYS_write, fd, s, len);
}


// Пример использования системного вызова для завершения работы программы
void _exit(int code);
__asm__(R"(
_exit:
    mov   eax, 1
    mov   ebx, [esp + 4]
    int   0x80
)");


const char hello_s[] = "Hello world from function 'write'!\n";
const int hello_s_size = sizeof(hello_s);

// Пример использования системного вызова для вывода в stdout
int write();
__asm__(R"(
write:
    push ebx
    mov eax, 4 
    mov ebx, 1
    lea ecx, [hello_s]
    mov edx, hello_s_size
    int 0x80
    pop ebx
    ret
)");


// Именно с этой функции всегда начинается выполнение программы
void _start() {
    const int size = 100 * 1000 * 1000;
    int* data_start = (void*)syscall(SYS_brk, 0);
    int* data_end = (void*)syscall(SYS_brk, (int)data_start + size);
    
    print_s(1, "Data begin: "); print_int(1, (int)(void*)data_start); print_s(1, "\n");
    print_s(1, "Data end: ");  print_int(1, (int)(void*)data_end); print_s(1, "\n");
    
    data_start[0] = 1;
    for (int i = 1; i < (data_end - data_start); ++i) {
        data_start[i] = data_start[i - 1] + 1;
    }
    
    print_int(1, data_end[-1]); print_s(1, "\n");
    
    _exit(0);
}

