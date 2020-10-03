


# ARM ASM

<table width=100%> <tr>
    <th width=20%> <b>Видеозапись семинара &rarr; </b> </th>
    <th>
    <a href="https://www.youtube.com/watch?v=OYgKVtWp2l4&list=PLjzMm8llUm4AmU6i_hPU0NobgA4VsBowc&index=6">
        <img src="video.jpg" width="320"  height="160" align="left" alt="Видео с семинара"> 
    </a>
    </th>
    <th> </th>
 </table>


[Ридинг Яковлева: Инструменты для ARM](https://github.com/victor-yacovlev/mipt-diht-caos/blob/master/practice/arm) 
<br>[Ридинг Яковлева: Ассемблер ARM](https://github.com/victor-yacovlev/mipt-diht-caos/blob/master/practice/asm/arm_basics) 

[Тут на 18 странице написано о доступных регистрах и их назначениях](https://static.docs.arm.com/ihi0042/g/aapcs32.pdf)

Сегодня в программе:
* <a href="#install_instr" style="color:#856024"> Установка инструментов </a>
* <a href="#compile_run" style="color:#856024"> Компиляция и запуск </a>
* <a href="#asm" style="color:#856024"> Ассемблер ARM </a>
  * <a href="#easy_asm" style="color:#856024"> Простой способ написать функцию на ассемблере </a>

## <a name="install_instr"></a> Установка инструментов

1) Скачать набор компиляторов и динамических библиотек из проекта linaro
<br> `wget http://releases.linaro.org/components/toolchain/binaries/7.3-2018.05/arm-linux-gnueabi/gcc-linaro-7.3.1-2018.05-i686_arm-linux-gnueabi.tar.xz`

2) Распаковать этот набор
<br> xvf = eXtract Verbose File если не ошибаюсь
<br> `tar xvf gcc-linaro-7.3.1-2018.05-i686_arm-linux-gnueabi.tar.xz`

3) Установить qemu-arm
<br> `sudo apt-get install qemu-user-static qemu-system-arm qemu-user`
<br> (возможно что-то в списке лишнее)

## <a name="compile_run"></a> Компиляция и запуск
1. Стоит добавить путь до скачанных компиляторов в PATH, чтобы их можно было запускать по имени программы, а не по пути до нее.


```python
import os

linaro_download_dir = os.path.expanduser("~/arm/gcc-linaro-7.3.1-2018.05-i686_arm-linux-gnueabi")

gcc_linaro_path = os.path.join(linaro_download_dir, "bin")
if gcc_linaro_path not in os.environ["PATH"]:
    os.environ["PATH"] += ":" + gcc_linaro_path
```

В консоли можно сделать

`export PATH=$PATH:/home/pechatnov/arm/gcc-linaro-7.3.1-2018.05-i686_arm-linux-gnueabi/bin`

2. Стоит записать путь до динамических библиотек ARM в QEMU_LD_PREFIX, чтобы не писать -L опцию.


```python
os.environ["QEMU_LD_PREFIX"] = os.path.join(linaro_download_dir, "arm-linux-gnueabi/libc")    
```

В консоли можно написать 
<br>`export QEMU_LD_PREFIX=/home/pechatnov/arm/gcc-linaro-7.3.1-2018.05-i686_arm-linux-gnueabi/arm-linux-gnueabi/libc`

Или запускать как 
<br>`qemu-arm -L /home/pechatnov/arm/gcc-linaro-7.3.1-2018.05-i686_arm-linux-gnueabi/arm-linux-gnueabi/libc ./program.exe`

3. Все, можно компилировать и запускать


```cpp
%%cpp size.c
%run arm-linux-gnueabi-gcc -marm size.c -o size.exe
%run qemu-arm ./size.exe

#include <stdio.h>

int main() {
    printf("is char signed = %d, ", (int)((char)(-1) > 0));
    printf("sizeof(long int) = %d\n", (int)sizeof(long int));
}
```


Run: `arm-linux-gnueabi-gcc -marm size.c -o size.exe`



Run: `qemu-arm ./size.exe`


    is char signed = 1, sizeof(long int) = 4


## <a name="asm"></a> Ассемблер ARM

Микроблиотека


```cpp
%%cpp lib.c
%run arm-linux-gnueabi-gcc -c -Os -fno-asynchronous-unwind-tables -marm lib.c -o lib.o
%// run arm-linux-gnueabi-gcc -S -Os -fno-asynchronous-unwind-tables -marm lib.c -o /dev/stdout
%run arm-linux-gnueabi-objdump -D lib.o | grep '<sum>' -A 3

int sum(int a, int b) {
    return a + b;
}
```


Run: `arm-linux-gnueabi-gcc -c -Os -fno-asynchronous-unwind-tables -marm lib.c -o lib.o`



