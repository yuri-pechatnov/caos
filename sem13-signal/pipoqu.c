// %%cpp pipoqu.c

#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>

sig_atomic_t last_signal = 0;
sig_atomic_t last_signal_value = 0;

// через info принимаем дополнительный int
static void handler(int signum, siginfo_t* info, void* ucontext) {
    last_signal = signum; 
    last_signal_value = info->si_value.sival_int; // сохраняем переданное число
}

int main() {
    sigset_t mask;
    sigfillset(&mask);
    sigprocmask(SIG_BLOCK, &mask, NULL);
    
    int signals[] = {SIGUSR1, SIGINT, 0};
    for (int* signal = signals; *signal; ++signal) {
        // обратите внимание, что хендлер теперь принимает больше аргументов
        // и записывается в другое поле
        // и еще есть флаг SA_SIGINFO, говорящий, что именно такой хендлер будет использоваться
        sigaction(*signal, &(struct sigaction){.sa_sigaction = handler, .sa_flags = SA_RESTART | SA_SIGINFO}, NULL);
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
                    printf("Child process: Pong (get %d, send %d)\n", last_signal_value, last_signal_value * 2); 
                    fflush(stdout);
                    // вместе с сигналом передаем число
                    sigqueue(parent_pid, SIGUSR1, (union sigval) {.sival_int = last_signal_value * 2 });
                } else {
                    printf("Child process finish\n"); fflush(stdout);
                    return 0;
                }
                last_signal = 0;
            }
        }
    } else {
        int child_response = 10;
        for (int i = 0; i < 3; ++i) {
            printf("Parent process: Ping (got %d, send %d)\n", child_response, child_response + 1); fflush(stdout);
            sigqueue(child_pid, SIGUSR1, (union sigval) {.sival_int = child_response + 1 });
            while (!last_signal) {
                sigsuspend(&mask); 
            }
            last_signal = 0;
            child_response = last_signal_value;
        }
        printf("Parent process: Request child finish\n"); fflush(stdout);
        kill(child_pid, SIGINT); 
        int status;
        waitpid(child_pid, &status, 0);
    }
    return 0;
}

