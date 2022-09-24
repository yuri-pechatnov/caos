// %%cpp write_for_gdb_2.c
// %run gcc -O0 -g write_for_gdb_2.c -c -o write_for_gdb_2.o

#include <stdio.h>

void my_print() {
    printf("X");
}

void my_flush() {
    fflush(stdout);
}

