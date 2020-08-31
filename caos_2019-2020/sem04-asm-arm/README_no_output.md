


```python
# Add path to compilers to PATH
import os
os.environ["PATH"] = os.environ["PATH"] + ":" + \
    "/home/pechatnov/Downloads/gcc-linaro-7.3.1-2018.05-i686_arm-linux-gnueabi/bin/"

```

#### Makefile в котором будет компиляция и запуски всех примеров


```python
%%makefile 

GCC=arm-linux-gnueabi-gcc -marm
RUN=qemu-arm -L ~/Downloads/sysroot-glibc-linaro-2.25-2018.05-arm-linux-gnueabi

hello:
    ${GCC} hello.c -o hello.exe
    ${GCC} hello.c -S -o hello.S

hello_run: hello
    ${RUN} ./hello.exe   
    
lib_sum:
    ${GCC} lib_sum.c -c
    ${GCC} lib_sum.c -S -o lib_sum.S
    ${GCC} lib_sum.c -O0 -S -o lib_sum_o0.S
    ${GCC} lib_sum.c -O3 -S -o lib_sum_o3.S
    
my_lib:
    ${GCC} -g my_lib_sum.S -c
    
my_lib_example: my_lib
    ${GCC} -std=c99 -g my_lib_sum.o my_lib_example.c -o my_lib_example.exe
    
my_lib_example_run: my_lib_example
    ${RUN} ./my_lib_example.exe 
    
my_lib_example_run_gdb: my_lib_example
    ${RUN} -g 1234 ./my_lib_example.exe 
    # Подключаться с помощью gdb можно так:
    # gdb-multiarch -q --nh   -ex 'set architecture arm'   -ex 'set sysroot ~/Downloads/sysroot-glibc-linaro-2.25-2018.05-arm-linux-gnueabi'   -ex 'file ./my_lib_example.exe'   -ex 'target remote localhost:1234'   -ex 'break main'   -ex continue   -ex 'layout split'
    
asm_inline_example:
    ${GCC} asm_inline_example.c -o asm_inline_example.exe
    
asm_inline_example_run: asm_inline_example
    ${RUN} ./asm_inline_example.exe
    
```


```cpp
%%cpp hello.c
%run make hello_run

// Скомпилируем под arm и запустим hello_world 

#include <stdio.h>

int main() {
    printf("hello world!\n");
    return 0;
}

```


```python
!cat hello.S
```

#### Напишем и скомпилируем до состояния arm'ного ассемблера простую функцию


```cpp
%%cpp lib_sum.c
%run make lib_sum

int sum(int a, int b) {
    return a + b;
}
```


```python
# Здесь можно посмотреть, что получается при O0 и O3
!cat lib_sum_o0.S
```

#### А теперь самостоятельно напишем ту же функцию. Как видим кода стало меньше :) И потом вызовем ее из кода на С


```python
%%asm my_lib_sum.S
.text
.global sum
sum:
    add r0, r0, r1
    bx  lr
```


```cpp
%%cpp my_lib_example.c
%run make my_lib_example_run

#include <stdio.h>

int sum(int, int);

int main() {
    printf("40 + 2 = %d\n", sum(40, 2));
    return 0;
}
```


```python

```


```cpp
%%cpp asm_inline_example.c
%run make asm_inline_example_run

#include <stdio.h>

int sum(int a, int b) {
    return a + b;
}


int sum2(int, int);
__asm__ (R"(
.global sum2
sum2:
    add r0, r0, r1
    bx  lr
)");


int main() {
    printf("40 + 2 = %d\n", sum(40, 2));
    printf("40 + 2 = %d\n", sum2(40, 2));
    return 0;
}
```


```python

```

# Полезные фишки

`cmp r0, #'9'` - так можно писать char-константы

`push {r4, r5, r6, r7, r8, lr}` <-> `push {r4-r8, lr}`


```python

```
