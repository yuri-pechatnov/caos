// %%cpp simple_open.c
// %run gcc simple_open.c -o simple_open.exe
// %run ./simple_open.exe
// %run ./simple_open.exe < a.txt
// %run ./simple_open.exe < a.txt 2> b.txt

#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


void apply_lsof() {
    char cmd[100];
    snprintf(cmd, sizeof(cmd), "lsof -p %d", getpid());
    system(cmd);
}


int main()
{  
    apply_lsof();
    return 0;
}

