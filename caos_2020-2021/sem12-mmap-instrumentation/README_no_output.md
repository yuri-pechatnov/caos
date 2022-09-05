

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


```python
# компилируем с санитайзером и запускаем как обычно (семинарист рекомендует)
!gcc -fsanitize=address segfault.cpp -o segfault.exe
!./segfault.exe
```


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


```python
!./normal_program.exe
```


```python
!ASAN_OPTIONS=verbosity=10 ./normal_program.exe
```


```python
!./segfault.exe
```


```python
!ASAN_OPTIONS=verbosity=10 ./segfault.exe
```


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

### <a name="gdb"></a> GDB: Обнаружение проезда по памяти с помощью GDB


```python
# компилируем с отладочной информацией и запускаем под gdb
!gcc -g segfault.cpp -o segfault.exe
!gdb -ex=r -batch --args ./segfault.exe
```

### <a name="valgrind"></a> VALGRIND: Обнаружение проезда по памяти с помощью valgrind


```python
# компилируем как обычно и запускаем с valgrind
!gcc segfault.cpp -o segfault.exe
!valgrind --tool=memcheck ./segfault.exe 2>&1 | head -n 8 # берем только первые 8 строк выхлопа, а то там много
```

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

### <a name="asan_leak"></a> ASAN: Обнаружение утечек памяти с помощью address-санитайзера


```python
# компилируем с санитайзером и запускаем как обычно
!gcc -fsanitize=address memory_leak.cpp -o memory_leak.exe
!./memory_leak.exe
```

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


```python
!env | grep PASS
```


```python
# сравним с программой без stdlib
!cat ~/.password.txt | sudo -S perf stat ./printing_asm.exe
```

Результат perf report можно отрисовывать в более красивом виде с помощью [flamegraph](https://github.com/brendangregg/FlameGraph)


```python
# компилируем как обычно и запускаем с perf record и потом смотрим результаты
!gcc work_hard.cpp -o work_hard.exe
!cat ~/.password.txt | sudo -S perf record ./work_hard.exe 2>&1 > perf.log
!cat ~/.password.txt | sudo -S chmod 0666 perf.data
!perf report | cat
```

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


```python
!cat ~/.password.txt | sudo -S perf stat ./wcc.exe 1 input.txt 2>&1 | head -n 5
!cat ~/.password.txt | sudo -S perf stat ./wcc.exe 10 input.txt 2>&1 | head -n 5
!cat ~/.password.txt | sudo -S perf stat ./wcc.exe 100 input.txt 2>&1 | head -n 5
!cat ~/.password.txt | sudo -S perf stat ./wcc.exe 1000 input.txt 2>&1 | head -n 5
!cat ~/.password.txt | sudo -S perf stat ./wcc.exe 10000 input.txt 2>&1 | head -n 5
!cat ~/.password.txt | sudo -S perf stat ./wcc.exe 100000 input.txt 2>&1 | head -n 5
!cat ~/.password.txt | sudo -S perf stat ./wcc.exe 1000000 input.txt 2>&1 | head -n 5
```

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


```python
!clang -Os -fsanitize=undefined ub.c -S -o /dev/stdout | grep main: -A 30
```


```python
!clang ub.c -Os -S -o /dev/stdout | grep main: -A 5
```


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


```python
!ulimit -c unlimited ; ./segfault.exe
```


```python
ls /var/crash
```


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

Еще один пример по мотивам задачи про jit компиляцию. Интерпретируем байты как функцию и выполняем.


```cpp
%%cpp func.c
%run gcc -g -fPIC func.c -c -o func.o 
%run objdump -F -d func.o | grep my_func -A 15

int my_func(int a, int b) {
    return a + b + 1;
}
```

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
