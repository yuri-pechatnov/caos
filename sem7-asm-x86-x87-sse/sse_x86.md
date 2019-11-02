```python
# make magics here. Look at previous notebooks to see readable version
exec('\nget_ipython().run_cell_magic(\'javascript\', \'\', \'// setup cpp code highlighting\\nIPython.CodeCell.options_default.highlight_modes["text/x-c++src"] = {\\\'reg\\\':[/^%%cpp/]} ;\')\n\n# creating magics\nfrom IPython.core.magic import register_cell_magic, register_line_magic\nfrom IPython.display import display, Markdown\n\n@register_cell_magic\ndef save_file(fname, cell):\n    cell = cell if cell[-1] == \'\\n\' else cell + "\\n"\n    cmds = []\n    with open(fname, "w") as f:\n        for line in cell.split("\\n"):\n            if line.startswith("%"):\n                run_prefix = "%run "\n                assert line.startswith(run_prefix)\n                cmds.append(line[len(run_prefix):].strip())\n            else:\n                f.write(line + "\\n")\n    for cmd in cmds:\n        display(Markdown("Run: `%s`" % cmd))\n        get_ipython().system(cmd)\n\n@register_cell_magic\ndef cpp(fname, cell):\n    save_file(fname, cell)\n\n@register_cell_magic\ndef asm(fname, cell):\n    save_file(fname, cell)\n    \n@register_cell_magic\ndef makefile(fname, cell):\n    assert not fname\n    save_file("makefile", cell.replace(" " * 4, "\\t"))\n        \n@register_line_magic\ndef p(line):\n    print("{} = {}".format(line, eval(line)))\n')
```


    <IPython.core.display.Javascript object>


# SSE

Streaming SIMD extensions

Single instruction multiple data

Сравним sse и x87 на 32-битной и 64-битной архитектуре


```python
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


Run: `gcc -m32 -mfpmath=387 -masm=intel -O3 double_mul.c -S -o double_mul.S # SSE, 32bit`



Run: `cat double_mul.S | grep -v "^\s*\."`


    mul:
    	fldz
    	fmul	QWORD PTR [esp+4]
    	ret
    mul2:
    	fld	QWORD PTR [esp+12]
    	fmul	QWORD PTR [esp+4]
    	ret



```python
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


Run: `gcc -m64 -mfpmath=387 -masm=intel -O3 double_mul.c -S -o double_mul.S # x87, 64bit`



Run: `cat double_mul.S | grep -v "^\s*\."`


    mul:
    	movsd	QWORD PTR [rsp-8], xmm0
    	fld	QWORD PTR [rsp-8]
    	fmul	DWORD PTR .LC0[rip]
    	fstp	QWORD PTR [rsp-8]
    	movsd	xmm0, QWORD PTR [rsp-8]
    	ret
    mul2:
    	movsd	QWORD PTR [rsp-8], xmm0
    	fld	QWORD PTR [rsp-8]
    	movsd	QWORD PTR [rsp-8], xmm1
    	fld	QWORD PTR [rsp-8]
    	fmulp	st(1), st
    	fstp	QWORD PTR [rsp-8]
    	movsd	xmm0, QWORD PTR [rsp-8]
    	ret



```python
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


Run: `gcc -m32 -mfpmath=sse -msse4 -masm=intel -O3 double_mul.c -S -o double_mul.S # SSE, 32bit (add -msse4!)`



Run: `cat double_mul.S | grep -v "^\s*\."`


    mul:
    	sub	esp, 12
    	movsd	xmm0, QWORD PTR .LC0
    	mulsd	xmm0, QWORD PTR [esp+16]
    	movsd	QWORD PTR [esp], xmm0
    	fld	QWORD PTR [esp]
    	add	esp, 12
    	ret
    mul2:
    	sub	esp, 12
    	movsd	xmm0, QWORD PTR [esp+24]
    	mulsd	xmm0, QWORD PTR [esp+16]
    	movsd	QWORD PTR [esp], xmm0
    	fld	QWORD PTR [esp]
    	add	esp, 12
    	ret



