


# Инструменты разработки

Видеозаписи этого семинара нет (звук не записался). Но если очень нужно [запись старого семинара про этапы компиляции, запуск и завершение программы](https://www.youtube.com/watch?v=E8a0m6HG2x8&list=PLjzMm8llUm4AmU6i_hPU0NobgA4VsBowc&index=3) и [запись старого семинара про санитайзеры](https://www.youtube.com/watch?v=R_P4FSxH1AY&list=PLjzMm8llUm4AmU6i_hPU0NobgA4VsBowc&index=13).

[Ридинг Яковлева про компиляцию, python.ctypes, gdb](https://github.com/victor-yacovlev/mipt-diht-caos/blob/master/practice/linux_basics/devtools.md)


Сегодня в программе:
* <a href="#io" style="color:#856024"> Ввод-вывод </a>
* <a href="#compile" style="color:#856024"> Компиляция и ее этапы </a>
  * <a href="#simple_compile" style="color:#856024"> Просто скомпилировать! </a>
  * <a href="#preprocess" style="color:#856024"> Прeпроцессинг </a>
  * <a href="#compilation" style="color:#856024"> Компиляция </a>
  * <a href="#assembling" style="color:#856024"> Acceмблирование </a>
  * <a href="#linking" style="color:#856024"> Компоновка </a>

* <a href="#elf" style="color:#856024"> Динамические библиотеки, объектные и исполняемые файлы </a>

* <a href="#debug" style="color:#856024"> Отладка и инструментирование </a>
  * <a href="#gdb" style="color:#856024"> GDB </a>
  * <a href="#sanitizers" style="color:#856024"> Sanitizers </a>
    * <a href="#asan_segv" style="color:#856024"> ASAN и проезды по памяти </a>
    * <a href="#asan_leak" style="color:#856024"> ASAN: Обнаружение утечек памяти с помощью address-санитайзера </a>
  * <a href="#strace" style="color:#856024"> STRACE: Отладка системных вызовов с помощью strace </a>

* <a href="#run" style="color:#856024"> Запуск и завершение программы </a>

* <a href="#macro" style="color:#856024"> Дополнение: макросы в C/C++ </a>


## <a name="io"></a> Ввод-вывод

[Семейство printf на cppreference](https://en.cppreference.com/w/c/io/fprintf)

[Семейство scanff на cppreference](https://en.cppreference.com/w/c/io/fscanf)

`sprintf` не проверяет выход за пределы буффера. Нужно использовать крайне аккуратно. А лучше `snprintf`.


```cpp
%%cpp io.c
%// .exe не имеет никакого практического смысла с точки зрения запуска программы
%// однако позволяет выбирать все бинари регуляркой *.exe, автор кода этим пользуется
%run gcc io.c -o io.exe -fsanitize=address  
%run echo 42 | ./io.exe 7
%run cat out.txt

#undef NDEBUG // Ensure assert works.
#include <assert.h>
#include <stdio.h>

int main(int argc, char** argv) {
    assert(argc == 2);
    int cmd_arg_value = 0, stdin_value = 0;
    assert(sscanf(argv[1], "%d", &cmd_arg_value) == 1); // Чтение из строки
    int printf_ret = printf("stdout: cmd_arg_value = %d\n", cmd_arg_value); // Запись в stdout. 
    assert(printf_ret > 0); // Проверять, что нет ошибки полезно.
    assert(printf_ret == 26); // Если нет ошибки, то в printf_ret количество записанных символов. Такое проверять не надо, разумеется.
    assert(fprintf(stderr, "stderr: cmd_arg_value = %d\n", cmd_arg_value) > 0); // Запись в stderr.
    
    assert(scanf("%d", &stdin_value) > 0);
    char buf[100];
    int snprintf_ret = snprintf(buf, sizeof(buf), "stdin_value = %d", stdin_value); // Печать в буфер.
    assert(snprintf_ret > 0 && snprintf_ret + 1 < sizeof(buf)); // Нет ошибки и влезли в буффер.
    
    FILE* f = fopen("out.txt", "w");
    fprintf(f, "file: %s\n", buf); // Печать в файл.
    fclose(f);
    return 0;
}
```


\#\#\#\# `.exe не имеет никакого практического смысла с точки зрения запуска программы`



\#\#\#\# `однако позволяет выбирать все бинари регуляркой *.exe, автор кода этим пользуется`



Run: `gcc io.c -o io.exe -fsanitize=address`



Run: `echo 42 | ./io.exe 7`


    stdout: cmd_arg_value = 7
    stderr: cmd_arg_value = 7



Run: `cat out.txt`


    file: stdin_value = 42


#### Нетривиальные моменты


```cpp
%%cpp spec.c
%// .exe не имеет никакого практического смысла с точки зрения запуска программы
%// однако позволяет выбирать все бинари регуляркой *.exe, автор кода этим пользуется
%run gcc spec.c -o spec.exe -fsanitize=address  
%run ./spec.exe

#include <stdio.h>
#include <inttypes.h>

int main() {
    printf("s = %.*s\n", 4, "01234567" + 2); // Распечатать 4 символа, начиная с 2го
    uint32_t i_32 = 42;
    printf("i_32 = %" PRIu32 ", sizeof(i_32) = %d\n", i_32, (int)sizeof(i_32)); // Совместимый макрос PRId32
    
    // Не очень просто безопасно прочитать строчку)
    char buffer[10];
    int max_len = (int)sizeof(buffer) - 1, actual_len = 0;
    char format[32]; // Гарантированно достаточный буффер для генерируемой форматной строки.
    snprintf(format, sizeof(format), "%%%ds%%n", (int)max_len);
    if (sscanf("string_input_input_input", format, buffer, &actual_len) == 1 && actual_len != max_len) {
        printf("complete read: %s\n", buffer);
    } else {
        printf("incomplete read: %s\n", buffer);
    }
    return 0;
}
```


\#\#\#\# `.exe не имеет никакого практического смысла с точки зрения запуска программы`



\#\#\#\# `однако позволяет выбирать все бинари регуляркой *.exe, автор кода этим пользуется`



Run: `gcc spec.c -o spec.exe -fsanitize=address`



Run: `./spec.exe`


    s = 2345
    i_32 = 42, sizeof(i_32) = 4
    incomplete read: string_in


## <a name="compile"></a> Компиляция и ее этапы

[Процесс компиляции программ на C++ / Хабр](https://habr.com/ru/post/478124/)

[Этапы компиляции / cppreference](https://en.cppreference.com/w/c/language/translation_phases) - тут выделены другие этапы, подробнее описано все про препроцессор.

Этапы компиляции:
1. Препроцессинг: разворачиваем инклюды и макросы
2. Компиляция: превращаем код на С/С++ в ассемблерный код
3. Ассемблирование: ассемблерный код -> машинный
4. Компоновка: сборка исполняемого файла из объектных файлов и статических библиотек

### <a name="simple_compile"></a> Просто скомпилировать!

Скомпилировать исходный код в исполняемый файл несложно: просто передаем все исходные файлы компилятору (файлы могут быть с разных этапов компиляции: исходные, после препроцессора, ассемблерный код (.S), объектные файлы (.o), ...)


```cpp
%%cpp hello_world.c
%// .exe не имеет никакого практического смысла с точки зрения запуска программы
%// однако позволяет выбирать все бинари регуляркой *.exe, автор кода этим пользуется
%run gcc hello_world.c -o hello_world_c.exe  
%run ./hello_world_c.exe

#include <stdio.h>

int main() {
    printf("Hello world!\n");
    return 0;
}
```


\#\#\#\# `.exe не имеет никакого практического смысла с точки зрения запуска программы`



\#\#\#\# `однако позволяет выбирать все бинари регуляркой *.exe, автор кода этим пользуется`



Run: `gcc hello_world.c -o hello_world_c.exe`



Run: `./hello_world_c.exe`


    Hello world!


### <a name="preprocess"></a> Препроцессинг


```cpp
%%cpp preprocessing_max.h
int f(int a, int b);
```


```cpp
%%cpp preprocessing_max.c
%run gcc -E preprocessing_max.c -o preprocessing_max_E.c
%run cat preprocessing_max_E.c
#include "preprocessing_max.h"
#define max(a, b) ((a) > (b) ? (a) : (b))

int f(int a, int b) { // it's comment
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
    
    
    
    # 1 "preprocessing_max.h" 1
    
    int f(int a, int b);
    # 5 "preprocessing_max.c" 2
    
    
    int f(int a, int b) {
        return ((a) > (b) ? (a) : (b));
    }


Результат препроцессинга является корректным кодом, его можно использовать так же, как и обычный код. Просто теперь в нем раскрыты все инструкции препроцессора


```cpp
%%cpp preprocessing_max_main.c
%run gcc preprocessing_max_main.c preprocessing_max_E.c -o preprocessing_max.exe
%run ./preprocessing_max.exe

#include "preprocessing_max.h"

#include <stdio.h>

int main() {
    printf("max(5, 7) = %d\n", f(5, 7));
    return 0;
}
```


Run: `gcc preprocessing_max_main.c preprocessing_max_E.c -o preprocessing_max.exe`



Run: `./preprocessing_max.exe`


    max(5, 7) = 7


### <a name="compilation"></a> Компиляция

Превратим исходный код в ассемблерный.


```python
# здесь необязательно брать результат работы препроцессора
# -Os -fno-asynchronous-unwind-tables помогает получить более короткий выхлоп
# -fverbose-asm позволит получить более длинный, но более удобный для чтения
!gcc -S preprocessing_max_E.c -o preprocessing_max_E.S -Os -Wl,--gc-sections -fno-asynchronous-unwind-tables -fcf-protection=branch -mmanual-endbr 
!cat preprocessing_max_E.S 
```

    	.file	"preprocessing_max_E.c"
    	.text
    	.globl	f
    	.type	f, @function
    f:
    	cmpl	%edi, %esi
    	movl	%edi, %eax
    	cmovge	%esi, %eax
    	ret
    	.size	f, .-f
    	.ident	"GCC: (Ubuntu 9.3.0-17ubuntu1~20.04) 9.3.0"
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
    	.long	 0x1
    3:
    	.align 8
    4:


Способ получить ассемблерный код функции. Правда тут не просто проводится компиляция, здесь создается машинный код, а потом gdb его обратно дизассемблирует.


```python
!gcc -c preprocessing_max_E.c -o preprocessing_max.o -Os -fno-asynchronous-unwind-tables -fcf-protection=branch -mmanual-endbr 
!gdb preprocessing_max.o -batch -ex="disass f"
```

    Dump of assembler code for function f:
       0x0000000000000000 <+0>:	cmp    %edi,%esi
       0x0000000000000002 <+2>:	mov    %edi,%eax
       0x0000000000000004 <+4>:	cmovge %esi,%eax
       0x0000000000000007 <+7>:	retq   
    End of assembler dump.


Еще один способ


```python
!gcc -g -c preprocessing_max.c -Os -fno-asynchronous-unwind-tables -fcf-protection=branch -mmanual-endbr 
!objdump -d -S preprocessing_max.o
```

    
    preprocessing_max.o:     file format elf64-x86-64
    
    
    Disassembly of section .text:
    
    0000000000000000 <f>:
    // %run cat preprocessing_max_E.c
    #include "preprocessing_max.h"
    #define max(a, b) ((a) > (b) ? (a) : (b))
    
    int f(int a, int b) { // it's comment
        return max(a, b);
       0:	39 fe                	cmp    %edi,%esi
       2:	89 f8                	mov    %edi,%eax
       4:	0f 4d c6             	cmovge %esi,%eax
    }
       7:	c3                   	retq   



```python

```

### <a name="assembling"></a> Ассемблирование

Ничего интересного, ассемблер и так слишком приближен к машинному коду. 


```python
!gcc -c preprocessing_max_E.S -o preprocessing_max.o
```

### <a name="linking"></a> Компоновка / Линковка

Финальная сборка одного исполняемого файла

Она производится утилитой `ld`, но проще ее не запоминать и пользоваться gcc, который сам ее вызовет.

(А как именно вызовет можно узнать, добавив опции `-v -Wl,-v`, конструкция `-Wl,` позволяет указать опциию, которую gcc прокинет линкеру)


```python
!gcc preprocessing_max.o preprocessing_max_main.c -o preprocessing_max_main.exe
!./preprocessing_max_main.exe
```

    max(5, 7) = 7


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



Run: `ar rcs lib.a lib.o #// делаем из объектного файла статическую библиотеку (что по сути архив объектных файлов)`



Run: `gcc -shared -fPIC lib.o -o lib.so #// делаем из объектного файла динамическую библиотеку`



```python
!echo lib.o:  ; hexdump -C lib.o | head -n 2  # обратите внимание на 0x7f E L F - магическое начало файла
```

    lib.o:
    00000000  7f 45 4c 46 02 01 01 00  00 00 00 00 00 00 00 00  |.ELF............|
    00000010  01 00 3e 00 01 00 00 00  00 00 00 00 00 00 00 00  |..>.............|



```python
!echo lib.a:    ; hexdump -C lib.a  | head -n 2 # заметим, что это не ELF но тоже имеет магическое начало
!echo libb.so:  ; hexdump -C lib.so | head -n 2 # а это тоже ELF
```

    lib.a:
    00000000  21 3c 61 72 63 68 3e 0a  2f 20 20 20 20 20 20 20  |!<arch>./       |
    00000010  20 20 20 20 20 20 20 20  30 20 20 20 20 20 20 20  |        0       |
    libb.so:
    00000000  7f 45 4c 46 02 01 01 00  00 00 00 00 00 00 00 00  |.ELF............|
    00000010  03 00 3e 00 01 00 00 00  40 10 00 00 00 00 00 00  |..>.....@.......|



```python
!echo preprocessing_max.exe:  ; hexdump -C preprocessing_max.exe | head -n 2 # а это тоже ELF
```

    preprocessing_max.exe:
    00000000  7f 45 4c 46 02 01 01 00  00 00 00 00 00 00 00 00  |.ELF............|
    00000010  03 00 3e 00 01 00 00 00  60 10 00 00 00 00 00 00  |..>.....`.......|



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



```cpp
%%cpp main.c
%run gcc main.c lib.a -o main.exe
%run ./main.exe

#include <stdio.h>

int sum(int a, int b);

int main() {
    printf("%d", sum(1, 2));
    return 0;
}
```


Run: `gcc main.c lib.a -o main.exe`



Run: `./main.exe`


    3


```python

```

## <a name="debug"></a> Отладка и инструментирование

### <a name="gdb"></a> GDB

Полезные команды с моей практики:
* ```gdb -p $(pidof <your_process_name>) -ex 'set height 0' -ex 'thread apply all bt' -ex 'detach' -ex 'quit' > bt.txt``` - прям основное - снять бектрейсы всех потоков.
* `set scheduler-locking on/off` - никогда не надо забывать, если хотите вызывать функции многопоточной программы из gdb. Блокирует/включает работу других тредов (кроме того, на котором вы остановились в gdb). 



```cpp
%%cpp segfault.cpp

#include<stdio.h>

int access(int* a, int i) { 
    return a[i]; 
}

int main() {
    int a[2] = {41, 42}, i = 100501;
    printf("a[%d] = %d\n", i, access(a, i)); // проезд по памяти
}
```

Бывает просто полезно запустить программу под gdb


```python
# компилируем с отладочной информацией и запускаем под gdb
!gcc -g segfault.cpp -o segfault.exe
!gdb -ex=r -batch --args ./segfault.exe
```

    
    Program received signal SIGSEGV, Segmentation fault.
    0x000055555555518c in access (a=0x7fffffffdce0, i=100501) at segfault.cpp:6
    6	    return a[i]; 


Если запустили не под GDB, но коркнулось - не беда)


```python
!sudo sysctl kernel.core_pattern="./core"              # Говорим складывать корки рядом в файлик core
!ulimit -c unlimited ; ./segfault.exe                  # Устанавливаем системный лимит и запускаем. Оно коркается.
!gdb -ex "bt" -ex "p i" -batch ./segfault.exe ./core   # Смотрим на корку с помощью GDB
```

    kernel.core_pattern = ./core
    Segmentation fault (core dumped)
    [New LWP 206584]
    Core was generated by `./segfault.exe'.
    Program terminated with signal SIGSEGV, Segmentation fault.
    #0  0x000055aee2c9718c in access (a=0x7ffce3101bd0, i=100501) at segfault.cpp:6
    6	    return a[i]; 
    #0  0x000055aee2c9718c in access (a=0x7ffce3101bd0, i=100501) at segfault.cpp:6
    #1  0x000055aee2c971d1 in main () at segfault.cpp:11
    $1 = 100501



```python
!gdb -ex "frame 1" -ex "p a" -batch ./segfault.exe ./core 
```

    [New LWP 206584]
    Core was generated by `./segfault.exe'.
    Program terminated with signal SIGSEGV, Segmentation fault.
    #0  0x000055aee2c9718c in access (a=0x7ffce3101bd0, i=100501) at segfault.cpp:6
    6	    return a[i]; 
    #1  0x000055aee2c971d1 in main () at segfault.cpp:11
    11	    printf("a[%d] = %d\n", i, access(a, i)); // проезд по памяти
    $1 = {41, 42}


Можно вмешаться в ход программы. Например, уровень логирования так можно подрутить экстренно


```python
!gdb -ex "b segfault.cpp:6" -ex "r" -ex "set var i = 0" -ex "c" -batch --args ./segfault.exe
```

    Breakpoint 1 at 0x1178: file segfault.cpp, line 6.
    
    Breakpoint 1, access (a=0x7fffffffdce0, i=100501) at segfault.cpp:6
    6	    return a[i]; 
    a[100501] = 41
    [Inferior 1 (process 206594) exited normally]


### <a name="sanitizers"></a> Sanitizers

#### <a name="asan_segv"></a> ASAN и проезды по памяти

`ASAN_OPTIONS=verbosity=10 ./segfault.exe` - регулировка уровня многословности asan.


```cpp
%%cpp segfault_access.cpp

#include<stdio.h>

int a[] = {41, 42};

int get_element(int i) {
    return a[i]; // проезд по памяти при некорректных i
}
```


```cpp
%%cpp segfault_2.cpp

#include<stdio.h>

int get_element(int i);

int main() {
    printf("%d\n", get_element(100500));// проезд по памяти
}
```


```python
# компилируем и запускаем как обычно
!gcc segfault_2.cpp segfault_access.cpp -o segfault.exe
!./segfault.exe
```

    Segmentation fault



```python
# компилируем с санитайзером и запускаем как обычно (семинарист рекомендует)
!gcc -g -fsanitize=address segfault_2.cpp segfault_access.cpp -o segfault.exe
!./segfault.exe
```

    AddressSanitizer:DEADLYSIGNAL
    =================================================================
    [1m[31m==206617==ERROR: AddressSanitizer: SEGV on unknown address 0x5569aa7a12b0 (pc 0x5569aa73c2c2 bp 0x7fff87b44ae0 sp 0x7fff87b44ad0 T0)
    [1m[0m==206617==The signal is caused by a READ memory access.
        #0 0x5569aa73c2c1 in get_element(int) /home/pechatnov/vbox/caos/sem01-instruments-compilation-libc/segfault_access.cpp:8
        #1 0x5569aa73c1fa in main /home/pechatnov/vbox/caos/sem01-instruments-compilation-libc/segfault_2.cpp:8
        #2 0x7fa6dd4da0b2 in __libc_start_main (/lib/x86_64-linux-gnu/libc.so.6+0x270b2)
        #3 0x5569aa73c12d in _start (/home/pechatnov/vbox/caos/sem01-instruments-compilation-libc/segfault.exe+0x112d)
    
    AddressSanitizer can not provide additional info.
    SUMMARY: AddressSanitizer: SEGV /home/pechatnov/vbox/caos/sem01-instruments-compilation-libc/segfault_access.cpp:8 in get_element(int)
    ==206617==ABORTING


Как работает ASAN? Генерирует код с проверками.


```python
!gcc -c segfault_access.cpp -Os -fno-asynchronous-unwind-tables -fcf-protection=branch -mmanual-endbr 
!gdb segfault_access.o -batch -ex="disass get_element"
```

    Dump of assembler code for function _Z11get_elementi:
       0x0000000000000000 <+0>:	movslq %edi,%rdi
       0x0000000000000003 <+3>:	lea    0x0(%rip),%rax        # 0xa <_Z11get_elementi+10>
       0x000000000000000a <+10>:	mov    (%rax,%rdi,4),%eax
       0x000000000000000d <+13>:	retq   
    End of assembler dump.



```python
!gcc -fsanitize=address -c segfault_access.cpp -Os -fno-asynchronous-unwind-tables -fcf-protection=branch -mmanual-endbr 
!gdb segfault_access.o -batch -ex="disass get_element"
```

    Dump of assembler code for function _Z11get_elementi:
       0x0000000000000000 <+0>:	movslq %edi,%rdi
       0x0000000000000003 <+3>:	lea    0x0(%rip),%rax        # 0xa <_Z11get_elementi+10>
       0x000000000000000a <+10>:	lea    (%rax,%rdi,4),%r8
       0x000000000000000e <+14>:	mov    %r8,%rdx
       0x0000000000000011 <+17>:	shr    $0x3,%rdx
       0x0000000000000015 <+21>:	mov    0x7fff8000(%rdx),%cl
       0x000000000000001b <+27>:	mov    %r8,%rdx
       0x000000000000001e <+30>:	and    $0x7,%edx
       0x0000000000000021 <+33>:	add    $0x3,%edx
       0x0000000000000024 <+36>:	cmp    %cl,%dl
       0x0000000000000026 <+38>:	jl     0x35 <_Z11get_elementi+53>
       0x0000000000000028 <+40>:	test   %cl,%cl
       0x000000000000002a <+42>:	je     0x35 <_Z11get_elementi+53>
       0x000000000000002c <+44>:	push   %rax
       0x000000000000002d <+45>:	mov    %r8,%rdi
       0x0000000000000030 <+48>:	callq  0x35 <_Z11get_elementi+53>
       0x0000000000000035 <+53>:	mov    (%rax,%rdi,4),%eax
       0x0000000000000038 <+56>:	retq   
    End of assembler dump.


#### <a name="asan_leak"></a> ASAN: Обнаружение утечек памяти с помощью address-санитайзера


```cpp
%%cpp memory_leak.c

#include <stdio.h>
#include <stdlib.h>

int main() {
    malloc(16);
    return 0;
}
```


```python
# компилируем с санитайзером и запускаем как обычно
!gcc -fsanitize=address memory_leak.c -o memory_leak.exe
!./memory_leak.exe
```

    
    =================================================================
    [1m[31m==206639==ERROR: LeakSanitizer: detected memory leaks
    [1m[0m
    [1m[34mDirect leak of 16 byte(s) in 1 object(s) allocated from:
    [1m[0m    #0 0x7f2b8bb37bc8 in malloc (/lib/x86_64-linux-gnu/libasan.so.5+0x10dbc8)
        #1 0x55c53c47619a in main (/home/pechatnov/vbox/caos/sem01-instruments-compilation-libc/memory_leak.exe+0x119a)
        #2 0x7f2b8b85f0b2 in __libc_start_main (/lib/x86_64-linux-gnu/libc.so.6+0x270b2)
    
    SUMMARY: AddressSanitizer: 16 byte(s) leaked in 1 allocation(s).


**Всегда используйте ASAN при тестировании ваших программ.** За исключением, пожалуй, очень тяжелых тестов и бенчмарков.

### <a name="strace"></a> STRACE: Отладка системных вызовов с помощью strace


```cpp
%%cpp printing.cpp

#include<stdio.h>

int main() {
    printf("Hello, world!");
}
```


```python
# компилируем как обычно и запускаем с strace
!gcc printing.cpp -o printing.exe
!strace ./printing.exe > out.txt 2> err.txt
!echo "Trace:"            ; cat err.txt | sed 's/^/    /'
!echo "Program output:"   ; cat out.txt | sed 's/^/    /'
```

    Trace:
        execve("./printing.exe", ["./printing.exe"], 0x7ffc30c56a60 /* 68 vars */) = 0
        brk(NULL)                               = 0x56365d22a000
        arch_prctl(0x3001 /* ARCH_??? */, 0x7ffcda271e30) = -1 EINVAL (Invalid argument)
        access("/etc/ld.so.preload", R_OK)      = -1 ENOENT (No such file or directory)
        openat(AT_FDCWD, "/etc/ld.so.cache", O_RDONLY|O_CLOEXEC) = 3
        fstat(3, {st_mode=S_IFREG|0644, st_size=105883, ...}) = 0
        mmap(NULL, 105883, PROT_READ, MAP_PRIVATE, 3, 0) = 0x7f4084cf4000
        close(3)                                = 0
        openat(AT_FDCWD, "/lib/x86_64-linux-gnu/libc.so.6", O_RDONLY|O_CLOEXEC) = 3
        read(3, "\177ELF\2\1\1\3\0\0\0\0\0\0\0\0\3\0>\0\1\0\0\0\360q\2\0\0\0\0\0"..., 832) = 832
        pread64(3, "\6\0\0\0\4\0\0\0@\0\0\0\0\0\0\0@\0\0\0\0\0\0\0@\0\0\0\0\0\0\0"..., 784, 64) = 784
        pread64(3, "\4\0\0\0\20\0\0\0\5\0\0\0GNU\0\2\0\0\300\4\0\0\0\3\0\0\0\0\0\0\0", 32, 848) = 32
        pread64(3, "\4\0\0\0\24\0\0\0\3\0\0\0GNU\0cBR\340\305\370\2609W\242\345)q\235A\1"..., 68, 880) = 68
        fstat(3, {st_mode=S_IFREG|0755, st_size=2029224, ...}) = 0
        mmap(NULL, 8192, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = 0x7f4084cf2000
        pread64(3, "\6\0\0\0\4\0\0\0@\0\0\0\0\0\0\0@\0\0\0\0\0\0\0@\0\0\0\0\0\0\0"..., 784, 64) = 784
        pread64(3, "\4\0\0\0\20\0\0\0\5\0\0\0GNU\0\2\0\0\300\4\0\0\0\3\0\0\0\0\0\0\0", 32, 848) = 32
        pread64(3, "\4\0\0\0\24\0\0\0\3\0\0\0GNU\0cBR\340\305\370\2609W\242\345)q\235A\1"..., 68, 880) = 68
        mmap(NULL, 2036952, PROT_READ, MAP_PRIVATE|MAP_DENYWRITE, 3, 0) = 0x7f4084b00000
        mprotect(0x7f4084b25000, 1847296, PROT_NONE) = 0
        mmap(0x7f4084b25000, 1540096, PROT_READ|PROT_EXEC, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x25000) = 0x7f4084b25000
        mmap(0x7f4084c9d000, 303104, PROT_READ, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x19d000) = 0x7f4084c9d000
        mmap(0x7f4084ce8000, 24576, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x1e7000) = 0x7f4084ce8000
        mmap(0x7f4084cee000, 13528, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_ANONYMOUS, -1, 0) = 0x7f4084cee000
        close(3)                                = 0
        arch_prctl(ARCH_SET_FS, 0x7f4084cf3540) = 0
        mprotect(0x7f4084ce8000, 12288, PROT_READ) = 0
        mprotect(0x56365c34d000, 4096, PROT_READ) = 0
        mprotect(0x7f4084d3b000, 4096, PROT_READ) = 0
        munmap(0x7f4084cf4000, 105883)          = 0
        fstat(1, {st_mode=S_IFREG|0664, st_size=0, ...}) = 0
        brk(NULL)                               = 0x56365d22a000
        brk(0x56365d24b000)                     = 0x56365d24b000
        write(1, "Hello, world!", 13)           = 13
        exit_group(0)                           = ?
        +++ exited with 0 +++
    Program output:
        Hello, world!


```python

```


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
%run gcc -std=gnu11 -m32 -masm=intel -nostdlib -Os -s no_main_func.c -o no_main_func.exe 
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


Run: `gcc -std=gnu11 -m32 -masm=intel -nostdlib -Os -s no_main_func.c -o no_main_func.exe`



Run: `./no_main_func.exe`


    Hello world from 'syscall'!



```cpp
%%cpp main_func.c
%run gcc -std=gnu11 -Os -s main_func.c -o main_func.exe 
%run ./main_func.exe

#include <stdio.h>

int main() {
    printf("Hello world from 'syscall'!\n");
    return 0;
}
```


Run: `gcc -std=gnu11 -Os -s main_func.c -o main_func.exe`



Run: `./main_func.exe`


    Hello world from 'syscall'!



```python
!stat ./no_main_func.exe ; echo -n "\n"
!stat ./main_func.exe
```

      File: ./no_main_func.exe
      Size: 13076     	Blocks: 32         IO Block: 4096   regular file
    Device: 805h/2053d	Inode: 4723931     Links: 1
    Access: (0775/-rwxrwxr-x)  Uid: ( 1000/pechatnov)   Gid: ( 1000/pechatnov)
    Access: 2022-09-10 16:35:43.672306045 +0300
    Modify: 2022-09-10 16:35:43.556303628 +0300
    Change: 2022-09-10 16:35:43.556303628 +0300
     Birth: -
    
      File: ./main_func.exe
      Size: 14472     	Blocks: 32         IO Block: 4096   regular file
    Device: 805h/2053d	Inode: 4723932     Links: 1
    Access: (0775/-rwxrwxr-x)  Uid: ( 1000/pechatnov)   Gid: ( 1000/pechatnov)
    Access: 2022-09-10 16:35:43.968312212 +0300
    Modify: 2022-09-10 16:35:43.852309797 +0300
    Change: 2022-09-10 16:35:43.852309797 +0300
     Birth: -



```python
!echo no_main_func.exe: && ldd ./no_main_func.exe
!echo main_func.exe: && ldd ./main_func.exe
```

    no_main_func.exe:
    	statically linked
    main_func.exe:
    	linux-vdso.so.1 (0x00007ffeddf5f000)
    	libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (0x00007f324fa90000)
    	/lib64/ld-linux-x86-64.so.2 (0x00007f324fca3000)



```python
!perf stat ./no_main_func.exe   2>&1 | grep -E -v '<not supported>|^$' ; echo -n "\n"
!perf stat ./main_func.exe      2>&1 | grep -E -v '<not supported>|^$'
```

    Hello world from 'syscall'!
     Performance counter stats for './no_main_func.exe':
                  0,17 msec task-clock                #    0,266 CPUs utilized          
                     0      context-switches          #    0,000 K/sec                  
                     0      cpu-migrations            #    0,000 K/sec                  
                    16      page-faults               #    0,095 M/sec                  
           0,000632771 seconds time elapsed
           0,000542000 seconds user
           0,000000000 seconds sys
    
    Hello world from 'syscall'!
     Performance counter stats for './main_func.exe':
                  0,51 msec task-clock                #    0,358 CPUs utilized          
                     1      context-switches          #    0,002 M/sec                  
                     1      cpu-migrations            #    0,002 M/sec                  
                    55      page-faults               #    0,108 M/sec                  
           0,001426995 seconds time elapsed
           0,000922000 seconds user
           0,000000000 seconds sys



```python
# stace на статически скомпилированню программу
!strace ./main_func.exe > out.txt 2> err.txt
!echo main_func.exe:
!echo "    Trace:"            ; cat err.txt | sed 's/^/        /' | cut -c -80
!echo "    Program output:"   ; cat out.txt | sed 's/^/        /'
!strace ./no_main_func.exe > out.txt 2> err.txt
!echo no_main_func.exe:
!echo "    Trace:"            ; cat err.txt | sed 's/^/        /' | cut -c -80
!echo "    Program output:"   ; cat out.txt | sed 's/^/        /'
```

    main_func.exe:
        Trace:
            execve("./main_func.exe", ["./main_func.exe"], 0x7fff00fda000 /* 68 vars
            brk(NULL)                               = 0x55ef536fb000
            arch_prctl(0x3001 /* ARCH_??? */, 0x7ffd6164def0) = -1 EINVAL (Invalid a
            access("/etc/ld.so.preload", R_OK)      = -1 ENOENT (No such file or dir
            openat(AT_FDCWD, "/etc/ld.so.cache", O_RDONLY|O_CLOEXEC) = 3
            fstat(3, {st_mode=S_IFREG|0644, st_size=105883, ...}) = 0
            mmap(NULL, 105883, PROT_READ, MAP_PRIVATE, 3, 0) = 0x7f3de3645000
            close(3)                                = 0
            openat(AT_FDCWD, "/lib/x86_64-linux-gnu/libc.so.6", O_RDONLY|O_CLOEXEC) 
            read(3, "\177ELF\2\1\1\3\0\0\0\0\0\0\0\0\3\0>\0\1\0\0\0\360q\2\0\0\0\0\0
            pread64(3, "\6\0\0\0\4\0\0\0@\0\0\0\0\0\0\0@\0\0\0\0\0\0\0@\0\0\0\0\0\0\
            pread64(3, "\4\0\0\0\20\0\0\0\5\0\0\0GNU\0\2\0\0\300\4\0\0\0\3\0\0\0\0\0
            pread64(3, "\4\0\0\0\24\0\0\0\3\0\0\0GNU\0cBR\340\305\370\2609W\242\345)
            fstat(3, {st_mode=S_IFREG|0755, st_size=2029224, ...}) = 0
            mmap(NULL, 8192, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0)
            pread64(3, "\6\0\0\0\4\0\0\0@\0\0\0\0\0\0\0@\0\0\0\0\0\0\0@\0\0\0\0\0\0\
            pread64(3, "\4\0\0\0\20\0\0\0\5\0\0\0GNU\0\2\0\0\300\4\0\0\0\3\0\0\0\0\0
            pread64(3, "\4\0\0\0\24\0\0\0\3\0\0\0GNU\0cBR\340\305\370\2609W\242\345)
            mmap(NULL, 2036952, PROT_READ, MAP_PRIVATE|MAP_DENYWRITE, 3, 0) = 0x7f3d
            mprotect(0x7f3de3476000, 1847296, PROT_NONE) = 0
            mmap(0x7f3de3476000, 1540096, PROT_READ|PROT_EXEC, MAP_PRIVATE|MAP_FIXED
            mmap(0x7f3de35ee000, 303104, PROT_READ, MAP_PRIVATE|MAP_FIXED|MAP_DENYWR
            mmap(0x7f3de3639000, 24576, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|
            mmap(0x7f3de363f000, 13528, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|
            close(3)                                = 0
            arch_prctl(ARCH_SET_FS, 0x7f3de3644540) = 0
            mprotect(0x7f3de3639000, 12288, PROT_READ) = 0
            mprotect(0x55ef51be5000, 4096, PROT_READ) = 0
            mprotect(0x7f3de368c000, 4096, PROT_READ) = 0
            munmap(0x7f3de3645000, 105883)          = 0
            fstat(1, {st_mode=S_IFREG|0664, st_size=0, ...}) = 0
            brk(NULL)                               = 0x55ef536fb000
            brk(0x55ef5371c000)                     = 0x55ef5371c000
            write(1, "Hello world from 'syscall'!\n", 28) = 28
            exit_group(0)                           = ?
            +++ exited with 0 +++
        Program output:
            Hello world from 'syscall'!
    no_main_func.exe:
        Trace:
            execve("./no_main_func.exe", ["./no_main_func.exe"], 0x7fffbc49b330 /* 6
            strace: [ Process PID=206749 runs in 32 bit mode. ]
            brk(NULL)                               = 0x57050000
            arch_prctl(0x3001 /* ARCH_??? */, 0xffc78c88) = -1 EINVAL (Invalid argum
            access("/etc/ld.so.nohwcap", F_OK)      = -1 ENOENT (No such file or dir
            mmap2(NULL, 8192, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0
            access("/etc/ld.so.preload", R_OK)      = -1 ENOENT (No such file or dir
            set_thread_area({entry_number=-1, base_addr=0xf7fac9c0, limit=0x0fffff, 
            mprotect(0x56610000, 4096, PROT_READ)   = 0
            write(1, "Hello world from 'syscall'!\n", 28) = 28
            exit(0)                                 = ?
            +++ exited with 0 +++
        Program output:
            Hello world from 'syscall'!



```python

```

## <a name="macro"></a> Дополнение: макросы в C/C++ </a>

[Статья про макросы / opennet](https://www.opennet.ru/docs/RUS/cpp/cpp-5.html)

* 

<details> <summary> Почему использование макросов стоит минимизировать </summary>
  <pre> <code> 
Тимур Демченко
расскажите, пожалуйста, почему объявление констант в c++ через define считается плохим кодстайлом
Yuri Pechatnov
Макросы в принципе считаются плохим кодстайлом, но они очень полезны бывают
Но не в случае #define CONST 5, так как это хорошо заменяется на constexpr int CONST = 5;
Mikhail Tsion
Because all macros (which are what #defines define) are in a single namespace and they take effect everywhere. Variables, including const-qualified variables, can be encapsulated in classes and namespaces.
Macros are used in C because in C, a const-qualified variable is not actually a constant, it is just a variable that cannot be modified. A const-qualified variable cannot appear in a constant expression, so it can't be used as an array size, for example.
In C++, a const-qualified object that is initialized with a constant expression (like const int x = 5 * 2;) is a constant and can be used in a constant expression, so you can and should use them.
Yuri Pechatnov
Ну и они вне неймспейсов, да
Если использовать много библиотек, то можно наткнуться на клеш дефайнов, или получить еще что-то удивительное в плохом смысле
  </code> </pre>
</details>


* Макросы это именно макросы, они ничего не знают про синтаксис С/С++


```cpp
%%cpp macro_example_0.c
%run gcc -E macro_example_0.c -o macro_example_0_E.c
%run cat macro_example_0_E.c

#define people students and students
#define goodbye(var) Good bye var! 

Hello people!
#undef people
Hello people!
goodbye(bad grades)
```


Run: `gcc -E macro_example_0.c -o macro_example_0_E.c`



Run: `cat macro_example_0_E.c`


    # 1 "macro_example_0.c"
    # 1 "<built-in>"
    # 1 "<command-line>"
    # 31 "<command-line>"
    # 1 "/usr/include/stdc-predef.h" 1 3 4
    # 32 "<command-line>" 2
    # 1 "macro_example_0.c"
    
    
    
    
    
    
    
    Hello students and students!
    
    Hello people!
    Good bye bad grades!


* Переменные в макросах это просто куски исходного текста
* При передаче аргументов в макрос стоит помнить, что значение имеют только запятые `,` и скобки `()`


```cpp
%%cpp macro_example_0_2.c
%run gcc -E macro_example_0_2.c -o macro_example_0_2_E.c
%run cat macro_example_0_2_E.c

#define macro(type, var, value) type var = value;

// #define protect(...) __VA_ARGS__

macro(protect(std::pair<int, int>), a, protect({1, 2, 3}))
```


Run: `gcc -E macro_example_0_2.c -o macro_example_0_2_E.c`



Run: `cat macro_example_0_2_E.c`


    # 1 "macro_example_0_2.c"
    # 1 "<built-in>"
    # 1 "<command-line>"
    # 31 "<command-line>"
    # 1 "/usr/include/stdc-predef.h" 1 3 4
    # 32 "<command-line>" 2
    # 1 "macro_example_0_2.c"
    # 9 "macro_example_0_2.c"
    protect(std::pair<int, int>) a = protect({1, 2, 3});


Больше примеров


```cpp
%%cpp macro_example.c
%run gcc macro_example.c -o macro_example.exe
%run ./macro_example.exe
%run gcc -DDEBUG macro_example.c -o macro_example.exe
%run ./macro_example.exe

#include <stdio.h>

#if !defined(DEBUG)
//#ifndef DEBUG
    #define DEBUG 0
#endif


#define CONST_A 123

#define mult(a, b) ((a) * (b))

#define mult_bad(a, b) (a * b)

// Склеивание имен
#define add_prefix_aba_(w) aba_##w

int main() {
    printf("START\n");
    #if DEBUG
        const char* file_name = "001.txt";
        printf("Read from '%s'. DEBUG define is enabled!\n", file_name);
        freopen(file_name, "rt", stdin);
    #endif

    printf("CONST_A %d\n", CONST_A);
    printf("mult(4, 6) = %d\n", mult(2 + 2, 3 + 3));
    printf("mult_bad(4, 6) = %d\n", mult_bad(2 + 2, 3 + 3));

    int aba_x = 42;
    int x = 420;
    printf("aba_x ? x = %d\n", add_prefix_aba_(x)); // aba_x
    
    printf("DEBUG = %d\n", DEBUG);

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
    DEBUG = 0



Run: `gcc -DDEBUG macro_example.c -o macro_example.exe`



Run: `./macro_example.exe`


    START
    Read from '001.txt'. DEBUG define is enabled!
    CONST_A 123
    mult(4, 6) = 24
    mult_bad(4, 6) = 11
    aba_x ? x = 42
    DEBUG = 1


И полезных примеров:


```cpp
%%cpp macro_example_2.c
%run gcc macro_example_2.c -o macro_example_2.exe -fsanitize=address
%run ./macro_example_2.exe

#include <stdio.h>
#include <string.h>
#include <assert.h>

/* #VAR_NAME разворачивается в строковый литерал "VAR_NAME" */
#define print_int(i) printf(#i " = %d\n", (i));

/* Полезный макрос для вывода в поток ошибок */
#define eprintf(...) fprintf(stderr, __VA_ARGS__)

#define logprintf_impl(fmt, line, ...) eprintf(__FILE__ ":" #line " " fmt, __VA_ARGS__)
#define logprintf_impl_2(line, fmt, ...) logprintf_impl(fmt "%s", line, __VA_ARGS__)
#define logprintf(...) logprintf_impl_2(__LINE__, __VA_ARGS__, "")

#define SWAP(a, b) do { __typeof__(a) __swap_c = (a); (a) = (b); (b) = (__swap_c); } while (0)

/* Способ сделать макрос с переменным числом аргументов
 * И это единственный способ "перегрузить функцию в С" */
#define sum_2(a, b, c) ((a) + (b))
#define sum_3(a, b, c) ((a) + (b) + (c))

#define sum_impl(a, b, c, sum_func, ...) sum_func(a, b, c)

#define sum(...) sum_impl(__VA_ARGS__, sum_3, sum_2)


int main() {
    /* assert(3 > 4); */
    print_int(9 * 9 + 1);

    eprintf("It is in stderr: %d\n", 431);

    int x = 1, y = 2;
    eprintf("(x, y) = (%d, %d)\n", x, y);
    SWAP(x, y);
    eprintf("(x, y) = (%d, %d)\n", x, y);

    print_int(sum(1, 1));
    print_int(sum(1, 1, 1));
    
    eprintf("%s:%d %s\n", __FILE__, __LINE__, __FUNCTION__);
    
    logprintf("Before exit with code %d\n", 0);
    return 0;
}
```


Run: `gcc macro_example_2.c -o macro_example_2.exe -fsanitize=address`



Run: `./macro_example_2.exe`


    9 * 9 + 1 = 82
    It is in stderr: 431
    (x, y) = (1, 2)
    (x, y) = (2, 1)
    sum(1, 1) = 2
    sum(1, 1, 1) = 3
    macro_example_2.c:45 main
    macro_example_2.c:47 Before exit with code 0



```python
!gcc -E macro_example_2.c -o out &&  cat out
```

    # 1 "macro_example_2.c"
    # 1 "<built-in>"
    # 1 "<command-line>"
    # 31 "<command-line>"
    # 1 "/usr/include/stdc-predef.h" 1 3 4
    # 32 "<command-line>" 2
    # 1 "macro_example_2.c"
    
    
    
    
    # 1 "/usr/include/stdio.h" 1 3 4
    # 27 "/usr/include/stdio.h" 3 4
    # 1 "/usr/include/x86_64-linux-gnu/bits/libc-header-start.h" 1 3 4
    # 33 "/usr/include/x86_64-linux-gnu/bits/libc-header-start.h" 3 4
    # 1 "/usr/include/features.h" 1 3 4
    # 461 "/usr/include/features.h" 3 4
    # 1 "/usr/include/x86_64-linux-gnu/sys/cdefs.h" 1 3 4
    # 452 "/usr/include/x86_64-linux-gnu/sys/cdefs.h" 3 4
    # 1 "/usr/include/x86_64-linux-gnu/bits/wordsize.h" 1 3 4
    # 453 "/usr/include/x86_64-linux-gnu/sys/cdefs.h" 2 3 4
    # 1 "/usr/include/x86_64-linux-gnu/bits/long-double.h" 1 3 4
    # 454 "/usr/include/x86_64-linux-gnu/sys/cdefs.h" 2 3 4
    # 462 "/usr/include/features.h" 2 3 4
    # 485 "/usr/include/features.h" 3 4
    # 1 "/usr/include/x86_64-linux-gnu/gnu/stubs.h" 1 3 4
    # 10 "/usr/include/x86_64-linux-gnu/gnu/stubs.h" 3 4
    # 1 "/usr/include/x86_64-linux-gnu/gnu/stubs-64.h" 1 3 4
    # 11 "/usr/include/x86_64-linux-gnu/gnu/stubs.h" 2 3 4
    # 486 "/usr/include/features.h" 2 3 4
    # 34 "/usr/include/x86_64-linux-gnu/bits/libc-header-start.h" 2 3 4
    # 28 "/usr/include/stdio.h" 2 3 4
    
    
    
    
    
    # 1 "/usr/lib/gcc/x86_64-linux-gnu/9/include/stddef.h" 1 3 4
    # 209 "/usr/lib/gcc/x86_64-linux-gnu/9/include/stddef.h" 3 4
    
    # 209 "/usr/lib/gcc/x86_64-linux-gnu/9/include/stddef.h" 3 4
    typedef long unsigned int size_t;
    # 34 "/usr/include/stdio.h" 2 3 4
    
    
    # 1 "/usr/lib/gcc/x86_64-linux-gnu/9/include/stdarg.h" 1 3 4
    # 40 "/usr/lib/gcc/x86_64-linux-gnu/9/include/stdarg.h" 3 4
    typedef __builtin_va_list __gnuc_va_list;
    # 37 "/usr/include/stdio.h" 2 3 4
    
    # 1 "/usr/include/x86_64-linux-gnu/bits/types.h" 1 3 4
    # 27 "/usr/include/x86_64-linux-gnu/bits/types.h" 3 4
    # 1 "/usr/include/x86_64-linux-gnu/bits/wordsize.h" 1 3 4
    # 28 "/usr/include/x86_64-linux-gnu/bits/types.h" 2 3 4
    # 1 "/usr/include/x86_64-linux-gnu/bits/timesize.h" 1 3 4
    # 29 "/usr/include/x86_64-linux-gnu/bits/types.h" 2 3 4
    
    
    typedef unsigned char __u_char;
    typedef unsigned short int __u_short;
    typedef unsigned int __u_int;
    typedef unsigned long int __u_long;
    
    
    typedef signed char __int8_t;
    typedef unsigned char __uint8_t;
    typedef signed short int __int16_t;
    typedef unsigned short int __uint16_t;
    typedef signed int __int32_t;
    typedef unsigned int __uint32_t;
    
    typedef signed long int __int64_t;
    typedef unsigned long int __uint64_t;
    
    
    
    
    
    
    typedef __int8_t __int_least8_t;
    typedef __uint8_t __uint_least8_t;
    typedef __int16_t __int_least16_t;
    typedef __uint16_t __uint_least16_t;
    typedef __int32_t __int_least32_t;
    typedef __uint32_t __uint_least32_t;
    typedef __int64_t __int_least64_t;
    typedef __uint64_t __uint_least64_t;
    
    
    
    typedef long int __quad_t;
    typedef unsigned long int __u_quad_t;
    
    
    
    
    
    
    
    typedef long int __intmax_t;
    typedef unsigned long int __uintmax_t;
    # 141 "/usr/include/x86_64-linux-gnu/bits/types.h" 3 4
    # 1 "/usr/include/x86_64-linux-gnu/bits/typesizes.h" 1 3 4
    # 142 "/usr/include/x86_64-linux-gnu/bits/types.h" 2 3 4
    # 1 "/usr/include/x86_64-linux-gnu/bits/time64.h" 1 3 4
    # 143 "/usr/include/x86_64-linux-gnu/bits/types.h" 2 3 4
    
    
    typedef unsigned long int __dev_t;
    typedef unsigned int __uid_t;
    typedef unsigned int __gid_t;
    typedef unsigned long int __ino_t;
    typedef unsigned long int __ino64_t;
    typedef unsigned int __mode_t;
    typedef unsigned long int __nlink_t;
    typedef long int __off_t;
    typedef long int __off64_t;
    typedef int __pid_t;
    typedef struct { int __val[2]; } __fsid_t;
    typedef long int __clock_t;
    typedef unsigned long int __rlim_t;
    typedef unsigned long int __rlim64_t;
    typedef unsigned int __id_t;
    typedef long int __time_t;
    typedef unsigned int __useconds_t;
    typedef long int __suseconds_t;
    
    typedef int __daddr_t;
    typedef int __key_t;
    
    
    typedef int __clockid_t;
    
    
    typedef void * __timer_t;
    
    
    typedef long int __blksize_t;
    
    
    
    
    typedef long int __blkcnt_t;
    typedef long int __blkcnt64_t;
    
    
    typedef unsigned long int __fsblkcnt_t;
    typedef unsigned long int __fsblkcnt64_t;
    
    
    typedef unsigned long int __fsfilcnt_t;
    typedef unsigned long int __fsfilcnt64_t;
    
    
    typedef long int __fsword_t;
    
    typedef long int __ssize_t;
    
    
    typedef long int __syscall_slong_t;
    
    typedef unsigned long int __syscall_ulong_t;
    
    
    
    typedef __off64_t __loff_t;
    typedef char *__caddr_t;
    
    
    typedef long int __intptr_t;
    
    
    typedef unsigned int __socklen_t;
    
    
    
    
    typedef int __sig_atomic_t;
    # 39 "/usr/include/stdio.h" 2 3 4
    # 1 "/usr/include/x86_64-linux-gnu/bits/types/__fpos_t.h" 1 3 4
    
    
    
    
    # 1 "/usr/include/x86_64-linux-gnu/bits/types/__mbstate_t.h" 1 3 4
    # 13 "/usr/include/x86_64-linux-gnu/bits/types/__mbstate_t.h" 3 4
    typedef struct
    {
      int __count;
      union
      {
        unsigned int __wch;
        char __wchb[4];
      } __value;
    } __mbstate_t;
    # 6 "/usr/include/x86_64-linux-gnu/bits/types/__fpos_t.h" 2 3 4
    
    
    
    
    typedef struct _G_fpos_t
    {
      __off_t __pos;
      __mbstate_t __state;
    } __fpos_t;
    # 40 "/usr/include/stdio.h" 2 3 4
    # 1 "/usr/include/x86_64-linux-gnu/bits/types/__fpos64_t.h" 1 3 4
    # 10 "/usr/include/x86_64-linux-gnu/bits/types/__fpos64_t.h" 3 4
    typedef struct _G_fpos64_t
    {
      __off64_t __pos;
      __mbstate_t __state;
    } __fpos64_t;
    # 41 "/usr/include/stdio.h" 2 3 4
    # 1 "/usr/include/x86_64-linux-gnu/bits/types/__FILE.h" 1 3 4
    
    
    
    struct _IO_FILE;
    typedef struct _IO_FILE __FILE;
    # 42 "/usr/include/stdio.h" 2 3 4
    # 1 "/usr/include/x86_64-linux-gnu/bits/types/FILE.h" 1 3 4
    
    
    
    struct _IO_FILE;
    
    
    typedef struct _IO_FILE FILE;
    # 43 "/usr/include/stdio.h" 2 3 4
    # 1 "/usr/include/x86_64-linux-gnu/bits/types/struct_FILE.h" 1 3 4
    # 35 "/usr/include/x86_64-linux-gnu/bits/types/struct_FILE.h" 3 4
    struct _IO_FILE;
    struct _IO_marker;
    struct _IO_codecvt;
    struct _IO_wide_data;
    
    
    
    
    typedef void _IO_lock_t;
    
    
    
    
    
    struct _IO_FILE
    {
      int _flags;
    
    
      char *_IO_read_ptr;
      char *_IO_read_end;
      char *_IO_read_base;
      char *_IO_write_base;
      char *_IO_write_ptr;
      char *_IO_write_end;
      char *_IO_buf_base;
      char *_IO_buf_end;
    
    
      char *_IO_save_base;
      char *_IO_backup_base;
      char *_IO_save_end;
    
      struct _IO_marker *_markers;
    
      struct _IO_FILE *_chain;
    
      int _fileno;
      int _flags2;
      __off_t _old_offset;
    
    
      unsigned short _cur_column;
      signed char _vtable_offset;
      char _shortbuf[1];
    
      _IO_lock_t *_lock;
    
    
    
    
    
    
    
      __off64_t _offset;
    
      struct _IO_codecvt *_codecvt;
      struct _IO_wide_data *_wide_data;
      struct _IO_FILE *_freeres_list;
      void *_freeres_buf;
      size_t __pad5;
      int _mode;
    
      char _unused2[15 * sizeof (int) - 4 * sizeof (void *) - sizeof (size_t)];
    };
    # 44 "/usr/include/stdio.h" 2 3 4
    # 52 "/usr/include/stdio.h" 3 4
    typedef __gnuc_va_list va_list;
    # 63 "/usr/include/stdio.h" 3 4
    typedef __off_t off_t;
    # 77 "/usr/include/stdio.h" 3 4
    typedef __ssize_t ssize_t;
    
    
    
    
    
    
    typedef __fpos_t fpos_t;
    # 133 "/usr/include/stdio.h" 3 4
    # 1 "/usr/include/x86_64-linux-gnu/bits/stdio_lim.h" 1 3 4
    # 134 "/usr/include/stdio.h" 2 3 4
    
    
    
    extern FILE *stdin;
    extern FILE *stdout;
    extern FILE *stderr;
    
    
    
    
    
    
    extern int remove (const char *__filename) __attribute__ ((__nothrow__ , __leaf__));
    
    extern int rename (const char *__old, const char *__new) __attribute__ ((__nothrow__ , __leaf__));
    
    
    
    extern int renameat (int __oldfd, const char *__old, int __newfd,
           const char *__new) __attribute__ ((__nothrow__ , __leaf__));
    # 173 "/usr/include/stdio.h" 3 4
    extern FILE *tmpfile (void) ;
    # 187 "/usr/include/stdio.h" 3 4
    extern char *tmpnam (char *__s) __attribute__ ((__nothrow__ , __leaf__)) ;
    
    
    
    
    extern char *tmpnam_r (char *__s) __attribute__ ((__nothrow__ , __leaf__)) ;
    # 204 "/usr/include/stdio.h" 3 4
    extern char *tempnam (const char *__dir, const char *__pfx)
         __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__malloc__)) ;
    
    
    
    
    
    
    
    extern int fclose (FILE *__stream);
    
    
    
    
    extern int fflush (FILE *__stream);
    # 227 "/usr/include/stdio.h" 3 4
    extern int fflush_unlocked (FILE *__stream);
    # 246 "/usr/include/stdio.h" 3 4
    extern FILE *fopen (const char *__restrict __filename,
          const char *__restrict __modes) ;
    
    
    
    
    extern FILE *freopen (const char *__restrict __filename,
            const char *__restrict __modes,
            FILE *__restrict __stream) ;
    # 279 "/usr/include/stdio.h" 3 4
    extern FILE *fdopen (int __fd, const char *__modes) __attribute__ ((__nothrow__ , __leaf__)) ;
    # 292 "/usr/include/stdio.h" 3 4
    extern FILE *fmemopen (void *__s, size_t __len, const char *__modes)
      __attribute__ ((__nothrow__ , __leaf__)) ;
    
    
    
    
    extern FILE *open_memstream (char **__bufloc, size_t *__sizeloc) __attribute__ ((__nothrow__ , __leaf__)) ;
    
    
    
    
    
    extern void setbuf (FILE *__restrict __stream, char *__restrict __buf) __attribute__ ((__nothrow__ , __leaf__));
    
    
    
    extern int setvbuf (FILE *__restrict __stream, char *__restrict __buf,
          int __modes, size_t __n) __attribute__ ((__nothrow__ , __leaf__));
    
    
    
    
    extern void setbuffer (FILE *__restrict __stream, char *__restrict __buf,
             size_t __size) __attribute__ ((__nothrow__ , __leaf__));
    
    
    extern void setlinebuf (FILE *__stream) __attribute__ ((__nothrow__ , __leaf__));
    
    
    
    
    
    
    
    extern int fprintf (FILE *__restrict __stream,
          const char *__restrict __format, ...);
    
    
    
    
    extern int printf (const char *__restrict __format, ...);
    
    extern int sprintf (char *__restrict __s,
          const char *__restrict __format, ...) __attribute__ ((__nothrow__));
    
    
    
    
    
    extern int vfprintf (FILE *__restrict __s, const char *__restrict __format,
           __gnuc_va_list __arg);
    
    
    
    
    extern int vprintf (const char *__restrict __format, __gnuc_va_list __arg);
    
    extern int vsprintf (char *__restrict __s, const char *__restrict __format,
           __gnuc_va_list __arg) __attribute__ ((__nothrow__));
    
    
    
    extern int snprintf (char *__restrict __s, size_t __maxlen,
           const char *__restrict __format, ...)
         __attribute__ ((__nothrow__)) __attribute__ ((__format__ (__printf__, 3, 4)));
    
    extern int vsnprintf (char *__restrict __s, size_t __maxlen,
            const char *__restrict __format, __gnuc_va_list __arg)
         __attribute__ ((__nothrow__)) __attribute__ ((__format__ (__printf__, 3, 0)));
    # 379 "/usr/include/stdio.h" 3 4
    extern int vdprintf (int __fd, const char *__restrict __fmt,
           __gnuc_va_list __arg)
         __attribute__ ((__format__ (__printf__, 2, 0)));
    extern int dprintf (int __fd, const char *__restrict __fmt, ...)
         __attribute__ ((__format__ (__printf__, 2, 3)));
    
    
    
    
    
    
    
    extern int fscanf (FILE *__restrict __stream,
         const char *__restrict __format, ...) ;
    
    
    
    
    extern int scanf (const char *__restrict __format, ...) ;
    
    extern int sscanf (const char *__restrict __s,
         const char *__restrict __format, ...) __attribute__ ((__nothrow__ , __leaf__));
    
    
    
    
    
    
    extern int fscanf (FILE *__restrict __stream, const char *__restrict __format, ...) __asm__ ("" "__isoc99_fscanf")
    
                                   ;
    extern int scanf (const char *__restrict __format, ...) __asm__ ("" "__isoc99_scanf")
                                  ;
    extern int sscanf (const char *__restrict __s, const char *__restrict __format, ...) __asm__ ("" "__isoc99_sscanf") __attribute__ ((__nothrow__ , __leaf__))
    
                          ;
    # 432 "/usr/include/stdio.h" 3 4
    extern int vfscanf (FILE *__restrict __s, const char *__restrict __format,
          __gnuc_va_list __arg)
         __attribute__ ((__format__ (__scanf__, 2, 0))) ;
    
    
    
    
    
    extern int vscanf (const char *__restrict __format, __gnuc_va_list __arg)
         __attribute__ ((__format__ (__scanf__, 1, 0))) ;
    
    
    extern int vsscanf (const char *__restrict __s,
          const char *__restrict __format, __gnuc_va_list __arg)
         __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__format__ (__scanf__, 2, 0)));
    
    
    
    
    extern int vfscanf (FILE *__restrict __s, const char *__restrict __format, __gnuc_va_list __arg) __asm__ ("" "__isoc99_vfscanf")
    
    
    
         __attribute__ ((__format__ (__scanf__, 2, 0))) ;
    extern int vscanf (const char *__restrict __format, __gnuc_va_list __arg) __asm__ ("" "__isoc99_vscanf")
    
         __attribute__ ((__format__ (__scanf__, 1, 0))) ;
    extern int vsscanf (const char *__restrict __s, const char *__restrict __format, __gnuc_va_list __arg) __asm__ ("" "__isoc99_vsscanf") __attribute__ ((__nothrow__ , __leaf__))
    
    
    
         __attribute__ ((__format__ (__scanf__, 2, 0)));
    # 485 "/usr/include/stdio.h" 3 4
    extern int fgetc (FILE *__stream);
    extern int getc (FILE *__stream);
    
    
    
    
    
    extern int getchar (void);
    
    
    
    
    
    
    extern int getc_unlocked (FILE *__stream);
    extern int getchar_unlocked (void);
    # 510 "/usr/include/stdio.h" 3 4
    extern int fgetc_unlocked (FILE *__stream);
    # 521 "/usr/include/stdio.h" 3 4
    extern int fputc (int __c, FILE *__stream);
    extern int putc (int __c, FILE *__stream);
    
    
    
    
    
    extern int putchar (int __c);
    # 537 "/usr/include/stdio.h" 3 4
    extern int fputc_unlocked (int __c, FILE *__stream);
    
    
    
    
    
    
    
    extern int putc_unlocked (int __c, FILE *__stream);
    extern int putchar_unlocked (int __c);
    
    
    
    
    
    
    extern int getw (FILE *__stream);
    
    
    extern int putw (int __w, FILE *__stream);
    
    
    
    
    
    
    
    extern char *fgets (char *__restrict __s, int __n, FILE *__restrict __stream)
         ;
    # 603 "/usr/include/stdio.h" 3 4
    extern __ssize_t __getdelim (char **__restrict __lineptr,
                                 size_t *__restrict __n, int __delimiter,
                                 FILE *__restrict __stream) ;
    extern __ssize_t getdelim (char **__restrict __lineptr,
                               size_t *__restrict __n, int __delimiter,
                               FILE *__restrict __stream) ;
    
    
    
    
    
    
    
    extern __ssize_t getline (char **__restrict __lineptr,
                              size_t *__restrict __n,
                              FILE *__restrict __stream) ;
    
    
    
    
    
    
    
    extern int fputs (const char *__restrict __s, FILE *__restrict __stream);
    
    
    
    
    
    extern int puts (const char *__s);
    
    
    
    
    
    
    extern int ungetc (int __c, FILE *__stream);
    
    
    
    
    
    
    extern size_t fread (void *__restrict __ptr, size_t __size,
           size_t __n, FILE *__restrict __stream) ;
    
    
    
    
    extern size_t fwrite (const void *__restrict __ptr, size_t __size,
            size_t __n, FILE *__restrict __s);
    # 673 "/usr/include/stdio.h" 3 4
    extern size_t fread_unlocked (void *__restrict __ptr, size_t __size,
             size_t __n, FILE *__restrict __stream) ;
    extern size_t fwrite_unlocked (const void *__restrict __ptr, size_t __size,
              size_t __n, FILE *__restrict __stream);
    
    
    
    
    
    
    
    extern int fseek (FILE *__stream, long int __off, int __whence);
    
    
    
    
    extern long int ftell (FILE *__stream) ;
    
    
    
    
    extern void rewind (FILE *__stream);
    # 707 "/usr/include/stdio.h" 3 4
    extern int fseeko (FILE *__stream, __off_t __off, int __whence);
    
    
    
    
    extern __off_t ftello (FILE *__stream) ;
    # 731 "/usr/include/stdio.h" 3 4
    extern int fgetpos (FILE *__restrict __stream, fpos_t *__restrict __pos);
    
    
    
    
    extern int fsetpos (FILE *__stream, const fpos_t *__pos);
    # 757 "/usr/include/stdio.h" 3 4
    extern void clearerr (FILE *__stream) __attribute__ ((__nothrow__ , __leaf__));
    
    extern int feof (FILE *__stream) __attribute__ ((__nothrow__ , __leaf__)) ;
    
    extern int ferror (FILE *__stream) __attribute__ ((__nothrow__ , __leaf__)) ;
    
    
    
    extern void clearerr_unlocked (FILE *__stream) __attribute__ ((__nothrow__ , __leaf__));
    extern int feof_unlocked (FILE *__stream) __attribute__ ((__nothrow__ , __leaf__)) ;
    extern int ferror_unlocked (FILE *__stream) __attribute__ ((__nothrow__ , __leaf__)) ;
    
    
    
    
    
    
    
    extern void perror (const char *__s);
    
    
    
    
    
    # 1 "/usr/include/x86_64-linux-gnu/bits/sys_errlist.h" 1 3 4
    # 26 "/usr/include/x86_64-linux-gnu/bits/sys_errlist.h" 3 4
    extern int sys_nerr;
    extern const char *const sys_errlist[];
    # 782 "/usr/include/stdio.h" 2 3 4
    
    
    
    
    extern int fileno (FILE *__stream) __attribute__ ((__nothrow__ , __leaf__)) ;
    
    
    
    
    extern int fileno_unlocked (FILE *__stream) __attribute__ ((__nothrow__ , __leaf__)) ;
    # 800 "/usr/include/stdio.h" 3 4
    extern FILE *popen (const char *__command, const char *__modes) ;
    
    
    
    
    
    extern int pclose (FILE *__stream);
    
    
    
    
    
    extern char *ctermid (char *__s) __attribute__ ((__nothrow__ , __leaf__));
    # 840 "/usr/include/stdio.h" 3 4
    extern void flockfile (FILE *__stream) __attribute__ ((__nothrow__ , __leaf__));
    
    
    
    extern int ftrylockfile (FILE *__stream) __attribute__ ((__nothrow__ , __leaf__)) ;
    
    
    extern void funlockfile (FILE *__stream) __attribute__ ((__nothrow__ , __leaf__));
    # 858 "/usr/include/stdio.h" 3 4
    extern int __uflow (FILE *);
    extern int __overflow (FILE *, int);
    # 873 "/usr/include/stdio.h" 3 4
    
    # 6 "macro_example_2.c" 2
    # 1 "/usr/include/string.h" 1 3 4
    # 26 "/usr/include/string.h" 3 4
    # 1 "/usr/include/x86_64-linux-gnu/bits/libc-header-start.h" 1 3 4
    # 27 "/usr/include/string.h" 2 3 4
    
    
    
    
    
    
    # 1 "/usr/lib/gcc/x86_64-linux-gnu/9/include/stddef.h" 1 3 4
    # 34 "/usr/include/string.h" 2 3 4
    # 43 "/usr/include/string.h" 3 4
    extern void *memcpy (void *__restrict __dest, const void *__restrict __src,
           size_t __n) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2)));
    
    
    extern void *memmove (void *__dest, const void *__src, size_t __n)
         __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2)));
    
    
    
    
    
    extern void *memccpy (void *__restrict __dest, const void *__restrict __src,
            int __c, size_t __n)
         __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2)));
    
    
    
    
    extern void *memset (void *__s, int __c, size_t __n) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1)));
    
    
    extern int memcmp (const void *__s1, const void *__s2, size_t __n)
         __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));
    # 91 "/usr/include/string.h" 3 4
    extern void *memchr (const void *__s, int __c, size_t __n)
          __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));
    # 122 "/usr/include/string.h" 3 4
    extern char *strcpy (char *__restrict __dest, const char *__restrict __src)
         __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2)));
    
    extern char *strncpy (char *__restrict __dest,
            const char *__restrict __src, size_t __n)
         __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2)));
    
    
    extern char *strcat (char *__restrict __dest, const char *__restrict __src)
         __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2)));
    
    extern char *strncat (char *__restrict __dest, const char *__restrict __src,
            size_t __n) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2)));
    
    
    extern int strcmp (const char *__s1, const char *__s2)
         __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));
    
    extern int strncmp (const char *__s1, const char *__s2, size_t __n)
         __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));
    
    
    extern int strcoll (const char *__s1, const char *__s2)
         __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));
    
    extern size_t strxfrm (char *__restrict __dest,
             const char *__restrict __src, size_t __n)
         __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (2)));
    
    
    
    # 1 "/usr/include/x86_64-linux-gnu/bits/types/locale_t.h" 1 3 4
    # 22 "/usr/include/x86_64-linux-gnu/bits/types/locale_t.h" 3 4
    # 1 "/usr/include/x86_64-linux-gnu/bits/types/__locale_t.h" 1 3 4
    # 28 "/usr/include/x86_64-linux-gnu/bits/types/__locale_t.h" 3 4
    struct __locale_struct
    {
    
      struct __locale_data *__locales[13];
    
    
      const unsigned short int *__ctype_b;
      const int *__ctype_tolower;
      const int *__ctype_toupper;
    
    
      const char *__names[13];
    };
    
    typedef struct __locale_struct *__locale_t;
    # 23 "/usr/include/x86_64-linux-gnu/bits/types/locale_t.h" 2 3 4
    
    typedef __locale_t locale_t;
    # 154 "/usr/include/string.h" 2 3 4
    
    
    extern int strcoll_l (const char *__s1, const char *__s2, locale_t __l)
         __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2, 3)));
    
    
    extern size_t strxfrm_l (char *__dest, const char *__src, size_t __n,
        locale_t __l) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (2, 4)));
    
    
    
    
    
    extern char *strdup (const char *__s)
         __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__malloc__)) __attribute__ ((__nonnull__ (1)));
    
    
    
    
    
    
    extern char *strndup (const char *__string, size_t __n)
         __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__malloc__)) __attribute__ ((__nonnull__ (1)));
    # 226 "/usr/include/string.h" 3 4
    extern char *strchr (const char *__s, int __c)
         __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));
    # 253 "/usr/include/string.h" 3 4
    extern char *strrchr (const char *__s, int __c)
         __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));
    # 273 "/usr/include/string.h" 3 4
    extern size_t strcspn (const char *__s, const char *__reject)
         __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));
    
    
    extern size_t strspn (const char *__s, const char *__accept)
         __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));
    # 303 "/usr/include/string.h" 3 4
    extern char *strpbrk (const char *__s, const char *__accept)
         __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));
    # 330 "/usr/include/string.h" 3 4
    extern char *strstr (const char *__haystack, const char *__needle)
         __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));
    
    
    
    
    extern char *strtok (char *__restrict __s, const char *__restrict __delim)
         __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (2)));
    
    
    
    extern char *__strtok_r (char *__restrict __s,
        const char *__restrict __delim,
        char **__restrict __save_ptr)
         __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (2, 3)));
    
    extern char *strtok_r (char *__restrict __s, const char *__restrict __delim,
             char **__restrict __save_ptr)
         __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (2, 3)));
    # 385 "/usr/include/string.h" 3 4
    extern size_t strlen (const char *__s)
         __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));
    
    
    
    
    extern size_t strnlen (const char *__string, size_t __maxlen)
         __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));
    
    
    
    
    extern char *strerror (int __errnum) __attribute__ ((__nothrow__ , __leaf__));
    # 410 "/usr/include/string.h" 3 4
    extern int strerror_r (int __errnum, char *__buf, size_t __buflen) __asm__ ("" "__xpg_strerror_r") __attribute__ ((__nothrow__ , __leaf__))
    
                            __attribute__ ((__nonnull__ (2)));
    # 428 "/usr/include/string.h" 3 4
    extern char *strerror_l (int __errnum, locale_t __l) __attribute__ ((__nothrow__ , __leaf__));
    
    
    
    # 1 "/usr/include/strings.h" 1 3 4
    # 23 "/usr/include/strings.h" 3 4
    # 1 "/usr/lib/gcc/x86_64-linux-gnu/9/include/stddef.h" 1 3 4
    # 24 "/usr/include/strings.h" 2 3 4
    
    
    
    
    
    
    
    
    
    
    extern int bcmp (const void *__s1, const void *__s2, size_t __n)
         __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));
    
    
    extern void bcopy (const void *__src, void *__dest, size_t __n)
      __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2)));
    
    
    extern void bzero (void *__s, size_t __n) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1)));
    # 68 "/usr/include/strings.h" 3 4
    extern char *index (const char *__s, int __c)
         __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));
    # 96 "/usr/include/strings.h" 3 4
    extern char *rindex (const char *__s, int __c)
         __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));
    
    
    
    
    
    
    extern int ffs (int __i) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__));
    
    
    
    
    
    extern int ffsl (long int __l) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__));
    __extension__ extern int ffsll (long long int __ll)
         __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__));
    
    
    
    extern int strcasecmp (const char *__s1, const char *__s2)
         __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));
    
    
    extern int strncasecmp (const char *__s1, const char *__s2, size_t __n)
         __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));
    
    
    
    
    
    
    extern int strcasecmp_l (const char *__s1, const char *__s2, locale_t __loc)
         __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2, 3)));
    
    
    
    extern int strncasecmp_l (const char *__s1, const char *__s2,
         size_t __n, locale_t __loc)
         __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2, 4)));
    
    
    
    # 433 "/usr/include/string.h" 2 3 4
    
    
    
    extern void explicit_bzero (void *__s, size_t __n) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1)));
    
    
    
    extern char *strsep (char **__restrict __stringp,
           const char *__restrict __delim)
         __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2)));
    
    
    
    
    extern char *strsignal (int __sig) __attribute__ ((__nothrow__ , __leaf__));
    
    
    extern char *__stpcpy (char *__restrict __dest, const char *__restrict __src)
         __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2)));
    extern char *stpcpy (char *__restrict __dest, const char *__restrict __src)
         __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2)));
    
    
    
    extern char *__stpncpy (char *__restrict __dest,
       const char *__restrict __src, size_t __n)
         __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2)));
    extern char *stpncpy (char *__restrict __dest,
            const char *__restrict __src, size_t __n)
         __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2)));
    # 499 "/usr/include/string.h" 3 4
    
    # 7 "macro_example_2.c" 2
    # 1 "/usr/include/assert.h" 1 3 4
    # 66 "/usr/include/assert.h" 3 4
    
    
    
    extern void __assert_fail (const char *__assertion, const char *__file,
          unsigned int __line, const char *__function)
         __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__noreturn__));
    
    
    extern void __assert_perror_fail (int __errnum, const char *__file,
          unsigned int __line, const char *__function)
         __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__noreturn__));
    
    
    
    
    extern void __assert (const char *__assertion, const char *__file, int __line)
         __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__noreturn__));
    
    
    
    # 8 "macro_example_2.c" 2
    # 31 "macro_example_2.c"
    
    # 31 "macro_example_2.c"
    int main() {
    
        printf("9 * 9 + 1" " = %d\n", (9 * 9 + 1));;
    
        fprintf(
    # 35 "macro_example_2.c" 3 4
       stderr
    # 35 "macro_example_2.c"
       , "It is in stderr: %d\n", 431);
    
        int x = 1, y = 2;
        fprintf(
    # 38 "macro_example_2.c" 3 4
       stderr
    # 38 "macro_example_2.c"
       , "(x, y) = (%d, %d)\n", x, y);
        do { __typeof__(x) __swap_c = (x); (x) = (y); (y) = (__swap_c); } while (0);
        fprintf(
    # 40 "macro_example_2.c" 3 4
       stderr
    # 40 "macro_example_2.c"
       , "(x, y) = (%d, %d)\n", x, y);
    
        printf("sum(1, 1)" " = %d\n", (((1) + (1))));;
        printf("sum(1, 1, 1)" " = %d\n", (((1) + (1) + (1))));;
    
        fprintf(
    # 45 "macro_example_2.c" 3 4
       stderr
    # 45 "macro_example_2.c"
       , "%s:%d %s\n", "macro_example_2.c", 45, __FUNCTION__);
    
        fprintf(
    # 47 "macro_example_2.c" 3 4
       stderr
    # 47 "macro_example_2.c"
       , "macro_example_2.c" ":" "47" " " "Before exit with code %d\n" "%s", 0, "");
        return 0;
    }


