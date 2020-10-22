

# Вещественная арифметика на x86 и SSE


<p><a href="https://www.youtube.com/watch?v=obufMgdWPKI&list=PLjzMm8llUm4AmU6i_hPU0NobgA4VsBowc&index=9" target="_blank">
    <h3>Видеозапись семинара</h3>
</a></p>

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


```python

```


```python

```

## Полезные интринсики для векторных операций с float'ами

`_mm_dp_pd`, `_mm_dp_ps`, `_mm_add_epi8`, `_mm_loadu_ps`, `_mm_setzero_ps`, `_mm_mul_ss`, `_mm_add_ps`, `_mm_hadd_ps`, `_mm_cvtss_f32`


```python

```
