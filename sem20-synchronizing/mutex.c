// %%cpp mutex.c
// %run gcc -O3 -fsanitize=thread mutex.c -lpthread -o mutex.exe
//%run ./mutex.exe

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <pthread.h>
#include <assert.h>

pthread_mutex_t mutex[2] = {PTHREAD_MUTEX_INITIALIZER, PTHREAD_MUTEX_INITIALIZER};

struct {
    char state_0: 8; // protected by mutex[0]
    char state_1: 8; // protected by mutex[1]
} states = {.state_0 = 0, .state_1 = 0};

static void* tread_safe_func_0(void* arg) 
{
    for (int iter = 0; iter < 1000; ++iter) {
        pthread_mutex_lock(&mutex[0]);
        assert(states.state_0 == (iter & 1));
        states.state_0 ^= 1;
        pthread_mutex_unlock(&mutex[0]);
    }
}

static void* tread_safe_func_1(void* arg) 
{
    for (int iter = 0; iter < 1000; ++iter) {
        pthread_mutex_lock(&mutex[1]);
        assert(states.state_1 == (iter & 1));
        states.state_1 ^= 1;
        pthread_mutex_unlock(&mutex[1]);
    }
}

int main()
{
    pthread_t threads[2];
    pthread_create(&threads[0], NULL, tread_safe_func_0, NULL);
    pthread_create(&threads[1], NULL, tread_safe_func_1, NULL);
    for (int t = 0; t < 2; ++t) {
        pthread_join(threads[t], NULL); 
    }
    return 0;
}

