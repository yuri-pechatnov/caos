// %%cpp atomic.c
// %run gcc -fsanitize=thread atomic.c -lpthread -o atomic.exe
// %run # ./atomic.exe

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <pthread.h>
#include <stdatomic.h>

#define size 3
_Atomic int* data = NULL;

static void* consumer_func(void* arg) {
    int* local_data;
    while ((local_data = (int*)atomic_load(&data)) == NULL) {
        sched_yield();
    }
    for (int i = 0; i < size; ++i) {
        printf("%d ", local_data[i]);
    }
    return NULL;
}

int main()
{
    pthread_t consumer_thread;
    pthread_create(&consumer_thread, NULL, consumer_func, NULL);
    sched_yield();
    int created_data[size] = {10, 20, 30};
    atomic_store(&data, (void*)created_data);
    pthread_join(consumer_thread, NULL); 
    return 0;
}

