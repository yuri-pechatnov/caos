// %%cpp terminator.c

#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>

int val_x = 0;

static void
handler(int signum) {
    // Сейчас у нас есть некоторая гарантия, что обработчик будет вызван только внутри sigprocmask 
    // (ну или раньше изначального sigprocmask)
    // поэтому в случае однопоточного приложения можно использовать асинхронно-небезопасные функции
    fprintf(stderr, "Get signal %d, val_x = %d ( == 1 ?), do nothing\n", signum, val_x); 
}

int main() {
    for (int signal = 0; signal < 100; ++signal) {
        sigaction(signal,
                  &(struct sigaction)
                  {.sa_handler=handler, .sa_flags=SA_RESTART},
                  NULL);
    }
    sigset_t mask;
    sigfillset(&mask);
    sigprocmask(SIG_BLOCK, &mask, NULL); // try comment out
    sigemptyset(&mask);
    printf("pid = %d\n", getpid());
    val_x = 0;
    int res = 0;
    while (1) {
        val_x = 1;
        sigsuspend(&mask); // try comment out
        val_x = 0;
        for (int i = 0; i < 10000000; ++i) {
            res ^= i;
        }
    }
    return res;
}

