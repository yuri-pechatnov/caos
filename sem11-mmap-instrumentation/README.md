```python
get_ipython().run_cell_magic('javascript', '', '// setup cpp code highlighting\nIPython.CodeCell.options_default.highlight_modes["text/x-c++src"] = {\'reg\':[/^%%cpp/]} ;')

# creating magics
from IPython.core.magic import register_cell_magic, register_line_magic
from IPython.display import display, Markdown

@register_cell_magic
def save_file(fname, cell, line_comment_start="#"):
    cell = cell if cell[-1] == '\n' else cell + "\n"
    cmds = []
    with open(fname, "w") as f:
        f.write(line_comment_start + " %%cpp " + fname + "\n")
        for line in cell.split("\n"):
            if line.startswith("%"):
                run_prefix = "%run "
                assert line.startswith(run_prefix)
                cmds.append(line[len(run_prefix):].strip())
            else:
                f.write(line + "\n")
    for cmd in cmds:
        display(Markdown("Run: `%s`" % cmd))
        get_ipython().system(cmd)

@register_cell_magic
def cpp(fname, cell):
    save_file(fname, cell, "//")

@register_cell_magic
def asm(fname, cell):
    save_file(fname, cell, "//")
    
@register_cell_magic
def makefile(fname, cell):
    assert not fname
    save_file("makefile", cell.replace(" " * 4, "\t"))
        
@register_line_magic
def p(line):
    try:
        expr, comment = line.split(" #")
        display(Markdown("`{} = {}`  # {}".format(expr.strip(), eval(expr), comment.strip())))
    except:
        display(Markdown("{} = {}".format(line, eval(line))))
    
```


    <IPython.core.display.Javascript object>


# Инструментирование в linux и mmap

## Инструментирование

Что это такое? Можно считать, что отладка. Получение информации о работающей программе и вмешательство в ее работу. Или не о программе, а о ядре системы. Известный вам пример - gdb.

Инструментарий для отладки
* Статический - прям в коде: счетчики, метрики (пример: санитайзеры)
  * Оверхед
  * Занимают место в коде
  * ...
