```python
# make magics here. Look at previous notebooks to see readable version
exec('\nget_ipython().run_cell_magic(\'javascript\', \'\', \'// setup cpp code highlighting\\nIPython.CodeCell.options_default.highlight_modes["text/x-c++src"] = {\\\'reg\\\':[/^%%cpp/]} ;\')\n\n# creating magics\nfrom IPython.core.magic import register_cell_magic, register_line_magic\nfrom IPython.display import display, Markdown\n\n@register_cell_magic\ndef save_file(fname, cell):\n    cell = cell if cell[-1] == \'\\n\' else cell + "\\n"\n    cmds = []\n    with open(fname, "w") as f:\n        for line in cell.split("\\n"):\n            if line.startswith("%"):\n                run_prefix = "%run "\n                assert line.startswith(run_prefix)\n                cmds.append(line[len(run_prefix):].strip())\n            else:\n                f.write(line + "\\n")\n    for cmd in cmds:\n        display(Markdown("Run: `%s`" % cmd))\n        get_ipython().system(cmd)\n\n@register_cell_magic\ndef cpp(fname, cell):\n    save_file(fname, cell)\n\n@register_cell_magic\ndef asm(fname, cell):\n    save_file(fname, cell)\n    \n@register_cell_magic\ndef makefile(fname, cell):\n    assert not fname\n    save_file("makefile", cell.replace(" " * 4, "\\t"))\n        \n@register_line_magic\ndef p(line):\n    print("{} = {}".format(line, eval(line)))\n')
```


    <IPython.core.display.Javascript object>


# Assembler x86

* \xd0\x9c\xd0\xb0\xd0\xbb\xd0\xbe \xd1\x80\xd0\xb5\xd0\xb3\xd0\xb8\xd1\x81\xd1\x82\xd1\x80\xd0\xbe\xd0\xb2
* \xd0\x9c\xd0\xbd\xd0\xbe\xd0\xb3\xd0\xbe \xd0\xba\xd0\xbe\xd0\xbc\xd0\xb0\xd0\xbd\xd0\xb4
* \xd0\x9c\xd0\xbd\xd0\xbe\xd0\xb3\xd0\xbe \xd0\xbb\xd0\xb5\xd0\xb3\xd0\xb0\xd1\x81\xd0\xb8
* \xd0\x9c\xd0\xbd\xd0\xbe\xd0\xb3\xd0\xbe \xd1\x81\xd0\xbe\xd0\xb3\xd0\xbb\xd0\xb0\xd1\x88\xd0\xb5\xd0\xbd\xd0\xb8\xd0\xb9 \xd0\xbe \xd0\xb2\xd1\x8b\xd0\xb7\xd0\xbe\xd0\xb2\xd0\xb0\xd1\x85
* \xd0\xa0\xd0\xb0\xd0\xb7\xd0\xbd\xd1\x8b\xd0\xb5 \xd1\x81\xd0\xb8\xd0\xbd\xd1\x82\xd0\xb0\xd0\xba\xd1\x81\xd0\xb8\xd1\x81\xd1\x8b

# Syntaxes
### AT&T