\#\#\#\# `run arm-linux-gnueabi-gcc -S -Os -fno-asynchronous-unwind-tables -marm lib.c -o /dev/stdout`



Run: `arm-linux-gnueabi-objdump -D lib.o | grep '<sum>' -A 3`


    00000000 <sum>:
       0:	e0800001 	add	r0, r0, r1
       4:	e12fff1e 	bx	lr
    


Hello world


```cpp
%%cpp hello.c
%run arm-linux-gnueabi-gcc -S -Os -fno-asynchronous-unwind-tables -marm hello.c -o /dev/stdout

// Скомпилируем под arm и запустим hello_world 

#include <stdio.h>

int main() {
    printf("hello world!\n");
    return 0;
}
```


Run: `arm-linux-gnueabi-gcc -S -Os -fno-asynchronous-unwind-tables -marm hello.c -o /dev/stdout`


    	.arch armv7-a
    	.eabi_attribute 20, 1
    	.eabi_attribute 21, 1
    	.eabi_attribute 23, 3
    	.eabi_attribute 24, 1
    	.eabi_attribute 25, 1
    	.eabi_attribute 26, 2
    	.eabi_attribute 30, 4
    	.eabi_attribute 34, 1
    	.eabi_attribute 18, 4
    	.file	"hello.c"
    	.text
    	.section	.text.startup,"ax",%progbits
    	.align	2
    	.global	main
    	.syntax unified
    	.arm
    	.fpu softvfp
    	.type	main, %function
    main:
    	@ args = 0, pretend = 0, frame = 0
    	@ frame_needed = 0, uses_anonymous_args = 0
    	push	{r4, lr}
    	ldr	r0, .L3
    	bl	puts
    	mov	r0, #0
    	pop	{r4, pc}
    .L4:
    	.align	2
    .L3:
    	.word	.LC0
    	.size	main, .-main
    	.section	.rodata.str1.1,"aMS",%progbits,1
    .LC0:
    	.ascii	"hello world!\000"
    	.ident	"GCC: (Linaro GCC 7.3-2018.05) 7.3.1 20180425 [linaro-7.3-2018.05 revision d29120a424ecfbc167ef90065c0eeb7f91977701]"
    	.section	.note.GNU-stack,"",%progbits


Самое основное:
* Есть регистры и есть память
* Стек тоже память и он растет вниз
* Есть инструкции для операций с регистрами `ADD`, `MUL`, `CMP`, ...
* Есть инструкции условных и безусловных переходов (на самом деле те же операции с регистрами). `B`, `BL`, `BX`, `BGT`...
* Есть инструкции, для взаимодействия с памятью `LDR`, `STR`
* Первые 4 аргумента функции принимаются через `r0`..`r3`, остальные через стек
* После вызова функции (вашей или не вашей) значения `r4`-`r8`, `r10`, `r11` не должны изменяться. (Функция может их использовать, но обязана восстановить прежние значения) 
* `r9`, `r12`-`r15` имеют специальное значение, проще их не использовать
* `pc` - в этом регистре хранится адрес текущей инструкции, его изменение - то же самое, что безусловный переход
* `sp` - указатель на стек
* `lr` - адрес возврата



```python

```

##  <a name="easy_asm"></a> Простой способ написать что-то на ассемблере

### 1. Пишем что-то на Си 


```cpp
%%cpp lib.c
%run arm-linux-gnueabi-gcc -S -Os -marm lib.c -o /dev/stdout | grep -v eabi

int strange_function(int n, int a) {
    if (n < 5) {
        return 5 - n;
    }
    return n * a + 41000000;
}
```


Run: `arm-linux-gnueabi-gcc -S -Os -marm lib.c -o /dev/stdout | grep -v eabi`


    	.arch armv7-a
    	.file	"lib.c"
    	.text
    	.align	2
    	.global	strange_function
    	.syntax unified
    	.arm
    	.fpu softvfp
    	.type	strange_function, %function
    strange_function:
    	@ args = 0, pretend = 0, frame = 0
    	@ frame_needed = 0, uses_anonymous_args = 0
    	@ link register save eliminated.
    	cmp	r0, #4
    	ldrgt	r3, .L4
    	rsble	r0, r0, #5
    	mlagt	r0, r1, r0, r3
    	bx	lr
    .L5:
    	.align	2
    .L4:
    	.word	41000000
    	.size	strange_function, .-strange_function
    	.ident	"GCC: (Linaro GCC 7.3-2018.05) 7.3.1 20180425 [linaro-7.3-2018.05 revision d29120a424ecfbc167ef90065c0eeb7f91977701]"
    	.section	.note.GNU-stack,"",%progbits



