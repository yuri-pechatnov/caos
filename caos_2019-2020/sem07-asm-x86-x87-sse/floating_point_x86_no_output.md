

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


```python

```