```python
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

DWORD PTR \xe2\x80\x94 \xd1\x8d\xd1\x82\xd0\xbe \xd0\xbf\xd0\xb5\xd1\x80\xd0\xb5\xd0\xbc\xd0\xb5\xd0\xbd\xd0\xbd\xd0\xb0\xd1\x8f \xd1\x82\xd0\xb8\xd0\xbf\xd0\xb0 \xd0\xb4\xd0\xb2\xd0\xbe\xd0\xb9\xd0\xbd\xd0\xbe\xd0\xb3\xd0\xbe \xd1\x81\xd0\xbb\xd0\xbe\xd0\xb2\xd0\xb0. \xd0\xa1\xd0\xbb\xd0\xbe\xd0\xb2\xd0\xbe \xe2\x80\x94 \xd1\x8d\xd1\x82\xd0\xbe 16 \xd0\xb1\xd0\xb8\xd1\x82. \xd0\xa2\xd0\xb5\xd1\x80\xd0\xbc\xd0\xb8\xd0\xbd \xd0\xbf\xd0\xbe\xd0\xbb\xd1\x83\xd1\x87\xd0\xb8\xd0\xbb \xd1\x80\xd0\xb0\xd1\x81\xd0\xbf\xd1\x80\xd0\xbe\xd1\x81\xd1\x82\xd1\x80\xd0\xb0\xd0\xbd\xd0\xb5\xd0\xbd\xd0\xb8\xd0\xb5 \xd0\xb2 \xd1\x8d\xd0\xbf\xd0\xbe\xd1\x85\xd1\x83 16-\xd1\x82\xd0\xb8 \xd0\xb1\xd0\xb8\xd1\x82\xd0\xbd\xd1\x8b\xd1\x85 \xd0\xbf\xd1\x80\xd0\xbe\xd1\x86\xd0\xb5\xd1\x81\xd1\x81\xd0\xbe\xd1\x80\xd0\xbe\xd0\xb2, \xd1\x82\xd0\xbe\xd0\xb3\xd0\xb4\xd0\xb0 \xd0\xb2 \xd1\x80\xd0\xb5\xd0\xb3\xd0\xb8\xd1\x81\xd1\x82\xd1\x80 \xd0\xbf\xd0\xbe\xd0\xbc\xd0\xb5\xd1\x89\xd0\xb0\xd0\xbb\xd0\xbe\xd1\x81\xd1\x8c \xd1\x80\xd0\xbe\xd0\xb2\xd0\xbd\xd0\xbe 16 \xd0\xb1\xd0\xb8\xd1\x82. \xd0\xa2\xd0\xb0\xd0\xba\xd0\xbe\xd0\xb9 \xd0\xbe\xd0\xb1\xd1\x8a\xd0\xb5\xd0\xbc \xd0\xb8\xd0\xbd\xd1\x84\xd0\xbe\xd1\x80\xd0\xbc\xd0\xb0\xd1\x86\xd0\xb8\xd0\xb8 \xd1\x81\xd1\x82\xd0\xb0\xd0\xbb\xd0\xb8 \xd0\xbd\xd0\xb0\xd0\xb7\xd1\x8b\xd0\xb2\xd0\xb0\xd1\x82\xd1\x8c \xd1\x81\xd0\xbb\xd0\xbe\xd0\xb2\xd0\xbe\xd0\xbc (word). \xd0\xa2. \xd0\xb5. \xd0\xb2 \xd0\xbd\xd0\xb0\xd1\x88\xd0\xb5\xd0\xbc \xd1\x81\xd0\xbb\xd1\x83\xd1\x87\xd0\xb0\xd0\xb5 dword (double word) 2*16 = 32 \xd0\xb1\xd0\xb8\xd1\x82\xd0\xb0 = 4 \xd0\xb1\xd0\xb0\xd0\xb9\xd1\x82\xd0\xb0 (\xd0\xbe\xd0\xb1\xd1\x8b\xd1\x87\xd0\xbd\xd1\x8b\xd0\xb9 int). 

https://habr.com/ru/post/344896/


```python
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


# \xd0\x9f\xd0\xb8\xd1\x88\xd0\xb5\xd0\xbc \xd1\x84\xd1\x83\xd0\xbd\xd0\xba\xd1\x86\xd0\xb8\xd1\x8e clamp \xd1\x82\xd1\x80\xd0\xb5\xd0\xbc\xd1\x8f \xd1\x81\xd0\xbf\xd0\xbe\xd1\x81\xd0\xbe\xd0\xb1\xd0\xb0\xd0\xbc\xd0\xb8


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


```python
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


```python
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

# \xd0\x9f\xd0\xbe\xd1\x80\xd0\xb0\xd0\xb1\xd0\xbe\xd1\x82\xd0\xb0\xd0\xb5\xd0\xbc \xd1\x81 \xd0\xbf\xd0\xb0\xd0\xbc\xd1\x8f\xd1\x82\xd1\x8c\xd1\x8e

\xd0\x94\xd0\xb0\xd0\xbd\xd1\x8b n, x. \xd0\x9f\xd0\xbe\xd1\x81\xd1\x87\xd0\xb8\xd1\x82\xd0\xb0\xd0\xb5\xd0\xbc $\sum_{i=0}^{n - 1} (-1)^i \cdot x[i]$


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


```python
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


# \xd0\xa0\xd0\xb0\xd0\xb7\xd0\xb2\xd0\xbb\xd0\xb5\xd0\xba\xd0\xb0\xd1\x82\xd0\xb5\xd0\xbb\xd1\x8c\xd0\xbd\xd0\xbe-\xd0\xbf\xd0\xbe\xd0\xb7\xd0\xbd\xd0\xb0\xd0\xb2\xd0\xb0\xd1\x82\xd0\xb5\xd0\xbb\xd1\x8c\xd0\xbd\xd0\xb0\xd1\x8f \xd1\x87\xd0\xb0\xd1\x81\xd1\x82\xd1\x8c


```python
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



