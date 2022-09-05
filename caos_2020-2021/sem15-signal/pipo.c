// %%cpp pipo.c
// %run gcc -g pipo.c -o pipo.exe
// %run ./pipo.exe 

#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>

// если здесь не поставить volatile, то компилятор может 
// соптимизировать `if (last_signal)` до `if (0)`. 
// Так как, если компилятору не указывать явно, он будет оптимизировать 
// код как однопоточный (+ без учета возможности прерываний хендлерами сигналов).
volatile sig_atomic_t last_signal = 0;

static void handler(int signum) {
    last_signal = signum;  // что плохо с таким обработчиком?
}

int main() {
    sigset_t mask;
    sigfillset(&mask);
    sigprocmask(SIG_BLOCK, &mask, NULL);
    
    int signals[] = {SIGUSR1, SIGINT, 0};
    for (int* signal = signals; *signal; ++signal) {
        sigaction(*signal, &(struct sigaction){.sa_handler=handler, .sa_flags=SA_RESTART, .sa_mask=mask}, NULL);
    }
    
    sigemptyset(&mask);
    
    int parent_pid = getpid();
    
    int child_pid = fork();
    assert(child_pid >= 0);
    if (child_pid == 0) {
        while (1) {
            sigsuspend(&mask);
            if (last_signal) {
                if (last_signal == SIGUSR1) {
                    printf("Child process: Pong\n"); fflush(stdout);
                    kill(parent_pid, SIGUSR1);
                } else {
                    printf("Child process finish\n"); fflush(stdout);
                    return 0;
                }
                last_signal = 0;
            }
        }
    } else {
        for (int i = 0; i < 3; ++i) {
            printf("Parent process: Ping\n"); fflush(stdout);
            kill(child_pid, SIGUSR1);
            while (1) {
                sigsuspend(&mask);
                if (last_signal) { last_signal = 0; break; }
            }
        }
        printf("Parent process: Request child finish\n"); fflush(stdout);
        kill(child_pid, SIGINT); 
        int status;
        waitpid(child_pid, &status, 0);
    }
    return 0;
}

