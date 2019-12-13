// %%cpp terminator.c

#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>

int inside_sigsuspend = 0;

static void handler(int signum) {
    // Сейчас у нас есть некоторая гарантия, что обработчик будет вызван только внутри sigprocmask 
    // (ну или раньше изначального sigprocmask)
    // поэтому в случае однопоточного приложения можно использовать асинхронно-небезопасные функции
    fprintf(stderr, "Get signal %d, inside_sigsuspend = %d ( == 1 ?), do nothing\n", 
            signum, inside_sigsuspend);  
}

int main() {
    sigset_t mask;
    sigfillset(&mask);
    sigprocmask(SIG_BLOCK, &mask, NULL); // try comment out
    
    for (int signal = 0; signal < 100; ++signal) {
        sigaction(signal,
                  &(struct sigaction)
                  {.sa_handler=handler, .sa_flags=SA_RESTART, .sa_mask=mask},
                  NULL);
    }
    
    sigemptyset(&mask);
    printf("pid = %d\n", getpid());
    
    int res = 0;
    
    raise(SIGINT);
    raise(SIGCHLD);
    raise(SIGCHLD);
    
    while (1) {
        inside_sigsuspend = 1;
        sigsuspend(&mask); // try comment out
        inside_sigsuspend = 0;
        for (int i = 0; i < 10000000; ++i) {
            res ^= i;
        }
    }
    return res;
}

