// %%cpp condvar_2.c
// %run gcc -fsanitize=thread condvar_2.c -lpthread -o condvar_2.exe
// %run # ./condvar_2.exe

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <pthread.h>
#include <stdatomic.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; 
pthread_cond_t condvar = PTHREAD_COND_INITIALIZER;
_Atomic int value = -1;

static void* producer_func(void* arg) 
{
    pthread_mutex_lock(&mutex);
    atomic_store(&value, 42);
    pthread_cond_signal(&condvar);
    pthread_mutex_unlock(&mutex);
}

static void* consumer_func(void* arg) 
{ 
    pthread_mutex_lock(&mutex);
    while (atomic_load(&value) == -1) {
        pthread_cond_wait(&condvar, &mutex); 
    }
    printf("Value = %d\n", atomic_load(&value));
    pthread_mutex_unlock(&mutex);
    return NULL;
}

int main()
{
    pthread_t producer_thread;
    pthread_create(&producer_thread, NULL, producer_func, NULL);
    
    pthread_t consumer_thread;
    pthread_create(&consumer_thread, NULL, consumer_func, NULL);

    pthread_join(producer_thread, NULL); 
    pthread_join(consumer_thread, NULL); 
    return 0;
}

