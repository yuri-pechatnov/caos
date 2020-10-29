

# Жизнь без стандартной библиотеки


<p><a href="https://www.youtube.com/watch?v=6_7ojZXErDU&list=PLjzMm8llUm4AmU6i_hPU0NobgA4VsBowc&index=10" target="_blank">
    <h3>Видеозапись семинара</h3>
</a></p>

Что это значит? Значит, что функции взаимодействия с внешним миром (чтение, запись файлов и т. д.) будут реализованы в самом бинаре программы. Возможно, вы даже лично напишите их код.


[Ридинг Яковлева](https://github.com/victor-yacovlev/mipt-diht-caos/tree/master/practice/asm/nostdlib_baremetal) 


Сегодня в программе:
* <a href="#syscall" style="color:#856024"> Системные вызовы </a>
* <a href="#asm_32_64_diff" style="color:#856024"> Системные вызовы </a>
* <a href="#nolibc" style="color:#856024"> Пишем сами без libc </a>
* <a href="#brk" style="color:#856024"> Разбираемся в системным вызовом brk </a>
* <a href="#addr" style="color:#856024"> Развлекательная часть: смотрим на адреса различных переменных </a>



# <a name="syscall"></a> Системные вызовы


```cpp
%%cpp main.c
%run gcc -m64 main.c -o main.exe
%run strace ./main.exe 2> strace.out
%run cat strace.out

#include <stdio.h>

int main() {
    printf("Hello world!");
    return 0;
}
```

## Компилим как обычно


```cpp
%%cpp main.c --under-spoiler-threshold 5
%run gcc -m64 -masm=intel -fno-asynchronous-unwind-tables -O3 main.c -S -o main.S
%run gcc -m64 -masm=intel -O3 main.c -o main.exe
%run ls -la main.exe
%run ldd main.exe  # Выводим зависимости по динамическим библиотекам
%run cat main.S
%run objdump -M intel -d main.exe | grep main
%run strace ./main.exe

#include <unistd.h>

int main() {
    int w = write(1, "Hello world!", 12);
    return 0;
}
```


```python

```

## Компилим, статически линкуя libc


```cpp
%%cpp main2.c --under-spoiler-threshold 5
%run gcc -m64 -masm=intel -fno-asynchronous-unwind-tables -static -flto -O3 main2.c -S -o main2.S
%run gcc -m64 -masm=intel -static -flto -O3 main2.c -o main2.exe
%run ls -la main2.exe  # Заметьте, что размер стал сильно больше
%run ldd main2.exe || echo "fails with code=$?"
%run objdump -M intel -d main2.exe | grep "<_start>" -A 10 # Вот она функция с которой все начинается
%run objdump -M intel -d main2.exe | grep "<main>" -A 10 # Вот она функция main
%run objdump -M intel -d main2.exe | grep "<__libc_write>:" -A 8 | head -n 100 # А тут мы найдем syscall
%run strace ./main2.exe

#include <unistd.h>

int main() {
    int w = write(1, "X", 1);
    return 0;
}
```


```python
!gcc -E -m64 main2.c -o /dev/stdout | grep "main()" -A 10
```


```python

```


```python

```

Тут видим, что в `main` вызывается `__libc_write` (`write` либо макрос, либо соптимизировался), а в `__libc_write` происходит syscall с 0x1 в eax.

# <a name="asm_32_64_diff"></a> Отличие 32 и 64 битных архитектур в этом месте

Во первых номера системных вызовов разные

32 https://gist.github.com/yamnikov-oleg/454f48c3c45b735631f2

64 https://filippo.io/linux-syscall-table/


```cpp
%%cpp main2.c --under-spoiler-threshold 20
%run gcc -m32 -masm=intel -static -flto -O3 main2.c -o main32.exe
%run gcc -m64 -masm=intel -static -flto -O3 main2.c -o main64.exe
%run objdump -M intel -d main64.exe | grep "<__libc_write>:" -A 8 | head -n 100 # Тут мы найдем syscall
%run objdump -M intel -d main32.exe | grep "<__libc_write>:" -A 15 | head -n 100 # А тут мы найдем...
%// call   DWORD PTR gs:0x10 -- это что-то про сегментные регситры и VDSO и там дальше сложно раскопать
%run objdump -M intel -d main32.exe | grep "<_exit>:" -A 9 | head -n 1000 # Попробуем покопать другую функцию - exit

#include <unistd.h>

int main() {
    int w = write(1, "X", 1);
    return 0;
}
```

В 32-битной архитектуре системный вызов осуществляется с помощью `int 0x80`, в 64-битной - `syscall`.

Хотя в примере
```
mov    eax,0x1
int    0x80
```
это вызов exit на 32-битной архитектуре.

# <a name="nolibc"></a> Пишем сами без libc


```cpp
%%cpp example1.c
%run gcc -m64 -masm=intel -nostdlib -O3 example1.c -o example1.exe
%run strace ./example1.exe || echo "Failed with code=$?" 

// Именно с этой функции всегда начинается выполнение программы
void _start() {
    
}
```

Получаем сегфолт, так эта функция - входная точка в программу и в нее не передается разумного адреса возврата.
То есть делать return из этой функции - путь в никуда.

Интересный факт, что до старта `_start` все-таки есть жизнь, насколько я понимаю, это работа загрузчика программы и выполнение функций из секции `preinitarray`.


```cpp
%%cpp exit.c
%run gcc -m64 -masm=intel -O3 exit.c -o exit.exe
%run strace ./exit.exe 2>&1 | tail -n 3

// обычная программа с пустым main
int main() {}
```

Выполнение обычной программы заканчивается системным вызовом exit_group как подсказывает нам strace.

Можно нагуглить про этот системный вызов, например, это https://filippo.io/linux-syscall-table/


```cpp
%%cpp example2.c
%run gcc -m64 -masm=intel -nostdlib -O3 example2.c -o example2.exe
%run strace ./example2.exe ; echo "Exited with code=$?" 
    
int my_exit(int code);
__asm__(R"(
my_exit:
    mov rax, 231 /* номер системного вызова exit_group */
    syscall
    /* не нужно возвращаться из функции, на этом программа завершится */
)");
    
// Именно с этой функции всегда начинается выполнение программы
void _start() {
    my_exit(0);
}
```

Научились завершать нашу программу, отлично :)