```cpp
%%cpp program.c
%run arm-linux-gnueabi-gcc -marm program.c lib.c -o program.exe
%run qemu-arm ./program.exe

#include <stdio.h>
#include <assert.h>

int strange_function(int n, int a);

int main() {
    printf("%d\n", strange_function(4, 0));
    assert(strange_function(4, 0) == 1);
    printf("%d\n", strange_function(4, 2));
    assert(strange_function(4, 2) == 1);
    printf("%d\n", strange_function(5, 0));
    assert(strange_function(5, 0) == 41000000);
    printf("%d\n", strange_function(5, 2));
    assert(strange_function(5, 2) == 41000010);
    return 0;
}
```


Run: `arm-linux-gnueabi-gcc -marm program.c lib.c -o program.exe`



Run: `qemu-arm ./program.exe`


    1
    1
    41000000
    41000010


### 2. Копипастим ассемблерный код и собираемся с ним


```cpp
%%cpp lib.S
%run arm-linux-gnueabi-gcc -marm program.c lib.S -o program.exe
%run qemu-arm ./program.exe

.arch armv7-a
	.file	"lib.c"
	.text
	.align	2
	.global	strange_function
	.syntax unified
	.arm
	.fpu softvfp
	.type	strange_function, %function
strange_function:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	cmp	r0, #4
	ldrgt	r3, .L4
	rsble	r0, r0, #5
	mlagt	r0, r1, r0, r3
	bx	lr
.L5:
	.align	2
.L4:
	.word	41000000
	.size	strange_function, .-strange_function
	.ident	"GCC: (Linaro GCC 7.3-2018.05) 7.3.1 20180425 [linaro-7.3-2018.05 revision d29120a424ecfbc167ef90065c0eeb7f91977701]"
	.section	.note.GNU-stack,"",%progbits
```


Run: `arm-linux-gnueabi-gcc -marm program.c lib.S -o program.exe`



Run: `qemu-arm ./program.exe`


    1
    1
    41000000
    41000010


### 3. Минимизируем ассемблерный код


```cpp
%%cpp lib.S
%run arm-linux-gnueabi-gcc -marm program.c lib.S -o program.exe
%run qemu-arm ./program.exe

.text                         // указание, что дальше пойдет секция с кодом
.global strange_function      // этот символ должен быть виден извне (аналог extern в Си и противоположность static)
strange_function:             // метка как для goto / функция / потенциальный символ
    cmp r0, #4 // сравниваем r0 и 4, выставляем флаги
    ldrgt r3, .L4 // если, было GT=больше, то загружаем в регистр r3 значение из .L4 (там число 41000000)
    rsble r0, r0, #5 // если было LE=(меньше или равно), то r0=5-r0
    mlagt r0, r1, r0, r3 // если было GT=больше, то r0=r1*r0+r3
    bx lr // перейти по адресу lr (там хранят адрес возврата из фунции)
.L4:
    .word 41000000
        
```


Run: `arm-linux-gnueabi-gcc -marm program.c lib.S -o program.exe`



Run: `qemu-arm ./program.exe`


    1
    1
    41000000
    41000010


### 4. Понимаем, что можем большее - написать код по-другому и почти самостоятельно


```cpp
%%cpp lib.S
%run arm-linux-gnueabi-gcc -marm program.c lib.S -o program.exe
%run qemu-arm ./program.exe

.text                         
.global strange_function      
strange_function:             
    cmp r0, #5
    blt .strange_function_if_less
    ldr r3, .strange_function_C1
    mul r0, r0, r1
    add r0, r0, r3
    bx lr
.strange_function_if_less:
    //sub r0, #5, r0 // а так нельзя, константы могут быть только последним аргументов
    rsb r0, r0, #5
    bx lr
.strange_function_C1:
    .word 41000000
    
```


Run: `arm-linux-gnueabi-gcc -marm program.c lib.S -o program.exe`



Run: `qemu-arm ./program.exe`


    1
    1
    41000000
    41000010


Еще можно писать код в ассемблерных вставках, если хочется:


```cpp
%%cpp program_asm_p.c
%run arm-linux-gnueabi-gcc -marm program_asm_p.c -o program.exe
%run qemu-arm ./program.exe

#include <stdio.h>

int strange_function(int n, int a);
__asm__ (R"(
.global strange_function
strange_function:
    cmp r0, #4 
    ldrgt r3, .strange_function_L4 
    rsble r0, r0, #5
    mlagt r0, r1, r0, r3
    bx lr 
.strange_function_L4: // более длинное название метки, чтобы точно не пересечься со сгенерированными компилятором
    .word 41000000
)");

int main() {
    printf("%d\n", strange_function(4, 0));
    printf("%d\n", strange_function(4, 2));
    printf("%d\n", strange_function(5, 0));
    printf("%d\n", strange_function(5, 2));
    return 0;
}

```


Run: `arm-linux-gnueabi-gcc -marm program_asm_p.c -o program.exe`



Run: `qemu-arm ./program.exe`


    1
    1
    41000000
    41000010


