//%run ldd minimal.exe

//%run cat minimal.S
//%run objdump -d minimal.exe


#include <sys/syscall.h>

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



void _exit(int code);
__asm__(R"(
_exit:
    mov   eax, 1
    mov   ebx, [esp + 4]
    int   0x80
)");


const char hello_s[] = "Hello world from function 'write'!\n";
const int hello_s_size = sizeof(hello_s);

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



void _start() {
    const char hello_s_2[] = "Hello world from 'syscall'!\n";
    write();
    syscall(SYS_write, 1, hello_s_2, sizeof(hello_s_2));
    print_s(1, "Look at this value: "); print_int(1, 10050042); print_s(1, "\n");
    print_s(1, "Look at this value: "); print_int(1, s_to_int("123456")); print_s(1, "\n");
    
    // syscall(SYS_exit, 0);
    _exit(-1);
}

