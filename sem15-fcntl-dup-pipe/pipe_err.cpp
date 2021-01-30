// %%cpp pipe_err.cpp
// %run gcc pipe_err.cpp -o pipe_err.exe
// %run ./pipe_err.exe ; echo  "Exit code is $?"

// #ifndef _GNU_SOURCE
//   #define _GNU_SOURCE
// #endif
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
#include <signal.h>

    
static void handler(int signum) {
    fprintf(stderr, "Get signal %d, do nothing\n", signum);  
}

    
    
int main() {
    sigset_t mask;
    sigfillset(&mask);
    sigprocmask(SIG_BLOCK, &mask, NULL); // try comment out
    
    for (int signal = 0; signal < 100; ++signal) {
        sigaction(signal,
                  &(struct sigaction) {
                      .sa_handler=handler, 
                      .sa_flags=SA_RESTART, 
                      // этот параметр говорит, что во время вызова обработчика сигнала
                      // будут заблокированы сигналы указанные в маске (то есть все)
                      .sa_mask=mask 
                  },
                  NULL);
    }
    
    int size = 100000;
    int fd[2];
    pipe(fd); 
    
    pid_t pid_1 = -1;
    if ((pid_1 = fork()) == 0) { 
        close(fd[1]);
        pause();
        fprintf(stderr, "unreachable?");
        return 0;
    }
    close(fd[0]);
    
    pid_t pid_2 = -1;
    if ((pid_2 = fork()) == 0) { 
        close(fd[1]);
        struct timespec t = {.tv_sec = 0, .tv_nsec = 100000000}; // 100 ms
        nanosleep(&t, &t); 
        kill(pid_1, SIGKILL);
        return 0;
    }
    char* data = (char*)calloc(size, sizeof(char));
    
    int ret = write(fd[1], data, size);
    if (ret == size) {
        printf("OK\n");
    } else {
        printf("ret = %d\n", ret);
        perror("FAILED to read");
    }
    
    free(data);
    
    int status;
    assert(waitpid(pid_1, &status, 0) != -1);
    assert(waitpid(pid_2, &status, 0) != -1);
    
    close(fd[1]);
    return 0;
}

