


# Вещественная арифметика на x86 и SSE

<table width=100%> <tr>
    <th width=20%> <b>Видеозапись семинара &rarr; </b> </th>
    <th>
    <a href="https://www.youtube.com/watch?v=i_eeouEiXnI&list=PLjzMm8llUm4AmU6i_hPU0NobgA4VsBowc&index=8">
        <img src="video.jpg" width="320"  height="160" align="left" alt="Видео с семинара"> 
    </a>
    </th>
    <th> </th>
 </table>


[Ридинг Яковлева](https://github.com/victor-yacovlev/mipt-diht-caos/tree/master/practice/asm/x86_fpmath) 


Сегодня в программе:
* <a href="#x87" style="color:#856024"> Вещественная арифметика на сопроцессоре x87 </a>
* <a href="#sse" style="color:#856024"> SSE </a>
    * <a href="#fp_sse" style="color:#856024"> Вещественная арифметика на SSE </a>
    * <a href="#int_sse" style="color:#856024"> Векторные операции на SSE </a>
    



```python
%%save_file asm_filter_useless
%run chmod +x asm_filter_useless
#!/bin/bash
grep -v "^\s*\." | grep -v "^[0-9]"
```


Run: `chmod +x asm_filter_useless`


# <a name="x87"></a> Вещественная арифметика на сопроцессоре x87

(На самом деле нет, сопроцессор для вещественных операций уже не используется, но инструкции остались с тех времен, когда использовался)


```cpp
%%cpp double_mul.c
%run gcc -m32 -masm=intel -Os double_mul.c -S -o double_mul.S
%run cat double_mul.S
    
double mul(double a) { 
    return a * 13;
}

double mul2(double a, double b) { 
    return a * b;
}
```


Run: `gcc -m32 -masm=intel -Os double_mul.c -S -o double_mul.S`



Run: `cat double_mul.S`


    	.file	"double_mul.c"
    	.intel_syntax noprefix
    	.text
    	.globl	mul
    	.type	mul, @function
    mul:
    .LFB0:
    	.cfi_startproc
    	endbr32
    	call	__x86.get_pc_thunk.ax
    	add	eax, OFFSET FLAT:_GLOBAL_OFFSET_TABLE_
    	push	ebp
    	.cfi_def_cfa_offset 8
    	.cfi_offset 5, -8
    	mov	ebp, esp
    	.cfi_def_cfa_register 5
    	fld	DWORD PTR .LC0@GOTOFF[eax]
    	fmul	QWORD PTR 8[ebp]
    	pop	ebp
    	.cfi_restore 5
    	.cfi_def_cfa 4, 4
    	ret
    	.cfi_endproc
    .LFE0:
    	.size	mul, .-mul
    	.globl	mul2
    	.type	mul2, @function
    mul2:
    .LFB1:
    	.cfi_startproc
    	endbr32
    	push	ebp
    	.cfi_def_cfa_offset 8
    	.cfi_offset 5, -8
    	mov	ebp, esp
    	.cfi_def_cfa_register 5
    	fld	QWORD PTR 8[ebp]
    	fmul	QWORD PTR 16[ebp]
    	pop	ebp
    	.cfi_restore 5
    	.cfi_def_cfa 4, 4
    	ret
    	.cfi_endproc
    .LFE1:
    	.size	mul2, .-mul2
    	.section	.rodata.cst4,"aM",@progbits,4
    	.align 4
    .LC0:
    	.long	1095761920
    	.section	.text.__x86.get_pc_thunk.ax,"axG",@progbits,__x86.get_pc_thunk.ax,comdat
    	.globl	__x86.get_pc_thunk.ax
    	.hidden	__x86.get_pc_thunk.ax
    	.type	__x86.get_pc_thunk.ax, @function
    __x86.get_pc_thunk.ax:
    .LFB2:
    	.cfi_startproc
    	mov	eax, DWORD PTR [esp]
    	ret
    	.cfi_endproc
    .LFE2:
    	.ident	"GCC: (Ubuntu 9.3.0-17ubuntu1~20.04) 9.3.0"
    	.section	.note.GNU-stack,"",@progbits
    	.section	.note.gnu.property,"a"
    	.align 4
    	.long	 1f - 0f
    	.long	 4f - 1f
    	.long	 5
    0:
    	.string	 "GNU"
    1:
    	.align 4
    	.long	 0xc0000002
    	.long	 3f - 2f
    2:
    	.long	 0x3
    3:
    	.align 4
    4:


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


### Посчитаем exp(x)

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
    	.text
    	.p2align 4
    	.globl	my_exp
    	.type	my_exp, @function
    my_exp:
    .LFB0:
    	.cfi_startproc
    	endbr32
    	sub	esp, 12
    	.cfi_def_cfa_offset 16
    	mov	eax, 2
    	fld	QWORD PTR 16[esp]
    	fld1
    	fld	st(0)
    	fld	st(1)
    	fld	st(2)
    	jmp	.L3
    .L6:
    	fxch	st(3)
    	jmp	.L3
    	.p2align 4,,10
    	.p2align 3
    .L7:
    	fxch	st(3)
    .L3:
    	fld	st(2)
    	mov	DWORD PTR 4[esp], eax
    	add	eax, 1
    	faddp	st(2), st
    	fild	DWORD PTR 4[esp]
    	fmulp	st(1), st
    	fxch	st(3)
    	fmul	st, st(4)
    	fld	st(0)
    	fdiv	st, st(4)
    	fxch	st(2)
    	fucomi	st, st(3)
    	fstp	st(3)
    	jp	.L6
    	jne	.L7
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
    	.ident	"GCC: (Ubuntu 9.3.0-17ubuntu1~20.04) 9.3.0"
    	.section	.note.GNU-stack,"",@progbits
    	.section	.note.gnu.property,"a"
    	.align 4
    	.long	 1f - 0f
    	.long	 4f - 1f
    	.long	 5
    0:
    	.string	 "GNU"
    1:
    	.align 4
    	.long	 0xc0000002
    	.long	 3f - 2f
    2:
    	.long	 0x3
    3:
    	.align 4
    4:



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

```

# <a name="sse"></a> SSE


[MMX](https://ru.wikipedia.org/wiki/MMX) (1997) (Multimedia Extensions — мультимедийные расширения) 

64-битные регистры mm0..mm7 (устарело)

[SSE](https://ru.wikipedia.org/wiki/SSE) (1999) (Streaming SIMD Extensions)

128-битные регистры xmm0..xmm7 (связи с регистрами MMX нет вроде бы) (количество может быть 16 и 32 на новых процессорах)

[AVX](https://ru.wikipedia.org/wiki/AVX) (2008) (Advanced Vector Extensions)

256-битные регистры ymm0 — ymm15 (регистры SSE становятся младшими половинками регистров AVX) (количество может быть 32 на новых процессорах)

[AVX-512](https://en.wikipedia.org/wiki/AVX-512) (2013) (Advanced Vector Extensions 512 bits)

512-битные регистры zmm0-zmm31 (регистры AVX становятся младшими половинками регистров AVX-512)


Регистры поддерживают множество различных операций, по разному интерпретирующих содержимое регистров: регистр SSE может быть парой double, 4 float'ами, 8 short'ами и т. д. Над ними можно производить разные операции

# <a name="fp_sse"></a> Вещественная арифметика на SSE


```cpp
%%cpp example.c
%run gcc -m64 -masm=intel -O3 example.c -S -o example.S # -mavx
%run cat example.S | ./asm_filter_useless

#include <stdio.h>

double add(double a, double b) {
    return a + b;
}

double mult(double a, double b) {
    return a * b;
}

int cmp(double a) {
    return a > 0 ? 42 : 0;
}

double max(double a, double b) {
    return a > b ? a : b;
}

double muldi(double a, int b) {
    return a * b;
}

```


Run: `gcc -m64 -masm=intel -O3 example.c -S -o example.S # -mavx`



Run: `cat example.S | ./asm_filter_useless`


    add:
    	endbr64
    	addsd	xmm0, xmm1
    	ret
    mult:
    	endbr64
    	mulsd	xmm0, xmm1
    	ret
    cmp:
    	endbr64
    	comisd	xmm0, QWORD PTR .LC0[rip]
    	mov	edx, 0
    	mov	eax, 42
    	cmovbe	eax, edx
    	ret
    max:
    	endbr64
    	maxsd	xmm0, xmm1
    	ret
    muldi:
    	endbr64
    	movapd	xmm1, xmm0
    	pxor	xmm0, xmm0
    	cvtsi2sd	xmm0, edi
    	mulsd	xmm0, xmm1
    	ret



```python

```


```python

```

# <a name="int_sse"></a> Векторные операции на SSE


```cpp
%%cpp bitmask.c
%run gcc -m64 -masm=intel -msse4 -O3 bitmask.c -S -o bitmask.S # SSE, 64bit
%run cat bitmask.S | grep -v "^\s*\."

#include <xmmintrin.h>
    
#define N 1
   
    
void bit_and_0(const int* a, const int* b, int* c) {
    for (int i = 0; i < 4 * N; ++i) {
        c[i] = a[i] & b[i];
    }
}
    
void bit_and_1(const int* restrict a, const int* restrict b, int* restrict c) {
    for (int i = 0; i < 4 * N; ++i) {
        c[i] = a[i] & b[i];
    }
}

void bit_and_2(const int* restrict a_, const int* restrict b_, int* restrict c_) {
    const int* a = __builtin_assume_aligned(a_, 16);
    const int* b = __builtin_assume_aligned(b_, 16);
    int* c = __builtin_assume_aligned(c_, 16);
    for (int i = 0; i < 4 * N; ++i) {
        c[i] = a[i] & b[i];
    }
}


void bit_and_intr(const int* restrict a, const int* restrict b, int* restrict c) {
    for (int i = 0; i < N; i += 1) {
        ((__m128i*)c)[i] = _mm_and_si128(((__m128i*)a)[i], ((__m128i*)b)[i]);
    }
}

```


Run: `gcc -m64 -masm=intel -msse4 -O3 bitmask.c -S -o bitmask.S # SSE, 64bit`



Run: `cat bitmask.S | grep -v "^\s*\."`


    bit_and_0:
    	endbr64
    	lea	rax, 15[rdi]
    	sub	rax, rdx
    	cmp	rax, 30
    	jbe	.L2
    	lea	rax, 15[rsi]
    	sub	rax, rdx
    	cmp	rax, 30
    	jbe	.L2
    	movdqu	xmm0, XMMWORD PTR [rdi]
    	movdqu	xmm1, XMMWORD PTR [rsi]
    	pand	xmm0, xmm1
    	movups	XMMWORD PTR [rdx], xmm0
    	ret
    	mov	eax, DWORD PTR [rdi]
    	and	eax, DWORD PTR [rsi]
    	mov	DWORD PTR [rdx], eax
    	mov	eax, DWORD PTR 4[rdi]
    	and	eax, DWORD PTR 4[rsi]
    	mov	DWORD PTR 4[rdx], eax
    	mov	eax, DWORD PTR 8[rdi]
    	and	eax, DWORD PTR 8[rsi]
    	mov	DWORD PTR 8[rdx], eax
    	mov	eax, DWORD PTR 12[rdi]
    	and	eax, DWORD PTR 12[rsi]
    	mov	DWORD PTR 12[rdx], eax
    	ret
    bit_and_1:
    	endbr64
    	movdqu	xmm0, XMMWORD PTR [rsi]
    	movdqu	xmm1, XMMWORD PTR [rdi]
    	pand	xmm0, xmm1
    	movups	XMMWORD PTR [rdx], xmm0
    	ret
    bit_and_2:
    	endbr64
    	movdqa	xmm0, XMMWORD PTR [rsi]
    	pand	xmm0, XMMWORD PTR [rdi]
    	movaps	XMMWORD PTR [rdx], xmm0
    	ret
    bit_and_intr:
    	endbr64
    	movdqa	xmm0, XMMWORD PTR [rdi]
    	pand	xmm0, XMMWORD PTR [rsi]
    	movaps	XMMWORD PTR [rdx], xmm0
    	ret
    0:
    1:
    2:
    3:
    4:


Про movdqa и movaps: https://stackoverflow.com/questions/6678073/difference-between-movdqa-and-movaps-x86-instructions

TLDR: одно и то же


```cpp
%%cpp bitmask_test.c
%run gcc -m64 -masm=intel -msse4 -O3 bitmask_test.c bitmask.c -o bitmask_test.exe # SSE, 64bit
%run ./bitmask_test.exe
 
#include <stdio.h>
#include <assert.h>
#include <xmmintrin.h>
    
typedef void (*and_func_t)(const int* a_, const int* b_, int* c_);
    
void bit_and_0(const int* a, const int* b, int* c);   
void bit_and_1(const int* restrict a, const int* restrict b, int* restrict c);
void bit_and_2(const int* restrict a_, const int* restrict b_, int* restrict c_);
void bit_and_intr(const int* restrict a, const int* restrict b, int* restrict c);

void bit_and_asm(const int* restrict a, const int* restrict b, int* restrict c);
__asm__(R"(
bit_and_asm:
    movaps xmm0, XMMWORD PTR [rsi]
    pand xmm0, XMMWORD PTR [rdi]
    movaps XMMWORD PTR [rdx], xmm0
    ret
)");

int main() {
    char __attribute__((aligned(16))) ac[16] = "ahjlvbshrvkbv";
    char __attribute__((aligned(16))) bc[16] = "ahjlascscsdaf";
    and_func_t funcs[] = {bit_and_0, bit_and_1, bit_and_2, bit_and_intr, bit_and_asm};
    char __attribute__((aligned(16))) cc[sizeof(funcs) / sizeof(funcs[0])][16];
    int M = sizeof(funcs) / sizeof(funcs[0]);
    for (int i = 0; i < M; ++i) {
        funcs[i]((int*)ac, (int*)bc, (int*)cc[i]);
    }
    printf("%p %p %p %p\n", ac, bc, cc[0], cc[1]);
    for (int j = 0; j + 1 < M; ++j) {
        for (int i = 0; i < 16; ++i) {
            assert(cc[j][i] == cc[j + 1][i]);
        }
    }
    printf("OK\n");
    return 0;
}
```


Run: `gcc -m64 -masm=intel -msse4 -O3 bitmask_test.c bitmask.c -o bitmask_test.exe # SSE, 64bit`



Run: `./bitmask_test.exe`


    0x7fff446c6810 0x7fff446c6820 0x7fff446c67c0 0x7fff446c67d0
    OK



```python

```


```cpp
%%cpp example.c
%run gcc -m64 -masm=intel -O3 example.c -S -o example.S #  -mavx
%run cat example.S | ./asm_filter_useless

#include <stdio.h>
#include <xmmintrin.h>

void add_mem_double(double* restrict a_, double* restrict b_) {
    double *a = __builtin_assume_aligned(a_, 16);
    double *b = __builtin_assume_aligned(b_, 16);
    for (int i = 0; i < 2; ++i)
        a[i] += b[i];
}

void add_mem_double_intr(double* a, double* b) {
    *(__m128d*)a = _mm_add_pd(*(__m128d*)a, *(__m128d*)b);
}

void add_mem_float(float* restrict a_, float* restrict b_) {
    float *a = __builtin_assume_aligned(a_, 32);
    float *b = __builtin_assume_aligned(b_, 32);
    for (int i = 0; i < 8; ++i)
        a[i] += b[i];
}
```


Run: `gcc -m64 -masm=intel -O3 example.c -S -o example.S #  -mavx`



Run: `cat example.S | ./asm_filter_useless`


    add_mem_double:
    	endbr64
    	movapd	xmm0, XMMWORD PTR [rdi]
    	addpd	xmm0, XMMWORD PTR [rsi]
    	movaps	XMMWORD PTR [rdi], xmm0
    	ret
    add_mem_double_intr:
    	endbr64
    	movapd	xmm0, XMMWORD PTR [rdi]
    	addpd	xmm0, XMMWORD PTR [rsi]
    	movaps	XMMWORD PTR [rdi], xmm0
    	ret
    add_mem_float:
    	endbr64
    	movaps	xmm0, XMMWORD PTR [rdi]
    	addps	xmm0, XMMWORD PTR [rsi]
    	movaps	XMMWORD PTR [rdi], xmm0
    	movaps	xmm0, XMMWORD PTR 16[rdi]
    	addps	xmm0, XMMWORD PTR 16[rsi]
    	movaps	XMMWORD PTR 16[rdi], xmm0
    	ret



```python

```


```python

```

## Полезные интринсики для векторных операций с float'ами

`_mm_dp_pd`, `_mm_dp_ps`, `_mm_add_epi8`, `_mm_loadu_ps`, `_mm_setzero_ps`, `_mm_mul_ss`, `_mm_add_ps`, `_mm_hadd_ps`, `_mm_cvtss_f32`


```python

```
