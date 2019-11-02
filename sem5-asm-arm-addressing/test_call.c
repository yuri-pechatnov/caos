
#include <stdio.h>

int print_a(int a);
__asm__(R"(
    .text
    .global print_a
print_a:
    mov r2, r0
        
    ldr r0, =stdout
    ldr r0, [r0]
    ldr r1, =.format_string
    
    push {lr}
    bl fprintf
    mov r0, #42
    pop {pc}
.format_string:
    .string "%d\n"
    .ascii "2\0"
)");

int main() {
    print_a(100500);
}

