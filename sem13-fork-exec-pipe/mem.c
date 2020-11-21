// %%cpp mem.c
// %run gcc mem.c -o mem.exe
// %run bash -c 'ulimit -m 100000 ; ./mem.exe ; /usr/bin/time -v ./mem.exe 2>&1 | grep resident'

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/wait.h>
#include <unistd.h>


#define N (int)1e8
#define M (int)1e6
    
int array[N];
    
int main() {
    int* mem = malloc(N * sizeof(int));

    pid_t pid = fork();
    if (pid != 0) {
        int status;
        pid_t w = waitpid(pid, &status, 0);
        assert(w != -1 && WIFEXITED(status));
    }
    
    memset(mem, 255, M * sizeof(int));
    free(mem);
    return 0;
}

