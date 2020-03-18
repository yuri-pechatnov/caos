// %%cpp tmp.c
// %run gcc -fsanitize=address tmp.c -lpthread -o tmp.exe
//%run ./tmp.exe


#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>

uint64_t N = 20;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t func_sleep = PTHREAD_COND_INITIALIZER;
pthread_cond_t main_sleep = PTHREAD_COND_INITIALIZER;
uint64_t number = -1;

void thread_func() {
    for (uint64_t i = 0; i < N; ++i) {
        pthread_mutex_lock(&mutex);
        number = i;
        
        while (number != -1) {
            pthread_cond_wait(&func_sleep, &mutex);
        }
        pthread_mutex_unlock(&mutex);
        pthread_cond_signal(&main_sleep);
    }
}

int main() {
    pthread_t thread;
    pthread_create(&thread, NULL, (void* (*)(void*))thread_func, NULL);
    for (uint32_t i = 0; i < N; ++i) {
        pthread_mutex_lock(&mutex);
        while (number == -1) {
            pthread_cond_wait(&main_sleep, &mutex);
        }
        uint64_t number_copy = number;
        number = -1;
        pthread_mutex_unlock(&mutex);
        pthread_cond_signal(&func_sleep);
        printf("%ld\n", number_copy);
    }
    pthread_join(thread, NULL);
    return 0;
}