```python
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



```python
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

# TMP


```python
%%cpp tmp.c
%run gcc -fno-PIC -m32 -masm=intel -O3 tmp.c -S -o tmp.S
%run cat tmp.S | grep -v "^\s*\.cfi"

extern int N;
extern int *A;
extern int *B;
extern int *R;

extern void summ() {
    for (int i = 0; i < N; ++i) {
        R[i] = A[i] + B[i];
    }
}
```


Run: `gcc -fno-PIC -m32 -masm=intel -O3 tmp.c -S -o tmp.S`



Run: `cat tmp.S | grep -v "^\s*\.cfi"`


    	.file	"tmp.c"
    	.intel_syntax noprefix
    	.section	.text.unlikely,"ax",@progbits
    .LCOLDB0:
    	.text
    .LHOTB0:
    	.p2align 4,,15
    	.globl	summ
    	.type	summ, @function
    summ:
    .LFB0:
    	mov	eax, DWORD PTR N
    	test	eax, eax
    	jle	.L8
    	push	esi
    	mov	ecx, DWORD PTR B
    	xor	eax, eax
    	push	ebx
    	mov	esi, DWORD PTR R
    	mov	ebx, DWORD PTR A
    	.p2align 4,,10
    	.p2align 3
    .L3:
    	mov	edx, DWORD PTR [ecx+eax*4]
    	add	edx, DWORD PTR [ebx+eax*4]
    	mov	DWORD PTR [esi+eax*4], edx
    	add	eax, 1
    	cmp	DWORD PTR N, eax
    	jg	.L3
    	pop	ebx
    	pop	esi
    .L8:
    	rep ret
    .LFE0:
    	.size	summ, .-summ
    	.section	.text.unlikely
    .LCOLDE0:
    	.text
    .LHOTE0:
    	.ident	"GCC: (Ubuntu 5.4.0-6ubuntu1~16.04.10) 5.4.0 20160609"
    	.section	.note.GNU-stack,"",@progbits



