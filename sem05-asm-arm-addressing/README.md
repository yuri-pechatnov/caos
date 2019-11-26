```python
# make magics here. Look at previous notebooks to see readable version
exec('\nget_ipython().run_cell_magic(\'javascript\', \'\', \'// setup cpp code highlighting\\nIPython.CodeCell.options_default.highlight_modes["text/x-c++src"] = {\\\'reg\\\':[/^%%cpp/]} ;\')\n\n# creating magics\nfrom IPython.core.magic import register_cell_magic, register_line_magic\nfrom IPython.display import display, Markdown\n\n@register_cell_magic\ndef save_file(fname, cell):\n    cell = cell if cell[-1] == \'\\n\' else cell + "\\n"\n    cmds = []\n    with open(fname, "w") as f:\n        for line in cell.split("\\n"):\n            if line.startswith("%"):\n                run_prefix = "%run "\n                assert line.startswith(run_prefix)\n                cmds.append(line[len(run_prefix):].strip())\n            else:\n                f.write(line + "\\n")\n    for cmd in cmds:\n        display(Markdown("Run: `%s`" % cmd))\n        get_ipython().system(cmd)\n\n@register_cell_magic\ndef cpp(fname, cell):\n    save_file(fname, cell)\n\n@register_cell_magic\ndef asm(fname, cell):\n    save_file(fname, cell)\n    \n@register_cell_magic\ndef makefile(fname, cell):\n    assert not fname\n    save_file("makefile", cell.replace(" " * 4, "\\t"))\n        \n@register_line_magic\ndef p(line):\n    print("{} = {}".format(line, eval(line)))\n')
```


    <IPython.core.display.Javascript object>


# Адресация памяти в ассемблере

