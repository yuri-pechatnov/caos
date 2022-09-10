

# Инструменты разработки

Видеозаписи этого семинара нет (звук не записался). Но если очень нужно [запись старого семинара про этапы компиляции, запуск и заершение программы](https://www.youtube.com/watch?v=E8a0m6HG2x8&list=PLjzMm8llUm4AmU6i_hPU0NobgA4VsBowc&index=3) и [запись старого семинара про санитайзеры](https://www.youtube.com/watch?v=R_P4FSxH1AY&list=PLjzMm8llUm4AmU6i_hPU0NobgA4VsBowc&index=13).

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

### <a name="compilation"></a> Компиляция

Превратим исходный код в ассемблерный.


```python
# здесь необязательно брать результат работы препроцессора
# -Os -fno-asynchronous-unwind-tables помогает получить более короткий выхлоп
# -fverbose-asm позволит получить более длинный, но более удобный для чтения
!gcc -S preprocessing_max_E.c -o preprocessing_max_E.S -Os -Wl,--gc-sections -fno-asynchronous-unwind-tables -fcf-protection=branch -mmanual-endbr 
!cat preprocessing_max_E.S 
```

Способ получить ассемблерный код функции. Правда тут не просто проводится компиляция, здесь создается машинный код, а потом gdb его обратно дизассемблирует.


```python
!gcc -c preprocessing_max_E.c -o preprocessing_max.o -Os -fno-asynchronous-unwind-tables -fcf-protection=branch -mmanual-endbr 
!gdb preprocessing_max.o -batch -ex="disass f"
```

Еще один способ


```python
!gcc -g -c preprocessing_max.c -Os -fno-asynchronous-unwind-tables -fcf-protection=branch -mmanual-endbr 
!objdump -d -S preprocessing_max.o
```


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


```python
!echo lib.o:  ; hexdump -C lib.o | head -n 2  # обратите внимание на 0x7f E L F - магическое начало файла
```


```python
!echo lib.a:    ; hexdump -C lib.a  | head -n 2 # заметим, что это не ELF но тоже имеет магическое начало
!echo libb.so:  ; hexdump -C lib.so | head -n 2 # а это тоже ELF
```


```python
!echo preprocessing_max.exe:  ; hexdump -C preprocessing_max.exe | head -n 2 # а это тоже ELF
```


```python
!objdump -t lib.o | grep sum  # symbols in shared library
```


```python
!objdump -t lib.so | grep sum  # symbols in shared library
```


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

Если запустили не под GDB, но коркнулось - не беда)


```python
!sudo sysctl kernel.core_pattern="./core"              # Говорим складывать корки рядом в файлик core
!ulimit -c unlimited ; ./segfault.exe                  # Устанавливаем системный лимит и запускаем. Оно коркается.
!gdb -ex "bt" -ex "p i" -batch ./segfault.exe ./core   # Смотрим на корку с помощью GDB
```


```python
!gdb -ex "frame 1" -ex "p a" -batch ./segfault.exe ./core 
```

Можно вмешаться в ход программы. Например, уровень логирования так можно подрутить экстренно


```python
!gdb -ex "b segfault.cpp:6" -ex "r" -ex "set var i = 0" -ex "c" -batch --args ./segfault.exe
```

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


```python
# компилируем с санитайзером и запускаем как обычно (семинарист рекомендует)
!gcc -g -fsanitize=address segfault_2.cpp segfault_access.cpp -o segfault.exe
!./segfault.exe
```

Как работает ASAN? Генерирует код с проверками.


```python
!gcc -c segfault_access.cpp -Os -fno-asynchronous-unwind-tables -fcf-protection=branch -mmanual-endbr 
!gdb segfault_access.o -batch -ex="disass get_element"
```


```python
!gcc -fsanitize=address -c segfault_access.cpp -Os -fno-asynchronous-unwind-tables -fcf-protection=branch -mmanual-endbr 
!gdb segfault_access.o -batch -ex="disass get_element"
```

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


```python
!stat ./no_main_func.exe ; echo -n "\n"
!stat ./main_func.exe
```


```python
!echo no_main_func.exe: && ldd ./no_main_func.exe
!echo main_func.exe: && ldd ./main_func.exe
```


```python
!perf stat ./no_main_func.exe   2>&1 | grep -E -v '<not supported>|^$' ; echo -n "\n"
!perf stat ./main_func.exe      2>&1 | grep -E -v '<not supported>|^$'
```


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


```python
!gcc -E macro_example_2.c -o out &&  cat out
```

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


```python

```


```python

```


```python

```


```python

```
