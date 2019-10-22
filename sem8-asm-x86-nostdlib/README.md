```python
# make magics here. Look at previous notebooks to see readable version
exec('\nget_ipython().run_cell_magic(\'javascript\', \'\', \'// setup cpp code highlighting\\nIPython.CodeCell.options_default.highlight_modes["text/x-c++src"] = {\\\'reg\\\':[/^%%cpp/]} ;\')\n\n# creating magics\nfrom IPython.core.magic import register_cell_magic, register_line_magic\nfrom IPython.display import display, Markdown\n\n@register_cell_magic\ndef save_file(fname, cell):\n    cell = cell if cell[-1] == \'\\n\' else cell + "\\n"\n    cmds = []\n    with open(fname, "w") as f:\n        for line in cell.split("\\n"):\n            if line.startswith("%"):\n                run_prefix = "%run "\n                assert line.startswith(run_prefix)\n                cmds.append(line[len(run_prefix):].strip())\n            else:\n                f.write(line + "\\n")\n    for cmd in cmds:\n        display(Markdown("Run: `%s`" % cmd))\n        get_ipython().system(cmd)\n\n@register_cell_magic\ndef cpp(fname, cell):\n    save_file(fname, cell)\n\n@register_cell_magic\ndef asm(fname, cell):\n    save_file(fname, cell)\n    \n@register_cell_magic\ndef makefile(fname, cell):\n    assert not fname\n    save_file("makefile", cell.replace(" " * 4, "\\t"))\n        \n@register_line_magic\ndef p(line):\n    print("{} = {}".format(line, eval(line)))\n')
```


    <IPython.core.display.Javascript object>


# Жизнь без стандартной библиотеки

Что это значит? Значит, что функции взаимодействия с внещним миром (чтение, запись файлов и т. д.) будут реализованы в самом бинаре программы. Возможно вы даже лично напишите их код.



## Компилим как обычно


```python
%%cpp main.c
%run gcc -m32 -masm=intel -fno-asynchronous-unwind-tables -O3 main.c -S -o main.S
%run gcc -m32 -masm=intel -O3 main.c -o main.exe
%run ls -la main.exe
%run ldd main.exe  # Выводим зависимости по библиотекам
%run cat main.S
%run objdump -M intel -d main.exe

int main() {
    return 0;
}
```


Run: `gcc -m32 -masm=intel -fno-asynchronous-unwind-tables -O3 main.c -S -o main.S`


Run: `gcc -m32 -masm=intel -O3 main.c -o main.exe`



Run: `ls -la main.exe`


    -rwxrwxr-x 1 pechatnov pechatnov 7312 Oct 22 22:50 main.exe



Run: `ldd main.exe  # Выводим зависимости по библиотекам`


    	linux-gate.so.1 =>  (0xf7f2e000)
    	libc.so.6 => /lib/i386-linux-gnu/libc.so.6 (0xf7d52000)
    	/lib/ld-linux.so.2 (0xf7f30000)



Run: `cat main.S`


    	.file	"main.c"
    	.intel_syntax noprefix
    	.section	.text.unlikely,"ax",@progbits
    .LCOLDB0:
    	.section	.text.startup,"ax",@progbits
    .LHOTB0:
    	.p2align 4,,15
    	.globl	main
    	.type	main, @function
    main:
    	xor	eax, eax
    	ret
    	.size	main, .-main
    	.section	.text.unlikely
    .LCOLDE0:
    	.section	.text.startup
    .LHOTE0:
    	.ident	"GCC: (Ubuntu 5.4.0-6ubuntu1~16.04.10) 5.4.0 20160609"
    	.section	.note.GNU-stack,"",@progbits