[Ридинг от Яковлева](https://github.com/victor-yacovlev/mipt-diht-caos/tree/master/practice/asm/arm_load_store)

Добавление от меня:

1. `str r0, [r1, #4]! (C-style: *(r1 += 4) = r0)` - то же самое, что и `str r0, [r1, #4] (C-style: *(r1 + 4) = r0)`, но в `r1`, будет сохранено `r1 + #4` после выполнения команды. Другими словами префиксный инкремент на 4.
1. `ldr r0, [r1], #4` - то же самое, что и `ldr r0, [r1] (C-style: r0 = *r1)` с последующим `add r1, r1, #4 (C-style: r1 += 4)`. Другими словами постфиксный инкремент.


```python
# Add path to compilers to PATH
import os
os.environ["PATH"] = os.environ["PATH"] + ":" + \
    "/home/pechatnov/Downloads/gcc-linaro-7.3.1-2018.05-i686_arm-linux-gnueabi/bin/"

```

# Пример работы с массивом из ассемблера


```cpp
%%cpp is_sorted.c
%run arm-linux-gnueabi-gcc -marm is_sorted.c -o is_sorted.exe
%run qemu-arm -L ~/Downloads/sysroot-glibc-linaro-2.25-2018.05-arm-linux-gnueabi ./is_sorted.exe

#include <stdio.h>
#include <assert.h>

int is_sorted(int n, unsigned int* x);
__asm__ (R"(
.global is_sorted
is_sorted:
    // r0 - n, r1 - x
    cmp r0, #1
    bls is_sorted_true
    sub r0, r0, #1
    add r1, r1, #4
    ldr r2, [r1, #-4]
    ldr r3, [r1]
    cmp r2, r3
    bhi is_sorted_false
    b is_sorted
is_sorted_false:
    mov r0, #0
    bx lr
is_sorted_true:
    mov r0, #1
    bx  lr
)");

#define check(result, ...) {\
    unsigned int a[] = {__VA_ARGS__}; \
    int r = is_sorted(sizeof(a) / sizeof(int), a); \
    printf("is_sorted({" #__VA_ARGS__ "}) = %d ?= %d\n", r, result);\
    assert(r == result); \
}

#define check_sorted(...) check(1, __VA_ARGS__)
#define check_not_sorted(...) check(0, __VA_ARGS__)


int test() {
    check_sorted();
    check_sorted(10);
    check_sorted(10, 20);
    check_sorted(10, 20, 30);
    check_sorted(30, 30, 30);
    check_not_sorted(20, 10);
    check_not_sorted(10, 30, 20);
}

int main() {
    test();
    return 0;
}
```


Run: `arm-linux-gnueabi-gcc -marm is_sorted.c -o is_sorted.exe`



Run: `qemu-arm -L ~/Downloads/sysroot-glibc-linaro-2.25-2018.05-arm-linux-gnueabi ./is_sorted.exe`


    is_sorted({}) = 1 ?= 1
    is_sorted({10}) = 1 ?= 1
    is_sorted({10, 20}) = 1 ?= 1
    is_sorted({10, 20, 30}) = 1 ?= 1
    is_sorted({30, 30, 30}) = 1 ?= 1
    is_sorted({20, 10}) = 0 ?= 0
    is_sorted({10, 30, 20}) = 0 ?= 0


# Пример приема более, чем 4 аргументов в функции

ip = r12. Почему он тут портится? У меня нет ответа `¯\_(ツ)_/¯ `


```cpp
%%cpp more_than_4.c
%run arm-linux-gnueabi-gcc -marm more_than_4.c -O2 -S -o more_than_4.s
%run cat more_than_4.s | grep -v "^\\s*\\." | grep -v "^\\s*@"

int mega_sum(int a1, int a2, int a3, int a4, int a5, int a6) {
    return a1 + a2 + a3 + a4 + a5 + a6;
}
```


Run: `arm-linux-gnueabi-gcc -marm more_than_4.c -O2 -S -o more_than_4.s`



Run: `cat more_than_4.s | grep -v "^\\s*\\." | grep -v "^\\s*@"`


    mega_sum:
    	add	r1, r0, r1
    	ldr	ip, [sp]
    	add	r1, r1, r2
    	ldr	r0, [sp, #4]
    	add	r1, r1, r3
    	add	r1, r1, ip
    	add	r0, r1, r0
    	bx	lr


# Пример чтения структуры из ассемблера


```cpp
%%cpp cut_struct.c
%run arm-linux-gnueabi-gcc -marm cut_struct.c -o cut_struct.exe
%run qemu-arm -L ~/Downloads/sysroot-glibc-linaro-2.25-2018.05-arm-linux-gnueabi ./cut_struct.exe

#include <stdio.h>
#include <assert.h>

struct Obj {
    char c;
    int i;
    short s;
    char c2;
} __attribute__((packed));

int cut_struct(struct Obj* obj, char* c, int* i, short* s, char* c2);
__asm__ (R"(
.global cut_struct
cut_struct:
    push {r4, r5} // notice that we decrease sp by pushing
    ldr r4, [sp, #8] // get last arg from stack (in fact we use initial value of sp)
    // r0 - obj, r1 - c, r2 - i, r3 - s, r4 - c2
    ldrb r5, [r0, #0]
    strb r5, [r1]
    ldr r5, [r0, #1]
    str r5, [r2]
    ldrh r5, [r0, #5]
    strh r5, [r3]
    ldrb r5, [r0, #7]
    strb r5, [r4]
    pop {r4, r5}
    bx  lr
)");

int test() {
    // designated initializers: https://en.cppreference.com/w/c/language/struct_initialization
    struct Obj obj = {.c = 123, .i = 100500, .s = 15000, .c2 = 67};
    char c = 0; int i = 0; short s = 0; char c2 = 0; // bad codestyle
    cut_struct(&obj, &c, &i, &s, &c2);
    fprintf(stderr, "Got c=%d, i=%d, s=%d, c2=%d", (int)c, (int)i, (int)s, (int)c2);
    assert(c == obj.c && i == obj.i && s == obj.s && c2 == obj.c2);
}

int main() {
    test();
    return 0;
}
```


Run: `arm-linux-gnueabi-gcc -marm cut_struct.c -o cut_struct.exe`



Run: `qemu-arm -L ~/Downloads/sysroot-glibc-linaro-2.25-2018.05-arm-linux-gnueabi ./cut_struct.exe`


    Got c=123, i=100500, s=15000, c2=67

Можно сравнить с дизассемблером


```cpp
%%cpp cut_struct_disasm.c
%run arm-linux-gnueabi-gcc -marm cut_struct_disasm.c -O2 -S -o cut_struct_disasm.s
%run cat cut_struct_disasm.s | grep -v "^\\s*\\." | grep -v "^\\s*@"

struct Obj {
    char c;
    int i;
    short s;
    char c2;
} __attribute__((packed));

int cut_struct(struct Obj* obj, char* c, int* i, short* s, char* c2) {
    *c = obj->c;
    *i = obj->i;
    *s = obj->s;
    *c2 = obj->c2;
}
```


Run: `arm-linux-gnueabi-gcc -marm cut_struct_disasm.c -O2 -S -o cut_struct_disasm.s`



Run: `cat cut_struct_disasm.s | grep -v "^\\s*\\." | grep -v "^\\s*@"`


    cut_struct:
    	mov	ip, r0
    	push	{r4, lr}
    	ldrb	r4, [ip]	@ zero_extendqisi2
    	mov	r0, lr
    	ldr	lr, [sp, #8]
    	strb	r4, [r1]
    	ldr	r1, [ip, #1]	@ unaligned
    	str	r1, [r2]
    	ldrsh	r2, [ip, #5]	@ unaligned
    	strh	r2, [r3]	@ movhi
    	ldrb	r3, [ip, #7]	@ zero_extendqisi2
    	strb	r3, [lr]
    	pop	{r4, pc}


# Размещение структур в памяти

Не все всегда расположено очевидным образом: для более эффективного выполнения программы бывает выгодно выровненное расположение объектов в памяти, например считывать ui64 из памяти выгоднее, если адрес делится на 8.

Примерные правила:
* выравнивание (то, на что адрес должен делиться) равно размеру для простых арифметических типов (указатели тоже здесь)
* в union берется максимум для размера и выравнивания
* в struct члены располагаются в том порядке, в котором указаны. Выравнивание структуры - максимум выравниваний. Каждый член располагается так, чтобы удовлетворять собственному выравниванию. Итоговый размер структуры делится на выравнивание структуры. С учетом этого размер струкуры минимизируется.

Для экспериментов можно использовать `sizeof()` и `_Alignof()`, чтобы получить размер и выравнивание.


```cpp
%%cpp structs_in_memory.c
%run arm-linux-gnueabi-gcc -marm structs_in_memory.c -o structs_in_memory.exe
%run qemu-arm -L ~/Downloads/sysroot-glibc-linaro-2.25-2018.05-arm-linux-gnueabi ./structs_in_memory.exe

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#define print_int(x) printf(#x " = %d\n", (int)x)

#define print_offset(type, field) {\
    type o; \
    printf("Shift of ." #field " in " #type ": %d\n", (int)((void*)&o.field - (void*)&o)); \
}

int main() {
    print_int(sizeof(char));
    print_int(_Alignof(char));
    print_int(sizeof(short));
    print_int(_Alignof(short));
    print_int(sizeof(int));
    print_int(_Alignof(int));
    print_int(sizeof(long long));
    print_int(_Alignof(long long));
    print_int(sizeof(double));
    print_int(_Alignof(double));

    typedef struct { // максимальное выравнивание у инта, значит выравнивание структуры 4
        char c;      // 0 байт
        int i;       // 4-7 байты
        char c2;     // 8 байт
    } Obj1_t;        // 9-11 - padding байты, чтобы размер делился на выравнивание
    print_int(sizeof(Obj1_t));
    print_int(_Alignof(Obj1_t));
    print_offset(Obj1_t, c);
    print_offset(Obj1_t, i);
    print_offset(Obj1_t, c2);
    
    typedef struct { // тут все правила про выравнивание не применимы, так как указан аттрибут упаковки
        char c;
        int i;
        char c2;
    } __attribute__((packed)) Obj2_t;
    print_int(sizeof(Obj2_t));
    print_int(_Alignof(Obj2_t));
    print_offset(Obj2_t, c);
    print_offset(Obj2_t, i);
    print_offset(Obj2_t, c2);
    
    typedef struct {  // максимальное выравнивание члена - 8, так что и у всей структуры такое же
        char c8;      // 0 байт 
        uint64_t u64; // 8-15 байты
    } Obj3_t;         // всего 16 байт, выравнивание 8
    print_int(sizeof(Obj3_t));
    print_int(_Alignof(Obj3_t));
    print_offset(Obj3_t, u64);
    print_offset(Obj3_t, c8);
    
    typedef struct {
        char c8;
        char c8_1;
        char c8_2;
    } Obj4_t;
    print_int(sizeof(Obj4_t));
    print_int(_Alignof(Obj4_t));
    
    typedef struct {     // тут пример двух структур равного размера, но с разным выравниванием
        long long a;
    } ObjS8A8;
    print_int(sizeof(ObjS8A8));
    print_int(_Alignof(ObjS8A8));
    typedef struct {
        int a;
        int b;
    } ObjS8A4;
    print_int(sizeof(ObjS8A4));
    print_int(_Alignof(ObjS8A4));
    
    typedef struct {    // и вот тут разное выравнивание ObjS8A8 и ObjS8A4 себя покажет
        ObjS8A8 o;
        char c;
    } Obj5_t;
    print_int(sizeof(Obj5_t)); // обратите внимание на разницу с Obj6_t!
    print_int(_Alignof(Obj5_t));
    
    typedef struct {
        ObjS8A4 o;
        char c;
    } Obj6_t;
    print_int(sizeof(Obj6_t));
    print_int(_Alignof(Obj6_t));
    
    return 0;
}
```


Run: `arm-linux-gnueabi-gcc -marm structs_in_memory.c -o structs_in_memory.exe`



Run: `qemu-arm -L ~/Downloads/sysroot-glibc-linaro-2.25-2018.05-arm-linux-gnueabi ./structs_in_memory.exe`


    sizeof(char) = 1
    _Alignof(char) = 1
    sizeof(short) = 2
    _Alignof(short) = 2
    sizeof(int) = 4
    _Alignof(int) = 4
    sizeof(long long) = 8
    _Alignof(long long) = 8
    sizeof(double) = 8
    _Alignof(double) = 8
    sizeof(Obj1_t) = 12
    _Alignof(Obj1_t) = 4
    Shift of .c in Obj1_t: 0
    Shift of .i in Obj1_t: 4
    Shift of .c2 in Obj1_t: 8
    sizeof(Obj2_t) = 6
    _Alignof(Obj2_t) = 1
    Shift of .c in Obj2_t: 0
    Shift of .i in Obj2_t: 1
    Shift of .c2 in Obj2_t: 5
    sizeof(Obj3_t) = 16
    _Alignof(Obj3_t) = 8
    Shift of .u64 in Obj3_t: 8
    Shift of .c8 in Obj3_t: 0
    sizeof(Obj4_t) = 3
    _Alignof(Obj4_t) = 1
    sizeof(ObjS8A8) = 8
    _Alignof(ObjS8A8) = 8
    sizeof(ObjS8A4) = 8
    _Alignof(ObjS8A4) = 4
    sizeof(Obj5_t) = 16
    _Alignof(Obj5_t) = 8
    sizeof(Obj6_t) = 12
    _Alignof(Obj6_t) = 4


# Вызов функций


```cpp
%%cpp call.c
%run arm-linux-gnueabi-gcc -marm call.c -O2 -S -o call.s
%run cat call.s | grep -v "^\\s*\\." | grep -v "^\\s*@"

#include <stdio.h>

int print_a(char a) {
    fputc(a, stdout);
    return 1;
}
```


Run: `arm-linux-gnueabi-gcc -marm call.c -O2 -S -o call.s`



Run: `cat call.s | grep -v "^\\s*\\." | grep -v "^\\s*@"`


    print_a:
    	movw	r3, #:lower16:stdout
    	push	{r4, lr}
    	movt	r3, #:upper16:stdout
    	ldr	r1, [r3]
    	bl	fputc
    	mov	r0, #1
    	pop	{r4, pc}



```cpp
%%cpp test_call.c
%run arm-linux-gnueabi-gcc -marm test_call.c -O2 -o test_call.exe
%run qemu-arm -L ~/Downloads/sysroot-glibc-linaro-2.25-2018.05-arm-linux-gnueabi ./test_call.exe

#include <stdio.h>

int print_a(char a);
__asm__(R"(
print_a:
    push {lr}
    ldr r3, =stdout
    ldr r1, [r3]
    bl fputc
    mov r0, #1
    pop {pc}
)");

int main() {
    print_a('?');
}
```


Run: `arm-linux-gnueabi-gcc -marm test_call.c -O2 -o test_call.exe`



Run: `qemu-arm -L ~/Downloads/sysroot-glibc-linaro-2.25-2018.05-arm-linux-gnueabi ./test_call.exe`


    ?

### Форматированный вывод


```cpp
%%cpp call.c
%run arm-linux-gnueabi-gcc -marm call.c -O2 -S -o call.s
%run cat call.s

#include <stdio.h>

int print_a(int a) {
    fprintf(stdout, "%d\n", a);
    return 42;
}
```


Run: `arm-linux-gnueabi-gcc -marm call.c -O2 -S -o call.s`


    /bin/sh: 1: arm-linux-gnueabi-gcc: not found



Run: `cat call.s`


    	.arch armv7-a
    	.eabi_attribute 20, 1
    	.eabi_attribute 21, 1
    	.eabi_attribute 23, 3
    	.eabi_attribute 24, 1
    	.eabi_attribute 25, 1
    	.eabi_attribute 26, 2
    	.eabi_attribute 30, 2
    	.eabi_attribute 34, 1
    	.eabi_attribute 18, 4
    	.file	"call.c"
    	.text
    	.align	2
    	.global	scan_a
    	.syntax unified
    	.arm
    	.fpu softvfp
    	.type	scan_a, %function
    scan_a:
    	@ args = 0, pretend = 0, frame = 0
    	@ frame_needed = 0, uses_anonymous_args = 0
    	movw	r3, #:lower16:stdin
    	movw	r1, #:lower16:.LC0
    	movt	r3, #:upper16:stdin
    	mov	r2, r0
    	push	{r4, lr}
    	movt	r1, #:upper16:.LC0
    	ldr	r0, [r3]
    	bl	__isoc99_fscanf
    	mov	r0, #42
    	pop	{r4, pc}
    	.size	scan_a, .-scan_a
    	.section	.rodata.str1.4,"aMS",%progbits,1
    	.align	2
    .LC0:
    	.ascii	"%d\000"
    	.ident	"GCC: (Linaro GCC 7.3-2018.05) 7.3.1 20180425 [linaro-7.3-2018.05 revision d29120a424ecfbc167ef90065c0eeb7f91977701]"
    	.section	.note.GNU-stack,"",%progbits



```cpp
%%cpp test_call.c
%run arm-linux-gnueabi-gcc -marm test_call.c -O2 -o test_call.exe
%run qemu-arm -L ~/Downloads/sysroot-glibc-linaro-2.25-2018.05-arm-linux-gnueabi ./test_call.exe

#include <stdio.h>

int print_a(int a);
__asm__(R"(
    .text
    .global print_a
print_a:
    mov r2, r0
        
    ldr r0, =stdout
    ldr r0, [r0]
    ldr r1, =.format_string
    
    push {lr}
    bl fprintf
    mov r0, #42
    pop {pc}
.format_string:
    .ascii "%d\n"
    .ascii "\0"
)");

int main() {
    print_a(100500);
}
```


Run: `arm-linux-gnueabi-gcc -marm test_call.c -O2 -o test_call.exe`



Run: `qemu-arm -L ~/Downloads/sysroot-glibc-linaro-2.25-2018.05-arm-linux-gnueabi ./test_call.exe`


    100500


### Форматированное чтение


```cpp
%%cpp call.c
%run arm-linux-gnueabi-gcc -marm call.c -O2 -S -o call.s
%run cat call.s

#include <stdio.h>

int scan_a(int* a) {
    fscanf(stdin, "%d", a);
    return 42;
}
```


Run: `arm-linux-gnueabi-gcc -marm call.c -O2 -S -o call.s`



Run: `cat call.s`


    	.arch armv7-a
    	.eabi_attribute 20, 1
    	.eabi_attribute 21, 1
    	.eabi_attribute 23, 3
    	.eabi_attribute 24, 1
    	.eabi_attribute 25, 1
    	.eabi_attribute 26, 2
    	.eabi_attribute 30, 2
    	.eabi_attribute 34, 1
    	.eabi_attribute 18, 4
    	.file	"call.c"
    	.text
    	.align	2
    	.global	scan_a
    	.syntax unified
    	.arm
    	.fpu softvfp
    	.type	scan_a, %function
    scan_a:
    	@ args = 0, pretend = 0, frame = 0
    	@ frame_needed = 0, uses_anonymous_args = 0
    	movw	r3, #:lower16:stdin
    	movw	r1, #:lower16:.LC0
    	movt	r3, #:upper16:stdin
    	mov	r2, r0
    	push	{r4, lr}
    	movt	r1, #:upper16:.LC0
    	ldr	r0, [r3]
    	bl	__isoc99_fscanf
    	mov	r0, #42
    	pop	{r4, pc}
    	.size	scan_a, .-scan_a
    	.section	.rodata.str1.4,"aMS",%progbits,1
    	.align	2
    .LC0:
    	.ascii	"%d\000"
    	.ident	"GCC: (Linaro GCC 7.3-2018.05) 7.3.1 20180425 [linaro-7.3-2018.05 revision d29120a424ecfbc167ef90065c0eeb7f91977701]"
    	.section	.note.GNU-stack,"",%progbits



```cpp
%%cpp test_call.c
%run arm-linux-gnueabi-gcc -marm test_call.c -O2 -o test_call.exe
%run echo "123 124 125" | qemu-arm -L ~/Downloads/sysroot-glibc-linaro-2.25-2018.05-arm-linux-gnueabi ./test_call.exe

#include <stdio.h>

int scan_a(int* a);
__asm__(R"(
    .text
    .global scan_a
scan_a:
    mov r2, r0
    mov r3, r0
    ldr r0, =stdin
    ldr r0, [r0]
    ldr r1, =.format_string
    push {lr}
    push {r2}
    bl __isoc99_fscanf
    pop {r2}
    mov r0, #42
    pop {pc}
.format_string:
    .ascii "%d %d %d\0"
)");

int main() {
    int a = 100500;
    scan_a(&a);
    printf("a = %d\n", a);
}
```


Run: `arm-linux-gnueabi-gcc -marm test_call.c -O2 -o test_call.exe`



Run: `echo "123 124 125" | qemu-arm -L ~/Downloads/sysroot-glibc-linaro-2.25-2018.05-arm-linux-gnueabi ./test_call.exe`


    a = 125



```cpp
%%cpp test_call.c
%run arm-linux-gnueabi-gcc -marm test_call.c -O2 -o test_call.exe
%run echo "123 124 125" | qemu-arm -L ~/Downloads/sysroot-glibc-linaro-2.25-2018.05-arm-linux-gnueabi ./test_call.exe

#include <stdio.h>

int ret_eof();
__asm__(R"(
#include <stdio.h>
    .text
    .global ret_eof
ret_eof:
    mov r0, =EOF
    bx lr
)");

int main() {
    printf("%d\n", ret_eof());
}
```


Run: `arm-linux-gnueabi-gcc -marm test_call.c -O2 -o test_call.exe`


    /tmp/ccC166AS.s: Assembler messages:
    /tmp/ccC166AS.s:19: Error: immediate expression requires a # prefix -- `mov r0,=EOF'



Run: `echo "123 124 125" | qemu-arm -L ~/Downloads/sysroot-glibc-linaro-2.25-2018.05-arm-linux-gnueabi ./test_call.exe`


    a = 125


# Решение одной домашней задачи


```python
%%asm sol.S
%run arm-linux-gnueabi-gcc -marm sol.S -O2 -o sol.exe
%run echo "123 124 125" | qemu-arm -L ~/Downloads/sysroot-glibc-linaro-2.25-2018.05-arm-linux-gnueabi ./sol.exe

.global main
main:
    push {lr}

cin_symb:
    ldr r0, =stdin
    ldr r0, [r0]
    bl fgetc

    cmp r0, #0
    blt out

    cmp r0, #'0'
    ble cin_symb

    cmp r0, #'9'
    bge cin_symb

    ldr r1, =stdout
    ldr r1, [r1]
    bl fputc
    b cin_symb
out:
    pop {pc}
```


Run: `arm-linux-gnueabi-gcc -marm sol.S -O2 -o sol.exe`



Run: `echo "123 124 125" | qemu-arm -L ~/Downloads/sysroot-glibc-linaro-2.25-2018.05-arm-linux-gnueabi ./sol.exe`


    123124125


```python

```

    [NbConvertApp] Converting notebook adressing.ipynb to markdown
    [NbConvertApp] Writing 8863 bytes to README.md



```python

```

# TMP


```python
%%asm sol.S
%run arm-linux-gnueabi-gcc -marm sol.S -O2 -o sol.exe
%run echo "123 124 125" | qemu-arm -L ~/Downloads/sysroot-glibc-linaro-2.25-2018.05-arm-linux-gnueabi ./sol.exe


      .text
        .global main
main:
        push {lr}

        ldr r0, =.format_scanf
        sub sp, #4
        mov r1, sp
        sub sp, #4
        mov r2, sp

        bl scanf
        ldr r1, [sp]
        ldr r2, [sp, #4]

        add r1, r1, r2

        ldr r0, printf_p
        bl printf

        add sp, sp, #8
        pop {lr}
        bx lr


printf_p:
        .word format_printf

        .data
.format_scanf:
        .ascii "%d%d\0"
format_printf:
        .ascii "%d\0"
```


Run: `arm-linux-gnueabi-gcc -marm sol.S -O2 -o sol.exe`



Run: `echo "123 124 125" | qemu-arm -L ~/Downloads/sysroot-glibc-linaro-2.25-2018.05-arm-linux-gnueabi ./sol.exe`


    247


```python


!jupyter nbconvert adressing.ipynb --to markdown --output README
```

    [NbConvertApp] Converting notebook adressing.ipynb to markdown
    [NbConvertApp] Writing 16574 bytes to README.md



```python

```
