


# Жизнь без стандартной библиотеки


<p><a href="-" target="_blank">
    <h3>Видеозапись семинара</h3>
</a></p>

Что это значит? Значит, что функции взаимодействия с внешним миром (чтение, запись файлов и т. д.) будут реализованы в самом бинаре программы. Возможно, вы даже лично напишите их код.


[Ридинг Яковлева](https://github.com/victor-yacovlev/mipt-diht-caos/tree/master/practice/asm/nostdlib_baremetal) 


Сегодня в программе:
* <a href="#x87" style="color:#856024"> Вещественная арифметика на сопроцессоре x87 </a>
* <a href="#sse" style="color:#856024"> SSE </a>
    * <a href="#fp_sse" style="color:#856024"> Вещественная арифметика на SSE </a>
    * <a href="#int_sse" style="color:#856024"> Векторные операции на SSE </a>
    




## Компилим как обычно


```cpp
%%cpp main.c --under-spoiler-threshold 5
%run gcc -m64 -masm=intel -fno-asynchronous-unwind-tables -O3 main.c -S -o main.S
%run gcc -m64 -masm=intel -O3 main.c -o main.exe
%run ls -la main.exe
%run ldd main.exe  # Выводим зависимости по динамическим библиотекам
%run cat main.S
%run objdump -M intel -d main.exe | grep main

#include <unistd.h>

int main() {
    int w = write(1, "X", 1);
    return 0;
}
```


Run: `gcc -m64 -masm=intel -fno-asynchronous-unwind-tables -O3 main.c -S -o main.S`



Run: `gcc -m64 -masm=intel -O3 main.c -o main.exe`



Run: `ls -la main.exe`



<pre><code>-rwxrwxr-x 1 pechatnov pechatnov 16696 окт 24 17:52 main.exe</code></pre>



Run: `ldd main.exe  # Выводим зависимости по динамическим библиотекам`



<pre><code>	linux-vdso.so.1 (0x00007fffc48d8000)
	libc.so.6 =&gt; /lib/x86_64-linux-gnu/libc.so.6 (0x00007fbf60524000)
	/lib64/ld-linux-x86-64.so.2 (0x00007fbf60731000)</code></pre>



Run: `cat main.S`



<details> <summary> output </summary> <pre><code>	.file	&quot;main.c&quot;
	.intel_syntax noprefix
	.text
	.section	.rodata.str1.1,&quot;aMS&quot;,@progbits,1
.LC0:
	.string	&quot;X&quot;
	.section	.text.startup,&quot;ax&quot;,@progbits
	.p2align 4
	.globl	main
	.type	main, @function
main:
	endbr64
	sub	rsp, 8
	mov	edx, 1
	mov	edi, 1
	lea	rsi, .LC0[rip]
	call	write@PLT
	xor	eax, eax
	add	rsp, 8
	ret
	.size	main, .-main
	.ident	&quot;GCC: (Ubuntu 9.3.0-17ubuntu1~20.04) 9.3.0&quot;
	.section	.note.GNU-stack,&quot;&quot;,@progbits
	.section	.note.gnu.property,&quot;a&quot;
	.align 8
	.long	 1f - 0f
	.long	 4f - 1f
	.long	 5
0:
	.string	 &quot;GNU&quot;
1:
	.align 8
	.long	 0xc0000002
	.long	 3f - 2f
2:
	.long	 0x3
3:
	.align 8
4:</code></pre></details>



Run: `objdump -M intel -d main.exe | grep main`



<pre><code>main.exe:     file format elf64-x86-64
0000000000001060 &lt;main&gt;:
    10b1:	48 8d 3d a8 ff ff ff 	lea    rdi,[rip+0xffffffffffffffa8]        # 1060 &lt;main&gt;
    10b8:	ff 15 22 2f 00 00    	call   QWORD PTR [rip+0x2f22]        # 3fe0 &lt;__libc_start_main@GLIBC_2.2.5&gt;</code></pre>


## Компилим, статически линкуя libc


```cpp
%%cpp main2.c --under-spoiler-threshold 5
%run gcc -m64 -masm=intel -fno-asynchronous-unwind-tables -static -flto -O3 main2.c -S -o main2.S
%run gcc -m64 -masm=intel -static -flto -O3 main2.c -o main2.exe
%run ls -la main2.exe  # Заметьте, что размер стал сильно больше
%run ldd main2.exe || echo "fails with code=$?"
%run objdump -M intel -d main2.exe | grep "<_start>" -A 10 # Вот она функция с которой все начинается
%run objdump -M intel -d main2.exe | grep "<main>" -A 10 # Вот она функция main
%run objdump -M intel -d main2.exe | grep "<__libc_write>:" -A 8 | head -n 100 # А тут мы найдем syscall
%run strace ./main2.exe

#include <unistd.h>

int main() {
    int w = write(1, "X", 1);
    return 0;
}
```


Run: `gcc -m64 -masm=intel -fno-asynchronous-unwind-tables -static -flto -O3 main2.c -S -o main2.S`



Run: `gcc -m64 -masm=intel -static -flto -O3 main2.c -o main2.exe`



Run: `ls -la main2.exe  # Заметьте, что размер стал сильно больше`



<pre><code>-rwxrwxr-x 1 pechatnov pechatnov 871544 окт 24 15:58 main2.exe</code></pre>



Run: `ldd main2.exe || echo "fails with code=$?"`



<pre><code>	not a dynamic executable
fails with code=1</code></pre>



Run: `objdump -M intel -d main2.exe | grep "<_start>" -A 10 # Вот она функция с которой все начинается`



<details> <summary> output </summary> <pre><code>0000000000401bf0 &lt;_start&gt;:
  401bf0:	f3 0f 1e fa          	endbr64 
  401bf4:	31 ed                	xor    ebp,ebp
  401bf6:	49 89 d1             	mov    r9,rdx
  401bf9:	5e                   	pop    rsi
  401bfa:	48 89 e2             	mov    rdx,rsp
  401bfd:	48 83 e4 f0          	and    rsp,0xfffffffffffffff0
  401c01:	50                   	push   rax
  401c02:	54                   	push   rsp
  401c03:	49 c7 c0 a0 2d 40 00 	mov    r8,0x402da0
  401c0a:	48 c7 c1 00 2d 40 00 	mov    rcx,0x402d00</code></pre></details>



Run: `objdump -M intel -d main2.exe | grep "<main>" -A 10 # Вот она функция main`



<details> <summary> output </summary> <pre><code>0000000000401730 &lt;main&gt;:
  401730:	f3 0f 1e fa          	endbr64 
  401734:	48 83 ec 08          	sub    rsp,0x8
  401738:	ba 01 00 00 00       	mov    edx,0x1
  40173d:	bf 01 00 00 00       	mov    edi,0x1
  401742:	48 8d 35 5e 8d 0a 00 	lea    rsi,[rip+0xa8d5e]        # 4aa4a7 &lt;__PRETTY_FUNCTION__.10741+0x47&gt;
  401749:	e8 02 6f 04 00       	call   448650 &lt;__libc_write&gt;
  40174e:	31 c0                	xor    eax,eax
  401750:	48 83 c4 08          	add    rsp,0x8
  401754:	c3                   	ret    
  401755:	66 2e 0f 1f 84 00 00 	nop    WORD PTR cs:[rax+rax*1+0x0]</code></pre></details>



Run: `objdump -M intel -d main2.exe | grep "<__libc_write>:" -A 8 | head -n 100 # А тут мы найдем syscall`



<details> <summary> output </summary> <pre><code>0000000000448650 &lt;__libc_write&gt;:
  448650:	f3 0f 1e fa          	endbr64 
  448654:	64 8b 04 25 18 00 00 	mov    eax,DWORD PTR fs:0x18
  44865b:	00 
  44865c:	85 c0                	test   eax,eax
  44865e:	75 10                	jne    448670 &lt;__libc_write+0x20&gt;
  448660:	b8 01 00 00 00       	mov    eax,0x1
  448665:	0f 05                	syscall 
  448667:	48 3d 00 f0 ff ff    	cmp    rax,0xfffffffffffff000</code></pre></details>



Run: `strace ./main2.exe`



<details> <summary> output </summary> <pre><code>execve(&quot;./main2.exe&quot;, [&quot;./main2.exe&quot;], 0x7ffdeea932a0 /* 66 vars */) = 0
arch_prctl(0x3001 /* ARCH_??? */, 0x7ffe63e9e560) = -1 EINVAL (Invalid argument)
brk(NULL)                               = 0xf9a000
brk(0xf9b1c0)                           = 0xf9b1c0
arch_prctl(ARCH_SET_FS, 0xf9a880)       = 0
uname({sysname=&quot;Linux&quot;, nodename=&quot;pechatnov-vbox&quot;, ...}) = 0
readlink(&quot;/proc/self/exe&quot;, &quot;/home/pechatnov/vbox/caos/sem09-&quot;..., 4096) = 58
brk(0xfbc1c0)                           = 0xfbc1c0
brk(0xfbd000)                           = 0xfbd000
mprotect(0x4bd000, 12288, PROT_READ)    = 0
write(1, &quot;X&quot;, 1X)                        = 1
exit_group(0)                           = ?
+++ exited with 0 +++</code></pre></details>


Тут видим, что в `main` вызывается `__libc_write` (`write` либо макрос, либо соптимизировался), а в `__libc_write` происходит syscall с 0x1 в eax.

# Отличие 32 и 64 битных архитектур в этом месте


```cpp
%%cpp main2.c --under-spoiler-threshold 20
%run gcc -m32 -masm=intel -static -flto -O3 main2.c -o main32.exe
%run gcc -m64 -masm=intel -static -flto -O3 main2.c -o main64.exe
%run objdump -M intel -d main64.exe | grep "<__libc_write>:" -A 8 | head -n 100 # Тут мы найдем syscall
%run objdump -M intel -d main32.exe | grep "<__libc_write>:" -A 15 | head -n 100 # А тут мы найдем...
%// call   DWORD PTR gs:0x10 -- это что-то про сегментные регситры и VDSO и там дальше сложно раскопать
%run objdump -M intel -d main32.exe | grep "<_exit>:" -A 9 | head -n 1000 # Попробуем покопать другую функцию - exit

#include <unistd.h>

int main() {
    int w = write(1, "X", 1);
    return 0;
}
```


Run: `gcc -m32 -masm=intel -static -flto -O3 main2.c -o main32.exe`



Run: `gcc -m64 -masm=intel -static -flto -O3 main2.c -o main64.exe`



Run: `objdump -M intel -d main64.exe | grep "<__libc_write>:" -A 8 | head -n 100 # Тут мы найдем syscall`



<pre><code>0000000000448650 &lt;__libc_write&gt;:
  448650:	f3 0f 1e fa          	endbr64 
  448654:	64 8b 04 25 18 00 00 	mov    eax,DWORD PTR fs:0x18
  44865b:	00 
  44865c:	85 c0                	test   eax,eax
  44865e:	75 10                	jne    448670 &lt;__libc_write+0x20&gt;
  448660:	b8 01 00 00 00       	mov    eax,0x1
  448665:	0f 05                	syscall 
  448667:	48 3d 00 f0 ff ff    	cmp    rax,0xfffffffffffff000</code></pre>



Run: `objdump -M intel -d main32.exe | grep "<__libc_write>:" -A 15 | head -n 100 # А тут мы найдем...`



<pre><code>0806ea60 &lt;__libc_write&gt;:
 806ea60:	f3 0f 1e fb          	endbr32 
 806ea64:	56                   	push   esi
 806ea65:	53                   	push   ebx
 806ea66:	83 ec 14             	sub    esp,0x14
 806ea69:	8b 5c 24 20          	mov    ebx,DWORD PTR [esp+0x20]
 806ea6d:	8b 4c 24 24          	mov    ecx,DWORD PTR [esp+0x24]
 806ea71:	8b 54 24 28          	mov    edx,DWORD PTR [esp+0x28]
 806ea75:	65 a1 0c 00 00 00    	mov    eax,gs:0xc
 806ea7b:	85 c0                	test   eax,eax
 806ea7d:	75 21                	jne    806eaa0 &lt;__libc_write+0x40&gt;
 806ea7f:	b8 04 00 00 00       	mov    eax,0x4
 806ea84:	65 ff 15 10 00 00 00 	call   DWORD PTR gs:0x10
 806ea8b:	89 c3                	mov    ebx,eax
 806ea8d:	3d 00 f0 ff ff       	cmp    eax,0xfffff000
 806ea92:	77 4c                	ja     806eae0 &lt;__libc_write+0x80&gt;</code></pre>



\#\#\#\# `call   DWORD PTR gs:0x10 -- это что-то про сегментные регситры и VDSO и там дальше сложно раскопать`



Run: `objdump -M intel -d main32.exe | grep "<_exit>:" -A 9 | head -n 1000 # Попробуем покопать другую функцию - exit`



<pre><code>0806e2aa &lt;_exit&gt;:
 806e2aa:	8b 5c 24 04          	mov    ebx,DWORD PTR [esp+0x4]
 806e2ae:	b8 fc 00 00 00       	mov    eax,0xfc
 806e2b3:	65 ff 15 10 00 00 00 	call   DWORD PTR gs:0x10
 806e2ba:	b8 01 00 00 00       	mov    eax,0x1
 806e2bf:	cd 80                	int    0x80
 806e2c1:	f4                   	hlt    
 806e2c2:	66 90                	xchg   ax,ax
 806e2c4:	66 90                	xchg   ax,ax
 806e2c6:	66 90                	xchg   ax,ax</code></pre>


В 32-битной архитектуре системный вызов осуществляется с помощью `int 0x80`, в 64-битной - `syscall`.

Хотя в примере
```
mov    eax,0x1
int    0x80
```
это вызов write (внезапно)

# Пишем сами без libc


```cpp
%%cpp example1.c
%run gcc -m64 -masm=intel -nostdlib -O3 example1.c -o example1.exe
%run strace ./example1.exe || echo "Failed with code=$?" 

// Именно с этой функции всегда начинается выполнение программы
void _start() {
    
}
```


Run: `gcc -m64 -masm=intel -nostdlib -O3 example1.c -o example1.exe`



Run: `strace ./example1.exe || echo "Failed with code=$?"`


    execve("./example1.exe", ["./example1.exe"], 0x7ffdec419330 /* 66 vars */) = 0
    brk(NULL)                               = 0x557c5e16b000
    arch_prctl(0x3001 /* ARCH_??? */, 0x7ffe540c0000) = -1 EINVAL (Invalid argument)
    access("/etc/ld.so.preload", R_OK)      = -1 ENOENT (No such file or directory)
    mmap(NULL, 8192, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = 0x7f8dfbb50000
    arch_prctl(ARCH_SET_FS, 0x7f8dfbb50a80) = 0
    mprotect(0x557c5dbc8000, 4096, PROT_READ) = 0
    --- SIGSEGV {si_signo=SIGSEGV, si_code=SEGV_MAPERR, si_addr=0x1} ---
    +++ killed by SIGSEGV (core dumped) +++
    Segmentation fault (core dumped)
    Failed with code=139


Получаем сегфолт, так эта функция - входная точка в программу и в нее не передается разумного адреса возврата.
То есть делать return из этой функции - путь в никуда.

Интересный факт, что до старта `_start` все-таки есть жизнь, насколько я понимаю, это работа загрузчика программы и выполнение функций из секции `preinitarray`.


```cpp
%%cpp exit.c
%run gcc -m64 -masm=intel -O3 exit.c -o exit.exe
%run strace ./exit.exe 2>&1 | tail -n 3

// обычная программа с пустым main
int main() {}
```


Run: `gcc -m64 -masm=intel -O3 exit.c -o exit.exe`



Run: `strace ./exit.exe 2>&1 | tail -n 3`


    munmap(0x7efe42a4a000, 79779)           = 0
    exit_group(0)                           = ?
    +++ exited with 0 +++


Выполнение обычной программы заканчивается системным вызовом exit_group как подсказывает нам strace.

Можно нагуглить про этот системный вызов, например, это https://filippo.io/linux-syscall-table/


```cpp
%%cpp example2.c
%run gcc -m64 -masm=intel -nostdlib -O3 example2.c -o example2.exe
%run strace ./example2.exe ; echo "Exited with code=$?" 
    
int my_exit(int code);
__asm__(R"(
my_exit:
    mov rax, 231 /* номер системного вызова exit_group */
    mov rbx, rdi
    syscall
    /* не нужно возвращаться из функции, на этом программа завершится */
)");
    
// Именно с этой функции всегда начинается выполнение программы
void _start() {
    my_exit(0);
}
```


Run: `gcc -m64 -masm=intel -nostdlib -O3 example2.c -o example2.exe`



Run: `strace ./example2.exe ; echo "Exited with code=$?"`


    execve("./example2.exe", ["./example2.exe"], 0x7ffce1ce2630 /* 66 vars */) = 0
    brk(NULL)                               = 0x557190df7000
    arch_prctl(0x3001 /* ARCH_??? */, 0x7ffca8a6e4f0) = -1 EINVAL (Invalid argument)
    access("/etc/ld.so.preload", R_OK)      = -1 ENOENT (No such file or directory)
    mmap(NULL, 8192, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = 0x7f7c8e721000
    arch_prctl(ARCH_SET_FS, 0x7f7c8e721a80) = 0
    mprotect(0x55718f983000, 4096, PROT_READ) = 0
    exit_group(0)                           = ?
    +++ exited with 0 +++
    Exited with code=0


Научились завершать нашу программу, отлично :)

Но не писать же всегда странные числа вроде 231?

`sudo apt-get install libc6-dev-amd64` - надо установить, чтобы `sys/syscall.h` нашелся.


```cpp
%%cpp example3.c
%run gcc -m64 -masm=intel -nostdlib -O3 example3.c -o example3.exe
%run ./example3.exe ; echo "Exited with code=$?" 

// Тут есть макросы с номерами системных вызовов
#include <sys/syscall.h>
    
// Превращение макроса с числовым литералом в строковый литерал
#define stringify_impl(x) #x
#define stringify(x) stringify_impl(x)

    
int my_exit(int code);
__asm__(R"(
my_exit:
    mov rax, )" stringify(SYS_exit_group) R"( /* В разрыв строкового литерала ассемблерной вставки вставляется строковый литерал системного вызова */
    mov rbx, rdi
    syscall
    /* не нужно возвращаться из функции, на этом программа завершится */
)");
    
// Именно с этой функции всегда начинается выполнение программы
void _start() {
    my_exit(0);
}
```


Run: `gcc -m64 -masm=intel -nostdlib -O3 example3.c -o example3.exe`



Run: `./example3.exe ; echo "Exited with code=$?"`


    Exited with code=0


Можно так вывернуться, например.

Да, все что я пишу на Си, можно писать и просто на ассемблере. И инклюды там так же работают :)


```python
%%asm example4.S
%//run gcc -E -m64 -masm=intel -nostdlib -O3 example4.S -o /dev/stdout
%run gcc -m64 -masm=intel -nostdlib -O3 example4.S -o example4.exe
%run ./example4.exe ; echo "Exited with code=$?" 

#include <sys/syscall.h>

.intel_syntax noprefix
.text

my_exit:
    mov rax, SYS_exit_group
    mov rbx, rdi
    syscall
 
.globl _start
_start:
    mov rdi, 0
    call my_exit
```


\#\#\#\# `run gcc -E -m64 -masm=intel -nostdlib -O3 example4.S -o /dev/stdout`



Run: `gcc -m64 -masm=intel -nostdlib -O3 example4.S -o example4.exe`



Run: `./example4.exe ; echo "Exited with code=$?"`


    Exited with code=0


Давайте напишем что-нибудь более сложное:


https://stackoverflow.com/questions/2535989/what-are-the-calling-conventions-for-unix-linux-system-calls-and-user-space-f


```cpp
%%cpp minimal.c
%run gcc -g -m64 -masm=intel -nostdlib -O3 minimal.c -o minimal.exe
%run gcc -m64 -masm=intel -nostdlib -fno-asynchronous-unwind-tables -O3 minimal.c -S -o minimal.S
%run ls -la minimal.exe  # Заметьте, что размер стал очень маленьким :)
//%run ldd minimal.exe

//%run cat minimal.S
//%run objdump -d minimal.exe

%run ./minimal.exe ; echo "Exit code = $?" 

#include <sys/syscall.h>

    
// Универсальная функция для совершения системных вызовов (до 5 аргументов системного вызова)
int syscall(int code, ...);
__asm__(R"(
syscall:
    /* Function arguments: rdi, rsi, rdx, rcx, r8, r9 */
    /* Syscall arguments: rax (syscall num), rdi, rsi, rdx, r10, r8, r9.*/
    mov rax, rdi 
    mov rdi, rsi 
    mov rsi, rdx
    mov rdx, rcx
    mov r10, r8
    mov r8, r9
    syscall
    ret
)");

void my_exit(int code) {
    syscall(SYS_exit, code);
}

int write(int fd, const void* data, int size) {
    return syscall(SYS_write, fd, data, size);
}


void int_to_s(unsigned int i, char* s, int* len) {
    int clen = 0;
    for (int ic = i; ic; ic /= 10, ++clen);
    clen = clen ?: 1;
    s[clen] = '\0';
    for (int j = 0; j < clen; ++j, i /= 10) {
        s[clen - j - 1] = '0' + i % 10;
    }
    *len = clen;
}

unsigned int s_to_int(char* s) {
    unsigned int res = 0;
    while ('0' <= *s && *s <= '9') {
        res *= 10;
        res += *s - '0';
        ++s;
    }
    return res;
}

int print_int(int fd, unsigned int i) {
    char s[20];
    int len;
    int_to_s(i, s, &len);
    return write(fd, s, len);
}

int print_s(int fd, const char* s) {
    int len = 0;
    while (s[len]) ++len;
    return write(fd, s, len);
}




const char hello_s[] = "Hello world from function 'write'!\n";
const int hello_s_size = sizeof(hello_s);

// Забавно, но перед вызовом функции start стек не был выровнен по 16 :)
// Вернее был, но видимо не положили адрес возврата (так как не нужен), а сишный компилятор его ожидает...
void _start();
__asm__(R"(
.globl _start
_start:
    sub rsp, 8
    jmp main
)");


void main() {
    const char hello_s_2[] = "Hello world from 'syscall'!\n";
    write(1, hello_s, sizeof(hello_s) - 1);
    syscall(SYS_write, 1, hello_s_2, sizeof(hello_s_2) - 1);
    print_s(1, "Look at this value: "); print_int(1, 10050042); print_s(1, "\n");
    print_s(1, "Look at this value: "); print_int(1, s_to_int("123456")); print_s(1, "\n");
    
    my_exit(0);
}

```


Run: `gcc -g -m64 -masm=intel -nostdlib -O3 minimal.c -o minimal.exe`



Run: `gcc -m64 -masm=intel -nostdlib -fno-asynchronous-unwind-tables -O3 minimal.c -S -o minimal.S`



Run: `ls -la minimal.exe  # Заметьте, что размер стал очень маленьким :)`


    -rwxrwxr-x 1 pechatnov pechatnov 22400 окт 24 18:30 minimal.exe



Run: `./minimal.exe ; echo "Exit code = $?"`


    Hello world from function 'write'!
    Hello world from 'syscall'!
    Look at this value: 10050042
    Look at this value: 123456
    Exit code = 0



```python

```

# Смотрим на адреса различных переменных. Проверяем, что секции памяти расположены так, как мы ожидаем


```cpp
%%cpp look_at_addresses.c --under-spoiler-threshold 30
%run gcc -m64 -masm=intel -O0 look_at_addresses.c -o look_at_addresses.exe
%run ./look_at_addresses.exe
%run gcc -S -m64 -masm=intel -Os look_at_addresses.c -o /dev/stdout

#include <stdio.h>
#include <stdlib.h>

int func(int a) {
    return a;
}


int* func_static_initialized() {
    static int a = 4;
    return &a;
}

const int* func_static_const_initialized() {
    static const int a = 4;
    return &a;
}

int* func_static_not_initialized() {
    static int a;
    return &a;
}


int global_initialized[123] = {1, 2, 3};
const int global_const_initialized[123] = {1, 2, 3};
int global_not_initialized[123];

int main2() {
   int local2 = 5;
   printf("Local 'local2' addr = %p\n", &local2); 
}


int main() {
    int local = 1;
    static int st = 2;
    int* all = malloc(12);
    
    printf("Func func addr = %p\n", (void*)func);
    printf("Func func_s addr = %p\n", (void*)func_static_initialized);
    
    printf("Global var (initialized) addr = %p\n", global_initialized);
    printf("Global var (const initialized) addr = %p\n", global_const_initialized);
    printf("Global var (not initialized) addr = %p\n", global_not_initialized);
    
    printf("Static 'st' addr = %p\n", &st);
    printf("Static 'func_static_initialized.a' addr = %p\n", func_static_initialized());
    printf("Static 'func_static_const_initialized.a' addr = %p\n", func_static_const_initialized());
    printf("Static 'func_static_not_initialized.a' addr = %p\n", func_static_not_initialized());
    
    printf("Local 'local' addr = %p\n", &local);
    main2();
    
    printf("Heap 'all' addr = %p\n", all);
    free(all);
    return 0;
}
```


Run: `gcc -m64 -masm=intel -O0 look_at_addresses.c -o look_at_addresses.exe`



Run: `./look_at_addresses.exe`



<pre><code>Func func addr = 0x55c0098191a9
Func func_s addr = 0x55c0098191b9
Global var (initialized) addr = 0x55c00981c020
Global var (const initialized) addr = 0x55c00981a020
Global var (not initialized) addr = 0x55c00981c240
Static &#x27;st&#x27; addr = 0x55c00981c210
Static &#x27;func_static_initialized.a&#x27; addr = 0x55c00981c20c
Static &#x27;func_static_const_initialized.a&#x27; addr = 0x55c00981a3bc
Static &#x27;func_static_not_initialized.a&#x27; addr = 0x55c00981c224
Local &#x27;local&#x27; addr = 0x7fffcfb0697c
Local &#x27;local2&#x27; addr = 0x7fffcfb06954
Heap &#x27;all&#x27; addr = 0x55c00adff2a0</code></pre>



Run: `gcc -S -m64 -masm=intel -Os look_at_addresses.c -o /dev/stdout`



<details> <summary> output </summary> <pre><code>	.file	&quot;look_at_addresses.c&quot;
	.intel_syntax noprefix
	.text
	.globl	func
	.type	func, @function
func:
.LFB24:
	.cfi_startproc
	endbr64
	mov	eax, edi
	ret
	.cfi_endproc
.LFE24:
	.size	func, .-func
	.globl	func_static_initialized
	.type	func_static_initialized, @function
func_static_initialized:
.LFB25:
	.cfi_startproc
	endbr64
	lea	rax, a.3081[rip]
	ret
	.cfi_endproc
.LFE25:
	.size	func_static_initialized, .-func_static_initialized
	.globl	func_static_const_initialized
	.type	func_static_const_initialized, @function
func_static_const_initialized:
.LFB26:
	.cfi_startproc
	endbr64
	lea	rax, a.3084[rip]
	ret
	.cfi_endproc
.LFE26:
	.size	func_static_const_initialized, .-func_static_const_initialized
	.globl	func_static_not_initialized
	.type	func_static_not_initialized, @function
func_static_not_initialized:
.LFB27:
	.cfi_startproc
	endbr64
	lea	rax, a.3087[rip]
	ret
	.cfi_endproc
.LFE27:
	.size	func_static_not_initialized, .-func_static_not_initialized
	.section	.rodata.str1.1,&quot;aMS&quot;,@progbits,1
.LC0:
	.string	&quot;Local &#x27;local2&#x27; addr = %p\n&quot;
	.text
	.globl	main2
	.type	main2, @function
main2:
.LFB28:
	.cfi_startproc
	endbr64
	sub	rsp, 24
	.cfi_def_cfa_offset 32
	lea	rsi, .LC0[rip]
	mov	edi, 1
	mov	rax, QWORD PTR fs:40
	mov	QWORD PTR 8[rsp], rax
	xor	eax, eax
	lea	rdx, 4[rsp]
	mov	DWORD PTR 4[rsp], 5
	call	__printf_chk@PLT
	mov	rax, QWORD PTR 8[rsp]
	xor	rax, QWORD PTR fs:40
	je	.L6
	call	__stack_chk_fail@PLT
.L6:
	xor	eax, eax
	add	rsp, 24
	.cfi_def_cfa_offset 8
	ret
	.cfi_endproc
.LFE28:
	.size	main2, .-main2
	.section	.rodata.str1.1
.LC1:
	.string	&quot;Func func addr = %p\n&quot;
.LC2:
	.string	&quot;Func func_s addr = %p\n&quot;
.LC3:
	.string	&quot;Global var (initialized) addr = %p\n&quot;
.LC4:
	.string	&quot;Global var (const initialized) addr = %p\n&quot;
.LC5:
	.string	&quot;Global var (not initialized) addr = %p\n&quot;
.LC6:
	.string	&quot;Static &#x27;st&#x27; addr = %p\n&quot;
.LC7:
	.string	&quot;Static &#x27;func_static_initialized.a&#x27; addr = %p\n&quot;
.LC8:
	.string	&quot;Static &#x27;func_static_const_initialized.a&#x27; addr = %p\n&quot;
.LC9:
	.string	&quot;Static &#x27;func_static_not_initialized.a&#x27; addr = %p\n&quot;
.LC10:
	.string	&quot;Local &#x27;local&#x27; addr = %p\n&quot;
.LC11:
	.string	&quot;Heap &#x27;all&#x27; addr = %p\n&quot;
	.section	.text.startup,&quot;ax&quot;,@progbits
	.globl	main
	.type	main, @function
main:
.LFB29:
	.cfi_startproc
	endbr64
	push	rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	mov	edi, 12
	sub	rsp, 16
	.cfi_def_cfa_offset 32
	mov	rax, QWORD PTR fs:40
	mov	QWORD PTR 8[rsp], rax
	xor	eax, eax
	mov	DWORD PTR 4[rsp], 1
	call	malloc@PLT
	lea	rdx, func[rip]
	mov	edi, 1
	lea	rsi, .LC1[rip]
	mov	rbp, rax
	xor	eax, eax
	call	__printf_chk@PLT
	lea	rdx, func_static_initialized[rip]
	lea	rsi, .LC2[rip]
	xor	eax, eax
	mov	edi, 1
	call	__printf_chk@PLT
	lea	rdx, global_initialized[rip]
	lea	rsi, .LC3[rip]
	xor	eax, eax
	mov	edi, 1
	call	__printf_chk@PLT
	lea	rdx, global_const_initialized[rip]
	lea	rsi, .LC4[rip]
	xor	eax, eax
	mov	edi, 1
	call	__printf_chk@PLT
	lea	rdx, global_not_initialized[rip]
	lea	rsi, .LC5[rip]
	xor	eax, eax
	mov	edi, 1
	call	__printf_chk@PLT
	lea	rdx, st.3097[rip]
	lea	rsi, .LC6[rip]
	xor	eax, eax
	mov	edi, 1
	call	__printf_chk@PLT
	lea	rdx, a.3081[rip]
	lea	rsi, .LC7[rip]
	xor	eax, eax
	mov	edi, 1
	call	__printf_chk@PLT
	lea	rdx, a.3084[rip]
	lea	rsi, .LC8[rip]
	xor	eax, eax
	mov	edi, 1
	call	__printf_chk@PLT
	lea	rdx, a.3087[rip]
	lea	rsi, .LC9[rip]
	xor	eax, eax
	mov	edi, 1
	call	__printf_chk@PLT
	lea	rdx, 4[rsp]
	lea	rsi, .LC10[rip]
	xor	eax, eax
	mov	edi, 1
	call	__printf_chk@PLT
	xor	eax, eax
	call	main2
	mov	rdx, rbp
	mov	edi, 1
	xor	eax, eax
	lea	rsi, .LC11[rip]
	call	__printf_chk@PLT
	mov	rdi, rbp
	call	free@PLT
	mov	rax, QWORD PTR 8[rsp]
	xor	rax, QWORD PTR fs:40
	je	.L10
	call	__stack_chk_fail@PLT
.L10:
	add	rsp, 16
	.cfi_def_cfa_offset 16
	xor	eax, eax
	pop	rbp
	.cfi_def_cfa_offset 8
	ret
	.cfi_endproc
.LFE29:
	.size	main, .-main
	.data
	.align 4
	.type	st.3097, @object
	.size	st.3097, 4
st.3097:
	.long	2
	.local	a.3087
	.comm	a.3087,4,4
	.section	.rodata
	.align 4
	.type	a.3084, @object
	.size	a.3084, 4
a.3084:
	.long	4
	.data
	.align 4
	.type	a.3081, @object
	.size	a.3081, 4
a.3081:
	.long	4
	.comm	global_not_initialized,492,32
	.globl	global_const_initialized
	.section	.rodata
	.align 32
	.type	global_const_initialized, @object
	.size	global_const_initialized, 492
global_const_initialized:
	.long	1
	.long	2
	.long	3
	.zero	480
	.globl	global_initialized
	.data
	.align 32
	.type	global_initialized, @object
	.size	global_initialized, 492
global_initialized:
	.long	1
	.long	2
	.long	3
	.zero	480
	.ident	&quot;GCC: (Ubuntu 9.3.0-17ubuntu1~20.04) 9.3.0&quot;
	.section	.note.GNU-stack,&quot;&quot;,@progbits
	.section	.note.gnu.property,&quot;a&quot;
	.align 8
	.long	 1f - 0f
	.long	 4f - 1f
	.long	 5
0:
	.string	 &quot;GNU&quot;
1:
	.align 8
	.long	 0xc0000002
	.long	 3f - 2f
2:
	.long	 0x3
3:
	.align 8
4:</code></pre></details>


# Разбираемся в системным вызовом brk

`void *sbrk(intptr_t increment);`


```cpp
%%cpp minimal.c
%run gcc -m64 -masm=intel -nostdlib -O3 minimal.c -o minimal.exe
%run gcc -m64 -masm=intel -nostdlib -fno-asynchronous-unwind-tables -O3 minimal.c -S -o minimal.S

//%run cat minimal.S
//%run objdump -d minimal.exe

%run ./minimal.exe ; echo $? 

#include <sys/syscall.h>
#include <stdint.h>

    
// Универсальная функция для совершения системных вызовов (до 5 аргументов системного вызова)
int64_t syscall(int64_t code, ...);
__asm__(R"(
syscall:
    /* Function arguments: rdi, rsi, rdx, rcx, r8, r9 */
    /* Syscall arguments: rax (syscall num), rdi, rsi, rdx, r10, r8, r9.*/
    mov rax, rdi 
    mov rdi, rsi 
    mov rsi, rdx
    mov rdx, rcx
    mov r10, r8
    mov r8, r9
    syscall
    ret
)");

void my_exit(int code) {
    syscall(SYS_exit, code);
}

int64_t write(int fd, const void* data, int64_t size) {
    return syscall(SYS_write, fd, data, size);
}

void int_to_s(uint64_t i, char* s, int* len) {
    int clen = 0;
    for (int ic = i; ic; ic /= 10, ++clen);
    clen = clen ?: 1;
    s[clen] = '\0';
    for (int j = 0; j < clen; ++j, i /= 10) {
        s[clen - j - 1] = '0' + i % 10;
    }
    *len = clen;
}

unsigned int s_to_int(char* s) {
    unsigned int res = 0;
    while ('0' <= *s && *s <= '9') {
        res *= 10;
        res += *s - '0';
        ++s;
    }
    return res;
}

int print_int(int fd, int64_t i) {
    char s[40];
    int len;
    int_to_s(i, s, &len);
    return syscall(SYS_write, fd, s, len);
}

int print_s(int fd, const char* s) {
    int len = 0;
    while (s[len]) ++len;
    return syscall(SYS_write, fd, s, len);
}


const char hello_s[] = "Hello world from function 'write'!\n";
const int hello_s_size = sizeof(hello_s);


// Именно с этой функции всегда начинается выполнение программы
void _start() {
    const int size = 100 * 1000 * 1000;
    int* data_start = (void*)syscall(SYS_brk, 0);
    int* data_end = (void*)syscall(SYS_brk, (int*)data_start + size);
    
    print_s(1, "Data begin: "); print_int(1, (int64_t)(void*)data_start); print_s(1, "\n");
    print_s(1, "Data end: ");  print_int(1, (int64_t)(void*)data_end); print_s(1, "\n");
    
    data_start[0] = 1;
    for (int i = 1; i < (data_end - data_start); ++i) {
        data_start[i] = data_start[i - 1] + 1;
    }
    
    print_int(1, data_end[-1]); print_s(1, "\n");
    
    my_exit(0);
}

```


Run: `gcc -m64 -masm=intel -nostdlib -O3 minimal.c -o minimal.exe`



Run: `gcc -m64 -masm=intel -nostdlib -fno-asynchronous-unwind-tables -O3 minimal.c -S -o minimal.S`



Run: `./minimal.exe ; echo $?`


    Data begin: 7768686592
    Data end: 8168686592
    100000000
    0



```python

```


```python
%%asm asm.S
%run gcc -m64 -nostdlib asm.S -o asm.exe
%run ./asm.exe

#include <sys/syscall.h>

    .intel_syntax noprefix
    .text
    .global _start
_start:
    mov rax, SYS_write
    mov rdi, 1
    lea rsi, hello_world[rip]
    mov rdx, 14
    syscall

    mov rax, SYS_exit_group
    mov rdi, 1
    syscall

hello_world:
    .string "Hello, World!\n"
```


Run: `gcc -m64 -nostdlib asm.S -o asm.exe`



Run: `./asm.exe`


    Hello, World!



```python

```