Можно упороться и сделать себе подобие деструкторов локальных переменных в Си:


```cpp
%%cpp macro_local_vars.c
%run gcc -fsanitize=address macro_local_vars.c -o macro_local_vars.exe
%run echo -n "Hello123" > a.txt
%run ./macro_local_vars.exe

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

struct defer_record {
    struct defer_record* previous;
    void (*func) (void*);
    void* arg;
};

#define _EXECUTE_DEFERRED(to) do { \
    while (last_defer_record != to) { \
        last_defer_record->func(last_defer_record->arg); \
        last_defer_record = last_defer_record->previous; \
    } \
} while (0)

// Интересная особенность, но здесь нужна глубина раскрытия 2, чтобы __LINE__ правильно подставился
#define _DEFER_NAME_2(line) defer_record_ ## line
#define _DEFER_NAME(line) _DEFER_NAME_2(line)

// Добавляем элемент в односвязанный список отложенных функций
#define DEFER(func, arg) \
    struct defer_record _DEFER_NAME(__LINE__) = {last_defer_record, (void (*)(void*))func, (void*)arg}; \
    last_defer_record = &_DEFER_NAME(__LINE__);

// DFB = Defer Friendly Block
// Запоминаем начала блока, в котором может использоватсья DEFER (но не во вложенных блоках!)
#define DFB_BEGIN \
    struct defer_record* first_defer_record = last_defer_record; \
    { \
        struct defer_record* last_defer_record = first_defer_record; 

// Конец блока (выполнение отложенных функций)
#define DFB_END \
        _EXECUTE_DEFERRED(first_defer_record); \
    } 

// Запоминаем начала блока функции
#define DFB_FUNCTION_BEGIN \
    struct defer_record* last_defer_record = NULL; \
    DFB_BEGIN 

// Запоминаем начала блока следующего после for, while, do
#define DFB_BREAKABLE_BEGIN \
    struct defer_record* first_breakable_defer_record = last_defer_record; \
    DFB_BEGIN

// DF = Defer Friendly
#define DF_RETURN(value) { \
    _EXECUTE_DEFERRED(NULL); \
    return value; \
}

#define DF_BREAK { \
    _EXECUTE_DEFERRED(first_breakable_defer_record); \
    break; \
} 


void func(int i) { DFB_FUNCTION_BEGIN
    void* data = malloc(145); DEFER(free, data);
    void* data2 = malloc(14); DEFER(free, data2);
    if (i % 10 == 0) {
        DF_RETURN();
    }
    if (i % 4 == 0) {
        while (1) { DFB_BREAKABLE_BEGIN
            void* data = malloc(145); DEFER(free, data);
            if (++i > 99) {
                DF_BREAK;
            }
        DFB_END }
        
        DF_RETURN();
    }
    
DFB_END } 

int main() { DFB_FUNCTION_BEGIN 
    void* data = malloc(145); DEFER(free, data);
    
    { DFB_BEGIN   
        void* data = malloc(145); DEFER(free, data);
    DFB_END }
            
    { DFB_BEGIN   
        int fd = open("a.txt", O_RDONLY);
        if (fd < 1) {
            fprintf(stderr, "Can't open file\n");
            DF_RETURN(-1);
        }
        DEFER(close, (size_t)fd);
        char buff[10];
        int len = read(fd, buff, sizeof(buff) - 1);
        buff[len] = '\0';
        printf("Read string '%s'\n", buff);
    DFB_END }
        
    
    for (int i = 0; i < 100; ++i) { DFB_BREAKABLE_BEGIN    
        void* data = malloc(145); DEFER(free, data);
        if (i % 10 == 0) { DFB_BEGIN
            DF_BREAK;
        DFB_END }
    DFB_END }
            
    DF_RETURN(0);
DFB_END }
```


Run: `gcc -fsanitize=address macro_local_vars.c -o macro_local_vars.exe`



Run: `echo -n "Hello123" > a.txt`



Run: `./macro_local_vars.exe`


    Read string 'Hello123'


# Тест

[Google-форма с тестом](https://docs.google.com/forms/d/e/1FAIpQLScXEHUnrnqPxDCLPYHK5wXyA-zXbrNPFncdKcveg1prloUJnQ/viewform?usp=sf_link)


```python
from IPython.display import display, IFrame, Markdown
display(IFrame("https://vclock.com/embed/timer/#countdown=00:05:00&showmessage=0&theme=0&ampm=1&sound=xylophone", width=560, height=300))

```



<iframe
    width="560"
    height="300"
    src="https://vclock.com/embed/timer/#countdown=00:05:00&showmessage=0&theme=0&ampm=1&sound=xylophone"
    frameborder="0"
    allowfullscreen
></iframe>




```python

```