Run: `objdump -M intel -d main.exe`


    
    main.exe:     file format elf32-i386
    
    
    Disassembly of section .init:
    
    0804828c <_init>:
     804828c:	53                   	push   ebx
     804828d:	83 ec 08             	sub    esp,0x8
     8048290:	e8 7b 00 00 00       	call   8048310 <__x86.get_pc_thunk.bx>
     8048295:	81 c3 6b 1d 00 00    	add    ebx,0x1d6b
     804829b:	8b 83 fc ff ff ff    	mov    eax,DWORD PTR [ebx-0x4]
     80482a1:	85 c0                	test   eax,eax
     80482a3:	74 05                	je     80482aa <_init+0x1e>
     80482a5:	e8 26 00 00 00       	call   80482d0 <__libc_start_main@plt+0x10>
     80482aa:	83 c4 08             	add    esp,0x8
     80482ad:	5b                   	pop    ebx
     80482ae:	c3                   	ret    
    
    Disassembly of section .plt:
    
    080482b0 <__libc_start_main@plt-0x10>:
     80482b0:	ff 35 04 a0 04 08    	push   DWORD PTR ds:0x804a004
     80482b6:	ff 25 08 a0 04 08    	jmp    DWORD PTR ds:0x804a008
     80482bc:	00 00                	add    BYTE PTR [eax],al
    	...
    
    080482c0 <__libc_start_main@plt>:
     80482c0:	ff 25 0c a0 04 08    	jmp    DWORD PTR ds:0x804a00c
     80482c6:	68 00 00 00 00       	push   0x0
     80482cb:	e9 e0 ff ff ff       	jmp    80482b0 <_init+0x24>
    
    Disassembly of section .plt.got:
    
    080482d0 <.plt.got>:
     80482d0:	ff 25 fc 9f 04 08    	jmp    DWORD PTR ds:0x8049ffc
     80482d6:	66 90                	xchg   ax,ax
    
    Disassembly of section .text:
    
    080482e0 <main>:
     80482e0:	31 c0                	xor    eax,eax
     80482e2:	c3                   	ret    
    
    080482e3 <_start>:
     80482e3:	31 ed                	xor    ebp,ebp
     80482e5:	5e                   	pop    esi
     80482e6:	89 e1                	mov    ecx,esp
     80482e8:	83 e4 f0             	and    esp,0xfffffff0
     80482eb:	50                   	push   eax
     80482ec:	54                   	push   esp
     80482ed:	52                   	push   edx
     80482ee:	68 40 84 04 08       	push   0x8048440
     80482f3:	68 e0 83 04 08       	push   0x80483e0
     80482f8:	51                   	push   ecx
     80482f9:	56                   	push   esi
     80482fa:	68 e0 82 04 08       	push   0x80482e0
     80482ff:	e8 bc ff ff ff       	call   80482c0 <__libc_start_main@plt>
     8048304:	f4                   	hlt    
     8048305:	66 90                	xchg   ax,ax
     8048307:	66 90                	xchg   ax,ax
     8048309:	66 90                	xchg   ax,ax
     804830b:	66 90                	xchg   ax,ax
     804830d:	66 90                	xchg   ax,ax
     804830f:	90                   	nop
    
    08048310 <__x86.get_pc_thunk.bx>:
     8048310:	8b 1c 24             	mov    ebx,DWORD PTR [esp]
     8048313:	c3                   	ret    
     8048314:	66 90                	xchg   ax,ax
     8048316:	66 90                	xchg   ax,ax
     8048318:	66 90                	xchg   ax,ax
     804831a:	66 90                	xchg   ax,ax
     804831c:	66 90                	xchg   ax,ax
     804831e:	66 90                	xchg   ax,ax
    
    08048320 <deregister_tm_clones>:
     8048320:	b8 1b a0 04 08       	mov    eax,0x804a01b
     8048325:	2d 18 a0 04 08       	sub    eax,0x804a018
     804832a:	83 f8 06             	cmp    eax,0x6
     804832d:	76 1a                	jbe    8048349 <deregister_tm_clones+0x29>
     804832f:	b8 00 00 00 00       	mov    eax,0x0
     8048334:	85 c0                	test   eax,eax
     8048336:	74 11                	je     8048349 <deregister_tm_clones+0x29>
     8048338:	55                   	push   ebp
     8048339:	89 e5                	mov    ebp,esp
     804833b:	83 ec 14             	sub    esp,0x14
     804833e:	68 18 a0 04 08       	push   0x804a018
     8048343:	ff d0                	call   eax
     8048345:	83 c4 10             	add    esp,0x10
     8048348:	c9                   	leave  
     8048349:	f3 c3                	repz ret 
     804834b:	90                   	nop
     804834c:	8d 74 26 00          	lea    esi,[esi+eiz*1+0x0]
    
    08048350 <register_tm_clones>:
     8048350:	b8 18 a0 04 08       	mov    eax,0x804a018
     8048355:	2d 18 a0 04 08       	sub    eax,0x804a018
     804835a:	c1 f8 02             	sar    eax,0x2
     804835d:	89 c2                	mov    edx,eax
     804835f:	c1 ea 1f             	shr    edx,0x1f
     8048362:	01 d0                	add    eax,edx
     8048364:	d1 f8                	sar    eax,1
     8048366:	74 1b                	je     8048383 <register_tm_clones+0x33>
     8048368:	ba 00 00 00 00       	mov    edx,0x0
     804836d:	85 d2                	test   edx,edx
     804836f:	74 12                	je     8048383 <register_tm_clones+0x33>
     8048371:	55                   	push   ebp
     8048372:	89 e5                	mov    ebp,esp
     8048374:	83 ec 10             	sub    esp,0x10
     8048377:	50                   	push   eax
     8048378:	68 18 a0 04 08       	push   0x804a018
     804837d:	ff d2                	call   edx
     804837f:	83 c4 10             	add    esp,0x10
     8048382:	c9                   	leave  
     8048383:	f3 c3                	repz ret 
     8048385:	8d 74 26 00          	lea    esi,[esi+eiz*1+0x0]
     8048389:	8d bc 27 00 00 00 00 	lea    edi,[edi+eiz*1+0x0]
    
    08048390 <__do_global_dtors_aux>:
     8048390:	80 3d 18 a0 04 08 00 	cmp    BYTE PTR ds:0x804a018,0x0
     8048397:	75 13                	jne    80483ac <__do_global_dtors_aux+0x1c>
     8048399:	55                   	push   ebp
     804839a:	89 e5                	mov    ebp,esp
     804839c:	83 ec 08             	sub    esp,0x8
     804839f:	e8 7c ff ff ff       	call   8048320 <deregister_tm_clones>
     80483a4:	c6 05 18 a0 04 08 01 	mov    BYTE PTR ds:0x804a018,0x1
     80483ab:	c9                   	leave  
     80483ac:	f3 c3                	repz ret 
     80483ae:	66 90                	xchg   ax,ax
    
    080483b0 <frame_dummy>:
     80483b0:	b8 10 9f 04 08       	mov    eax,0x8049f10
     80483b5:	8b 10                	mov    edx,DWORD PTR [eax]
     80483b7:	85 d2                	test   edx,edx
     80483b9:	75 05                	jne    80483c0 <frame_dummy+0x10>
     80483bb:	eb 93                	jmp    8048350 <register_tm_clones>
     80483bd:	8d 76 00             	lea    esi,[esi+0x0]
     80483c0:	ba 00 00 00 00       	mov    edx,0x0
     80483c5:	85 d2                	test   edx,edx
     80483c7:	74 f2                	je     80483bb <frame_dummy+0xb>
     80483c9:	55                   	push   ebp
     80483ca:	89 e5                	mov    ebp,esp
     80483cc:	83 ec 14             	sub    esp,0x14
     80483cf:	50                   	push   eax
     80483d0:	ff d2                	call   edx
     80483d2:	83 c4 10             	add    esp,0x10
     80483d5:	c9                   	leave  
     80483d6:	e9 75 ff ff ff       	jmp    8048350 <register_tm_clones>
     80483db:	66 90                	xchg   ax,ax
     80483dd:	66 90                	xchg   ax,ax
     80483df:	90                   	nop
    
    080483e0 <__libc_csu_init>:
     80483e0:	55                   	push   ebp
     80483e1:	57                   	push   edi
     80483e2:	56                   	push   esi
     80483e3:	53                   	push   ebx
     80483e4:	e8 27 ff ff ff       	call   8048310 <__x86.get_pc_thunk.bx>
     80483e9:	81 c3 17 1c 00 00    	add    ebx,0x1c17
     80483ef:	83 ec 0c             	sub    esp,0xc
     80483f2:	8b 6c 24 20          	mov    ebp,DWORD PTR [esp+0x20]
     80483f6:	8d b3 0c ff ff ff    	lea    esi,[ebx-0xf4]
     80483fc:	e8 8b fe ff ff       	call   804828c <_init>
     8048401:	8d 83 08 ff ff ff    	lea    eax,[ebx-0xf8]
     8048407:	29 c6                	sub    esi,eax
     8048409:	c1 fe 02             	sar    esi,0x2
     804840c:	85 f6                	test   esi,esi
     804840e:	74 25                	je     8048435 <__libc_csu_init+0x55>
     8048410:	31 ff                	xor    edi,edi
     8048412:	8d b6 00 00 00 00    	lea    esi,[esi+0x0]
     8048418:	83 ec 04             	sub    esp,0x4
     804841b:	ff 74 24 2c          	push   DWORD PTR [esp+0x2c]
     804841f:	ff 74 24 2c          	push   DWORD PTR [esp+0x2c]
     8048423:	55                   	push   ebp
     8048424:	ff 94 bb 08 ff ff ff 	call   DWORD PTR [ebx+edi*4-0xf8]
     804842b:	83 c7 01             	add    edi,0x1
     804842e:	83 c4 10             	add    esp,0x10
     8048431:	39 f7                	cmp    edi,esi
     8048433:	75 e3                	jne    8048418 <__libc_csu_init+0x38>
     8048435:	83 c4 0c             	add    esp,0xc
     8048438:	5b                   	pop    ebx
     8048439:	5e                   	pop    esi
     804843a:	5f                   	pop    edi
     804843b:	5d                   	pop    ebp
     804843c:	c3                   	ret    
     804843d:	8d 76 00             	lea    esi,[esi+0x0]
    
    08048440 <__libc_csu_fini>:
     8048440:	f3 c3                	repz ret 
    
    Disassembly of section .fini:
    
    08048444 <_fini>:
     8048444:	53                   	push   ebx
     8048445:	83 ec 08             	sub    esp,0x8
     8048448:	e8 c3 fe ff ff       	call   8048310 <__x86.get_pc_thunk.bx>
     804844d:	81 c3 b3 1b 00 00    	add    ebx,0x1bb3
     8048453:	83 c4 08             	add    esp,0x8
     8048456:	5b                   	pop    ebx
     8048457:	c3                   	ret    