```python
%%asm tmp.S
%run gcc -fno-PIC -m32 -O3 tmp.S -c -o tmp.o

.intel_syntax noprefix
.text
        .globl  merge
        .globl  mergesort
unsign:
        .string "HERE: %d \n"  /* DELETE IT */
merge:
        // stack :: adr, arr, left, mid, right
        push ebp
        push edi
        push esi
        push ebx

        mov esi, 0 /* it1 = 0 */
        mov edi, 0 /* it2 = 0 */
        mov ebp, esp /* ebp = esp */
        add ebp, 16 /* ebp - initial esp */

        mov ecx, DWORD PTR [ebp + 16]
        sub ecx, DWORD PTR [ebp + 8] /* ecx = shift = right - left */
        add ecx, ecx
        add ecx, ecx /* ecx *= 4*/
        sub esp, ecx /* esp -= shift */
cycle1:
        mov eax, DWORD PTR [ebp + 8] /* eax = left */
        mov ebx, DWORD PTR [ebp + 12] /* ebx = mid */
        add eax, esi /* eax += it1 */
        cmp eax, ebx /* left + it1 < mid ??? */
        jge cycle2
        mov eax, DWORD PTR [ebp + 12] /* eax = mid */
        mov ebx, DWORD PTR [ebp + 16] /* ebx = right */
        add eax, edi /* eax += it2*/
        cmp eax, ebx
        jge cycle2
        mov eax, DWORD PTR [ebp + 4] /* eax = arr */
        mov ebx, DWORD PTR [ebp + 8] /* ebx = left */
        add ebx, esi /* ebx += it1 */
        mov ebx, DWORD PTR [eax + 4*ebx] /* ebx = arr[ebx] = arr[ left + it1 ] */
        mov ecx, DWORD PTR [ebp + 12] /* ecx = mid */
        add ecx, edi /* ecx += it2*/
        mov ecx, DWORD PTR [eax + 4*ecx] /* ecx = arr[ecx] = arr[ mid + it2] */


        /*push  ebx
        push    ecx
        push    OFFSET FLAT:unsign
        call    printf /* printf("%u", eax) */
        /*add   esp, 12 /* return sp to the initial value*/




        mov eax, esi
        add eax, edi /* eax = it1 + it2 */
        lea edx, DWORD PTR [esp + 4*eax] /* edx = esp + eax = result + it1+it2 */
        cmp ebx, ecx /* arr[left + id1] < arr[mid + it2]??? */
        jge case2
        /* case 1 here */
        mov DWORD PTR [edx], ebx /* *edx = arr[ left + it1]*/
        add esi, 1 /* ++it1 */
        jmp cycle1

case2: /* arr[left + it1] >= arr[mid + it2] */
        mov DWORD PTR [edx], ecx /* *edx = arr[ mid + it2]*/
        add edi, 1 /* ++it2 */
        jmp cycle1

cycle2:
        mov eax, DWORD PTR [ebp + 8] /* eax = left */
        add eax, esi /* eax += it1 */
        mov ebx, DWORD PTR [ebp + 12] /* ebx = mid */
        cmp eax, ebx /* left + it1 < mid ??? */
        jge cycle3 /* >= -> next cycle */
        mov ebx, DWORD PTR [ebp + 4] /* ebx = arr */
        mov eax, DWORD PTR [ebx + 4*eax] /* eax = arr[eax] = arr[left + it1]*/
        mov ebx, esi
        add ebx, edi /* ebx = it1 + it2*/
        lea ebx, DWORD PTR [esp + 4*ebx] /* ebx = result + it1 + it2]*/
        mov DWORD PTR [ebx], eax /* result[it1+it2] = eax */
        add esi, 1 /* ++it1*/
        jmp cycle2




cycle3:
        mov eax, DWORD PTR [ebp + 12] /* eax = mid */
        add eax, edi /* eax += it2 */
        mov ebx, DWORD PTR [ebp + 16] /* ebx = right */
        cmp eax, ebx /* left + it1 < mid ??? */
        jge fill /* >= -> next cycle */
        mov ebx, DWORD PTR [ebp + 4] /* ebx = arr */
        mov eax, DWORD PTR [ebx + 4*eax] /* eax = arr[eax] = arr[mid + it2]*/
        mov ebx, esi
        add ebx, edi /* ebx = it1 + it2*/
        lea ebx, DWORD PTR [esp + 4*ebx] /* ebx = result + it1 + it2]*/
        mov DWORD PTR [ebx], eax /* result[it1+it2] = eax */
        add edi, 1 /* ++it2*/
        jmp cycle3

fill:

        mov edx, 0 /* i = 0*/
        mov eax, DWORD PTR [ebp + 4] /* eax = arr */
        mov ebx, DWORD PTR [ebp + 8] /* ebx = left */
        lea eax, DWORD PTR [eax + ebx * 4] /* eax += left */
        mov ecx, esi
        add ecx, edi /* ecx = it1 + it2 */
fill_cycle:
        cmp edx, ecx
        jge finish /* i >= it1 + it2 -> finish */
        mov esi, DWORD PTR [esp + 4*edx] /* esi = result[i]*/
        mov DWORD PTR [eax + 4*edx], esi /* arr[edx] = esi*/
        add edx, 1
        jmp fill_cycle



finish:
        mov eax, DWORD PTR [ebp + 16]
        sub eax, DWORD PTR [ebp + 8]
        add eax, eax
        add eax, eax
        add esp, eax
        pop ebx
        pop esi
        pop edi
        pop ebp
        ret


mergesort:
        push ebp
        push edi
        push esi
        push ebx

        mov eax, 0 /* i = 0*/
        mov ebx, DWORD PTR [esp + 20] /* ebx = from */
        mov ecx, DWORD PTR [esp + 24] /* ecx = to */
        sub ecx, ebx /* ecx = to - from */
        mov edx, DWORD PTR [esp + 32] /* edx = out */
        mov esi, DWORD PTR [esp + 28] /* esi = in*/
init_cycle:
        cmp eax, ecx /* i < to - from ?? */
        jge main_sort
        mov edi, ebx
        add edi, eax /* edi = from + i*/
        mov edi, DWORD PTR [esi + 4 * edi] /* edi = arr[from + i]*/
        mov DWORD PTR [edx + 4 * eax], edi /* out[i] = edi*/
        add eax, 1
        jmp init_cycle
main_sort:
        mov eax, 1 /* i = 1*/
        /*ebx - from, ecx = to - from, edx = out ::: from the initial label*/
main_sort_loop:
        cmp eax, ecx /* i <= to - from ? */
        jge sort_finish
        mov esi, 0 /* begin = 0 */
        mov ebp, ecx
        sub ebp, eax /* ebp = to - from - i  where i represents current half of the current length */

inner_sort_loop:
        cmp esi, ebp /* begin < to - from - i ??? */
        jge main_loop_finish
        mov edi, esi
        add edi, eax
        add edi, eax /* edi = esi +2eax = begin + 2*i = right*/
        cmp edi, ecx /* right < to - from ??? */
        jl skip
        /* not skipped part, condition if true */
        mov edi, ecx
skip:
        push eax /* saving changable registers */
        push ecx
        push edx /* finishing saving */

        push edi /* push right */
        add esi, eax /* begin += i*/
        push esi
        sub esi, eax /* begin -= i*/
        push esi
        push edx /* push out*/
        call merge /* merge(out, begin, begin + i, right)*/
        add esp, 16 /* delete input variables according to convention */

        pop edx /* recovery of the changed registers */
        pop ecx
        pop eax

        add esi, eax
        add esi, eax /* begin += 2*i */
        jmp inner_sort_loop

main_loop_finish:
        add eax, eax
        jmp main_sort_loop
sort_finish:
        pop ebx
        pop esi
        pop edi
        pop ebp
        ret
```


