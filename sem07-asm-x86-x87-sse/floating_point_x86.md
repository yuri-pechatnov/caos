```python
# make magics here. Look at previous notebooks to see readable version
exec('\nget_ipython().run_cell_magic(\'javascript\', \'\', \'// setup cpp code highlighting\\nIPython.CodeCell.options_default.highlight_modes["text/x-c++src"] = {\\\'reg\\\':[/^%%cpp/]} ;\')\n\n# creating magics\nfrom IPython.core.magic import register_cell_magic, register_line_magic\nfrom IPython.display import display, Markdown\n\n@register_cell_magic\ndef save_file(fname, cell):\n    cell = cell if cell[-1] == \'\\n\' else cell + "\\n"\n    cmds = []\n    with open(fname, "w") as f:\n        for line in cell.split("\\n"):\n            if line.startswith("%"):\n                run_prefix = "%run "\n                assert line.startswith(run_prefix)\n                cmds.append(line[len(run_prefix):].strip())\n            else:\n                f.write(line + "\\n")\n    for cmd in cmds:\n        display(Markdown("Run: `%s`" % cmd))\n        get_ipython().system(cmd)\n\n@register_cell_magic\ndef cpp(fname, cell):\n    save_file(fname, cell)\n\n@register_cell_magic\ndef asm(fname, cell):\n    save_file(fname, cell)\n    \n@register_cell_magic\ndef makefile(fname, cell):\n    assert not fname\n    save_file("makefile", cell.replace(" " * 4, "\\t"))\n        \n@register_line_magic\ndef p(line):\n    print("{} = {}".format(line, eval(line)))\n')
```


    <IPython.core.display.Javascript object>


# Floating-point arithmetic


```cpp
%%cpp double_mul.c
%run gcc -m32 -masm=intel -O3 double_mul.c -S -o double_mul.S
%run cat double_mul.S
    
double mul(double a) { 
    return a * 13;
}

double mul2(double a, double b) { 
    return a * b;
}
```


Run: `gcc -m32 -masm=intel -O3 double_mul.c -S -o double_mul.S`



Run: `cat double_mul.S`


    	.file	"double_mul.c"
    	.intel_syntax noprefix
    	.section	.text.unlikely,"ax",@progbits
    .LCOLDB2:
    	.text
    .LHOTB2:
    	.p2align 4,,15
    	.globl	mul
    	.type	mul, @function
    mul:
    .LFB0:
    	.cfi_startproc
    	fld	DWORD PTR .LC0
    	fmul	QWORD PTR [esp+4]
    	ret
    	.cfi_endproc
    .LFE0:
    	.size	mul, .-mul
    	.section	.text.unlikely
    .LCOLDE2:
    	.text
    .LHOTE2:
    	.section	.text.unlikely
    .LCOLDB3:
    	.text
    .LHOTB3:
    	.p2align 4,,15
    	.globl	mul2
    	.type	mul2, @function
    mul2:
    .LFB1:
    	.cfi_startproc
    	fld	QWORD PTR [esp+12]
    	fmul	QWORD PTR [esp+4]
    	ret
    	.cfi_endproc
    .LFE1:
    	.size	mul2, .-mul2
    	.section	.text.unlikely
    .LCOLDE3:
    	.text
    .LHOTE3:
    	.section	.rodata.cst4,"aM",@progbits,4
    	.align 4
    .LC0:
    	.long	1095761920
    	.ident	"GCC: (Ubuntu 5.4.0-6ubuntu1~16.04.10) 5.4.0 20160609"
    	.section	.note.GNU-stack,"",@progbits


В отфильтрованном виде выглядит так

```
	.intel_syntax noprefix
	.text
	.globl	mul
mul:
    fld	DWORD PTR .LC0
	fmul	QWORD PTR [esp+4]
	ret

.globl	mul2
mul2:
	fld	QWORD PTR [esp+12]
	fmul	QWORD PTR [esp+4]
	ret

.align 4
.LC0:
	.long	1095761920
```


```python
%%asm mul.S
.intel_syntax noprefix
    .text
    .globl    mul
mul:
    fld    DWORD PTR .LC0
    fmul    QWORD PTR [esp+4]
    ret

.align 4
.LC0:
    .long    1095761920
```


```cpp
%%cpp mul_test.c
%run gcc -g3 -m32 -masm=intel mul_test.c mul.S -o mul_test.exe
%run ./mul_test.exe

#include <stdio.h>
#include <assert.h>

double mul(double a);

int main() {
    printf("mul(2) = %0.9lf\n", mul(2));
    return 0;
}
```


Run: `gcc -g3 -m32 -masm=intel mul_test.c mul.S -o mul_test.exe`



Run: `./mul_test.exe`


    mul(2) = 26.000000000


# Посчитаем exp(x)

Дополнительные команды:

* `fxch` - поменять местами два элемента на стеке (st(0) и st(i)).
* `fstp` - сохранить из стека в память + снять элемент со стека.


```cpp
%%cpp exp.c
%run gcc -m32 -masm=intel -O3 exp.c -S -o exp.S
%run cat exp.S
    
double my_exp(double x) { 
    double xn = 1.0, fac = 1.0, part = 1.0, result = 1.0, old_result = 0.0;
    for (int i = 2; result != old_result; ++i) {
        old_result = result;
        result += part;
        fac *= i;
        xn *= x;
        part = xn / fac;
    }
    return result;
}
```