## Компилим, статически линкуя libc


```python
%%cpp main2.c
%run gcc -m32 -masm=intel -fno-asynchronous-unwind-tables -static -flto -O3  main2.c -S -o main2.S
%run gcc -m32 -masm=intel -static -flto -O3 main2.c -o main2.exe
//%run ls -la main2.exe  # Заметьте, что размер стал сильно больше
//%run ldd main2.exe
//%run objdump -M intel -d main2.exe
%run ./main2.exe

#include <unistd.h>
#include <sys/syscall.h>

int main() {
    syscall(SYS_write, 1, "Hello!", 7);
    close(1);
    return 0;
}
```


Run: `gcc -m32 -masm=intel -fno-asynchronous-unwind-tables -static -flto -O3  main2.c -S -o main2.S`



Run: `gcc -m32 -masm=intel -static -flto -O3 main2.c -o main2.exe`



Run: `./main2.exe`


    Hello!


```python
!objdump -M intel -d main2.exe | grep -A 30 "syscall>:"
#!objdump -M intel -d main2.exe | grep -A 30 "s80ea9f0"
```

    0806daf0 <syscall>:
     806daf0:	55                   	push   ebp
     806daf1:	57                   	push   edi
     806daf2:	56                   	push   esi
     806daf3:	53                   	push   ebx
     806daf4:	8b 6c 24 2c          	mov    ebp,DWORD PTR [esp+0x2c]
     806daf8:	8b 7c 24 28          	mov    edi,DWORD PTR [esp+0x28]
     806dafc:	8b 74 24 24          	mov    esi,DWORD PTR [esp+0x24]
     806db00:	8b 54 24 20          	mov    edx,DWORD PTR [esp+0x20]
     806db04:	8b 4c 24 1c          	mov    ecx,DWORD PTR [esp+0x1c]
     806db08:	8b 5c 24 18          	mov    ebx,DWORD PTR [esp+0x18]
     806db0c:	8b 44 24 14          	mov    eax,DWORD PTR [esp+0x14]
     806db10:	ff 15 f0 a9 0e 08    	call   DWORD PTR ds:0x80ea9f0
     806db16:	5b                   	pop    ebx
     806db17:	5e                   	pop    esi
     806db18:	5f                   	pop    edi
     806db19:	5d                   	pop    ebp
     806db1a:	3d 01 f0 ff ff       	cmp    eax,0xfffff001
     806db1f:	0f 83 7b 2b 00 00    	jae    80706a0 <__syscall_error>
     806db25:	c3                   	ret    
     806db26:	66 90                	xchg   ax,ax
     806db28:	66 90                	xchg   ax,ax
     806db2a:	66 90                	xchg   ax,ax
     806db2c:	66 90                	xchg   ax,ax
     806db2e:	66 90                	xchg   ax,ax
    
    0806db30 <__mmap>:
     806db30:	55                   	push   ebp
     806db31:	57                   	push   edi
     806db32:	56                   	push   esi
     806db33:	53                   	push   ebx