Run: `gcc -fno-PIC -m32 -O3 tmp.S -c -o tmp.o`


    tmp.S: Assembler messages:
    tmp.S:14: Error: too many memory references for `mov'
    tmp.S:15: Error: too many memory references for `mov'
    tmp.S:16: Error: too many memory references for `mov'
    tmp.S:17: Error: too many memory references for `add'
    tmp.S:19: Error: too many memory references for `mov'
    tmp.S:20: Error: too many memory references for `sub'
    tmp.S:21: Error: too many memory references for `add'
    tmp.S:22: Error: too many memory references for `add'
    tmp.S:23: Error: too many memory references for `sub'
    tmp.S:25: Error: too many memory references for `mov'
    tmp.S:26: Error: too many memory references for `mov'
    tmp.S:27: Error: too many memory references for `add'
    tmp.S:28: Error: too many memory references for `cmp'
    tmp.S:30: Error: too many memory references for `mov'
    tmp.S:31: Error: too many memory references for `mov'
    tmp.S:32: Error: too many memory references for `add'
    tmp.S:33: Error: too many memory references for `cmp'
    tmp.S:35: Error: too many memory references for `mov'
    tmp.S:36: Error: too many memory references for `mov'
    tmp.S:37: Error: too many memory references for `add'
    tmp.S:38: Error: too many memory references for `mov'
    tmp.S:39: Error: too many memory references for `mov'
    tmp.S:40: Error: too many memory references for `add'
    tmp.S:41: Error: too many memory references for `mov'
    tmp.S:53: Error: too many memory references for `mov'
    tmp.S:54: Error: too many memory references for `add'
    tmp.S:55: Error: too many memory references for `lea'
    tmp.S:56: Error: too many memory references for `cmp'
    tmp.S:59: Error: junk `PTR [edx]' after expression
    tmp.S:59: Error: too many memory references for `mov'
    tmp.S:60: Error: too many memory references for `add'
    tmp.S:64: Error: junk `PTR [edx]' after expression
    tmp.S:64: Error: too many memory references for `mov'
    tmp.S:65: Error: too many memory references for `add'
    tmp.S:69: Error: too many memory references for `mov'
    tmp.S:70: Error: too many memory references for `add'
    tmp.S:71: Error: too many memory references for `mov'
    tmp.S:72: Error: too many memory references for `cmp'
    tmp.S:74: Error: too many memory references for `mov'
    tmp.S:75: Error: too many memory references for `mov'
    tmp.S:76: Error: too many memory references for `mov'
    tmp.S:77: Error: too many memory references for `add'
    tmp.S:78: Error: too many memory references for `lea'
    tmp.S:79: Error: junk `PTR [ebx]' after expression
    tmp.S:79: Error: too many memory references for `mov'
    tmp.S:80: Error: too many memory references for `add'
    tmp.S:87: Error: too many memory references for `mov'
    tmp.S:88: Error: too many memory references for `add'
    tmp.S:89: Error: too many memory references for `mov'
    tmp.S:90: Error: too many memory references for `cmp'
    tmp.S:92: Error: too many memory references for `mov'
    tmp.S:93: Error: too many memory references for `mov'
    tmp.S:94: Error: too many memory references for `mov'
    tmp.S:95: Error: too many memory references for `add'
    tmp.S:96: Error: too many memory references for `lea'
    tmp.S:97: Error: junk `PTR [ebx]' after expression
    tmp.S:97: Error: too many memory references for `mov'
    tmp.S:98: Error: too many memory references for `add'
    tmp.S:103: Error: too many memory references for `mov'
    tmp.S:104: Error: too many memory references for `mov'
    tmp.S:105: Error: too many memory references for `mov'
    tmp.S:106: Error: too many memory references for `lea'
    tmp.S:107: Error: too many memory references for `mov'
    tmp.S:108: Error: too many memory references for `add'
    tmp.S:110: Error: too many memory references for `cmp'
    tmp.S:112: Error: too many memory references for `mov'
    tmp.S:113: Error: junk `PTR [eax+4*edx]' after expression
    tmp.S:113: Error: too many memory references for `mov'
    tmp.S:114: Error: too many memory references for `add'
    tmp.S:120: Error: too many memory references for `mov'
    tmp.S:121: Error: too many memory references for `sub'
    tmp.S:122: Error: too many memory references for `add'
    tmp.S:123: Error: too many memory references for `add'
    tmp.S:124: Error: too many memory references for `add'
    tmp.S:138: Error: too many memory references for `mov'
    tmp.S:139: Error: too many memory references for `mov'
    tmp.S:140: Error: too many memory references for `mov'
    tmp.S:141: Error: too many memory references for `sub'
    tmp.S:142: Error: too many memory references for `mov'
    tmp.S:143: Error: too many memory references for `mov'
    tmp.S:145: Error: too many memory references for `cmp'
    tmp.S:147: Error: too many memory references for `mov'
    tmp.S:148: Error: too many memory references for `add'
    tmp.S:149: Error: too many memory references for `mov'
    tmp.S:150: Error: junk `PTR [edx+4 * eax]' after expression
    tmp.S:150: Error: too many memory references for `mov'
    tmp.S:151: Error: too many memory references for `add'
    tmp.S:154: Error: too many memory references for `mov'
    tmp.S:157: Error: too many memory references for `cmp'
    tmp.S:159: Error: too many memory references for `mov'
    tmp.S:160: Error: too many memory references for `mov'
    tmp.S:161: Error: too many memory references for `sub'
    tmp.S:164: Error: too many memory references for `cmp'
    tmp.S:166: Error: too many memory references for `mov'
    tmp.S:167: Error: too many memory references for `add'
    tmp.S:168: Error: too many memory references for `add'
    tmp.S:169: Error: too many memory references for `cmp'
    tmp.S:172: Error: too many memory references for `mov'
    tmp.S:179: Error: too many memory references for `add'
    tmp.S:181: Error: too many memory references for `sub'
    tmp.S:185: Error: too many memory references for `add'
    tmp.S:191: Error: too many memory references for `add'
    tmp.S:192: Error: too many memory references for `add'
    tmp.S:196: Error: too many memory references for `add'



