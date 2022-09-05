// %%cpp test_call.c
// %run arm-linux-gnueabi-gcc -marm test_call.c -O2 -o test_call.exe
// %run qemu-arm ./test_call.exe

#include <stdio.h>


int print_a(int a);
__asm__(R"(
    .text
    .global print_a
print_a:
    push {lr}
    
    mov r2, r0    
    ldr r0, =stdout // load address of variable 'stdout' 
    ldr r0, [r0]    // load variable by address (value is FILE* pointer)
    ldr r1, =.format_string // load address
    ldr r3, .const_number   // load value
    
    bl fprintf

    pop {pc}
.format_string:
    .ascii "Your number is %d, const number is %d\n" // заметьте, оно конкатенируется со слеюущим литералом
    .ascii "\0" // а \0 тут надо писать! Можете попытаться закомментировать эту строчку
.const_number:
    .word 100200300
)");

int main() {
    fprintf(stdout, "Your number is %d, const number is %d\n", 100500, 100200300);
    print_a(100500);
}

