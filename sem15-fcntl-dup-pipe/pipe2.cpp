// %%cpp pipe2.cpp
// %run gcc pipe2.cpp -o pipe2.exe
// %run ./pipe2.exe

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

int main() {
    int fd[2];
    pipe(fd); 
    pid_t pid_1, pid_2;
    if ((pid_1 = fork()) == 0) {
        dup2(fd[1], 1); 
        close(fd[0]); 
        close(fd[1]);
        for (int i = 0; i < 1000; ++i) {
            write(1, "X", 1);
            //sched_yield();
            struct timespec t = {.tv_sec = 0, .tv_nsec = 10000};
            nanosleep(&t, &t);  
        }
        return 0;
    }
    if ((pid_2 = fork()) == 0) {
        // no close calls here
        dup2(fd[0], 0);
        close(fd[0]); 
        close(fd[1]);
        fcntl(0, F_SETFL, fcntl(0, F_GETFL, 0) | O_NONBLOCK);
        while (true) {
            char c;
            int r = read(0, &c, 1);
            if (r > 0) {
                write(1, &c, 1);
            } else if (r < 0) {
                assert(errno == EAGAIN);
                write(1, "?", 1);
            } else {
                break;
            }
        }
        return 0;
    }
    close(fd[0]);
    close(fd[1]);
    int status;
    assert(waitpid(pid_1, &status, 0) != -1);
    assert(waitpid(pid_2, &status, 0) != -1);
    return 0;
}