# Полезные фишки

`cmp r0, #'9'` - так можно писать char-константы

`push {r4, r5, r6, r7, r8, lr}` <-> `push {r4-r8, lr}`


`str r0, [r1, #4]! (C-style: *(r1 += 4) = r0)` - то же самое, что и `str r0, [r1, #4] (C-style: *(r1 + 4) = r0)`, но в `r1`, будет сохранено `r1 + #4` после выполнения команды. Другими словами префиксный инкремент на 4.

`ldr r0, [r1], #4` - то же самое, что и `ldr r0, [r1] (C-style: r0 = *r1)` с последующим `add r1, r1, #4 (C-style: r1 += 4)`. Другими словами постфиксный инкремент.

### Посмотрим на передачу аргументов в функции


```cpp
%%cpp program_asm_p.c
%run arm-linux-gnueabi-gcc -marm program_asm_p.c -o program.exe
%run qemu-arm ./program.exe ; echo 

#include <stdio.h>

void print_args(int a, int b, int c, int d, int e, int f) {
    printf("%d %d %d %d %d %d\n", a, b, c, d, e, f);
}

void call_print_args_first_3();
__asm__ (R"(
.global call_print_args_first_3
call_print_args_first_3:
    push {lr} // надо сохранить lr, так как BL его затрет
    mov r0, #1
    mov r1, #2
    mov r2, #3
    bl print_args
    pop {pc} // достаем lr со стека и сразу записываем в pc - тем самым выходим из функции
)");


void call_print_args_first_6();
__asm__ (R"(
.global call_print_args_first_6
call_print_args_first_6:
    push {lr} // сохраняем lr
    mov r0, #5
    mov r1, #6
    push {r0, r1} // кладем на стек 5, 6 в качестве 5-6 аргументов функции
    
    mov r0, #1
    mov r1, #2
    mov r2, #3
    mov r3, #4
    
    bl print_args
    pop {r0, r1} // снимаем со стека аргументы функции
    
    pop {pc} // достаем lr со стека и сразу записываем в pc - тем самым выходим из функции
)");

void call_print_args_first_6_2();
__asm__ (R"(
.global call_print_args_first_6_2
call_print_args_first_6_2:
    push {r4-r5, lr} // сохраняем lr, а так же r5-r6 так как мы их должны вернуть в изначальное состояние

    mov r0, #1
    mov r1, #2
    mov r2, #3
    mov r3, #4
    mov r4, #5
    mov r5, #6
    
    // Три эквивалентных варианта
    // 1
    //push {r4, r5} // кладем на стек 5, 6 в качестве 5-6 аргументов функции
    // 2
    //sub sp, sp, #4
    //str r5, [sp]
    //sub sp, sp, #4
    //str r4, [sp]
    // 3
    str r5, [sp, #-4]!
    str r4, [sp, #-4]!
    bl print_args
    add sp, sp, #8 // вместо pop {r4, r5}, нам же не нужны больше эти аргументы, зачем их возвращать в r4 и r5?
    
    pop {r4-r5, pc} // достаем lr со стека и сразу записываем в pc - тем самым выходим из функции
)");


int main() {
    call_print_args_first_3();
    call_print_args_first_6();
    call_print_args_first_6_2();
    return 0;
}

```


Run: `arm-linux-gnueabi-gcc -marm program_asm_p.c -o program.exe`



Run: `qemu-arm ./program.exe ; echo`


    1 2 3 66712 66724 0
    1 2 3 4 5 6
    1 2 3 4 5 6
    



```python

```

### Пример вызова функции из ассемблерного кода


```cpp
%%cpp test_call.c
%run arm-linux-gnueabi-gcc -marm test_call.c -O2 -o test_call.exe
%run qemu-arm ./test_call.exe

#include <stdio.h>


int print_a(int a);
__asm__(R"(
    .text
    .global print_a
print_a:
    push {lr}
    
    mov r2, r0    
    ldr r0, =stdout // load address of variable 'stdout' 
    ldr r0, [r0]    // load variable by address (value is FILE* pointer)
    ldr r1, =.format_string // load address
    ldr r3, .const_number   // load value
    
    bl fprintf

    pop {pc}
.format_string:
    .ascii "Your number is %d, const number is %d\n" // заметьте, оно конкатенируется со слеюущим литералом
    .ascii "\0" // а \0 тут надо писать! Можете попытаться закомментировать эту строчку
.const_number:
    .word 100200300
)");

int main() {
    fprintf(stdout, "Your number is %d, const number is %d\n", 100500, 100200300);
    print_a(100500);
}
```


Run: `arm-linux-gnueabi-gcc -marm test_call.c -O2 -o test_call.exe`



Run: `qemu-arm ./test_call.exe`


    Your number is 100500, const number is 100200300
    Your number is 100500, const number is 100200300



```python

```
