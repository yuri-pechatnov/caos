// %%cpp terminator2.c

#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>

static void handler(int signum) {
    // Сейчас у нас есть некоторая гарантия, что обработчик будет вызван только внутри sigprocmask 
    // (ну или раньше изначального sigprocmask)
    // поэтому в случае однопоточного приложения можно использовать асинхронно-небезопасные функции
    fprintf(stderr, "Get signal %d, do nothing\n", signum); 
}

int main() {
    for (int signal = 0; signal < 100; ++signal) {
        sigaction(signal,
                  &(struct sigaction)
                  {.sa_handler=handler, .sa_flags=SA_RESTART},
                  NULL);
    }
    int a;
    read(0, &a, sizeof(a));
    return 0;
}