# Пишем сами без libc


```python
%%cpp minimal.c
%run gcc -m32 -masm=intel -nostdlib -O3 minimal.c -o minimal.exe
%run gcc -m32 -masm=intel -nostdlib -fno-asynchronous-unwind-tables -O3 minimal.c -S -o minimal.S
%run ls -la minimal.exe  # Заметьте, что размер стал очень маленьким :)
//%run ldd minimal.exe

//%run cat minimal.S
//%run objdump -d minimal.exe

%run ./minimal.exe ; echo $? 

#include <sys/syscall.h>

    
// Универсальная функция для совершения системных вызовов
int syscall(int code, ...);
__asm__(R"(
syscall:
    push ebx
    push ebp
    push esi
    push edi
    mov eax, DWORD PTR [esp + 20] 
    mov ebx, DWORD PTR [esp + 24] 
    mov ecx, DWORD PTR [esp + 28] 
    mov edx, DWORD PTR [esp + 32]
    mov esi, DWORD PTR [esp + 36]
    mov edi, DWORD PTR [esp + 40]
    int 0x80
    pop edi
    pop esi
    pop ebp
    pop ebx
    ret
)");


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
    return syscall(SYS_write, fd, s, len);
}

int print_s(int fd, const char* s) {
    int len = 0;
    while (s[len]) ++len;
    return syscall(SYS_write, fd, s, len);
}


// Пример использования системного вызова для завершения работы программы
void _exit(int code);
__asm__(R"(
_exit:
    mov   eax, 1
    mov   ebx, [esp + 4]
    int   0x80
)");


const char hello_s[] = "Hello world from function 'write'!\n";
const int hello_s_size = sizeof(hello_s);

// Пример использования системного вызова для вывода в stdout
int write();
__asm__(R"(
write:
    push ebx
    mov eax, 4 
    mov ebx, 1
    lea ecx, [hello_s]
    mov edx, hello_s_size
    int 0x80
    pop ebx
    ret
)");


// Именно с этой функции всегда начинается выполнение программы
void _start() {
    const char hello_s_2[] = "Hello world from 'syscall'!\n";
    write();
    syscall(SYS_write, 1, hello_s_2, sizeof(hello_s_2));
    print_s(1, "Look at this value: "); print_int(1, 10050042); print_s(1, "\n");
    print_s(1, "Look at this value: "); print_int(1, s_to_int("123456")); print_s(1, "\n");
    
    // syscall(SYS_exit, 0);
    _exit(-1);
}

```


Run: `gcc -m32 -masm=intel -nostdlib -O3 minimal.c -o minimal.exe`



Run: `gcc -m32 -masm=intel -nostdlib -fno-asynchronous-unwind-tables -O3 minimal.c -S -o minimal.S`



Run: `ls -la minimal.exe  # Заметьте, что размер стал очень маленьким :)`


    -rwxrwxr-x 1 pechatnov pechatnov 2440 Oct 22 23:44 minimal.exe



Run: `./minimal.exe ; echo $?`


    Hello world from function 'write'!
    Hello world from 'syscall'!
    Look at this value: 10050042
    Look at this value: 123456
    255



```python

```


```python

```
