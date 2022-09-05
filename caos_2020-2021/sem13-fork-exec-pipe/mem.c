// %%cpp mem.c
// %run gcc mem.c -o mem.exe
// %run bash -c 'ulimit -v 1000000 ; ./mem.exe'
// %run bash -c 'ulimit -v 1000000 ; /usr/bin/time -v ./mem.exe 2>&1 | grep resident'

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N (int)1e8
#define M (int)1e6
    
int array[N];
    
int main() {
    int* mem = malloc(N * sizeof(int));
    memset(mem, 255, M * sizeof(int));
    free(mem);
    return 0;
}