Но не писать же всегда странные числа вроде 231?

`sudo apt-get install libc6-dev-amd64` - надо установить, чтобы `sys/syscall.h` нашелся.


```cpp
%%cpp example3.c
%run gcc -m64 -masm=intel -nostdlib -O3 example3.c -o example3.exe
%run ./example3.exe ; echo "Exited with code=$?" 

// Тут есть макросы с номерами системных вызовов
#include <sys/syscall.h>
    
// Превращение макроса с числовым литералом в строковый литерал
#define stringify_impl(x) #x
#define stringify(x) stringify_impl(x)

    
int my_exit(int code);
__asm__(R"(
my_exit:
    mov rax, )" stringify(SYS_exit_group) R"( /* В разрыв строкового литерала ассемблерной вставки вставляется строковый литерал системного вызова */
    syscall
    /* не нужно возвращаться из функции, на этом программа завершится */
)");
    
// Именно с этой функции всегда начинается выполнение программы
void _start() {
    my_exit(0);
}
```

Можно так вывернуться, например.

Да, все что я пишу на Си, можно писать и просто на ассемблере. И инклюды там так же работают :)


```python
%%asm example4.S
%//run gcc -E -m64 -masm=intel -nostdlib -O3 example4.S -o /dev/stdout
%run gcc -m64 -masm=intel -nostdlib -O3 example4.S -o example4.exe
%run ./example4.exe ; echo "Exited with code=$?" 

#include <sys/syscall.h>

.intel_syntax noprefix
.text

my_exit:
    mov rax, SYS_exit_group
    syscall
 
.globl _start
_start:
    mov rdi, 0
    call my_exit
```

Давайте напишем что-нибудь более сложное:


https://stackoverflow.com/questions/2535989/what-are-the-calling-conventions-for-unix-linux-system-calls-and-user-space-f


