// %%cpp sigqueue.c

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>

volatile sig_atomic_t last_signal = 0;

static void handler(int signum) {
    if (signum == SIGUSR1) {
        printf("Child process: got SIGUSR1\n"); fflush(stdout);
    } else if (signum == SIGINT) {
        printf("Child process: got SIGINT, finish\n"); fflush(stdout);
        exit(0);
    } else {
        printf("Child process: got SIGRTMIN\n"); fflush(stdout);
    }
}

int main() {
    assert(SIGRTMIN < SIGRTMAX);
    sigset_t mask;
    sigfillset(&mask);
    sigprocmask(SIG_BLOCK, &mask, NULL);
    
    int signals[] = {SIGUSR1, SIGINT, SIGRTMIN, 0};
    for (int* signal = signals; *signal; ++signal) {
        sigaction(*signal, &(struct sigaction){.sa_handler=handler, .sa_flags=SA_RESTART, .sa_mask=mask}, NULL);
    }
    
    sigemptyset(&mask);
    
    int parent_pid = getpid();
    int child_pid = fork();
    assert(child_pid >= 0);
    if (child_pid == 0) {
        while (1) { sigsuspend(&mask); }
    } else {
        for (int i = 0; i < 10; ++i) 
            assert(kill(child_pid, SIGUSR1) == 0);
        for (int i = 0; i < 10; ++i)
            assert(sigqueue(child_pid, SIGRTMIN, (union sigval){0}) == 0);
        sleep(1);
        printf("Parent process: Request child finish with SIGINT\n"); fflush(stdout);
        kill(child_pid, SIGINT); 
        int status;
        waitpid(child_pid, &status, 0);
    }
    return 0;
}

