// %%cpp fcntl_flags.cpp
// %run gcc fcntl_flags.cpp -o fcntl_flags.exe
// %run ./fcntl_flags.exe

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

void describe_fd(const char* prefix, int fd) {
    int ret = fcntl(fd, F_GETFD, 0);
    if (ret & FD_CLOEXEC) {
        printf("%s: fd %d has CLOEXEC flag\n", prefix, fd);
    } else {
        printf("%s: fd %d doesn't have CLOEXEC flag\n", prefix, fd);
    } 
}

int main() {
    int fd[2];
    
    pipe(fd);
    describe_fd("pipe", fd[0]);

    pipe2(fd, O_CLOEXEC); 
    describe_fd("pipe2 + O_CLOEXEC: ", fd[0]);

    pipe(fd);
    fcntl(fd[0], F_SETFD, fcntl(fd[0], F_GETFD, 0) | FD_CLOEXEC);
    describe_fd("pipe + manually set flag: ", fd[0]);
    return 0;
}

