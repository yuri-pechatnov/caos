// %%cpp write_for_gdb.c
// %run gcc -O0 -g write_for_gdb.c -o write_for_gdb.exe
// %run timeout 1 ./write_for_gdb.exe 
    
#include <stdio.h>


void my_print() {
    printf("X");
}

void my_flush() {
    fflush(stdout);
}

int main()
{
    my_print();
    my_flush();
    int a;
    scanf("%d", &a);
    my_print();
    return 0;
}

