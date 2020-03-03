// %%cpp sig_while_true.c
// %run gcc -g -O3 sig_while_true.c -lpthread -o sig_while_true.exe
// %run timeout -s SIGTERM 1 ./sig_while_true.exe

#include <stdio.h>
#include <unistd.h>
#include <signal.h>

sig_atomic_t stop = 0;

static void handler(int signum) {
    stop = 1;
}

int main(int argc, char* argv[]) {
    int signals[] = {SIGINT, SIGTERM, 0};
    for (int* signal = signals; *signal; ++signal) {
        sigaction(*signal, &(struct sigaction){.sa_handler=handler}, NULL);
    }
    while (!stop);
    printf("Stopped\n");
    return 0;
}

