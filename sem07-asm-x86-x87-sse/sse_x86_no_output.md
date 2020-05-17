

# SSE

Streaming SIMD extensions

Single instruction multiple data

Сравним sse и x87 на 32-битной и 64-битной архитектуре


```cpp
%%cpp double_mul.c
%run gcc -m32 -mfpmath=387 -masm=intel -O3 double_mul.c -S -o double_mul.S # x87, 32bit
%run cat double_mul.S | grep -v "^\s*\."
    
double mul(double a) { 
    return a * 0;
}

double mul2(double a, double b) { 
    return a * b;
}
```


```cpp
%%cpp double_mul.c
%run gcc -m64 -mfpmath=387 -masm=intel -O3 double_mul.c -S -o double_mul.S # x87, 64bit
%run cat double_mul.S | grep -v "^\s*\."
    
double mul(double a) { 
    return a * 0;
}

double mul2(double a, double b) { 
    return a * b;
}
```


```cpp
%%cpp double_mul.c
%run gcc -m32 -mfpmath=sse -msse4 -masm=intel -O3 double_mul.c -S -o double_mul.S # SSE, 32bit (add -msse4!)
%run cat double_mul.S | grep -v "^\s*\."
    
double mul(double a) { 
    return a * 13;
}

double mul2(double a, double b) { 
    return a * b;
}
```


```cpp
%%cpp double_mul.c
%run gcc -m64 -mfpmath=sse -masm=intel -O3 double_mul.c -S -o double_mul.S # SSE, 64bit
%run cat double_mul.S | grep -v "^\s*\."
    
double mul(double a) { 
    return a * 0;
}

double mul2(double a, double b) { 
    return a * b;
}
```

## Через боль и страдания пишем аналогичный ассемблерный код для SSE 32bit


```cpp
%%cpp check_mul.c
%run gcc -msse4 -g3 -m32 -masm=intel check_mul.c -o check_mul.exe
%run ./check_mul.exe

#include <stdio.h>
#include <assert.h>

double mul(double a);
double mul2(double a, double b);
    
__asm__ (R"(
.text
mul:
    movsd    xmm0, [esp+4]  
    lea      eax, .mconst13
    mulsd    xmm0, QWORD PTR [eax]
    movsd    [esp+4], xmm0
    fld      QWORD PTR [esp+4]
    ret
mul2:
    movsd   xmm0, [esp+12]  
    mulsd   xmm0, QWORD PTR [esp+4]
    movsd   [esp+12], xmm0
    fld     QWORD PTR [esp+12]
    ret
.mconst13:
    .long 0
    .long 1076494336
)");

int main() {
    printf("mul(1.5) = %0.9lf\n", mul(1.5));
    printf("mul2(2.1, 20) = %0.9lf\n", mul2(2.1, 20));
    return 0;
}



```

# Intrinsics


```cpp
%%cpp bitmask.c
%run gcc -m32 -msse4 -O3 bitmask.c -S -o bitmask.S # SSE, 64bit
%run cat bitmask.S | grep -v "^\s*\."
  
    
#include <xmmintrin.h>
    
void bit_and(const int* __restrict__ a, 
             const int* __restrict__ b, 
             int* __restrict__ c) {
    for (int i = 0; i < 4 * 10; ++i) {
        c[i] = a[i] & b[i];
    }
}

void bit_and_2(const int* __restrict__ a, 
               const int* __restrict__ b, 
               int* __restrict__ c) {
    for (int i = 0; i < 10; i += 1) {
        ((__m128*)c)[i] = _mm_and_ps(((__m128*)a)[i], ((__m128*)b)[i]);
    }
}
    
```


