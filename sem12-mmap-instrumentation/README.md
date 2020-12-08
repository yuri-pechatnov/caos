


# Инструментирование в linux и mmap

<p><a href="https://www.youtube.com/watch?v=R_P4FSxH1AY&list=PLjzMm8llUm4AmU6i_hPU0NobgA4VsBowc&index=13" target="_blank">
    <h3>Видеозапись семинара</h3> 
</a></p>


[Ридинг Яковлева](https://github.com/victor-yacovlev/mipt-diht-caos/tree/master/practice/mmap) 


Сегодня в программе:
* <a href="#instr" style="color:#856024"> Инструментирование </a>  
  * <a href="#asan" style="color:#856024"> ASAN: Обнаружение проездов по памяти </a>  
  * <a href="#asan_gdb" style="color:#856024"> ASAN+GDB: Обнаружение проездов по памяти </a>  
  * <a href="#gdb" style="color:#856024"> GDB: Обнаружение проездов по памяти </a>  
  * <a href="#valgrind" style="color:#856024"> VALGRIND: Обнаружение проездов по памяти </a>  
  * <a href="#valgrind_leak" style="color:#856024"> VALGRIND: Обнаружение утечек памяти </a>  
  * <a href="#asan_leak" style="color:#856024"> ASAN: Обнаружение утечек памяти </a>  
  * <a href="#strace" style="color:#856024"> STRACE: Отладка системных вызовов </a>  
  * <a href="#perf" style="color:#856024"> PERF: CPU профайлинг </a>  
  * <a href="#ubsan" style="color:#856024"> UBSAN: Поиск UB </a>  
* <a href="#mmap" style="color:#856024"> MMAP </a>  
* <a href="#hw" style="color:#856024"> Комментарии к ДЗ </a>  





## <a name="instr"></a> Инструментирование

Что это такое? Можно считать, что отладка. Получение информации о работающей программе и вмешательство в ее работу. Или не о программе, а о ядре системы. Известный вам пример - gdb.

Инструментарий для отладки
* Статический - прям в коде: счетчики, метрики (пример: санитайзеры)
  * Оверхед
  * Занимают место в коде
  * Много может, так как имеет много возможностей
  
* Динамический - над кодом (примеры: gdb, strace, valgrind, perf, [eBPF](https://habr.com/ru/post/435142/))
  * Динамически можно выбирать, на что смотреть
  * ...

Разные подходы к инструментированию:
* Трейсинг - обрабатывать события. (пример: strace)
* Семплинг - условно смотреть состояние системы 100 раз в секунду. (пример: perf)

### <a name="asan"></a> ASAN: Примеры использования address-санитайзера для обнаружения проезда по памяти


```cpp
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

    ==41369==AddressSanitizer: libc interceptors initialized
    || `[0x10007fff8000, 0x7fffffffffff]` || HighMem    ||
    || `[0x02008fff7000, 0x10007fff7fff]` || HighShadow ||
    || `[0x00008fff7000, 0x02008fff6fff]` || ShadowGap  ||
    || `[0x00007fff8000, 0x00008fff6fff]` || LowShadow  ||
    || `[0x000000000000, 0x00007fff7fff]` || LowMem     ||
    MemToShadow(shadow): 0x00008fff7000 0x000091ff6dff 0x004091ff6e00 0x02008fff6fff
    redzone=16
    max_redzone=2048
    quarantine_size_mb=256M
    thread_local_quarantine_size_kb=1024K
    malloc_context_size=30
    SHADOW_SCALE: 3
    SHADOW_GRANULARITY: 8
    SHADOW_OFFSET: 0x7fff8000
    ==41369==Installed the sigaction for signal 11
    ==41369==Installed the sigaction for signal 7
    ==41369==Installed the sigaction for signal 8
    ==41369==T0: stack [0x7fff473ce000,0x7fff47bce000) size 0x800000; local=0x7fff47bcc0d4
    ==41369==AddressSanitizer Init done
    AddressSanitizer:DEADLYSIGNAL
    =================================================================
    [1m[31m==41369==ERROR: AddressSanitizer: SEGV on unknown address 0x7fff47c2e2b0 (pc 0x55ff22dd22e4 bp 0x7fff47bcc0c0 sp 0x7fff47bcc040 T0)
    [1m[0m==41369==The signal is caused by a READ memory access.
        #0 0x55ff22dd22e3 in main (/home/pechatnov/vbox/caos/sem12-mmap-instrumentation/segfault.exe+0x12e3)
        #1 0x7ff684ba50b2 in __libc_start_main (/lib/x86_64-linux-gnu/libc.so.6+0x270b2)
        #2 0x55ff22dd216d in _start (/home/pechatnov/vbox/caos/sem12-mmap-instrumentation/segfault.exe+0x116d)
    
    AddressSanitizer can not provide additional info.
    SUMMARY: AddressSanitizer: SEGV (/home/pechatnov/vbox/caos/sem12-mmap-instrumentation/segfault.exe+0x12e3) in main
    ==41369==ABORTING



```python

```

Про многословность выхлопа ASAN.


```cpp
%%cpp normal_program.cpp
%run gcc -fsanitize=address normal_program.cpp -o normal_program.exe

int main() {
    return 0;
}
```


Run: `gcc -fsanitize=address normal_program.cpp -o normal_program.exe`



```python
!./normal_program.exe
```


```python
!ASAN_OPTIONS=verbosity=10 ./normal_program.exe
```

    ==41392==info->dlpi_name = 	info->dlpi_addr = 0x557f4e0b2000
    ==41392==info->dlpi_name = linux-vdso.so.1	info->dlpi_addr = 0x7ffdfbbfa000
    ==41392==info->dlpi_name = /lib/x86_64-linux-gnu/libasan.so.5	info->dlpi_addr = 0x7fb03dada000
    ==41392==AddressSanitizer: libc interceptors initialized
    || `[0x10007fff8000, 0x7fffffffffff]` || HighMem    ||
    || `[0x02008fff7000, 0x10007fff7fff]` || HighShadow ||
    || `[0x00008fff7000, 0x02008fff6fff]` || ShadowGap  ||
    || `[0x00007fff8000, 0x00008fff6fff]` || LowShadow  ||
    || `[0x000000000000, 0x00007fff7fff]` || LowMem     ||
    MemToShadow(shadow): 0x00008fff7000 0x000091ff6dff 0x004091ff6e00 0x02008fff6fff
    redzone=16
    max_redzone=2048
    quarantine_size_mb=256M
    thread_local_quarantine_size_kb=1024K
    malloc_context_size=30
    SHADOW_SCALE: 3
    SHADOW_GRANULARITY: 8
    SHADOW_OFFSET: 0x7fff8000
    ==41392==Installed the sigaction for signal 11
    ==41392==Installed the sigaction for signal 7
    ==41392==Installed the sigaction for signal 8
    ==41392==SetCurrentThread: 0x7fb03e50f000 for thread 0x7fb03d745780
    ==41392==T0: stack [0x7ffdfb274000,0x7ffdfba74000) size 0x800000; local=0x7ffdfba71d64
    ==41392==Using libbacktrace symbolizer.
    ==41392==AddressSanitizer Init done
    ==41393==Attached to thread 41392.
    ==41393==Detached from thread 41392.



```python
!./segfault.exe
```

    AddressSanitizer:DEADLYSIGNAL
    =================================================================
    [1m[31m==41402==ERROR: AddressSanitizer: SEGV on unknown address 0x7fff9f822660 (pc 0x55f79f4032e4 bp 0x7fff9f7c0470 sp 0x7fff9f7c03f0 T0)
    [1m[0m==41402==The signal is caused by a READ memory access.
        #0 0x55f79f4032e3 in main (/home/pechatnov/vbox/caos/sem12-mmap-instrumentation/segfault.exe+0x12e3)
        #1 0x7f1ec92220b2 in __libc_start_main (/lib/x86_64-linux-gnu/libc.so.6+0x270b2)
        #2 0x55f79f40316d in _start (/home/pechatnov/vbox/caos/sem12-mmap-instrumentation/segfault.exe+0x116d)
    
    AddressSanitizer can not provide additional info.
    SUMMARY: AddressSanitizer: SEGV (/home/pechatnov/vbox/caos/sem12-mmap-instrumentation/segfault.exe+0x12e3) in main
    ==41402==ABORTING



```python
!ASAN_OPTIONS=verbosity=10 ./segfault.exe
```

    ==41404==info->dlpi_name = 	info->dlpi_addr = 0x55aa8b336000
    ==41404==info->dlpi_name = linux-vdso.so.1	info->dlpi_addr = 0x7ffcfef39000
    ==41404==info->dlpi_name = /lib/x86_64-linux-gnu/libasan.so.5	info->dlpi_addr = 0x7fad4a904000
    ==41404==AddressSanitizer: libc interceptors initialized
    || `[0x10007fff8000, 0x7fffffffffff]` || HighMem    ||
    || `[0x02008fff7000, 0x10007fff7fff]` || HighShadow ||
    || `[0x00008fff7000, 0x02008fff6fff]` || ShadowGap  ||
    || `[0x00007fff8000, 0x00008fff6fff]` || LowShadow  ||
    || `[0x000000000000, 0x00007fff7fff]` || LowMem     ||
    MemToShadow(shadow): 0x00008fff7000 0x000091ff6dff 0x004091ff6e00 0x02008fff6fff
    redzone=16
    max_redzone=2048
    quarantine_size_mb=256M
    thread_local_quarantine_size_kb=1024K
    malloc_context_size=30
    SHADOW_SCALE: 3
    SHADOW_GRANULARITY: 8
    SHADOW_OFFSET: 0x7fff8000
    ==41404==Installed the sigaction for signal 11
    ==41404==Installed the sigaction for signal 7
    ==41404==Installed the sigaction for signal 8
    ==41404==SetCurrentThread: 0x7fad4b339000 for thread 0x7fad4a56f780
    ==41404==T0: stack [0x7ffcfe6b3000,0x7ffcfeeb3000) size 0x800000; local=0x7ffcfeeb1444
    ==41404==Using libbacktrace symbolizer.
    ==41404==AddressSanitizer Init done
    AddressSanitizer:DEADLYSIGNAL
    =================================================================
    [1m[31m==41404==ERROR: AddressSanitizer: SEGV on unknown address 0x7ffcfef13620 (pc 0x55aa8b3372e4 bp 0x7ffcfeeb1430 sp 0x7ffcfeeb13b0 T0)
    [1m[0m==41404==The signal is caused by a READ memory access.
        #0 0x55aa8b3372e3 in main (/home/pechatnov/vbox/caos/sem12-mmap-instrumentation/segfault.exe+0x12e3)
        #1 0x7fad4a7390b2 in __libc_start_main (/lib/x86_64-linux-gnu/libc.so.6+0x270b2)
        #2 0x55aa8b33716d in _start (/home/pechatnov/vbox/caos/sem12-mmap-instrumentation/segfault.exe+0x116d)
    
    AddressSanitizer can not provide additional info.
    SUMMARY: AddressSanitizer: SEGV (/home/pechatnov/vbox/caos/sem12-mmap-instrumentation/segfault.exe+0x12e3) in main
    ==41404==ABORTING



```python

```


```python

```

### <a name="asan_gdb"></a> ASAN+GDB: Обнаружение проезда по памяти с address-санитайзера скомбинированного с запуском под GDB


```python
# комбинируем санитайзер и gdb (это семинарист рекомендует, если вы хотите больше подробностей)
# по идее это должно находить больше косяков, чем вариант в следующей ячейке
!gcc -g -fsanitize=address segfault.cpp -o segfault.exe
!gdb -ex=r -batch --args ./segfault.exe
```

    [Thread debugging using libthread_db enabled]
    Using host libthread_db library "/lib/x86_64-linux-gnu/libthread_db.so.1".
    
    Program received signal SIGSEGV, Segmentation fault.
    0x00005555555552c8 in main () at segfault.cpp:7
    7	    printf("%d\n", a[100500]); // проезд по памяти


### <a name="gdb"></a> GDB: Обнаружение проезда по памяти с помощью GDB


```python
# компилируем с отладочной информацией и запускаем под gdb
!gcc -g segfault.cpp -o segfault.exe
!gdb -ex=r -batch --args ./segfault.exe
```

    
    Program received signal SIGBUS, Bus error.
    main () at segfault.cpp:7
    7	    printf("%d\n", a[100500]); // проезд по памяти


### <a name="valgrind"></a> VALGRIND: Обнаружение проезда по памяти с помощью valgrind


```python
# компилируем как обычно и запускаем с valgrind
!gcc segfault.cpp -o segfault.exe
!valgrind --tool=memcheck ./segfault.exe 2>&1 | head -n 8 # берем только первые 8 строк выхлопа, а то там много
```

    ==2849== Memcheck, a memory error detector
    ==2849== Copyright (C) 2002-2017, and GNU GPL'd, by Julian Seward et al.
    ==2849== Using Valgrind-3.15.0 and LibVEX; rerun with -h for copyright info
    ==2849== Command: ./segfault.exe
    ==2849== 
    ==2849== Invalid read of size 4
    ==2849==    at 0x109184: main (in /home/pechatnov/vbox/caos/sem12-mmap-instrumentation/segfault.exe)
    ==2849==  Address 0x1fff061f00 is not stack'd, malloc'd or (recently) free'd
    Segmentation fault (core dumped)


### <a name="valgrind_leak"></a> VALGRIND: Обнаружение утечек памяти с помощью valgrind


```cpp
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

    ==2875== Memcheck, a memory error detector
    ==2875== Copyright (C) 2002-2017, and GNU GPL'd, by Julian Seward et al.
    ==2875== Using Valgrind-3.15.0 and LibVEX; rerun with -h for copyright info
    ==2875== Command: ./memory_leak.exe
    ==2875== 
    ==2875== 
    ==2875== HEAP SUMMARY:
    ==2875==     in use at exit: 16 bytes in 1 blocks
    ==2875==   total heap usage: 1 allocs, 0 frees, 16 bytes allocated
    ==2875== 
    ==2875== 16 bytes in 1 blocks are definitely lost in loss record 1 of 1
    ==2875==    at 0x483B7F3: malloc (in /usr/lib/x86_64-linux-gnu/valgrind/vgpreload_memcheck-amd64-linux.so)
    ==2875==    by 0x10915A: main (in /home/pechatnov/vbox/caos/sem12-mmap-instrumentation/memory_leak.exe)
    ==2875== 
    ==2875== LEAK SUMMARY:
    ==2875==    definitely lost: 16 bytes in 1 blocks
    ==2875==    indirectly lost: 0 bytes in 0 blocks
    ==2875==      possibly lost: 0 bytes in 0 blocks
    ==2875==    still reachable: 0 bytes in 0 blocks
    ==2875==         suppressed: 0 bytes in 0 blocks
    ==2875== 
    ==2875== For lists of detected and suppressed errors, rerun with: -s
    ==2875== ERROR SUMMARY: 1 errors from 1 contexts (suppressed: 0 from 0)


### <a name="asan_leak"></a> ASAN: Обнаружение утечек памяти с помощью address-санитайзера


```python
# компилируем с санитайзером и запускаем как обычно
!gcc -fsanitize=address memory_leak.cpp -o memory_leak.exe
!./memory_leak.exe
```

    
    =================================================================
    [1m[31m==2909==ERROR: LeakSanitizer: detected memory leaks
    [1m[0m
    [1m[34mDirect leak of 16 byte(s) in 1 object(s) allocated from:
    [1m[0m    #0 0x7f07f2ea6bc8 in malloc (/lib/x86_64-linux-gnu/libasan.so.5+0x10dbc8)
        #1 0x56269077819a in main (/home/pechatnov/vbox/caos/sem12-mmap-instrumentation/memory_leak.exe+0x119a)
        #2 0x7f07f2bce0b2 in __libc_start_main (/lib/x86_64-linux-gnu/libc.so.6+0x270b2)
    
    SUMMARY: AddressSanitizer: 16 byte(s) leaked in 1 allocation(s).


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
!strace ./printing.exe > out.txt
!echo "Program output:"
!cat out.txt
```

    execve("./printing.exe", ["./printing.exe"], 0x7ffdf7565680 /* 67 vars */) = 0
    brk(NULL)                               = 0x55a84cbdf000
    arch_prctl(0x3001 /* ARCH_??? */, 0x7fff46c95330) = -1 EINVAL (Invalid argument)
    access("/etc/ld.so.preload", R_OK)      = -1 ENOENT (No such file or directory)
    openat(AT_FDCWD, "/etc/ld.so.cache", O_RDONLY|O_CLOEXEC) = 3
    fstat(3, {st_mode=S_IFREG|0644, st_size=103517, ...}) = 0
    mmap(NULL, 103517, PROT_READ, MAP_PRIVATE, 3, 0) = 0x7f74dc352000
    close(3)                                = 0
    openat(AT_FDCWD, "/lib/x86_64-linux-gnu/libc.so.6", O_RDONLY|O_CLOEXEC) = 3
    read(3, "\177ELF\2\1\1\3\0\0\0\0\0\0\0\0\3\0>\0\1\0\0\0\360q\2\0\0\0\0\0"..., 832) = 832
    pread64(3, "\6\0\0\0\4\0\0\0@\0\0\0\0\0\0\0@\0\0\0\0\0\0\0@\0\0\0\0\0\0\0"..., 784, 64) = 784
    pread64(3, "\4\0\0\0\20\0\0\0\5\0\0\0GNU\0\2\0\0\300\4\0\0\0\3\0\0\0\0\0\0\0", 32, 848) = 32
    pread64(3, "\4\0\0\0\24\0\0\0\3\0\0\0GNU\0cBR\340\305\370\2609W\242\345)q\235A\1"..., 68, 880) = 68
    fstat(3, {st_mode=S_IFREG|0755, st_size=2029224, ...}) = 0
    mmap(NULL, 8192, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = 0x7f74dc350000
    pread64(3, "\6\0\0\0\4\0\0\0@\0\0\0\0\0\0\0@\0\0\0\0\0\0\0@\0\0\0\0\0\0\0"..., 784, 64) = 784
    pread64(3, "\4\0\0\0\20\0\0\0\5\0\0\0GNU\0\2\0\0\300\4\0\0\0\3\0\0\0\0\0\0\0", 32, 848) = 32
    pread64(3, "\4\0\0\0\24\0\0\0\3\0\0\0GNU\0cBR\340\305\370\2609W\242\345)q\235A\1"..., 68, 880) = 68
    mmap(NULL, 2036952, PROT_READ, MAP_PRIVATE|MAP_DENYWRITE, 3, 0) = 0x7f74dc15e000
    mprotect(0x7f74dc183000, 1847296, PROT_NONE) = 0
    mmap(0x7f74dc183000, 1540096, PROT_READ|PROT_EXEC, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x25000) = 0x7f74dc183000
    mmap(0x7f74dc2fb000, 303104, PROT_READ, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x19d000) = 0x7f74dc2fb000
    mmap(0x7f74dc346000, 24576, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x1e7000) = 0x7f74dc346000
    mmap(0x7f74dc34c000, 13528, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_ANONYMOUS, -1, 0) = 0x7f74dc34c000
    close(3)                                = 0
    arch_prctl(ARCH_SET_FS, 0x7f74dc351540) = 0
    mprotect(0x7f74dc346000, 12288, PROT_READ) = 0
    mprotect(0x55a84b294000, 4096, PROT_READ) = 0
    mprotect(0x7f74dc399000, 4096, PROT_READ) = 0
    munmap(0x7f74dc352000, 103517)          = 0
    fstat(1, {st_mode=S_IFREG|0664, st_size=0, ...}) = 0
    brk(NULL)                               = 0x55a84cbdf000
    brk(0x55a84cc00000)                     = 0x55a84cc00000
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
!strace ./printing_asm.exe > out.txt 2> err.txt
!cat err.txt 
!echo "Program output:"
!cat out.txt
```

    execve("./printing_asm.exe", ["./printing_asm.exe"], 0x7ffd650f8140 /* 67 vars */) = 0
    strace: [ Process PID=3411 runs in 32 bit mode. ]
    brk(NULL)                               = 0x56dd8000
    arch_prctl(0x3001 /* ARCH_??? */, 0xffc219c8) = -1 EINVAL (Invalid argument)
    access("/etc/ld.so.nohwcap", F_OK)      = -1 ENOENT (No such file or directory)
    mmap2(NULL, 8192, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = 0xf7f2c000
    access("/etc/ld.so.preload", R_OK)      = -1 ENOENT (No such file or directory)
    set_thread_area({entry_number=-1, base_addr=0xf7f2c9c0, limit=0x0fffff, seg_32bit=1, contents=0, read_exec_only=0, limit_in_pages=1, seg_not_present=0, useable=1}) = 0 (entry_number=12)
    mprotect(0x5656e000, 4096, PROT_READ|PROT_WRITE) = 0
    mprotect(0x5656f000, 4096, PROT_READ|PROT_WRITE|PROT_EXEC) = 0
    mprotect(0x56570000, 0, PROT_READ|PROT_WRITE) = 0
    mprotect(0x56570000, 0, PROT_READ)      = 0
    mprotect(0x5656f000, 4096, PROT_READ|PROT_EXEC) = 0
    mprotect(0x5656e000, 4096, PROT_READ)   = 0
    mprotect(0x56570000, 4096, PROT_READ)   = 0
    write(1, "Hello, World!\n", 14)         = 14
    exit(1)                                 = ?
    +++ exited with 1 +++
    Program output:
    Hello, World!


### <a name="perf"></a> PERF: CPU профайлинг с помощью perf


```cpp
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
!cat ~/.password.txt | sudo -S perf stat ./work_hard.exe
```

    [sudo] password for pechatnov: 
     Performance counter stats for './work_hard.exe':
    
              3 202,39 msec task-clock                #    0,988 CPUs utilized          
                    24      context-switches          #    0,007 K/sec                  
                     2      cpu-migrations            #    0,001 K/sec                  
                    44      page-faults               #    0,014 K/sec                  
       <not supported>      cycles                                                      
       <not supported>      instructions                                                
       <not supported>      branches                                                    
       <not supported>      branch-misses                                               
    
           3,242514096 seconds time elapsed
    
           3,181980000 seconds user
           0,020203000 seconds sys
    
    



```python
!env | grep PASS
```

    PASSWORD=simple_pass



```python
# сравним с программой без stdlib
!cat ~/.password.txt | sudo -S perf stat ./printing_asm.exe
```

    [sudo] password for pechatnov: Hello, World!
    
     Performance counter stats for './printing_asm.exe':
    
                  0,25 msec task-clock                #    0,288 CPUs utilized          
                     0      context-switches          #    0,000 K/sec                  
                     0      cpu-migrations            #    0,000 K/sec                  
                    13      page-faults               #    0,051 M/sec                  
       <not supported>      cycles                                                      
       <not supported>      instructions                                                
       <not supported>      branches                                                    
       <not supported>      branch-misses                                               
    
           0,000881132 seconds time elapsed
    
           0,000000000 seconds user
           0,000901000 seconds sys
    
    


Результат perf report можно отрисовывать в более красивом виде с помощью [flamegraph](https://github.com/brendangregg/FlameGraph)


```python
# компилируем как обычно и запускаем с perf record и потом смотрим результаты
!gcc work_hard.cpp -o work_hard.exe
!cat ~/.password.txt | sudo -S perf record ./work_hard.exe 2>&1 > perf.log
!cat ~/.password.txt | sudo -S chmod 0666 perf.data
!perf report | cat
```

    [sudo] password for pechatnov: [ perf record: Woken up 1 times to write data ]
    [ perf record: Captured and wrote 0,102 MB perf.data (2450 samples) ]
    [sudo] password for pechatnov: # To display the perf.data header info, please use --header/--header-only options.
    #
    #
    # Total Lost Samples: 0
    #
    # Samples: 2K of event 'cpu-clock:pppH'
    # Event count (approx.): 612500000
    #
    # Overhead  Command        Shared Object      Symbol                
    # ........  .............  .................  ......................
    #
        63.39%  work_hard.exe  work_hard.exe      [.] work_hard_1
        35.51%  work_hard.exe  work_hard.exe      [.] work_hard_2
         0.82%  work_hard.exe  [kernel.kallsyms]  [k] 0xffffffff9ae00076
         0.12%  work_hard.exe  [kernel.kallsyms]  [k] 0xffffffff9a0d3d9b
         0.04%  work_hard.exe  [kernel.kallsyms]  [k] 0xffffffff9a0040e4
         0.04%  work_hard.exe  [kernel.kallsyms]  [k] 0xffffffff9a0dc78c
         0.04%  work_hard.exe  [kernel.kallsyms]  [k] 0xffffffff9aad9505
         0.04%  work_hard.exe  ld-2.31.so         [.] strlen
    
    
    #
    # (Cannot load tips.txt file, please install perf!)
    #


Есть еще вариант посчитать количество выполненных программой/функцией инструкций. Оно не всегда хорошо коррелирует со временем выполнения, зато является стабильным значением от запуска к запуску.
Здесь приводить способ не буду, если интересно, связывайтесь со мной отдельно :)

#


```cpp
%%cpp wcc.cpp
%run gcc wcc.cpp -o wcc.exe
%run # bash -c "for i in {0..1000000} ; do echo -n '1' ; done" > input.txt
%run wc -c input.txt
%run time ./wcc.exe 1 input.txt
%run time ./wcc.exe 10 input.txt
%run time ./wcc.exe 100 input.txt
%run time ./wcc.exe 1000 input.txt
%run time ./wcc.exe 10000 input.txt

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>

int main(int argc, char** argv) {
    assert(argc == 3);
    int buff_size = 1;
    int ret = sscanf(argv[1], "%d", &buff_size);
    assert(ret == 1);
    int fd = open(argv[2], O_RDONLY);
    assert(fd >= 0);
    char buff[buff_size];
    int result = 0;
    int cnt = 0;
    while ((cnt = read(fd, buff, buff_size)) > 0) {
        for (int i = 0; i < cnt; ++i) {
            result += buff[i];
        }
    }
    printf("CNT: %d\n", cnt);
    return 0;
}
```


Run: `gcc wcc.cpp -o wcc.exe`



Run: `# bash -c "for i in {0..1000000} ; do echo -n '1' ; done" > input.txt`



Run: `wc -c input.txt`


    1000001 input.txt



Run: `time ./wcc.exe 1 input.txt`


    CNT: 0
    1.52user 2.08system 0:03.61elapsed 99%CPU (0avgtext+0avgdata 1484maxresident)k
    0inputs+0outputs (0major+64minor)pagefaults 0swaps



Run: `time ./wcc.exe 10 input.txt`


    CNT: 0
    0.11user 0.19system 0:00.30elapsed 99%CPU (0avgtext+0avgdata 1544maxresident)k
    0inputs+0outputs (0major+67minor)pagefaults 0swaps



Run: `time ./wcc.exe 100 input.txt`


    CNT: 0
    0.02user 0.03system 0:00.05elapsed 98%CPU (0avgtext+0avgdata 1516maxresident)k
    0inputs+0outputs (0major+65minor)pagefaults 0swaps



Run: `time ./wcc.exe 1000 input.txt`


    CNT: 0
    0.00user 0.00system 0:00.00elapsed 100%CPU (0avgtext+0avgdata 1548maxresident)k
    0inputs+0outputs (0major+66minor)pagefaults 0swaps



Run: `time ./wcc.exe 10000 input.txt`


    CNT: 0
    0.00user 0.00system 0:00.00elapsed 100%CPU (0avgtext+0avgdata 1524maxresident)k
    0inputs+0outputs (0major+68minor)pagefaults 0swaps



```python
!cat ~/.password.txt | sudo -S perf stat ./wcc.exe 1 input.txt 2>&1 | head -n 5
!cat ~/.password.txt | sudo -S perf stat ./wcc.exe 10 input.txt 2>&1 | head -n 5
!cat ~/.password.txt | sudo -S perf stat ./wcc.exe 100 input.txt 2>&1 | head -n 5
!cat ~/.password.txt | sudo -S perf stat ./wcc.exe 1000 input.txt 2>&1 | head -n 5
!cat ~/.password.txt | sudo -S perf stat ./wcc.exe 10000 input.txt 2>&1 | head -n 5
!cat ~/.password.txt | sudo -S perf stat ./wcc.exe 100000 input.txt 2>&1 | head -n 5
!cat ~/.password.txt | sudo -S perf stat ./wcc.exe 1000000 input.txt 2>&1 | head -n 5
```

    [sudo] password for pechatnov: CNT: 0
    
     Performance counter stats for './wcc.exe 1 input.txt':
    
              3 103,35 msec task-clock                #    0,999 CPUs utilized          
    [sudo] password for pechatnov: CNT: 0
    
     Performance counter stats for './wcc.exe 10 input.txt':
    
                301,68 msec task-clock                #    0,998 CPUs utilized          
    [sudo] password for pechatnov: CNT: 0
    
     Performance counter stats for './wcc.exe 100 input.txt':
    
                 49,02 msec task-clock                #    0,991 CPUs utilized          
    [sudo] password for pechatnov: CNT: 0
    
     Performance counter stats for './wcc.exe 1000 input.txt':
    
                  5,27 msec task-clock                #    0,839 CPUs utilized          
    [sudo] password for pechatnov: CNT: 0
    
     Performance counter stats for './wcc.exe 10000 input.txt':
    
                  3,74 msec task-clock                #    0,844 CPUs utilized          
    [sudo] password for pechatnov: CNT: 0
    
     Performance counter stats for './wcc.exe 100000 input.txt':
    
                  4,91 msec task-clock                #    0,888 CPUs utilized          
    [sudo] password for pechatnov: CNT: 0
    
     Performance counter stats for './wcc.exe 1000000 input.txt':
    
                 38,43 msec task-clock                #    0,983 CPUs utilized          


### <a name="ubsan"></a> UBSAN: Поиск UB (undefined behaviour) с помощью UB-санитайзера


```cpp
%%cpp ub.c

int main(int argc, char** argv) {
    return -argc << 31;
    // return (argc * (int)((1ull << 31) - 1)) + 1;
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
    SUMMARY: UndefinedBehaviorSanitizer: undefined-behavior ub.c:4:18 in 



```python
!clang -Os -fsanitize=undefined ub.c -S -o /dev/stdout | grep main: -A 30
```

    main:                                   # @main
    	.cfi_startproc
    # %bb.0:
    	pushq	%rbx
    	.cfi_def_cfa_offset 16
    	.cfi_offset %rbx, -16
    	movl	%edi, %ebx
    	negl	%ebx
    	jo	.LBB0_1
    .LBB0_2:
    	testl	%ebx, %ebx
    	jne	.LBB0_3
    .LBB0_4:
    	shll	$31, %ebx
    	movl	%ebx, %eax
    	popq	%rbx
    	.cfi_def_cfa_offset 8
    	retq
    .LBB0_1:
    	.cfi_def_cfa_offset 16
    	movl	%edi, %esi
    	movl	$.L__unnamed_1, %edi
    	callq	__ubsan_handle_negate_overflow
    	jmp	.LBB0_2
    .LBB0_3:
    	movl	%ebx, %esi
    	movl	$.L__unnamed_2, %edi
    	movl	$31, %edx
    	callq	__ubsan_handle_shift_out_of_bounds
    	jmp	.LBB0_4
    .Lfunc_end0:



```python
!clang ub.c -Os -S -o /dev/stdout | grep main: -A 5
```

    main:                                   # @main
    	.cfi_startproc
    # %bb.0:
    	movl	%edi, %eax
    	shll	$31, %eax
    	retq



```python

```


```python

```


```python
sudo sysctl kernel.core_pattern="/var/crash/%p_%s_%c_%d_%P_%E"
```


```cpp
%%cpp segfault.c
%run gcc -g segfault.c -o segfault.exe

#include<stdio.h>

int main() {
    int a[2];
    printf("%d\n", a[100500]); // проезд по памяти
}
```


Run: `gcc -g segfault.c -o segfault.exe`



```python
!ulimit -c unlimited ; ./segfault.exe
```

    Segmentation fault (core dumped)



```python
ls /var/crash
```

    '32161_11_18446744073709551615_1_32161_!home!pechatnov!vbox!caos!sem12-mmap-instrumentation!segfault.exe'
     _usr_bin_bash.1000.crash
     _usr_bin_echo.1000.crash



```python

```


```python

```

## <a name="mmap"></a> MMAP

(Копия [ридинга от Яковлева](https://github.com/victor-yacovlev/mipt-diht-caos/tree/master/practice/mmap))

```c
#include <sys/mman.h>

void *mmap(
    void *addr,    /* рекомендуемый адрес отображения. Должен быть выровнен! */
    size_t length, /* размер отображения */
    int prot,      /* аттрибуты доступа */
    int flags,     /* флаги совместного отображения */
    int fd,        /* файловый декскриптор файла */
    off_t offset   /* смещение относительно начала файла. Должен быть выровнен! */
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
 
 
Зачем mmap?
* Дешевая переподгрузка файла-ресурса (если файл не изменился, то не будет выделяться лишняя память).
* ...


```python

```


```python

```


```python

```

Пример с mmap (и с ftruncate). Простенькая программа, делающая циклический сдвиг (как цифры) второго символа в файле.

**Тут была некритичная ошибка так как по размеру страницы выравнивалась length, а должны быть выровнены только addr и offset**


```cpp
%%cpp mmap_example.c
%run gcc mmap_example.c -o mmap_example.exe
%run echo "000" > buf.txt && ./mmap_example.exe && cat buf.txt
%run echo "79" > buf.txt && ./mmap_example.exe && cat buf.txt
%run echo "xxx" > buf.txt && ./mmap_example.exe && cat buf.txt
%run echo -n "S" > buf.txt && ./mmap_example.exe && cat buf.txt

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <assert.h>

int get_page_size() {
    static int page_size = 0;
    return page_size = page_size ?: sysconf(_SC_PAGE_SIZE);
}

int upper_round_to_page_size(int sz) {
    return (sz + get_page_size() - 1) / get_page_size() * get_page_size();
}

int main() {
    printf("page size = %d\n", get_page_size());
    int fd = open("buf.txt", O_RDWR);
    struct stat s;
    assert(fstat(fd, &s) == 0);
    
    printf("file size = %d\n", (int)s.st_size);
    int old_st_size = s.st_size;
    if (s.st_size < 2) {
        const int new_size = 10;
        assert(ftruncate(fd, new_size) == 0); // изменяем размер файла
        assert(fstat(fd, &s) == 0);
        printf("new file size = %d\n", (int)s.st_size);
    }
    
    void* mapped = mmap(
        /* desired addr, addr = */ NULL, 
        /* length = */ s.st_size, 
        /* access attributes, prot = */ PROT_READ | PROT_WRITE,
        /* flags = */ MAP_SHARED,
        /* fd = */ fd,
        /* offset in file, offset = */ 0
    );
    assert(mapped != MAP_FAILED);
    assert(close(fd) == 0); // Не забываем закрывать файл
    
    char* buf = mapped;
    
    if (old_st_size != s.st_size) {
        for (int j = old_st_size; j < s.st_size; ++j) {
            buf[j] = '_';
        }
    }
    
    buf[1] = ('0' <= buf[1] && buf[1] <= '9') ? ((buf[1] - '0' + 1) % 10 + '0') : '0';
    
    assert(munmap(
        /* mapped addr, addr = */ mapped, 
        /* length = */ s.st_size
    ) == 0);
    
    return 0;
}
```


Run: `gcc mmap_example.c -o mmap_example.exe`



Run: `echo "000" > buf.txt && ./mmap_example.exe && cat buf.txt`


    page size = 4096
    file size = 4
    010



Run: `echo "79" > buf.txt && ./mmap_example.exe && cat buf.txt`


    page size = 4096
    file size = 3
    70



Run: `echo "xxx" > buf.txt && ./mmap_example.exe && cat buf.txt`


    page size = 4096
    file size = 4
    x0x



Run: `echo -n "S" > buf.txt && ./mmap_example.exe && cat buf.txt`


    page size = 4096
    file size = 1
    new file size = 10
    S0________

Еще один пример по мотивам задачи про jit компиляцию. Интерпретируем байты как функцию и выполняем.


```cpp
%%cpp func.c
%run gcc -g -fPIC func.c -c -o func.o 
%run objdump -F -d func.o | grep my_func -A 15

int my_func(int a, int b) {
    return a + b + 1;
}
```


Run: `gcc -g -fPIC func.c -c -o func.o`



Run: `objdump -F -d func.o | grep my_func -A 15`


    0000000000000000 <my_func> (File Offset: 0x40):
       0:	f3 0f 1e fa          	endbr64 
       4:	55                   	push   %rbp
       5:	48 89 e5             	mov    %rsp,%rbp
       8:	89 7d fc             	mov    %edi,-0x4(%rbp)
       b:	89 75 f8             	mov    %esi,-0x8(%rbp)
       e:	8b 55 fc             	mov    -0x4(%rbp),%edx
      11:	8b 45 f8             	mov    -0x8(%rbp),%eax
      14:	01 d0                	add    %edx,%eax
      16:	83 c0 01             	add    $0x1,%eax
      19:	5d                   	pop    %rbp
      1a:	c3                   	retq   


Замечаем, что `File Offset: 0x40`. То есть в объектном файле функция лежит начиная с `0x40` позиции.


```cpp
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

int main() {
    int fd = open("func.o", O_RDWR);
    struct stat s;
    assert(fstat(fd, &s) == 0);
    
    printf("file size = %d\n", (int)s.st_size);
    
    void* mapped = mmap(
        /* desired addr, addr = */ NULL, 
        /* length = */ s.st_size, 
        /* access attributes, prot = */ PROT_READ | PROT_EXEC, // обратите внимание на PROT_EXEC
        /* flags = */ MAP_PRIVATE,
        /* fd = */ fd,
        /* offset in file, offset = */ 0
    );
    assert(close(fd) == 0); // Не забываем закрывать файл (при закрытии регион памяти остается доступным)
    if (mapped == MAP_FAILED) {
        perror("Can't mmap");
        return -1;
    }
 
    int (*func)(int, int) = (void*)((char*)mapped + 0x40); // 0x40 - тот самый оффсет
   
    printf("func(1, 1) = %d\n", func(1, 1));
    printf("func(10, 100) = %d\n", func(10, 100));
    printf("func(40, 5000) = %d\n", func(40, 5000));
    
    assert(munmap(
        /* mapped addr, addr = */ mapped, 
        /* length = */ s.st_size
    ) == 0);
    return 0;
}
```


Run: `gcc -g mmap_exec_example.c -o mmap_exec_example.exe`



Run: `./mmap_exec_example.exe`


    file size = 2872
    func(1, 1) = 3
    func(10, 100) = 111
    func(40, 5000) = 5041


# <a name="hw"></a> Комментарии к ДЗ

* Нельзя считать, что размер страницы 4096!
* posix/mmap/print-list-using-mmap
  <br> В этой задаче не гарантируется выравнивания структур в файле. Поэтому делить позиции на размер структуры нельзя.
  <br> Можно делать, например, так:
  ```c
     char* buf = ...;
     ...
     struct Item* item = (void*)(buf + offset);
  ```
  
  Не забывайте, что арифметика указателей с void* - это UB.
  <br> https://stackoverflow.com/questions/3523145/pointer-arithmetic-for-void-pointer-in-c
  
* munmap забывать не надо
  
* posix/mmap/make-spiral-file <br>
    mmap, ftruncate, snprintf
    
* advanced-3 <br>
    Можно попробовать поддерживать список свободных сегментов памяти.
    При аллокации выбирать наименьший достаточного объема и отрезать от него.
    При деаллокации добавлять новый свободный сегмент и склеивать смежные.


```python

```


```python

```
