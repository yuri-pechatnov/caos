


# Инструменты разработки

<table width=100%> <tr>
    <th width=20%> <b>Видеозапись семинара &rarr; </b> </th>
    <th>
    <a href="https://youtu.be/???">
        <img src="https://placehold.it/320x100/000000/fff?text=None" width="320"  height="160" align="left" alt="Видео с семинара"> 
    </a>
    </th>
    <th> </th>
 </table>


[Ридинг Яковлева](https://github.com/victor-yacovlev/mipt-diht-caos/blob/master/practice/linux_basics/devtools.md)


Сегодня в программе:
* <a href="#compile" style="color:#856024"> Компиляция и ее этапы </a>
  * <a href="#simple_compile" style="color:#856024"> Просто скомпилировать! </a>
  * <a href="#preprocess" style="color:#856024"> Прeпроцессинг </a>
  * <a href="#compilation" style="color:#856024"> Компиляция </a>
  * <a href="#assembling" style="color:#856024"> Acceмблирование </a>
  * <a href="#linking" style="color:#856024"> Компоновка </a>


* <a href="#elf" style="color:#856024"> Динамические библиотеки, объектные и исполняемые файлы </a>

* <a href="#run" style="color:#856024"> Запуск и завершение программы </a>

* <a href="#macro" style="color:#856024"> Дополнение: макросы в C/C++ </a>


## <a name="compile"></a> Компиляция и ее этапы

[Процесс компиляции программ на C++ / Хабр](https://habr.com/ru/post/478124/)

Этапы компиляции:
1. Препроцессинг: разворачиваем инклюды и макросы
2. Компиляция: превращаем код на С/С++ в ассемблерный код
3. Ассемблирование: ассемблерный код -> машинный
4. Компоновка: сборка исполняемого файла из объектных файлов и статических библиотек

### <a name="simple_compile"></a> Просто скомпилировать!

Скомпилировать исходный код в исполняемый файл несложно: просто передаем все исходные файлы компилятору (файлы могут быть с разных этапов компиляции: исходные, после препроцессора, ассемблерный код (.S), объектные файлы (.o), ...)


```cpp
%%cpp hello_world.c
%run gcc hello_world.c -o hello_world_c.exe
%run ./hello_world_c.exe

#include <stdio.h>

int main() {
    printf("Hello world!\n");
    return 0;
}
```


Run: `gcc hello_world.c -o hello_world_c.exe`



Run: `./hello_world_c.exe`


    Hello world!



```cpp
%%cpp hello_world.cpp
%run g++ hello_world.cpp -o hello_world_cpp.exe
%run ./hello_world_cpp.exe

#include <iostream>

int main() {
    std::cout << "Hello world!\n";
    return 0;
}

```


Run: `g++ hello_world.cpp -o hello_world_cpp.exe`



Run: `./hello_world_cpp.exe`


    Hello world!


### <a name="preprocess"></a> Препроцессинг


```cpp
%%cpp preprocessing_max.c
%run gcc -E preprocessing_max.c -o preprocessing_max_E.c
%run cat preprocessing_max_E.c

#define max(a, b) ((a) > (b) ? (a) : (b))

int f(int a, int b) {
    return max(a, b);
}
```


Run: `gcc -E preprocessing_max.c -o preprocessing_max_E.c`



Run: `cat preprocessing_max_E.c`


    # 1 "preprocessing_max.c"
    # 1 "<built-in>"
    # 1 "<command-line>"
    # 31 "<command-line>"
    # 1 "/usr/include/stdc-predef.h" 1 3 4
    # 32 "<command-line>" 2
    # 1 "preprocessing_max.c"
    
    
    
    
    
    
    int f(int a, int b) {
        return ((a) > (b) ? (a) : (b));
    }


Результат препроцессинга является корректным кодом, его можно использовать так же, как и обычный код. Просто теперь в нем раскрыты все инструкции препроцессора


```cpp
%%cpp preprocessing_max_main.c
%run gcc preprocessing_max_main.c preprocessing_max_E.c -o preprocessing_max.exe
%run ./preprocessing_max.exe

#include <stdio.h>

int f(int a, int b);

int main() {
    printf("min(5, 7) = %d\n", f(5, 7));
    return 0;
}
```


Run: `gcc preprocessing_max_main.c preprocessing_max_E.c -o preprocessing_max.exe`



Run: `./preprocessing_max.exe`


    min(5, 7) = 7


### <a name="compilation"></a> Компиляция

Превратим исходный код в ассемблерный.


```python
# здесь необязательно брать результат работы препроцессора
# -Os -fno-asynchronous-unwind-tables помогает получить более короткий выхлоп
# -fverbose-asm позволит получить более длинный, но более удобный для чтения
!gcc -S preprocessing_max_E.c -Os -fno-asynchronous-unwind-tables -o preprocessing_max_E.S  
!cat preprocessing_max_E.S 
```

    	.file	"preprocessing_max_E.c"
    	.text
    	.globl	f
    	.type	f, @function
    f:
    	endbr64
    	cmpl	%edi, %esi
    	movl	%edi, %eax
    	cmovge	%esi, %eax
    	ret
    	.size	f, .-f
    	.ident	"GCC: (Ubuntu 9.3.0-10ubuntu2) 9.3.0"
    	.section	.note.GNU-stack,"",@progbits
    	.section	.note.gnu.property,"a"
    	.align 8
    	.long	 1f - 0f
    	.long	 4f - 1f
    	.long	 5
    0:
    	.string	 "GNU"
    1:
    	.align 8
    	.long	 0xc0000002
    	.long	 3f - 2f
    2:
    	.long	 0x3
    3:
    	.align 8
    4:


Способ получить ассемблерный код функции. Правда тут не просто проводится компиляция, здесь создается машинный код, а потом gdb его обратно дизассемблирует.


```python
!gcc -c preprocessing_max_E.c -Os -o preprocessing_max.o
!gdb preprocessing_max.o -batch -ex="disass f"
```

    Dump of assembler code for function f:
       0x0000000000000000 <+0>:	endbr64 
       0x0000000000000004 <+4>:	cmp    %edi,%esi
       0x0000000000000006 <+6>:	mov    %edi,%eax
       0x0000000000000008 <+8>:	cmovge %esi,%eax
       0x000000000000000b <+11>:	retq   
    End of assembler dump.


### <a name="assembling"></a> Ассемблирование

Ничего интересного, ассемблер и так слишком приЁближен к машинному коду. (Возможно этой фазы вообще нет при каких-то условиях компиляции).


```python
!gcc -c preprocessing_max_E.S -o preprocessing_max.o
```

### <a name="linking"></a> Компоновка / Линковка

Финальная сборка одного исполняемого файла

Она производится утилитой `ld`, но проще ее не запоминать и пользоваться gcc, который сам ее вызовет


```python
!gcc preprocessing_max.o preprocessing_max_main.c -o preprocessing_max_main.exe
!./preprocessing_max_main.exe
```

    min(5, 7) = 7


## <a name="elf"></a> Динамические библиотеки, объектные и исполняемые файлы


[ELF / Wiki](https://en.wikipedia.org/wiki/Executable_and_Linkable_Format) - Executable and Linkable Format


```cpp
%%cpp lib.c
%run gcc -c lib.c -o lib.o #// компилируем в объектный файл
%run ar rcs lib.a lib.o #// делаем из объектного файла статическую библиотеку (что по сути архив объектных файлов)
%run gcc -shared -fPIC lib.o -o lib.so #// делаем из объектного файла динамическую библиотеку

int sum(int a, int b) {
    return a + b;
}

float sum_f(float a, float b) {
    return a + b;
}
```


Run: `gcc -c lib.c -o lib.o #// компилируем в объектный файл`



Run: `ar rcs lib.a lib.o #// делаем из объектного файла статическую библиотеку`



Run: `gcc -shared -fPIC lib.o -o lib.so #// делаем из объектного файла динамическую библиотеку`



```python
!hexdump -C lib.o | head -n 2  # обратите внимание на 0x7f E L F - магическое начало файла
```

    00000000  7f 45 4c 46 02 01 01 00  00 00 00 00 00 00 00 00  |.ELF............|
    00000010  01 00 3e 00 01 00 00 00  00 00 00 00 00 00 00 00  |..>.............|



```python
!hexdump -C lib.a | head -n 2 # заметим, что это не ELF но тоже имеет магическое начало
!hexdump -C lib.so | head -n 2 # а это тоже ELF
```

    00000000  21 3c 61 72 63 68 3e 0a  2f 20 20 20 20 20 20 20  |!<arch>./       |
    00000010  20 20 20 20 20 20 20 20  30 20 20 20 20 20 20 20  |        0       |
    00000000  7f 45 4c 46 02 01 01 00  00 00 00 00 00 00 00 00  |.ELF............|
    00000010  03 00 3e 00 01 00 00 00  40 10 00 00 00 00 00 00  |..>.....@.......|



```python
!objdump -t lib.o | grep sum  # symbols in shared library
```

    0000000000000000 g     F .text	0000000000000018 sum
    0000000000000018 g     F .text	000000000000001e sum_f



```python
!objdump -t lib.so | grep sum  # symbols in shared library
```

    0000000000001111 g     F .text	000000000000001e sum_f
    00000000000010f9 g     F .text	0000000000000018 sum


Подгрузим динамическую библиотеку из Python


```python
import ctypes

lib = ctypes.CDLL("./lib.so")
%p lib.sum(3, 4)
%p lib.sum_f(3, 4)

lib.sum_f.restype = ctypes.c_float
%p lib.sum_f(3, 4) # with set return type

lib.sum_f.argtypes = [ctypes.c_float, ctypes.c_float]
lib.sum_f.restype = ctypes.c_float
%p lib.sum_f(3, 4) # with set return and arguments types
```


lib.sum(3, 4) = 7



lib.sum_f(3, 4) = 0



`lib.sum_f(3, 4) = 1.3243189739662118e-38`  # with set return type



`lib.sum_f(3, 4) = 7.0`  # with set return and arguments types



```python

```

## <a name="run"></a> Запуск и завершение программы

[Интересная статья про запуск и завершение программы](http://dbp-consulting.com/tutorials/debugging/linuxProgramStartup.html)

![](http://dbp-consulting.com/tutorials/debugging/images/callgraph.png)
    
    
Пример из статьи.


```cpp
%%cpp article_example.c
%run gcc article_example.c -o article_example.exe
%run ./article_example.exe

#include <stdio.h>
#include <stdlib.h>

void preinit(int argc, char **argv, char **envp) {
    printf("%s\n", __FUNCTION__);
}

void init(int argc, char **argv, char **envp) {
    printf("%s\n", __FUNCTION__);
}

void fini() {
    printf("%s\n", __FUNCTION__);
}

__attribute__((section(".init_array"))) typeof(init) *__init = init;
__attribute__((section(".preinit_array"))) typeof(preinit) *__preinit = preinit;
__attribute__((section(".fini_array"))) typeof(fini) *__fini = fini;

void  __attribute__ ((constructor)) constructor() {
    printf("%s\n", __FUNCTION__);
}

void __attribute__ ((destructor)) destructor() {
    printf("%s\n", __FUNCTION__);
}

void my_atexit() {
    printf("%s\n", __FUNCTION__);
}

void my_atexit2() {
    printf("%s\n", __FUNCTION__);
}

int main() {
    printf("%s 1\n", __FUNCTION__);
    atexit(my_atexit);
    atexit(my_atexit2);
    printf("%s 2\n", __FUNCTION__);
}
```


Run: `gcc article_example.c -o article_example.exe`



Run: `./article_example.exe`


    preinit
    init
    constructor
    main 1
    main 2
    my_atexit2
    my_atexit
    destructor
    fini


А вы думали программы без `main` не бывает?

`sudo apt-get install libc6-dev-amd64` - надо установить, чтобы `sys/syscall.h` нашелся. 


```cpp
%%cpp no_main_func.c
%run gcc -std=gnu11 -m32 -masm=intel -nostdlib -O3 no_main_func.c -o no_main_func.exe
%run ./no_main_func.exe

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
```


Run: `gcc -std=gnu11 -m32 -masm=intel -nostdlib -O3 no_main_func.c -o no_main_func.exe`



Run: `./no_main_func.exe`


    Hello world from 'syscall'!



```python

```

## <a name="macro"></a> Дополнение: макросы в C/C++ </a>

* Макросы это именно макросы, они ничего не знают про синтаксис С/С++


```cpp
%%cpp macro_example_0.c
%run gcc -E macro_example_0.c -o macro_example_0_E.c
%run cat macro_example_0_E.c

#define people students
#define goodbye(x) Good bye x! 

Hello people!
goodbye(bad grades)
```


Run: `gcc -E macro_example_0.c -o macro_example_E.c`



Run: `cat macro_example_E.c`


    # 1 "macro_example_0.c"
    # 1 "<built-in>"
    # 1 "<command-line>"
    # 31 "<command-line>"
    # 1 "/usr/include/stdc-predef.h" 1 3 4
    # 32 "<command-line>" 2
    # 1 "macro_example_0.c"
    
    
    
    
    
    
    
    Hello students!
    Good bye bad grades!


* Переменные в макросах это просто куски исходного текста
* При передаче аргументов в макрос стоит помнить, что значение имеют только запятые `,` и скобки `()`


```cpp
%%cpp macro_example_0_2.c
%run gcc -E macro_example_0_2.c -o macro_example_0_2_E.c
%run cat macro_example_0_2_E.c

#define macro(type, var) type var;

macro(std::pair<int, int>, a)
```


Run: `gcc -E macro_example_0_2.c -o macro_example_0_2_E.c`


    [01m[Kmacro_example_0_2.c:7:29:[m[K [01;31m[Kerror: [m[Kmacro "macro" passed 3 arguments, but takes just 2
        7 | macro(std::pair<int, int>, a[01;31m[K)[m[K
          |                             [01;31m[K^[m[K
    [01m[Kmacro_example_0_2.c:5:[m[K [01;36m[Knote: [m[Kmacro "macro" defined here
        5 | #define macro(type, var) type var;
          | 



Run: `cat macro_example_0_2_E.c`


    cat: macro_example_0_2_E.c: No such file or directory


Больше примеров


```cpp
%%cpp macro_example.c
%run gcc macro_example.c -o macro_example.exe
%run ./macro_example.exe
%run gcc -DDEBUG macro_example.c -o macro_example.exe
%run ./macro_example.exe

#include <stdio.h>

#define CONST_A 123

#define mult(a, b) ((a) * (b))

#define mult_bad(a, b) (a * b)

// Склеивание имен
#define add_prefix_aba_(w) aba_##w

int main() {
    printf("START\n");
    #ifdef DEBUG
        const char* file_name = "001.txt";
        printf("Read from '%s'\n", file_name);
        freopen(file_name, "rt", stdin);
    #endif

    printf("CONST_A %d\n", CONST_A);
    printf("mult(4, 6) = %d\n", mult(2 + 2, 3 + 3));
    printf("mult_bad(4, 6) = %d\n", mult_bad(2 + 2, 3 + 3));

    int aba_x = 42;
    int x = 420;
    printf("aba_x ? x = %d\n", add_prefix_aba_(x));

    return 0;
}
```


Run: `gcc macro_example.c -o macro_example.exe`



Run: `./macro_example.exe`


    START
    CONST_A 123
    mult(4, 6) = 24
    mult_bad(4, 6) = 11
    aba_x ? x = 42



Run: `gcc -DDEBUG macro_example.c -o macro_example.exe`



Run: `./macro_example.exe`


    START
    Read from '001.txt'
    CONST_A 123
    mult(4, 6) = 24
    mult_bad(4, 6) = 11
    aba_x ? x = 42


И полезных примеров:


```cpp
%%cpp macro_example_2.c
%run gcc macro_example_2.c -o macro_example_2.exe
%run ./macro_example_2.exe

#include <stdio.h>

// #VAR_NAME разворачивается в строковый литерал "VAR_NAME"
#define print_int(i) printf(#i " = %d\n", (i));

// Полезный макрос для вывода в поток ошибок
#define eprintf(...) fprintf(stderr, __VA_ARGS__)

// Способ сделать макрос с переменным числом аргументов
// И это единственный способ "перегрузить функцию в С"
#define sum_2(a, b, c) ((a) + (b))
#define sum_3(a, b, c) ((a) + (b) + (c))

#define sum_impl(a, b, c, sum_func, ...) sum_func(a, b, c)

#define sum(...) sum_impl(__VA_ARGS__, sum_3, sum_2)


int main() {
    print_int(9 * 9 + 1);

    eprintf("It is in stderr: %d\n", 431);

    print_int(sum(1, 1));
    print_int(sum(1, 1, 1));

    return 0;
}
```


Run: `gcc macro_example_2.c -o macro_example_2.exe`



Run: `./macro_example_2.exe`


    9 * 9 + 1 = 82
    It is in stderr: 431
    sum(1, 1) = 2
    sum(1, 1, 1) = 3



```python

```


```python

```


```python

```


```python

```


```python

```