Run: `gcc -m32 -masm=intel -O3 exp.c -S -o exp.S`



Run: `cat exp.S`


    	.file	"exp.c"
    	.intel_syntax noprefix
    	.section	.text.unlikely,"ax",@progbits
    .LCOLDB2:
    	.text
    .LHOTB2:
    	.p2align 4,,15
    	.globl	my_exp
    	.type	my_exp, @function
    my_exp:
    .LFB0:
    	.cfi_startproc
    	sub	esp, 12
    	.cfi_def_cfa_offset 16
    	mov	eax, 2
    	fld	QWORD PTR [esp+16]
    	fld1
    	fld	st(0)
    	fld	st(1)
    	fld	st(2)
    	.p2align 4,,10
    	.p2align 3
    .L2:
    	fxch	st(1)
    	mov	DWORD PTR [esp+4], eax
    	add	eax, 1
    	fadd	st, st(2)
    	fild	DWORD PTR [esp+4]
    	fmulp	st(2), st
    	fxch	st(3)
    	fmul	st, st(4)
    	fld	st(0)
    	fdiv	st, st(2)
    	fxch	st(4)
    	fucomi	st, st(3)
    	fstp	st(3)
    	jp	.L3
    	je	.L6
    .L3:
    	fxch	st(3)
    	fxch	st(1)
    	jmp	.L2
    	.p2align 4,,10
    	.p2align 3
    .L6:
    	fstp	st(0)
    	fstp	st(0)
    	fstp	st(1)
    	fstp	st(1)
    	add	esp, 12
    	.cfi_def_cfa_offset 4
    	ret
    	.cfi_endproc
    .LFE0:
    	.size	my_exp, .-my_exp
    	.section	.text.unlikely
    .LCOLDE2:
    	.text
    .LHOTE2:
    	.ident	"GCC: (Ubuntu 5.4.0-6ubuntu1~16.04.10) 5.4.0 20160609"
    	.section	.note.GNU-stack,"",@progbits



```python
%%asm exp2.S
%run gcc -m32 -masm=intel -O3 exp2.S -c -o exp2.o

    .intel_syntax noprefix
    .text
    .globl  my_exp
my_exp:
    sub     esp, 12
    mov     eax, 2
    fld     QWORD PTR [esp+16]
    fld1
    fld1
    fld1
    fld1
    // On stack (0-4): fac=1, part=1, old_result=1, xn=1, x 
.loop:
    mov     DWORD PTR [esp+4], eax
    add     eax, 1   
    fxch    st(1)             // On stack (0-4): part, fac, old_result, xn, x 
    fadd    st(0), st(2)      // On stack (0-4): result=part + old_result, fac, old_result, xn, x 
    fild    DWORD PTR [esp+4] // On stack (0-5): i, result, fac, old_result, xn, x 
    fmulp   st(2), st(0)      // On stack (0-4): result, fac=fac*i, old_result, xn, x
    fxch    st(3)             // On stack (0-4): xn, fac, old_result, result, x
    fmul    st(0), st(4)      // On stack (0-4): xn=xn*x, fac, old_result, result, x
    fld     st(0)             // On stack (0-5): xn, xn, fac, old_result, result, x
    fdiv    st(0), st(2)      // On stack (0-5): npart=xn/fac, xn, fac, old_result, result, x
    fxch    st(4)             // On stack (0-5): result, xn, fac, old_result, npart, x
    fcomi   st(0), st(3)      // On stack (0-5): result, xn, fac, old_result, npart, x (compare result and old_result)
    fstp    st(3)             // On stack (0-4): xn, fac, result, npart, x
    je      .finish           // if result == old_result then go to .finish 
    fxch    st(3)             // On stack (0-4): npart, fac, result, xn, x
    fxch    st(1)             // On stack (0-4): fac, npart, result, xn, x
    jmp     .loop
.finish:
    fxch    st(2)             // On stack (0-4): result, npart, fac, xn, x
    add     esp, 12
    ret

```


Run: `gcc -m32 -masm=intel -O3 exp2.S -c -o exp2.o`



```cpp
%%cpp check_exp.c
%run gcc -g3 -m32 -masm=intel check_exp.c exp.c -o check_exp.exe
%run ./check_exp.exe
%run gcc -g3 -m32 -masm=intel check_exp.c exp2.S -o check_exp2.exe
%run ./check_exp2.exe

#include <stdio.h>
#include <assert.h>

double my_exp(double x);

int main() {
    printf("exp(1) = %0.9lf\n", my_exp(1));
    return 0;
}
```


Run: `gcc -g3 -m32 -masm=intel check_exp.c exp.c -o check_exp.exe`



Run: `./check_exp.exe`


    exp(1) = 2.718281828



Run: `gcc -g3 -m32 -masm=intel check_exp.c exp2.S -o check_exp2.exe`



Run: `./check_exp2.exe`


    exp(1) = 2.718281828



```python
!jupyter nbconvert floating_point_x86.ipynb --to markdown --output floating_point_x86 
```

    [NbConvertApp] Converting notebook floating_point_x86.ipynb to markdown
    [NbConvertApp] Writing 7907 bytes to floating_point_x86.md



```python

```
