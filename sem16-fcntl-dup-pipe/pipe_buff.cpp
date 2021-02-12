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
    assert(argc == 2);
    int size = strtol(argv[1], NULL, 10);
    int fd[2];
    pipe2(fd, O_NONBLOCK); // try to comment and compare
    char* data = (char*)calloc(size, sizeof(char));
    
    printf("Start writing %d bytes %p\n", size, data);
    int written = write(fd[1], data, size);
    if (written != size) {
        printf("Write only %d bytes\n", written);
    } else {
        printf("Written %d bytes\n", written);
        assert(read(fd[0], data, size) == size);
    }
    
    free(data);
    
    close(fd[0]);
    close(fd[1]);
    return (written == size) ? 0 : -1;
}

