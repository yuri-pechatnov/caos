// %%cpp istty.c
// %run gcc istty.c -o istty.exe
// %run ./istty.exe > a.txt
// %run ./istty.exe 

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
    if (isatty(STDOUT_FILENO)) {
        fprintf(stderr, "\033[0;31mIt's terminal\033[0m\n");
    } else {
        fprintf(stderr, "It's NOT terminal\n");
    }
    return 0;
}

