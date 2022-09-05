// %%cpp minimal.c
// %run gcc -m64 -masm=intel -nostdlib -O3 minimal.c -o minimal.exe
// %run gcc -m64 -masm=intel -nostdlib -fno-asynchronous-unwind-tables -O3 minimal.c -S -o minimal.S

//%run cat minimal.S
//%run objdump -d minimal.exe

// %run ./minimal.exe ; echo $? 

#include <sys/syscall.h>
#include <stdint.h>

    
// Универсальная функция для совершения системных вызовов (до 5 аргументов системного вызова)
int64_t syscall(int64_t code, ...);
__asm__(R"(
syscall:
    /* Function arguments: rdi, rsi, rdx, rcx, r8, r9 */
    /* Syscall arguments: rax (syscall num), rdi, rsi, rdx, r10, r8, r9.*/
    mov rax, rdi 
    mov rdi, rsi 
    mov rsi, rdx
    mov rdx, rcx
    mov r10, r8
    mov r8, r9
    syscall
    ret
)");

void my_exit(int code) {
    syscall(SYS_exit, code);
}

int64_t write(int fd, const void* data, int64_t size) {
    return syscall(SYS_write, fd, data, size);
}

void int_to_s(uint64_t i, char* s, int* len) {
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

int print_int(int fd, int64_t i) {
    char s[40];
    int len;
    int_to_s(i, s, &len);
    return syscall(SYS_write, fd, s, len);
}

int print_s(int fd, const char* s) {
    int len = 0;
    while (s[len]) ++len;
    return syscall(SYS_write, fd, s, len);
}


const char hello_s[] = "Hello world from function 'write'!\n";
const int hello_s_size = sizeof(hello_s);


// Именно с этой функции всегда начинается выполнение программы
void _start() {
    const int size = 100 * 1000 * 1000;
    int* data_start = (void*)syscall(SYS_brk, 0);
    int* data_end = (void*)syscall(SYS_brk, (int*)data_start + size);
    
    print_s(1, "Data begin: "); print_int(1, (int64_t)(void*)data_start); print_s(1, "\n");
    print_s(1, "Data end: ");  print_int(1, (int64_t)(void*)data_end); print_s(1, "\n");
    
    data_start[0] = 1;
    for (int i = 1; i < (data_end - data_start); ++i) {
        data_start[i] = data_start[i - 1] + 1;
    }
    
    print_int(1, data_end[-1]); print_s(1, "\n");
    
    my_exit(0);
}