* Динамический - над кодом (примеры: gdb, strace, [eBPF](https://habr.com/ru/post/435142/))
  * Динамически можно выбирать, на что смотреть
  * ...

Разные подходы к инструментированию:
* Трейсинг - обрабатывать события. (пример: strace)
* Семплинг - условно смотреть состояние системы 100 раз в секунду. (пример: perf)

### Примеры использования для обнаружения проезда по памяти


```python
%%cpp segfault.cpp

#include<stdio.h>

int main() {
    int a[2];
    printf("%d\n", a[100500]); // проезд по памяти
}
```


```python
# компилируем и запускаем как обычно
!gcc segfault.cpp -o segfault.exe
!./segfault.exe
```

    Segmentation fault (core dumped)



```python
# компилируем с санитайзером и запускаем как обычно (семинарист рекомендует)
!gcc -fsanitize=address segfault.cpp -o segfault.exe
!./segfault.exe
```

    ASAN:SIGSEGV
    =================================================================
    [1m[31m==14219==ERROR: AddressSanitizer: SEGV on unknown address 0x7ffeb42de5a0 (pc 0x0000004009d9 bp 0x7ffeb427c3b0 sp 0x7ffeb427c330 T0)
    [1m[0m    #0 0x4009d8 in main /home/pechatnov/vbox/caos_2019-2020/sem11-mmap-instrumentation/segfault.cpp:7
        #1 0x7f6ac249d82f in __libc_start_main (/lib/x86_64-linux-gnu/libc.so.6+0x2082f)
        #2 0x400848 in _start (/home/pechatnov/vbox/caos_2019-2020/sem11-mmap-instrumentation/segfault.exe+0x400848)
    
    AddressSanitizer can not provide additional info.
    SUMMARY: AddressSanitizer: SEGV /home/pechatnov/vbox/caos_2019-2020/sem11-mmap-instrumentation/segfault.cpp:7 main
    ==14219==ABORTING



```python
# комбинируем санитайзер и gdb (это семинарист рекомендует, если вы хотите больше подробностей)
# по идее это должно находить больше косяков, чем вариант в следующей ячейке
!gcc -g -fsanitize=address segfault.cpp -o segfault.exe
!gdb -ex=r -batch --args ./segfault.exe
```

    [Thread debugging using libthread_db enabled]
    Using host libthread_db library "/lib/x86_64-linux-gnu/libthread_db.so.1".
    
    Program received signal SIGSEGV, Segmentation fault.
    0x00000000004009b3 in main () at segfault.cpp:7
    7	    printf("%d\n", a[100500]); // проезд по памяти



```python
# компилируем с отладочной информацией и запускаем под gdb
!gcc -g segfault.cpp -o segfault.exe
!gdb -ex=r -batch --args ./segfault.exe
```

    
    Program received signal SIGBUS, Bus error.
    main () at segfault.cpp:7
    7	    printf("%d\n", a[100500]); // проезд по памяти



```python
# компилируем как обычно и запускаем с valgrind
!gcc segfault.cpp -o segfault.exe
!valgrind --tool=memcheck ./segfault.exe 2>&1 | head -n 8 # берем только первые 8 строк выхлопа, а то там много
```

    ==12284== Memcheck, a memory error detector
    ==12284== Copyright (C) 2002-2015, and GNU GPL'd, by Julian Seward et al.
    ==12284== Using Valgrind-3.11.0 and LibVEX; rerun with -h for copyright info
    ==12284== Command: ./segfault.exe
    ==12284== 
    ==12284== Invalid read of size 4
    ==12284==    at 0x4005AD: main (in /home/pechatnov/vbox/caos_2019-2020/sem11-mmap-instrumentation/segfault.exe)
    ==12284==  Address 0xfff062330 is not stack'd, malloc'd or (recently) free'd
    Segmentation fault (core dumped)


### Примеры использования для обнаружения утечек памяти


```python
%%cpp memory_leak.cpp

#include<stdlib.h>

int main() {
    malloc(16);
}
```


```python
# компилируем как обычно и запускаем с valgrind
!gcc memory_leak.cpp -o memory_leak.exe
!valgrind --tool=memcheck --leak-check=full ./memory_leak.exe 2>&1 
```

    ==12377== Memcheck, a memory error detector
    ==12377== Copyright (C) 2002-2015, and GNU GPL'd, by Julian Seward et al.
    ==12377== Using Valgrind-3.11.0 and LibVEX; rerun with -h for copyright info
    ==12377== Command: ./memory_leak.exe
    ==12377== 
    ==12377== 
    ==12377== HEAP SUMMARY:
    ==12377==     in use at exit: 16 bytes in 1 blocks
    ==12377==   total heap usage: 1 allocs, 0 frees, 16 bytes allocated
    ==12377== 
    ==12377== 16 bytes in 1 blocks are definitely lost in loss record 1 of 1
    ==12377==    at 0x4C2DB8F: malloc (in /usr/lib/valgrind/vgpreload_memcheck-amd64-linux.so)
    ==12377==    by 0x400533: main (in /home/pechatnov/vbox/caos_2019-2020/sem11-mmap-instrumentation/memory_leak.exe)
    ==12377== 
    ==12377== LEAK SUMMARY:
    ==12377==    definitely lost: 16 bytes in 1 blocks
    ==12377==    indirectly lost: 0 bytes in 0 blocks
    ==12377==      possibly lost: 0 bytes in 0 blocks
    ==12377==    still reachable: 0 bytes in 0 blocks
    ==12377==         suppressed: 0 bytes in 0 blocks
    ==12377== 
    ==12377== For counts of detected and suppressed errors, rerun with: -v
    ==12377== ERROR SUMMARY: 1 errors from 1 contexts (suppressed: 0 from 0)



```python
# компилируем с санитайзером и запускаем как обычно
!gcc -fsanitize=leak memory_leak.cpp -o memory_leak.exe
!./memory_leak.exe
```

    
    =================================================================
    [1m[31m==12407==ERROR: LeakSanitizer: detected memory leaks
    [1m[0m
    [1m[34mDirect leak of 16 byte(s) in 1 object(s) allocated from:
    [1m[0m    #0 0x7f39e8adbafb in __interceptor_malloc (/usr/lib/x86_64-linux-gnu/liblsan.so.0+0xeafb)
        #1 0x400693 in main (/home/pechatnov/vbox/caos_2019-2020/sem11-mmap-instrumentation/memory_leak.exe+0x400693)
        #2 0x7f39e872382f in __libc_start_main (/lib/x86_64-linux-gnu/libc.so.6+0x2082f)
    
    SUMMARY: LeakSanitizer: 16 byte(s) leaked in 1 allocation(s).


### Примеры использования для отладки системных вызовов


```python
%%cpp printing.cpp

#include<stdio.h>

int main() {
    printf("Hello, world!");
}
```


```python
# компилируем как обычно и запускаем с strace
!gcc printing.cpp -o printing.exe
!strace ./printing.exe > out.txt
!echo "Program output:"
!cat out.txt
```

    execve("./printing.exe", ["./printing.exe"], [/* 42 vars */]) = 0
    brk(NULL)                               = 0xf76000
    access("/etc/ld.so.nohwcap", F_OK)      = -1 ENOENT (No such file or directory)
    access("/etc/ld.so.preload", R_OK)      = -1 ENOENT (No such file or directory)
    open("/etc/ld.so.cache", O_RDONLY|O_CLOEXEC) = 3
    fstat(3, {st_mode=S_IFREG|0644, st_size=137278, ...}) = 0
    mmap(NULL, 137278, PROT_READ, MAP_PRIVATE, 3, 0) = 0x7fdd74b2d000
    close(3)                                = 0
    access("/etc/ld.so.nohwcap", F_OK)      = -1 ENOENT (No such file or directory)
    open("/lib/x86_64-linux-gnu/libc.so.6", O_RDONLY|O_CLOEXEC) = 3
    read(3, "\177ELF\2\1\1\3\0\0\0\0\0\0\0\0\3\0>\0\1\0\0\0P\t\2\0\0\0\0\0"..., 832) = 832
    fstat(3, {st_mode=S_IFREG|0755, st_size=1868984, ...}) = 0
    mmap(NULL, 4096, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = 0x7fdd74b2c000
    mmap(NULL, 3971488, PROT_READ|PROT_EXEC, MAP_PRIVATE|MAP_DENYWRITE, 3, 0) = 0x7fdd74560000
    mprotect(0x7fdd74720000, 2097152, PROT_NONE) = 0
    mmap(0x7fdd74920000, 24576, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x1c0000) = 0x7fdd74920000
    mmap(0x7fdd74926000, 14752, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_ANONYMOUS, -1, 0) = 0x7fdd74926000
    close(3)                                = 0
    mmap(NULL, 4096, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = 0x7fdd74b2b000
    mmap(NULL, 4096, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = 0x7fdd74b2a000
    arch_prctl(ARCH_SET_FS, 0x7fdd74b2b700) = 0
    mprotect(0x7fdd74920000, 16384, PROT_READ) = 0
    mprotect(0x600000, 4096, PROT_READ)     = 0
    mprotect(0x7fdd74b4f000, 4096, PROT_READ) = 0
    munmap(0x7fdd74b2d000, 137278)          = 0
    fstat(1, {st_mode=S_IFREG|0664, st_size=0, ...}) = 0
    brk(NULL)                               = 0xf76000
    brk(0xf98000)                           = 0xf98000
    write(1, "Hello, world!", 13)           = 13
    exit_group(0)                           = ?
    +++ exited with 0 +++
    Program output:
    Hello, world!


```python
%%asm printing_asm.S
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
# компилируем как обычно и запускаем с strace
!gcc -m32 -nostdlib printing_asm.S -o printing_asm.exe
!strace ./printing_asm.exe > out.txt
!echo "Program output:"
!cat out.txt
```

    execve("./printing_asm.exe", ["./printing_asm.exe"], [/* 42 vars */]) = 0
    strace: [ Process PID=12489 runs in 32 bit mode. ]
    write(1, "Hello, World!\n", 14)         = 14
    exit(1)                                 = ?
    +++ exited with 1 +++
    Program output:
    Hello, World!


### Примеры использования для CPU профайлинга


```python
%%cpp work_hard.cpp

int work_hard_1(int n) {
    int ret = 0;
    for (int i = 0; i < n; i++) ret ^= i;
    return ret;
}

int work_hard_2(int n) {
    int ret = 0;
    for (int i = 0; i < n; i++) {
        ret ^= work_hard_1(i * 3);
        for (int j = 0; j < i * 2; ++j) {
            ret ^= j;
        }
    }
    return ret;
}

int main() {
    return work_hard_2(10000);
}
```


```python
# компилируем как обычно и запускаем с perf stat
!gcc work_hard.cpp -o work_hard.exe
!echo $PASSWORD | sudo -S perf stat ./work_hard.exe
```

    [sudo] password for pechatnov: 
     Performance counter stats for './work_hard.exe':
    
           1013,217609      task-clock (msec)         #    0,692 CPUs utilized          
                   214      context-switches          #    0,211 K/sec                  
                     0      cpu-migrations            #    0,000 K/sec                  
                    42      page-faults               #    0,041 K/sec                  
       <not supported>      cycles                                                      
       <not supported>      instructions                                                
       <not supported>      branches                                                    
       <not supported>      branch-misses                                               
    
           1,463685759 seconds time elapsed
    


Результат perf report можно отрисовывать в более красивом виде с помощью [flamegraph](https://github.com/brendangregg/FlameGraph)


```python
# компилируем как обычно и запускаем с perf record и потом смотрим результаты
!gcc work_hard.cpp -o work_hard.exe
!echo $PASSWORD | sudo -S perf record ./work_hard.exe 2>&1 > perf.log
!echo $PASSWORD | sudo -S chmod 0666 perf.data
!perf report | cat
```

    [sudo] password for pechatnov: [ perf record: Woken up 1 times to write data ]
    [ perf record: Captured and wrote 0.076 MB perf.data (1746 samples) ]
    [sudo] password for pechatnov: [kernel.kallsyms] with build id 40aa70fa3b5dccac2d277480f60e44dc3ae98dcb not found, continuing without symbols
    # To display the perf.data header info, please use --header/--header-only options.
    #
    #
    # Total Lost Samples: 0
    #
    # Samples: 1K of event 'cpu-clock'
    # Event count (approx.): 436500000
    #
    # Overhead  Command        Shared Object      Symbol                    
    # ........  .............  .................  ..........................
    #
        61.51%  work_hard.exe  work_hard.exe      [.] work_hard_1
        37.97%  work_hard.exe  work_hard.exe      [.] work_hard_2
         0.17%  work_hard.exe  [kernel.kallsyms]  [k] 0xffffffff91a00070
         0.06%  work_hard.exe  [e1000]            [k] e1000_alloc_rx_buffers
         0.06%  work_hard.exe  [e1000]            [k] e1000_clean
         0.06%  work_hard.exe  [kernel.kallsyms]  [k] 0xffffffff90e0362c
         0.06%  work_hard.exe  [kernel.kallsyms]  [k] 0xffffffff90eb772b
         0.06%  work_hard.exe  [kernel.kallsyms]  [k] 0xffffffff9102af8e
         0.06%  work_hard.exe  [kernel.kallsyms]  [k] 0xffffffff9104a9c0
    
    
    #
    # (Cannot load tips.txt file, please install perf!)
    #


Есть еще вариант посчитать количество выполненных программой/функцией инструкций. Оно не всегда хорошо коррелирует со временем выполнения, зато является стабильным значением от запуска к запуску.
Здесь приводить способ не буду, если интересно, связывайтесь со мной отдельно :)

### Примеры использования для поиска UB (undefined behaviour)


```python
%%cpp ub.c

int main(int argc, char** argv) {
    return -argc << 31;
}
```


```python
# компилируем с санитайзером и запускаем как обычно (семинарист рекомендует)
# к сожалению у меня ни gcc, ни g++ не работают с -fsanitize=undefined
# а разбираться не хочется, поэтому clang
!clang -fsanitize=undefined ub.c -o ub.exe
!./ub.exe
```

    [1mub.c:4:18:[1m[31m runtime error: [1m[0m[1mleft shift of negative value -1[1m[0m



```python

```

## MMAP

(Копия [ридинга от Яковлева](https://github.com/victor-yacovlev/mipt-diht-caos/tree/master/practice/mmap))

```c
#include <sys/mman.h>

void *mmap(
    void *addr,    /* рекомендуемый адрес отображения */
    size_t length, /* размер отображения */
    int prot,      /* аттрибуты доступа */
    int flags,     /* флаги совместного отображения */
    int fd,        /* файловый декскриптор фала */
    off_t offset   /* смещение относительно начала файла */
  );

int munmap(void *addr, size_t length) /* освободить отображение */
```

Системный вызов `mmap` предназначен для создания в виртуальном адресном пространстве процесса доступной области по определенному адресу. Эта область может быть как связана с определенным файлом (ранее открытым), так и располагаться в оперативной памяти. Второй способ использования обычно реализуется в функциях `malloc`/`calloc`.

Память можно выделять только постранично. Для большинства архитектур размер одной страницы равен 4Кб, хотя процессоры архитектуры x86_64 поддерживают страницы большего размера: 2Мб и 1Гб.

В общем случае, никогда нельзя полагаться на то, что размер страницы равен 4096 байт. Его можно узнать с помощью команды `getconf` или функции `sysconf`:

```bash
# Bash
> getconf PAGE_SIZE
4096
```
```c
/* Си */
#include <unistd.h>
long page_size = sysconf(_SC_PAGE_SIZE);
```

Параметр `offset` (если используется файл) обязан быть кратным размеру страницы; параметр `length` - нет, но ядро системы округляет это значение до размера страницы в большую сторону. Параметр `addr` (рекомендуемый адрес) может быть равным `NULL`, - в этом случае ядро само назначает адрес в виртуальном адресном пространстве.

При использовании отображения на файл, параметр `length` имеет значение длины отображаемых данных; в случае, если размер файла меньше размера страницы, или отображается его последний небольшой фрагмент, то оставшаяся часть страницы заполняется нулями.

Страницы памяти могут флаги аттрибутов доступа:
 * чтение `PROT_READ`;
 * запись `PROT_WRITE`;
 * выполнение `PROT_EXE`;
 * ничего `PROT_NONE`.

В случае использования отображения на файл, он должен быть открыт на чтение или запись в соответствии с требуемыми аттрибутами доступа.

Флаги `mmap`:
 * `MAP_FIXED` - требует, чтобы память была выделена по указаному в первом аргументе адресу; без этого флага ядро может выбрать адрес, наиболее близкий к указанному.
 * `MAP_ANONYMOUS` - выделить страницы в оперативной памяти, а не связать с файлом.
 * `MAP_SHARED` - выделить страницы, разделяемые с другими процессами; в случае с отображением на файл - синхронизировать изменения так, чтобы они были доступны другим процессам.
 * `MAP_PRIVATE` - в противоположность `MAP_SHARED`, не делать отображение доступным другим процессам. В случае отображения на файл, он доступен для чтения, а созданные процессом изменения, в файл не сохраняются.


```python

```

Пример с mmap. Простенькая программа, делающая циклический сдвиг (как цифры) второго символа в файле.


```python
%%cpp mmap_example.c
%run gcc mmap_example.c -o mmap_example.exe
%run echo "000" > buf.txt && ./mmap_example.exe && cat buf.txt
%run echo "79" > buf.txt && ./mmap_example.exe && cat buf.txt
%run echo "xxx" > buf.txt && ./mmap_example.exe && cat buf.txt

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <assert.h>

int PAGE_SIZE = 0;

int get_page_size() {
    return PAGE_SIZE = PAGE_SIZE ?: sysconf(_SC_PAGE_SIZE);
}

int upper_round_to_page_size(int sz) {
    return (sz + get_page_size() - 1) / get_page_size() * get_page_size();
}

int main() {
    printf("page size = %d\n", get_page_size());
    int fd = open("buf.txt", O_RDWR);
    struct stat s;
    assert(fstat(fd, &s) == 0);
    
    void* mapped = mmap(
        /* desired addr, addr = */ NULL, 
        /* length = */ upper_round_to_page_size(s.st_size), 
        /* access attributes, prot = */ PROT_READ | PROT_WRITE,
        /* flags = */ MAP_SHARED,
        /* fd = */ fd,
        /* offset in file, offset = */ 0
    );
    assert(mapped);
    
    char* buf = mapped;
    
    assert(s.st_size >= 2);
    buf[1] = ('0' <= buf[1] && buf[1] <= '9') ? ((buf[1] - '0' + 1) % 10 + '0') : '0';
    
    assert(munmap(
        /* mapped addr, addr = */ mapped, 
        /* length = */ upper_round_to_page_size(s.st_size)
    ) == 0);
    return 0;
}
```


Run: `gcc mmap_example.c -o mmap_example.exe`



Run: `echo "000" > buf.txt && ./mmap_example.exe && cat buf.txt`


    page size = 4096
    010



Run: `echo "79" > buf.txt && ./mmap_example.exe && cat buf.txt`


    page size = 4096
    70



Run: `echo "xxx" > buf.txt && ./mmap_example.exe && cat buf.txt`


    page size = 4096
    x0x


Еще один пример по мотивам advanced-1. Интерпретируем байты как функцию и выполняем.


```python
%%cpp func.c
%run gcc -g -fPIC func.c -c -o func.o 
%run objdump -F -d func.o | grep my_func

int my_func(int a, int b) {
    return a + b + 1;
}
```


Run: `gcc -g -fPIC func.c -c -o func.o`



Run: `objdump -F -d func.o | grep my_func`


    0000000000000000 <my_func> (File Offset: 0x40):


Замечаем, что `File Offset: 0x40`. То есть в объектном файле функция лежит начиная с `0x40` позиции.


```python
%%cpp mmap_exec_example.c
%run gcc -g mmap_exec_example.c -o mmap_exec_example.exe
%run ./mmap_exec_example.exe

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <assert.h>

int PAGE_SIZE = 0;

int get_page_size() {
    return PAGE_SIZE = PAGE_SIZE ?: sysconf(_SC_PAGE_SIZE);
}

int upper_round_to_page_size(int sz) {
    return (sz + get_page_size() - 1) / get_page_size() * get_page_size();
}

int main() {
    int fd = open("func.o", O_RDWR);
    struct stat s;
    assert(fstat(fd, &s) == 0);
    
    printf("file size = %d / %d\n", (int)s.st_size, upper_round_to_page_size(s.st_size));
    
    void* mapped = mmap(
        /* desired addr, addr = */ NULL, 
        /* length = */ upper_round_to_page_size(s.st_size), 
        /* access attributes, prot = */ PROT_READ | PROT_EXEC | PROT_WRITE, // обратите внимание на PROT_EXEC
        /* flags = */ MAP_SHARED,
        /* fd = */ fd,
        /* offset in file, offset = */ 0
    );
    if (mapped == MAP_FAILED) {
        perror("Can't mmap");
        return -1;
    }
 
    int (*func)(int, int) = ((char*)mapped) + 0x40; // 0x40 - тот самый оффсет
   
    printf("func(1, 1) = %d\n", func(1, 1));
    printf("func(1, 1) = %d\n", func(10, 100));
    printf("func(1, 1) = %d\n", func(40, 5000));
    
    assert(munmap(
        /* mapped addr, addr = */ mapped, 
        /* length = */ upper_round_to_page_size(s.st_size)
    ) == 0);
    return 0;
}
```


Run: `gcc -g mmap_exec_example.c -o mmap_exec_example.exe`


    [01m[Kmmap_exec_example.c:[m[K In function ‘[01m[Kmain[m[K’:
    [01m[Kmmap_exec_example.c:42:29:[m[K [01;35m[Kwarning: [m[Kinitialization from incompatible pointer type [-Wincompatible-pointer-types]
         int (*func)(int, int) = ((char*)mapped) + 0x40;
    [01;32m[K                             ^[m[K



Run: `./mmap_exec_example.exe`


    file size = 2680 / 4096
    func(1, 1) = 3
    func(1, 1) = 111
    func(1, 1) = 5041



```python

```


```python

```


```python

```
