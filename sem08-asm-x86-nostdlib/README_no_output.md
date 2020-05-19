

# Жизнь без стандартной библиотеки

Что это значит? Значит, что функции взаимодействия с внещним миром (чтение, запись файлов и т. д.) будут реализованы в самом бинаре программы. Возможно вы даже лично напишите их код.



## Компилим как обычно


```cpp
%%cpp main.c
%run gcc -m32 -masm=intel -fno-asynchronous-unwind-tables -O3 main.c -S -o main.S
%run gcc -m32 -masm=intel -O3 main.c -o main.exe
%run ls -la main.exe
%run ldd main.exe  # Выводим зависимости по динамическим библиотекам
%run cat main.S
%run objdump -M intel -d main.exe

int main() {
    return 0;
}
```

## Компилим, статически линкуя libc


```cpp
%%cpp main2.c
%run gcc -m32 -masm=intel -fno-asynchronous-unwind-tables -static -flto -O3  main2.c -S -o main2.S
%run gcc -m32 -masm=intel -static -flto -O3 main2.c -o main2.exe
%run ls -la main2.exe  # Заметьте, что размер стал сильно больше
%run ldd main2.exe
//%run objdump -M intel -d main2.exe
%run ./main2.exe

int main() {
    return 0;
}
```


```python
!objdump -M intel -d main2.exe | grep -A 30 "<main>:"
#!objdump -M intel -d main2.exe | grep -A 30 "s80ea9f0"
```

# Пишем сами без libc


```cpp
%%cpp minimal.c
%run gcc -m32 -masm=intel -nostdlib -O3 minimal.c -o minimal.exe
%run gcc -m32 -masm=intel -nostdlib -fno-asynchronous-unwind-tables -O3 minimal.c -S -o minimal.S
%run ls -la minimal.exe  # Заметьте, что размер стал очень маленьким :)
//%run ldd minimal.exe

//%run cat minimal.S
//%run objdump -d minimal.exe

%run ./minimal.exe ; echo $? 

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
    const char hello_s_2[] = "Hello world from 'syscall'!\n";
    write();
    syscall(SYS_write, 1, hello_s_2, sizeof(hello_s_2));
    print_s(1, "Look at this value: "); print_int(1, 10050042); print_s(1, "\n");
    print_s(1, "Look at this value: "); print_int(1, s_to_int("123456")); print_s(1, "\n");
    
    syscall(SYS_exit, 0);
    _exit(-1);
}

```


```python

```

# Смотрим на адреса различных переменных. Проверяем, что секции памяти расположены так, как мы ожидаем


```cpp
%%cpp look_at_addresses.c
%run gcc -m32 -masm=intel -O0 look_at_addresses.c -o look_at_addresses.exe
%run ./look_at_addresses.exe

#include <stdio.h>
#include <stdlib.h>

int func(int a) {
    return a;
}

int* func_s() {
    static int a;
    return &a;
}

int data[123] = {1, 2, 3};


int main2() {
   int local2 = 5;
   printf("Local 'local2' addr = %p\n", &local2); 
}


int main() {
    int local = 1;
    static int st = 2;
    int* all = malloc(12);
    
    printf("Func func addr = %p\n", (void*)func);
    printf("Func func_s addr = %p\n", (void*)func_s);
    printf("Global var addr = %p\n", data);
    
    printf("Static 'st' addr = %p\n", &st);
    printf("Static 'func_s.a' addr = %p\n", func_s());
    
    printf("Local 'local' addr = %p\n", &local);
    main2();
    
    printf("Heap 'all' addr = %p\n", all);
    
    return 0;
}
```

# Разбираемся в системным вызовом brk

`void *sbrk(intptr_t increment);`


```cpp
%%cpp minimal.c
%run gcc -m32 -masm=intel -nostdlib -O3 minimal.c -o minimal.exe
%run gcc -m32 -masm=intel -nostdlib -fno-asynchronous-unwind-tables -O3 minimal.c -S -o minimal.S

//%run cat minimal.S
//%run objdump -d minimal.exe

%run ./minimal.exe ; echo $? 

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

```


```python
hex(146067456)
```


```python
hex(100500000)
```


```python

```


```python
%%asm asm.S
%run gcc -m32 -nostdlib asm.S -o asm.exe
%run ./asm.exe
    .intel_syntax noprefix
    .text
    .global _start
_start:
    mov eax, 4
    mov ebx, 1
    mov ecx, hello_world_ptr
    mov edx, 14
    int 0x80

    mov eax, 1
    mov ebx, 1
    int 0x80

    .data
hello_world:
    .string "Hello, World!\n"
hello_world_ptr:
    .long hello_world

```


```python

```
