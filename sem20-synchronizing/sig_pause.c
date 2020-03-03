// %%cpp sig_pause.c
// %run gcc -g -O0 sig_pause.c -lpthread -o sig_pause.exe
// %run # ./sig_pause.exe

#include <unistd.h>
#include <signal.h>

volatile sig_atomic_t stop = 0;

static void handler(int signum) {
    stop = 1;
}

int main(int argc, char* argv[]) {
    int signals[] = {SIGINT, SIGTERM, 0};
    for (int* signal = signals; *signal; ++signal) {
        sigaction(*signal, &(struct sigaction){.sa_handler=handler}, NULL);
    }
    while (!stop) {
        int a = 1;
        pause();
    }
    return 0;
}

