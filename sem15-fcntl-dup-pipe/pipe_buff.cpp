// %%cpp pipe_buff.cpp
// %run gcc pipe_buff.cpp -o pipe_buff.exe
// %run timeout 1 ./pipe_buff.exe 1000 && echo SUCCESS || echo FAILED
// %run timeout 1 ./pipe_buff.exe 10000 && echo SUCCESS || echo FAILED
// %run timeout 1 ./pipe_buff.exe 100000 && echo SUCCESS || echo FAILED

#ifndef _GNU_SOURCE
  #define _GNU_SOURCE
#endif
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sched.h>
#include <time.h>
#include <errno.h>

int main(int argc, char** argv) {
    int size = 10;
    int fd[2];
    pipe(fd); 
    close(fd[0]);
    char* data = (char*)calloc(size, sizeof(char));
    
    assert(write(fd[1], data, size) == size);
    
    free(data);
    
    close(fd[1]);
    return 0;
}

