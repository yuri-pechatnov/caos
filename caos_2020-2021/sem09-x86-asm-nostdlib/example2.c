// %%cpp example2.c
// %run gcc -m64 -masm=intel -nostdlib -O3 example2.c -o example2.exe
// %run strace ./example2.exe ; echo "Exited with code=$?" 
    
int my_exit(int code);
__asm__(R"(
my_exit:
    mov rax, 231 /* номер системного вызова exit_group */
    syscall
    /* не нужно возвращаться из функции, на этом программа завершится */
)");
    
// Именно с этой функции всегда начинается выполнение программы
void _start() {
    my_exit(0);
}

