// %%cpp program_asm_p.c
// %run arm-linux-gnueabi-gcc -marm program_asm_p.c -o program.exe
// %run qemu-arm ./program.exe ; echo 

#include <stdio.h>

void print_args(int a, int b, int c, int d, int e, int f) {
    printf("%d %d %d %d %d %d\n", a, b, c, d, e, f);
}

void call_print_args_first_3();
__asm__ (R"(
.global call_print_args_first_3
call_print_args_first_3:
    push {lr} // надо сохранить lr, так как BL его затрет
    mov r0, #1
    mov r1, #2
    mov r2, #3
    bl print_args
    pop {pc} // достаем lr со стека и сразу записываем в pc - тем самым выходим из функции
)");


void call_print_args_first_6();
__asm__ (R"(
.global call_print_args_first_6
call_print_args_first_6:
    push {lr} // сохраняем lr
    mov r0, #5
    mov r1, #6
    push {r0, r1} // кладем на стек 5, 6 в качестве 5-6 аргументов функции
    
    mov r0, #1
    mov r1, #2
    mov r2, #3
    mov r3, #4
    
    bl print_args
    pop {r0, r1} // снимаем со стека аргументы функции
    
    pop {pc} // достаем lr со стека и сразу записываем в pc - тем самым выходим из функции
)");

void call_print_args_first_6_2();
__asm__ (R"(
.global call_print_args_first_6_2
call_print_args_first_6_2:
    push {r4-r5, lr} // сохраняем lr, а так же r5-r6 так как мы их должны вернуть в изначальное состояние

    mov r0, #1
    mov r1, #2
    mov r2, #3
    mov r3, #4
    mov r4, #5
    mov r5, #6
    
    // 1
    //push {r4, r5} // кладем на стек 5, 6 в качестве 5-6 аргументов функции
    // 2
    //sub sp, sp, #4
    //str r5, [sp]
    //sub sp, sp, #4
    //str r4, [sp]
    // 3
    str r5, [sp, #-4]!
    str r4, [sp, #-4]!
    bl print_args
    add sp, sp, #8 // вместо pop {r4, r5}, нам же не нужны больше эти аргументы, зачем их возвращать в r4 и r5?
    
    pop {r4-r5, pc} // достаем lr со стека и сразу записываем в pc - тем самым выходим из функции
)");


int main() {
    call_print_args_first_3();
    call_print_args_first_6();
    call_print_args_first_6_2();
    return 0;
}

