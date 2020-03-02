// %%cpp alarm_handle.c

#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>

static void handler(int signum) {
    static char buffer[100];
    int size = snprintf(buffer, sizeof(buffer), "Get signal %d, do nothing\n", signum);
    write(2, buffer, size); // можно использовать системные вызовы, они async-signal safe
    // fprintf(stderr, "Get signal %d, do nothing\n", signum); // А вот это уже использовать нелья
}

int main() {
    sigaction(SIGALRM,
              // лаконичный способ использования структуры, но не совместим с С++
              &(struct sigaction){
                  .sa_handler = handler, 
                  .sa_flags = SA_RESTART // используйте всегда. Знаю, что waitpid очень плохо себя ведет, когда прерывается сигналом
              },
              NULL);
    alarm(1);
    pause();
    printf("Is this command unreachable?\n"); // достижима ли эта команда?
    return 0;
}