```python
%%asm 1.S
//golear

.text
.global main

main:
    push    {r4-r8, lr}

    mov r0, #0
    ldr r1, =BUFSIZE
    mov r6, r1
    bl  realloc

    mov r4, r0 // buf
    mov r5, #0 // index
    ldr r7, =stdin // Load input-output variables
    ldr r8, =stdout
loop:
    ldr r0, [r7]
    bl  fgetc
    cmp r0, #-1 // EOF
    beq loop2

    strb r0, [r4, r5] // store read byte on r4 + r5
    add r5, #1
    cmp r5, r6
    blt loop

reallocate:
    add r6, r6
    mov r0, r4
    mov r1, r6
    bl realloc
    mov r4, r0
    b loop

loop2:
    sub r5, r5, #1
    cmp r5, #-1
    beq end
    ldrb r0, [r4, r5]
    ldr r1, [r8]
    bl  fputc
    b   loop2

end:
    mov r0, r4
    bl  free
    mov r0, #0
    pop {r4-r8, pc}

.data
BUFSIZE: .word 800000
```


```python
%%asm 2.S
//golear

.text
.global main

main:
        push    {r4-r8, lr}

        mov r0, #0
        ldr     r1, =BUFSIZE
        mov r6, r1
        bl      realloc

        mov     r4, r0 // buf
        mov r5, #0 // index
        ldr r7, =stdin // Load input-output variables
        ldr     r8, =stdout

loop:
        ldr r0, [r7]
        bl  fgetc
        cmp r0, #-1 // EOF
    beq loop2

    strb r0, [r4, r5] // store read byte on r4 + r5
        add r5, #1
    cmp r5, r6
    blt loop

reallocate:
    add r6, r6
    mov r0, r4
    mov r1, r6
    bl realloc
    mov r4, r0
    b loop

loop2:
    sub r5, r5, #1
    cmp r5, #-1
    beq end
    ldrb r0, [r4, r5]
        ldr r1, [r8]
        bl  fputc
    b   loop2

end:
        mov r0, r4
        bl      free
        mov     r0, #0
        pop     {r4-r8, pc}

.data
BUFSIZE: .word 800000
```


