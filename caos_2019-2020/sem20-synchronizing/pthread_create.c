// %%cpp pthread_create.c
// %run gcc -g -O0 pthread_create.c -lpthread -o pthread_create.exe
// %run # ./pthread_create.exe
// %run #() gdb -ex="b 21" -batch --args ./pthread_create.exe

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
    while (!stop) {
        int a = 1;
        pause();
    }
    return 0;
}

