// %%cpp sig_mutex.c
// %run gcc -g -O0 sig_mutex.c -lpthread -o sig_mutex.exe
// %run # через 1 секунду пошлется SIGINT, через 2 SIGKILL
// %run # timeout -s SIGKILL 2 timeout -s SIGINT 1 ./sig_mutex.exe

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
long long int value = 0;

static void handler(int signum) {
    pthread_mutex_lock(&mutex);
    printf("Value = %lld\n", value);
    pthread_mutex_unlock(&mutex);
    exit(0);
}

int main(int argc, char* argv[]) {
    sigaction(SIGINT, &(struct sigaction){.sa_handler=handler}, NULL);
    while (1) {
        pthread_mutex_lock(&mutex);
        ++value;
        pthread_mutex_unlock(&mutex);
    }
    printf("Stopped\n");
    return 0;
}

