


# Assembler x86

* Мало регистров
* Много команд
* Много легаси
* Много соглашений о вызовах
* Разные синтаксисы

# Syntaxes
### AT&T


```cpp
%%cpp att_example.c
%run gcc -m32 -masm=att -O3 att_example.c -S -o att_example.S
%run cat att_example.S | grep -v "^\s*\."

#include <stdint.h>
    
int32_t sum(int32_t a, int32_t b) {
    return a + b;
}
```


Run: `gcc -m32 -masm=att -O3 att_example.c -S -o att_example.S`



Run: `cat att_example.S | grep -v "^\s*\."`


    sum:
    	movl	8(%esp), %eax
    	addl	4(%esp), %eax
    	ret


### Intel

DWORD PTR — это переменная типа двойного слова. Слово — это 16 бит. Термин получил распространение в эпоху 16-ти битных процессоров, тогда в регистр помещалось ровно 16 бит. Такой объем информации стали называть словом (word). Т. е. в нашем случае dword (double word) 2*16 = 32 бита = 4 байта (обычный int). 

https://habr.com/ru/post/344896/


```cpp
%%cpp att_example.c
%run gcc -m32 -masm=intel -O3 att_example.c -S -o att_example.S
%run cat att_example.S | grep -v "^\s*\."

#include <stdint.h>
    
int32_t sum(int32_t a, int32_t b) {
    return a + b;
}
```


Run: `gcc -m32 -masm=intel -O3 att_example.c -S -o att_example.S`



Run: `cat att_example.S | grep -v "^\s*\."`


    sum:
    	mov	eax, DWORD PTR [esp+8]
    	add	eax, DWORD PTR [esp+4]
    	ret


# Пишем функцию clamp тремя способами


```python
%%asm clamp_disasm.S
.intel_syntax noprefix
.text
.globl clamp
clamp:
    mov edx, DWORD PTR [esp+4]
    mov eax, DWORD PTR [esp+8]
    cmp edx, eax
    jl .L2
    cmp edx, DWORD PTR [esp+12]
    mov eax, edx
    cmovg eax, DWORD PTR [esp+12]
.L2:
    rep ret
```


```python
%%asm clamp_if.S
.intel_syntax noprefix
.text
.globl clamp
clamp:
    mov edx, DWORD PTR [esp + 4] // X
    mov eax, DWORD PTR [esp + 8] // A
    cmp edx, eax
    jl return_eax // return A if X < A
    mov eax, DWORD PTR [esp + 12] // B
    cmp edx, eax
    jg return_eax // return B if X > B
    mov eax, edx
return_eax:
    ret
```


```python
%%asm clamp_cmov.S
.intel_syntax noprefix
.text
.globl clamp
clamp:
    mov eax, DWORD PTR [esp + 4] // X
    mov edx, DWORD PTR [esp + 8] // A
    cmp eax, edx
    cmovl eax, edx               // if (X < A) X = A 
    mov edx, DWORD PTR [esp + 12] // B
    cmp eax, edx
    cmovg eax, edx               // if (X > B) X = B
    ret
```


```cpp
%%cpp clamp_test.c
// compile and test using all three asm clamp implementations
%run gcc -m32 -masm=intel -O2 clamp.S clamp_test.c -o clamp_test.exe
%run ./clamp_test.exe
%run gcc -m32 -masm=intel -O2 clamp_if.S clamp_test.c -o clamp_if_test.exe
%run ./clamp_if_test.exe
%run gcc -m32 -masm=intel -O2 clamp_cmov.S clamp_test.c -o clamp_cmov_test.exe
%run ./clamp_cmov_test.exe

#include <stdint.h>
#include <stdio.h>
#include <assert.h>
    
int32_t clamp(int32_t a, int32_t b, int32_t c);

int main() {
    assert(clamp(1, 10, 20) == 10);
    assert(clamp(100, 10, 20) == 20);
    assert(clamp(15, 10, 20) == 15);
    fprintf(stderr, "All is OK");
    return 0;
}
```


Run: `gcc -m32 -masm=intel -O2 clamp.S clamp_test.c -o clamp_test.exe`



Run: `./clamp_test.exe`


    All is OK


Run: `gcc -m32 -masm=intel -O2 clamp_if.S clamp_test.c -o clamp_if_test.exe`



Run: `./clamp_if_test.exe`


    All is OK


Run: `gcc -m32 -masm=intel -O2 clamp_cmov.S clamp_test.c -o clamp_cmov_test.exe`



Run: `./clamp_cmov_test.exe`


    All is OK

# Inline ASM
http://asm.sourceforge.net/articles/linasm.html


