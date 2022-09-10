// %%cpp hello_world.c
// %// .exe не имеет никакого практического смысла с точки зрения запуска программы
// %// однако позволяет выбирать все бинари регуляркой *.exe, автор кода этим пользуется
// %run gcc hello_world.c -o hello_world_c.exe  
// %run ./hello_world_c.exe

#include <stdio.h>

int main() {
    printf("Hello world!\n");
    return 0;
}