```python
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


Run: `gcc -m64 -mfpmath=sse -masm=intel -O3 double_mul.c -S -o double_mul.S # SSE, 64bit`



Run: `cat double_mul.S | grep -v "^\s*\."`


    mul:
    	mulsd	xmm0, QWORD PTR .LC0[rip]
    	ret
    mul2:
    	mulsd	xmm0, xmm1
    	ret


## Через боль и страдания пишем аналогичный ассемблерный код для SSE 32bit


```python
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


Run: `gcc -msse4 -g3 -m32 -masm=intel check_mul.c -o check_mul.exe`



Run: `./check_mul.exe`


    mul(1.5) = 19.500000000
    mul2(2.1, 20) = 42.000000000


# Intrinsics


```python
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


Run: `gcc -m32 -msse4 -O3 bitmask.c -S -o bitmask.S # SSE, 64bit`



Run: `cat bitmask.S | grep -v "^\s*\."`


    bit_and:
    	pushl	%ebp
    	pushl	%edi
    	pushl	%esi
    	pushl	%ebx
    	subl	$28, %esp
    	movl	48(%esp), %edx
    	movl	52(%esp), %edi
    	movl	56(%esp), %esi
    	movl	%edx, %eax
    	andl	$15, %eax
    	shrl	$2, %eax
    	negl	%eax
    	andl	$3, %eax
    	je	.L7
    	movl	(%edx), %ecx
    	andl	(%edi), %ecx
    	cmpl	$1, %eax
    	movl	%ecx, (%esi)
    	je	.L8
    	movl	4(%edx), %ecx
    	andl	4(%edi), %ecx
    	cmpl	$3, %eax
    	movl	%ecx, 4(%esi)
    	jne	.L9
    	movl	8(%edi), %ecx
    	andl	8(%edx), %ecx
    	movl	$37, 4(%esp)
    	movl	$3, (%esp)
    	movl	%ecx, 8(%esi)
    	movl	$40, %ecx
    	movl	$36, %ebp
    	movl	$9, 12(%esp)
    	subl	%eax, %ecx
    	movl	%ecx, 8(%esp)
    	sall	$2, %eax
    	leal	(%edi,%eax), %ebx
    	leal	(%edx,%eax), %ecx
    	addl	%esi, %eax
    	cmpl	$10, 12(%esp)
    	movdqu	(%ebx), %xmm0
    	pand	(%ecx), %xmm0
    	movups	%xmm0, (%eax)
    	movdqu	16(%ebx), %xmm0
    	pand	16(%ecx), %xmm0
    	movups	%xmm0, 16(%eax)
    	movdqu	32(%ebx), %xmm0
    	pand	32(%ecx), %xmm0
    	movups	%xmm0, 32(%eax)
    	movdqu	48(%ebx), %xmm0
    	pand	48(%ecx), %xmm0
    	movups	%xmm0, 48(%eax)
    	movdqu	64(%ebx), %xmm0
    	pand	64(%ecx), %xmm0
    	movups	%xmm0, 64(%eax)
    	movdqu	80(%ebx), %xmm0
    	pand	80(%ecx), %xmm0
    	movups	%xmm0, 80(%eax)
    	movdqu	96(%ebx), %xmm0
    	pand	96(%ecx), %xmm0
    	movups	%xmm0, 96(%eax)
    	movdqu	112(%ebx), %xmm0
    	pand	112(%ecx), %xmm0
    	movups	%xmm0, 112(%eax)
    	movdqu	128(%ebx), %xmm0
    	pand	128(%ecx), %xmm0
    	movups	%xmm0, 128(%eax)
    	jne	.L4
    	movdqu	144(%ebx), %xmm0
    	pand	144(%ecx), %xmm0
    	movups	%xmm0, 144(%eax)
    	movl	(%esp), %eax
    	movl	4(%esp), %ecx
    	addl	%ebp, %eax
    	subl	%ebp, %ecx
    	cmpl	8(%esp), %ebp
    	je	.L1
    	movl	(%edi,%eax,4), %ebp
    	andl	(%edx,%eax,4), %ebp
    	cmpl	$1, %ecx
    	leal	0(,%eax,4), %ebx
    	movl	%ebp, (%esi,%eax,4)
    	je	.L1
    	movl	4(%edx,%ebx), %eax
    	andl	4(%edi,%ebx), %eax
    	cmpl	$2, %ecx
    	movl	%eax, 4(%esi,%ebx)
    	je	.L1
    	movl	8(%edx,%ebx), %eax
    	andl	8(%edi,%ebx), %eax
    	movl	%eax, 8(%esi,%ebx)
    	addl	$28, %esp
    	popl	%ebx
    	popl	%esi
    	popl	%edi
    	popl	%ebp
    	ret
    	movl	$40, %ebp
    	movl	$10, 12(%esp)
    	movl	$40, 8(%esp)
    	movl	$40, 4(%esp)
    	movl	$0, (%esp)
    	jmp	.L2
    	movl	$38, 4(%esp)
    	movl	$2, (%esp)
    	jmp	.L3
    	movl	$39, 4(%esp)
    	movl	$1, (%esp)
    	jmp	.L3
    bit_and_2:
    	movl	4(%esp), %ecx
    	movl	8(%esp), %edx
    	movl	12(%esp), %eax
    	movaps	(%ecx), %xmm0
    	andps	(%edx), %xmm0
    	movaps	%xmm0, (%eax)
    	movaps	16(%ecx), %xmm0
    	andps	16(%edx), %xmm0
    	movaps	%xmm0, 16(%eax)
    	movaps	32(%ecx), %xmm0
    	andps	32(%edx), %xmm0
    	movaps	%xmm0, 32(%eax)
    	movaps	48(%ecx), %xmm0
    	andps	48(%edx), %xmm0
    	movaps	%xmm0, 48(%eax)
    	movaps	64(%ecx), %xmm0
    	andps	64(%edx), %xmm0
    	movaps	%xmm0, 64(%eax)
    	movaps	80(%ecx), %xmm0
    	andps	80(%edx), %xmm0
    	movaps	%xmm0, 80(%eax)
    	movaps	96(%ecx), %xmm0
    	andps	96(%edx), %xmm0
    	movaps	%xmm0, 96(%eax)
    	movaps	112(%ecx), %xmm0
    	andps	112(%edx), %xmm0
    	movaps	%xmm0, 112(%eax)
    	movaps	128(%ecx), %xmm0
    	andps	128(%edx), %xmm0
    	movaps	%xmm0, 128(%eax)
    	movaps	144(%ecx), %xmm0
    	andps	144(%edx), %xmm0
    	movaps	%xmm0, 144(%eax)
    	ret



```python
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


Run: `gcc -m64 -msse4 -O3 bitmask_test.c bitmask.c -o bitmask_test.exe # SSE, 64bit`



Run: `./bitmask_test.exe`


    0x7ffc31c9a580 0x7ffc31c9ab00 0x7ffc31c9b080 0x7ffc31c9b600



```python

```

## Полезные интринсики для векторных операций с float'ами

`_mm_dp_pd`, `_mm_dp_ps`, `_mm_add_epi8`, `_mm_loadu_ps`, `_mm_setzero_ps`, `_mm_mul_ss`, `_mm_add_ps`, `_mm_hadd_ps`, `_mm_cvtss_f32`


```python

```


```python
!jupyter nbconvert sse_x86.ipynb --to markdown --output sse_x86 
```

    [NbConvertApp] Converting notebook sse_x86.ipynb to markdown
    [NbConvertApp] Writing 12953 bytes to sse_x86.md



```python

```
