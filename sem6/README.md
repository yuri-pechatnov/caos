```python
# make magics here. Look at previous notebooks to see readable version
exec('\nget_ipython().run_cell_magic(\'javascript\', \'\', \'// setup cpp code highlighting\\nIPython.CodeCell.options_default.highlight_modes["text/x-c++src"] = {\\\'reg\\\':[/^%%cpp/]} ;\')\n\n# creating magics\nfrom IPython.core.magic import register_cell_magic, register_line_magic\nfrom IPython.display import display, Markdown\n\n@register_cell_magic\ndef save_file(fname, cell):\n    cell = cell if cell[-1] == \'\\n\' else cell + "\\n"\n    cmds = []\n    with open(fname, "w") as f:\n        for line in cell.split("\\n"):\n            if line.startswith("%"):\n                run_prefix = "%run "\n                assert line.startswith(run_prefix)\n                cmds.append(line[len(run_prefix):].strip())\n            else:\n                f.write(line + "\\n")\n    for cmd in cmds:\n        display(Markdown("Run: `%s`" % cmd))\n        get_ipython().system(cmd)\n\n@register_cell_magic\ndef cpp(fname, cell):\n    save_file(fname, cell)\n\n@register_cell_magic\ndef asm(fname, cell):\n    save_file(fname, cell)\n    \n@register_cell_magic\ndef makefile(fname, cell):\n    assert not fname\n    save_file("makefile", cell.replace(" " * 4, "\\t"))\n        \n@register_line_magic\ndef p(line):\n    print("{} = {}".format(line, eval(line)))\n')
```


    <IPython.core.display.Javascript object>


# Assembler x86

* Мало регистров
* Много команд
* Много легаси
* Много соглашений о вызовах
* Разные синтаксисы

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

DWORD PTR — это переменная типа двойного слова. Слово — это 16 бит. Термин получил распространение в эпоху 16-ти битных процессоров, тогда в регистр помещалось ровно 16 бит. Такой объем информации стали называть словом (word). Т. е. в нашем случае dword (double word) 2*16 = 32 бита = 4 байта (обычный int). 

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
    mov edx, DWORD PTR [esp + 4]
    mov eax, DWORD PTR [esp + 8]
    cmp edx, eax
    jl return_eax
    mov eax, DWORD PTR [esp + 12]
    cmp edx, eax
    jg return_eax
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
    mov eax, DWORD PTR [esp + 4]
    mov edx, DWORD PTR [esp + 8]
    cmp eax, edx
    cmovl eax, edx
    mov edx, DWORD PTR [esp + 12]
    cmp eax, edx
    cmovg eax, edx
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


# Развлекательно-познавательная часть


```python
%%cpp mul.c
%run gcc -m32 -masm=intel -O3 mul.c -S -o mul.S
%run cat mul.S | grep -v "^\s*\."

#include <stdint.h>
    
int32_t mul(int32_t a) { 
    return a * 2;
}
```


Run: `gcc -m32 -masm=intel -O3 mul.c -S -o mul.S`



Run: `cat mul.S | grep -v "^\s*\."`


    mul:
    	mov	eax, DWORD PTR [esp+4]
    	add	eax, eax
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
!jupyter nbconvert asm_x86.ipynb --to markdown --output README
```

    [NbConvertApp] Converting notebook asm_x86.ipynb to markdown
    [NbConvertApp] Writing 8120 bytes to README.md



```python

```
