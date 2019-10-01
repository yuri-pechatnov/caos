```python
# make magics here. Look at previous notebooks to see readable version
exec('\nget_ipython().run_cell_magic(\'javascript\', \'\', \'// setup cpp code highlighting\\nIPython.CodeCell.options_default.highlight_modes["text/x-c++src"] = {\\\'reg\\\':[/^%%cpp/]} ;\')\n\n# creating magics\nfrom IPython.core.magic import register_cell_magic, register_line_magic\nfrom IPython.display import display, Markdown\n\n@register_cell_magic\ndef save_file(fname, cell):\n    cell = cell if cell[-1] == \'\\n\' else cell + "\\n"\n    cmds = []\n    with open(fname, "w") as f:\n        for line in cell.split("\\n"):\n            if line.startswith("%"):\n                run_prefix = "%run "\n                assert line.startswith(run_prefix)\n                cmds.append(line[len(run_prefix):].strip())\n            else:\n                f.write(line + "\\n")\n    for cmd in cmds:\n        display(Markdown("Run: `%s`" % cmd))\n        get_ipython().system(cmd)\n\n@register_cell_magic\ndef cpp(fname, cell):\n    save_file(fname, cell)\n\n@register_cell_magic\ndef asm(fname, cell):\n    save_file(fname, cell)\n    \n@register_cell_magic\ndef makefile(fname, cell):\n    assert not fname\n    save_file("makefile", cell.replace(" " * 4, "\\t"))\n        \n@register_line_magic\ndef p(line):\n    print("{} = {}".format(line, eval(line)))\n')
```


    <IPython.core.display.Javascript object>


# Адресация памяти в ассемблере

[Ридинг от Яковлева](https://github.com/victor-yacovlev/mipt-diht-caos/tree/master/practice/asm/arm_load_store)

Добавление от меня:

1. `str r0, [r1, #4]!` - то же самое, что и `str r0, [r1, #4]`, но в `r1`, будет сохранено `r1 + #4` после выполнения команды. Другими словами префиксный инкремент на 4.
1. `ldr r0, [r1], #4` - то же самое, что и `ldr r0, [r1]` с последующим `add r1, r1, #4`. Другими словами постфиксный инкремент.


```python
# Add path to compilers to PATH
import os
os.environ["PATH"] = os.environ["PATH"] + ":" + \
    "/home/pechatnov/Downloads/gcc-linaro-7.3.1-2018.05-i686_arm-linux-gnueabi/bin/"
```

# Пример работы с массивом из ассемблера


```python
%%cpp is_sorted.c
%run arm-linux-gnueabi-gcc -marm is_sorted.c -o is_sorted.exe
%run qemu-arm -L ~/Downloads/sysroot-glibc-linaro-2.25-2018.05-arm-linux-gnueabi ./is_sorted.exe

#include <stdio.h>
#include <assert.h>

int is_sorted(int n, int* x);
__asm__ (R"(
.global is_sorted
is_sorted:
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
    int a[] = {__VA_ARGS__}; \
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


```python
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


```python
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
    push {r4, r5} // notice that we increase sp by pushing
    ldr r4, [sp, #8] // get last arg from stack (in fact we use initial value of sp)
    // r0 - s, r1 - c, r2 - i, r3 - s, r4 - c2
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


```python
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



```python

```


```python
!jupyter nbconvert adressing.ipynb --to markdown --output README
```

    [NbConvertApp] Converting notebook adressing.ipynb to markdown
    [NbConvertApp] Writing 6833 bytes to README.md



```python

```