```cpp
%%cpp clamp_inline_test.c
%run gcc -m32 -masm=intel -O2 clamp_inline_test.c -o clamp_inline_test.exe
%run ./clamp_inline_test.exe

#include <stdint.h>
#include <stdio.h>
#include <assert.h>
    
int32_t clamp(int32_t a, int32_t b, int32_t c);
__asm__(R"(
clamp:
    mov eax, DWORD PTR [esp + 4]
    mov edx, DWORD PTR [esp + 8]
    cmp eax, edx
    cmovl eax, edx
    mov edx, DWORD PTR [esp + 12]
    cmp eax, edx
    cmovg eax, edx
    ret
)");

int main() {
    assert(clamp(1, 10, 20) == 10);
    assert(clamp(100, 10, 20) == 20);
    assert(clamp(15, 10, 20) == 15);
    fprintf(stderr, "All is OK");
    return 0;
}
```


Run: `gcc -m32 -masm=intel -O2 clamp_inline_test.c -o clamp_inline_test.exe`



Run: `./clamp_inline_test.exe`


    All is OK

# Поработаем с памятью

Даны n, x. Посчитаем $\sum_{i=0}^{n - 1} (-1)^i \cdot x[i]$


```python
%%asm my_sum.S
.intel_syntax noprefix
.text
.globl my_sum
my_sum:
    push ebx
    mov eax, 0
    mov edx, DWORD PTR [esp + 8]
    mov ebx, DWORD PTR [esp + 12]
start_loop:
    cmp edx, 0
    jle return_eax
    add eax, DWORD PTR [ebx]
    add ebx, 4
    dec edx
    
    cmp edx, 0
    jle return_eax
    sub eax, DWORD PTR [ebx]
    add ebx, 4
    dec edx
    
    jmp start_loop
return_eax:
    pop ebx
    ret
```


```cpp
%%cpp my_sum_test.c
%run gcc -g3 -m32 -masm=intel my_sum_test.c my_sum.S -o my_sum_test.exe
%run ./my_sum_test.exe

#include <stdint.h>
#include <stdio.h>
#include <assert.h>
    
int32_t my_sum(int32_t n, int32_t* x);

int main() {
    int32_t x[] = {100, 2, 200, 3};
    assert(my_sum(sizeof(x) / sizeof(int32_t), x) == 100 - 2 + 200 - 3);
    int32_t y[] = {100, 2, 200};
    assert(my_sum(sizeof(y) / sizeof(int32_t), y) == 100 - 2 + 200);
    return 0;
}
```


Run: `gcc -g3 -m32 -masm=intel my_sum_test.c my_sum.S -o my_sum_test.exe`



Run: `./my_sum_test.exe`


# Развлекательно-познавательная часть


```cpp
%%cpp mul.c
%run gcc -m32 -masm=intel -O3 mul.c -S -o mul.S
%run cat mul.S | grep -v "^\s*\."

#include <stdint.h>
    
int32_t mul(int32_t a) { 
    return a * 13;
}
```


Run: `gcc -m32 -masm=intel -O3 mul.c -S -o mul.S`



Run: `cat mul.S | grep -v "^\s*\."`


    mul:
    	mov	eax, DWORD PTR [esp+4]
    	lea	edx, [eax+eax*2]
    	lea	eax, [eax+edx*4]
    	ret



```cpp
%%cpp div.c
%run gcc -m32 -masm=intel -O3 div.c -S -o div.S
%run cat div.S | grep -v "^\s*\." | grep -v "^\s*\#"

#include <stdint.h>
    
int32_t div(int32_t a) { 
    return a / 4;
}

uint32_t udiv(uint32_t a) { 
    return a / 2;
}
```


Run: `gcc -m32 -masm=intel -O3 div.c -S -o div.S`



Run: `cat div.S | grep -v "^\s*\." | grep -v "^\s*\#"`


    div:
    	mov	edx, DWORD PTR [esp+4]
    	lea	eax, [edx+3]
    	test	edx, edx
    	cmovns	eax, edx
    	sar	eax, 2
    	ret
    udiv:
    	mov	eax, DWORD PTR [esp+4]
    	shr	eax
    	ret



```cpp
%%cpp simdiv.c
%run gcc -m32 -masm=intel -O3 simdiv.c -o simdiv.exe
%run ./simdiv.exe

#include <stdint.h>
#include <assert.h>
    
int32_t simdiv(int32_t a) { 
    uint32_t eax = ((uint32_t)a >> 31) + a;
    __asm__("sar %0" : "=a"(eax) : "a"(eax));
    return eax;
}

int main() {
    assert(simdiv(1) == 0);
    assert(simdiv(5) == 2);
    assert(simdiv(-1) == 0);
    assert(simdiv(-5) == -2);
}
```


Run: `gcc -m32 -masm=intel -O3 simdiv.c -o simdiv.exe`



Run: `./simdiv.exe`



```python

```

# Inline ASM
http://asm.sourceforge.net/articles/linasm.html


```python

```


```python

```