```cpp
%%cpp bitmask_test.c
%run gcc -m64 -msse4 -O3 bitmask_test.c bitmask.c -o bitmask_test.exe # SSE, 64bit
%run ./bitmask_test.exe
 
#include <stdio.h>
#include <assert.h>
#include <xmmintrin.h>
    
void bit_and(const int* __restrict__ a, 
             const int* __restrict__ b, 
             int* __restrict__ c);
void bit_and_2(const int* __restrict__ a, 
               const int* __restrict__ b, 
               int* __restrict__ c);
    

int main() {
    char __attribute__((aligned(128))) ac[128 * 10 + 3] = "ahjlvbshrvkbvjknkjsnbjskndsenckjwncskjncvsjckjsncksjncskjdnckjsncjksdncndkcnsdkjcnsdcjksndcjksdncjksdnjkdcnjknckdjcahjlvbshrvkbvjknkjsnbjskndsenckjwncskjncvsjckjsncksjncskjdnckjsncjksdncndkcnsdkjcnsdcjksndcjksdncjksdnjkdcnjknckdjcahjlvbshrvkbvjknkjsnbjskndsenckjwncskjncvsjckjsncksjncskjdnckjsncjksdncndkcnsdkjcnsdcjksndcjksdncjksdnjkdcnjknckdjcahjlvbshrvkbvjknkjsnbjskndsenckjwncskjncvsjckjsncksjncskjdnckjsncjksdncndkcnsdkjcnsdcjksndcjksdncjksdnjkdcnjknckdjcahjlvbshrvkbvjknkjsnbjskndsenckjwncskjncvsjckjsncksjncskjdnckjsncjksdncndkcnsdkjcnsdcjksndcjksdncjksdnjkdcnjknckdjcahjlvbshrvkbvjknkjsnbjskndsenckjwncskjncvsjckjsncksjncskjdnckjsncjksdncndkcnsdkjcnsdcjksndcjksdncjksdnjkdcnjknckdjcahjlvbshrvkbvjknkjsnbjskndsenckjwncskjncvsjckjsncksjncskjdnckjsncjksdncndkcnsdkjcnsdcjksndcjksdncjksdnjkdcnjknckdjcahjlvbshrvkbvjknkjsnbjskndsenckjwncskjncvsjckjsncksjncskjdnckjsncjksdncndkcnsdkjcnsdcjksndcjksdncjksdnjkdcnjknckdjcahjlvbshrvkbvjknkjsnbjskndsenckjwncskjncvsjckjsncksjncskjdnckjsncjksdncndkcnsdkjcnsdcjksndcjksdncjksdnjkdcnjknckdjcahjlvbshrvkbvjknkjsnbjskndsenckjwncskjncvsjckjsncksjncskjdnckjsncjksdncndkcnsdkjcnsdcjksndcjksdncjksdnjkdcnjknckdjc";
    char __attribute__((aligned(128))) bc[128 * 10 + 3] = "ahjlascscsdafbshrvkbvjknkjsnbjskndsenckjwncskjncvsjckjsncksjncskjdnckjsncjksdncndkcnsdkjcnsdcjksndcjksdncjksdnjkdcnjknckdjcahjlvbshrvkbvjknkjsnbjskndsenckjwncskjncvsjckjsncksjncskjdnckjsncjksdncndkcnsdkjcnsdcjksndcjksdncjksdnjkdcnjknckdjcahjlvbshrvkbvjknkjsnbjskndsenckjwncskjncvsjckjsncksjncskjdnckjsncjksdncndkcnsdkjcnsdcjksndcjksdncjksdnjkdcnjknckdjcahjlvbshrvkbvjknkjsnbjskndsenckjwncskjncvsjckjsncksjncskjdnckjsncjksdncndkcnsdkjcnsdcjksndcjksdncjksdnjkdcnjknckdjcahjlvbshrvkbvjknkjsnbjskndsenckjwncskjncvsjckjsncksjncskjdnckjsncjksdncndkcnsdkjcnsdcjksndcjksdncjksdnjkdcnjknckdjcahjlvbshrvkbvjknkjsnbjskndsenckjwncskjncvsjckjsncksjncskjdnckjsncjksdncndkcnsdkjcnsdcjksndcjksdncjksdnjkdcnjknckdjcahjlvbshrvkbvjknkjsnbjskndsenckjwncskjncvsjckjsncksjncskjdnckjsncjksdncndkcnsdkjcnsdcjksndcjksdncjksdnjkdcnjknckdjcahjlvbshrvkbvjknkjsnbjskndsenckjwncskjncvsjckjsncksjncskjdnckjsncjksdncndkcnsdkjcnsdcjksndcjksdncjksdnjkdcnjknckdjcahjlvbshrvkbvjknkjsnbjskndsenckjwncskjncvsjckjsncksjncskjdnckjsncjksdncndkcnsdkjcnsdcjksndcjksdncjksdnjkdcnjknckdjcahjlvbshrvkbvjknkjsnbjskndsenckjwncskjncvsjckjsncksjncskjdnckjsncjksdncndkcnsdkjcnsdcjksndcjksdncjksdnjkdcnjknckdjc";
    char __attribute__((aligned(128))) c1c[128 * 10 + 3];
    char __attribute__((aligned(128))) c2c[128 * 10 + 3];
    bit_and((int*)ac, (int*)bc, (int*)c1c);
    bit_and_2((int*)ac, (int*)bc, (int*)c2c);
    printf("%p %p %p %p\n", ac, bc, c1c, c2c);
    for (int i = 0; i < 4 * 4 * 10; ++i) {
        assert(c1c[i] == c2c[i]);
    }
    return 0;
}
```


```python

```

## Полезные интринсики для векторных операций с float'ами

`_mm_dp_pd`, `_mm_dp_ps`, `_mm_add_epi8`, `_mm_loadu_ps`, `_mm_setzero_ps`, `_mm_mul_ss`, `_mm_add_ps`, `_mm_hadd_ps`, `_mm_cvtss_f32`


```python

```


```python

```
