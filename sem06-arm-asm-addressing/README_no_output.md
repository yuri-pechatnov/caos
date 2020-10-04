

# ARM ASM ADDRESSING

<table width=100%> <tr>
    <th width=20%> <b>Видеозапись семинара &rarr; </b> </th>
    <th>
    <a href="https://www.">
        <img src="vide.jpg" width="320"  height="160" align="left" alt="Видео с семинара"> 
    </a>
    </th>
    <th> </th>
 </table>

[Ридинг Яковлева: Адресация данных в памяти и использование библиотечных функций](https://github.com/victor-yacovlev/mipt-diht-caos/tree/master/practice/arm_globals_plt)

Сегодня в программе:
* <a href="#install_instr" style="color:#856024"> Установка инструментов </a>
* <a href="#compile_run" style="color:#856024"> Компиляция и запуск </a>
* <a href="#asm" style="color:#856024"> Ассемблер ARM </a>
  * <a href="#easy_asm" style="color:#856024"> Простой способ написать функцию на ассемблере </a>


```python

```


```python
import os

linaro_download_dir = os.path.expanduser("~/arm/gcc-linaro-7.3.1-2018.05-i686_arm-linux-gnueabi")

gcc_linaro_path = os.path.join(linaro_download_dir, "bin")
if gcc_linaro_path not in os.environ["PATH"]:
    os.environ["PATH"] += ":" + gcc_linaro_path

os.environ["QEMU_LD_PREFIX"] = os.path.join(linaro_download_dir, "arm-linux-gnueabi/libc")    
```


```python

```


```cpp
%%cpp lib.c
%run arm-linux-gnueabi-gcc -c -Os -fno-asynchronous-unwind-tables -marm lib.c -o lib.o
%// run arm-linux-gnueabi-gcc -S -Os -fno-asynchronous-unwind-tables -marm lib.c -o /dev/stdout
%run arm-linux-gnueabi-objdump -D lib.o | grep '<sum_v>' -A 14

int sum_v(int* a, int* b, int n) {
    for (int i = 0; i < n; ++i) {
        a[i] += b[i];
    }
}
```


```python

```


```cpp
%%cpp size.c
%run arm-linux-gnueabi-gcc -marm -fsanitize=address lib.c size.c -o size.exe
%run qemu-arm ./size.exe

#include <stdio.h>
#include <assert.h>

int sum_v(int* a, int* b, int n);

void test() {
    {
        int a[] = {1, 2, 3};
        int b[] = {10, 20, 30};
        sum_v(a, b, 3);
        assert(a[0] == 11);
        assert(a[1] == 22);
        assert(a[2] == 33);
    }
    {
        int a[] = {};
        int b[] = {};
        sum_v(a, b, 0);    
    }
    printf("SUCCESS\n");
}

int main() {
    test();
}
```

### Вариант 1: причесанный дизассемблер


```cpp
%%cpp lib.S
%run arm-linux-gnueabi-gcc -marm -fsanitize=address lib.S size.c -o size.exe
%run qemu-arm ./size.exe

.global sum_v
sum_v:
    mov r3, #0
    cmp r3, r2
    blt long_path // if (else block)
        bx lr
    long_path:    // if (first block)
        push {lr}
        while_start: // while
            ldr ip, [r0, r3, lsl #2] // C-style: ip = *(r0 + (r3 << 2))
            ldr lr, [r1, r3, lsl #2]
            add ip, ip, lr
            str ip, [r0, r3, lsl #2] // C-style: *(r0 + (r3 << 2)) = ip
            add r3, r3, #1
            cmp r3, r2
            blt while_start
        pop {pc}
```

### Вариант 2: c нуля, придерживаясь сишной структуры кода


```cpp
%%cpp lib.S
%run arm-linux-gnueabi-gcc -marm -fsanitize=address lib.S size.c -o size.exe
%run qemu-arm ./size.exe

.global sum_v
sum_v:
    // C-style: r0 = a, r1 = b, r3 = n
    mov r3, #4 // sizeof(int)
    mla r2, r2, r3, r0 // C-style: r3 = (a + n)
    while_start: // while
        cmp r0, r2
        beq while_end
            ldr ip, [r0] // C-style: ip = *r0
            ldr r3, [r1] // C-style: r3 = *r1
            add r3, r3, ip // C-style: r3 += ip
            str r3, [r0] // C-style: *r0 = r3
            add r0, r0, #4
            add r1, r1, #4
        b while_start
    while_end: // while end
    bx lr
```

### Вариант 3: оптимизируем


```cpp
%%cpp lib.S
%run arm-linux-gnueabi-gcc -marm -fsanitize=address lib.S size.c -o size.exe
%run qemu-arm ./size.exe

.global sum_v
sum_v:
    // C-style: r0 = a, r1 = b, r3 = n
    mov r3, #4 // sizeof(int)
    mla r2, r2, r3, r0 // C-style: r3 = (a + n)
    while_start: // while
        cmp r0, r2
        beq while_end
            ldr ip, [r0] // C-style: ip = *r0
            ldr r3, [r1], #4 // C-style: r3 = *r1, r1 += 4
            add r3, r3, ip // C-style: r3 += ip
            str r3, [r0], #4 // C-style: *r0 = r3, r0 += 4
        b while_start
    while_end: // while end
    bx lr
```

### Разные варианты STR, LDR:


```cpp
%%cpp program_asm_p.c
%run arm-linux-gnueabi-gcc -marm program_asm_p.c -o program.exe
%run qemu-arm ./program.exe ; echo 

#include <stdio.h>
#include <assert.h>

int* load_1(int* a, int* b, int n);
__asm__ (R"(
.global load_1
load_1:
    ldr r3, [r0] // r3 = *r1
    str r3, [r1] // *b = r3
    bx lr
)");

int* load_2(int* a, int* b, int n);
__asm__ (R"(
.global load_2
load_2:
    ldr r3, [r0, r2, lsl #2] // r3 = *(r1 + r0 * 4)
    str r3, [r1] // *b = r3
    bx lr
)");

int* load_3(int* a, int* b, int n);
__asm__ (R"(
.global load_3
load_3:
    ldr r3, [r0, r2, lsl #2]! // r1 += r0 * 4; r3 = *r1
    str r3, [r1] // *b = r3
    bx lr
)");

int* load_4(int* a, int* b, int n);
__asm__ (R"(
.global load_4
load_4:
    ldr r3, [r0], r2, lsl #2 // r3 = *r1; r1 += r0 * 4
    str r3, [r1] // *b = r3
    bx lr
)");


int main() {
    int a[] = {10, 20, 30};
    int res;
    
    // Просто загрузить
    // ldr r3, [r0]
    assert(load_1(a, &res, 0) == a);
    assert(res == 10);
    
    // Загрузить по адресу со смещением
    // ldr r3, [r0, r2, lsl #2]
    assert(load_2(a, &res, 0) == a);
    assert(res == 10);
    assert(load_2(a, &res, 1) == a);
    assert(res == 20);
    
    // Загрузить по префикс-инкрементнутому адресу
    // ldr r3, [r0, r2, lsl #2]!
    assert(load_3(a, &res, 0) == a);
    assert(res == 10);
    assert(load_3(a, &res, 1) == a + 1); // (!)
    assert(res == 20);
    
    // Загрузить по постфикс-инкрементнутому адресу
    // ldr r3, [r0], r2, lsl #2
    assert(load_4(a, &res, 0) == a);
    assert(res == 10);
    assert(load_4(a, &res, 1) == a + 1); // (!)
    assert(res == 10); // (!)
        
    printf("SUCCESS\n");
    
    return 0;
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


```python

```


```python

```