```cpp
%%cpp minimal.c
%run gcc -g -m64 -masm=intel -nostdlib -O3 minimal.c -o minimal.exe
%run gcc -m64 -masm=intel -nostdlib -fno-asynchronous-unwind-tables -O3 minimal.c -S -o minimal.S
%run ls -la minimal.exe  # Заметьте, что размер стал очень маленьким :)
//%run ldd minimal.exe

//%run cat minimal.S
//%run objdump -d minimal.exe

%run ./minimal.exe ; echo "Exit code = $?" 

#include <sys/syscall.h>

    
// Универсальная функция для совершения системных вызовов (до 5 аргументов системного вызова)
int syscall(int code, ...);
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

int write(int fd, const void* data, int size) {
    return syscall(SYS_write, fd, data, size);
}


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
    return write(fd, s, len);
}

int print_s(int fd, const char* s) {
    int len = 0;
    while (s[len]) ++len;
    return write(fd, s, len);
}




const char hello_s[] = "Hello world from function 'write'!\n";
const int hello_s_size = sizeof(hello_s);

// Забавно, но перед вызовом функции start стек не был выровнен по 16 :)
// Вернее был, но видимо не положили адрес возврата (так как не нужен), а сишный компилятор его ожидает...
__asm__(R"(
.globl _start
_start:
    sub rsp, 8
    jmp main
)");


void main() {
    const char hello_s_2[] = "Hello world from 'syscall'!\n";
    write(1, hello_s, sizeof(hello_s) - 1);
    syscall(SYS_write, 1, hello_s_2, sizeof(hello_s_2) - 1);
    print_s(1, "Look at this value: "); print_int(1, 10050042); print_s(1, "\n");
    print_s(1, "Look at this value: "); print_int(1, s_to_int("123456")); print_s(1, "\n");
    
    my_exit(0);
}

```

# <a name="brk"></a> Разбираемся в системным вызовом brk

`void *sbrk(intptr_t increment);`


```cpp
%%cpp minimal.c
%run gcc -m64 -masm=intel -nostdlib -O3 minimal.c -o minimal.exe
%run gcc -m64 -masm=intel -nostdlib -fno-asynchronous-unwind-tables -O3 minimal.c -S -o minimal.S

//%run cat minimal.S
//%run objdump -d minimal.exe

%run ./minimal.exe ; echo $? 

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

```


```python

```


```python

```


```python

```


```python

```

# <a name="addr"></a> Смотрим на адреса различных переменных. Проверяем, что секции памяти расположены так, как мы ожидаем


```cpp
%%cpp look_at_addresses.c --under-spoiler-threshold 30
%run gcc -m64 -masm=intel -O0 look_at_addresses.c -o look_at_addresses.exe
%run ./look_at_addresses.exe
%run gcc -S -m64 -masm=intel -Os look_at_addresses.c -o /dev/stdout

#include <stdio.h>
#include <stdlib.h>

int func(int a) {
    return a;
}


int* func_static_initialized() {
    static int a = 4;
    return &a;
}

const int* func_static_const_initialized() {
    static const int a = 4;
    return &a;
}

int* func_static_not_initialized() {
    static int a;
    return &a;
}


int global_initialized[3] = {1, 2, 3};
const int global_const_initialized[3] = {1, 2, 3};
int global_not_initialized[3];

int main2() {
   int local2 = 5;
   printf("Local 'local2' addr = %p\n", &local2); 
}


int main() {
    int local = 1;
    static int st = 2;
    int* all = malloc(12);
    
    printf("Func func addr = %p\n", (void*)func);
    printf("Func func_s addr = %p\n", (void*)func_static_initialized);
    
    printf("Global var (initialized) addr = %p\n", global_initialized);
    printf("Global var (const initialized) addr = %p\n", global_const_initialized);
    printf("Global var (not initialized) addr = %p\n", global_not_initialized);
    
    printf("Static 'st' addr = %p\n", &st);
    printf("Static 'func_static_initialized.a' addr = %p\n", func_static_initialized());
    printf("Static 'func_static_const_initialized.a' addr = %p\n", func_static_const_initialized());
    printf("Static 'func_static_not_initialized.a' addr = %p\n", func_static_not_initialized());
    
    printf("Local 'local' addr = %p\n", &local);
    main2();
    
    printf("Heap 'all' addr = %p\n", all);
    free(all);
    return 0;
}
```




```python

```


```python

```