```python
!man diff
```

    DIFF(1)                          User Commands                         DIFF(1)
    
    NNAAMMEE
           diff - compare files line by line
    
    SSYYNNOOPPSSIISS
           ddiiffff [_O_P_T_I_O_N]... _F_I_L_E_S
    
    DDEESSCCRRIIPPTTIIOONN
           Compare FILES line by line.
    
           Mandatory  arguments  to  long  options are mandatory for short options
           too.
    
           ----nnoorrmmaall
                  output a normal diff (the default)
    
           --qq, ----bbrriieeff
                  report only when files differ
    
           --ss, ----rreeppoorrtt--iiddeennttiiccaall--ffiilleess
                  report when two files are the same
    
           --cc, --CC NUM, ----ccoonntteexxtt[=_N_U_M]
                  output NUM (default 3) lines of copied context
    
           --uu, --UU NUM, ----uunniiffiieedd[=_N_U_M]
                  output NUM (default 3) lines of unified context
    
           --ee, ----eedd
                  output an ed script
    
           --nn, ----rrccss
                  output an RCS format diff
    
           --yy, ----ssiiddee--bbyy--ssiiddee
                  output in two columns
    
           --WW, ----wwiiddtthh=_N_U_M
                  output at most NUM (default 130) print columns
    
           ----lleefftt--ccoolluummnn
                  output only the left column of common lines
    
           ----ssuupppprreessss--ccoommmmoonn--lliinneess
                  do not output common lines
    
           --pp, ----sshhooww--cc--ffuunnccttiioonn
                  show which C function each change is in
    
           --FF, ----sshhooww--ffuunnccttiioonn--lliinnee=_R_E
                  show the most recent line matching RE
    
           ----llaabbeell LABEL
                  use LABEL instead of file name (can be repeated)
    
           --tt, ----eexxppaanndd--ttaabbss
                  expand tabs to spaces in output
    
           --TT, ----iinniittiiaall--ttaabb
                  make tabs line up by prepending a tab
    
           ----ttaabbssiizzee=_N_U_M
                  tab stops every NUM (default 8) print columns
    
           ----ssuupppprreessss--bbllaannkk--eemmppttyy
                  suppress space or tab before empty output lines
    
           --ll, ----ppaaggiinnaattee
                  pass output through `pr' to paginate it
    
           --rr, ----rreeccuurrssiivvee
                  recursively compare any subdirectories found
    
           --NN, ----nneeww--ffiillee
                  treat absent files as empty
    
           ----uunniiddiirreeccttiioonnaall--nneeww--ffiillee
                  treat absent first files as empty
    
           ----iiggnnoorree--ffiillee--nnaammee--ccaassee
                  ignore case when comparing file names
    
           ----nnoo--iiggnnoorree--ffiillee--nnaammee--ccaassee
                  consider case when comparing file names
    
           --xx, ----eexxcclluuddee=_P_A_T
                  exclude files that match PAT
    
           --XX, ----eexxcclluuddee--ffrroomm=_F_I_L_E
                  exclude files that match any pattern in FILE
    
           --SS, ----ssttaarrttiinngg--ffiillee=_F_I_L_E
                  start with FILE when comparing directories
    
           ----ffrroomm--ffiillee=_F_I_L_E_1
                  compare FILE1 to all operands; FILE1 can be a directory
    
           ----ttoo--ffiillee=_F_I_L_E_2
                  compare all operands to FILE2; FILE2 can be a directory
    
           --ii, ----iiggnnoorree--ccaassee
                  ignore case differences in file contents
    
           --EE, ----iiggnnoorree--ttaabb--eexxppaannssiioonn
                  ignore changes due to tab expansion
    
           --ZZ, ----iiggnnoorree--ttrraaiilliinngg--ssppaaccee
                  ignore white space at line end
    
           --bb, ----iiggnnoorree--ssppaaccee--cchhaannggee
                  ignore changes in the amount of white space
    
           --ww, ----iiggnnoorree--aallll--ssppaaccee
                  ignore all white space
    
           --BB, ----iiggnnoorree--bbllaannkk--lliinneess
                  ignore changes whose lines are all blank
    
           --II, ----iiggnnoorree--mmaattcchhiinngg--lliinneess=_R_E
                  ignore changes whose lines all match RE
    
           --aa, ----tteexxtt
                  treat all files as text
    
           ----ssttrriipp--ttrraaiilliinngg--ccrr
                  strip trailing carriage return on input
    
           --DD, ----iiffddeeff=_N_A_M_E
                  output merged file with `#ifdef NAME' diffs
    
           ----GGTTYYPPEE--ggrroouupp--ffoorrmmaatt=_G_F_M_T
                  format GTYPE input groups with GFMT
    
           ----lliinnee--ffoorrmmaatt=_L_F_M_T
                  format all input lines with LFMT
    
           ----LLTTYYPPEE--lliinnee--ffoorrmmaatt=_L_F_M_T
                  format LTYPE input lines with LFMT
    
                  These format options provide fine-grained control over the  out\xe2\x80\x90
                  put
    
                  of diff, generalizing --DD/--ifdef.
    
           LTYPE is `old', `new', or `unchanged'.
                  GTYPE is LTYPE or `changed'.
    
                  GFMT (only) may contain:
    
           %<     lines from FILE1
    
           %>     lines from FILE2
    
           %=     lines common to FILE1 and FILE2
    
           %[-][WIDTH][.[PREC]]{doxX}LETTER
                  printf-style spec for LETTER
    
                  LETTERs are as follows for new group, lower case for old group:
    
           F      first line number
    
           L      last line number
    
           N      number of lines = L-F+1
    
           E      F-1
    
           M      L+1
    
           %(A=B?T:E)
                  if A equals B then T else E
    
                  LFMT (only) may contain:
    
           %L     contents of line
    
           %l     contents of line, excluding any trailing newline
    
           %[-][WIDTH][.[PREC]]{doxX}n
                  printf-style spec for input line number
    
                  Both GFMT and LFMT may contain:
    
           %%     %
    
           %c'C'  the single character C
    
           %c'\OOO'
                  the character with octal code OOO
    
           C      the character C (other characters represent themselves)
    
           --dd, ----mmiinniimmaall
                  try hard to find a smaller set of changes
    
           ----hhoorriizzoonn--lliinneess=_N_U_M
                  keep NUM lines of the common prefix and suffix
    
           ----ssppeeeedd--llaarrggee--ffiilleess
                  assume large files and many scattered small changes
    
           ----hheellpp display this help and exit
    
           --vv, ----vveerrssiioonn
                  output version information and exit
    
           FILES  are  `FILE1  FILE2'  or `DIR1 DIR2' or `DIR FILE...' or `FILE...
           DIR'.  If ----ffrroomm--ffiillee or ----ttoo--ffiillee is given, there are no  restrictions
           on  FILE(s).   If a FILE is `-', read standard input.  Exit status is 0
           if inputs are the same, 1 if different, 2 if trouble.
    
    AAUUTTHHOORR
           Written by Paul Eggert, Mike Haertel, David  Hayes,  Richard  Stallman,
           and Len Tower.
    
    RREEPPOORRTTIINNGG BBUUGGSS
           Report bugs to: bug-diffutils@gnu.org
           GNU diffutils home page: <http://www.gnu.org/software/diffutils/>
           General help using GNU software: <http://www.gnu.org/gethelp/>
    
    CCOOPPYYRRIIGGHHTT
           Copyright  \xc2\xa9  2011  Free Software Foundation, Inc.  License GPLv3+: GNU
           GPL version 3 or later <http://gnu.org/licenses/gpl.html>.
           This is free software: you are free  to  change  and  redistribute  it.
           There is NO WARRANTY, to the extent permitted by law.
    
    SSEEEE AALLSSOO
           wdiff(1), cmp(1), diff3(1), sdiff(1), patch(1)
    
           The  full documentation for ddiiffff is maintained as a Texinfo manual.  If
           the iinnffoo and ddiiffff programs are properly installed  at  your  site,  the
           command
    
                  iinnffoo ddiiffff
    
           should give you access to the complete manual.
    
    diffutils 3.3                     March 2013                           DIFF(1)



```python
!diff -b 1.S 2.S

```

    17a18
    > 



```python
!jupyter nbconvert asm_x86.ipynb --to markdown --output README
```

    [NbConvertApp] Converting notebook asm_x86.ipynb to markdown
    [NbConvertApp] Writing 8331 bytes to README.md



```python

```
