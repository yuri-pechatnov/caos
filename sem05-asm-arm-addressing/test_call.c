
#include <stdio.h>

int scan_a(int* a);
__asm__(R"(
    .text
    .global scan_a
scan_a:
    mov r2, r0
    mov r3, r0
    ldr r0, =stdin
    ldr r0, [r0]
    ldr r1, =.format_string
    push {lr}
    push {r2}
    bl __isoc99_fscanf
    pop {r2}
    mov r0, #42
    pop {pc}
.format_string:
    .ascii "%d %d %d\0"
)");

int main() {
    int a = 100500;
    scan_a(&a);
    printf("a = %d\n", a);
}

