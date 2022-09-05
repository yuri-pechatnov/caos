// %%cpp print.c
// %run gcc -m64 -O3 print.c -o print.exe
// %run ./print.exe

#include <stdio.h>
#include <stdint.h>
#include <assert.h>
    
// int hello(int a, int b) {
//     printf("Hello %d and %d\n", a, b);
//     return a;
// }
    
int hello(int a, int b);
__asm__(R"(
.intel_syntax noprefix

.format_s:
   .string "Hello %d and %d\n"
hello:
    /* rdi, rsi, rdx, rcx */
    push rdi /* сохраняем a и заодно обеспечиваем выравнивание по 16 байт для вызываемой функции */
    /* готовим аргументы для printf */
    mov rdx, rsi
    mov rsi, rdi
    lea rdi, .format_s[rip] /* вычисляем адрес форматной строки (он в данном случае относительный, а не абсолютный) */
    call printf@PLT /* вызываем функцию */
    pop rax /* восстанавливаем a и готовимся его возвращать */
    ret
    
.att_syntax prefix /* thanks to Askhat Khayrullin */
)");


int main() {
    hello(1, 2);
    hello(10, 20);
    printf("SUCCESS\n");
}

