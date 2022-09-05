// %%cpp example1.c
// %run gcc -m64 -masm=intel -nostdlib -O3 example1.c -o example1.exe

// %run ./example1.exe ; echo "Exited with code=$?" 

// Тут есть макросы с номерами системных вызовов
#include <sys/syscall.h>
    
// Превращение макроса с числовым литералом в строковый литерал
#define stringify_impl(x) #x
#define stringify(x) stringify_impl(x)

    
int my_exit(int code);
__asm__(R"(
my_exit:
    mov eax, )" stringify(SYS_exit_group) R"(
    mov ebx, edi
    syscall
    /* не нужно возвращаться из функции, на этом программа завершится */
)");
    
// Именно с этой функции всегда начинается выполнение программы
void _start() {
    my_exit(0);
}

