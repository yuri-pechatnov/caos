// %%cpp philosophical_lock.c
// %run gcc -fsanitize=thread philosophical_lock.c -lpthread -o philosophical_lock.exe
//%run ./philosophical_lock.exe 

#include <assert.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

struct philosopher_data {
    pthread_mutex_t* left_fork;
    pthread_mutex_t* right_fork;
    int number; // protected by left_fork mutex
};

void philosopher_func(struct philosopher_data* data) {
    pthread_mutex_lock(data->left_fork);
    pthread_mutex_lock(data->right_fork);

    printf("Philosopher %d has eaten\n", data->number); fflush(stdout);

    pthread_mutex_unlock(data->left_fork);
    pthread_mutex_unlock(data->right_fork);
}

int main() {
    const int num_threads = 2;
    pthread_mutex_t mutexes[num_threads];
    pthread_t threads[num_threads];
    struct philosopher_data data[num_threads];

    for (int i = 0; i < num_threads; ++i) {
        assert(pthread_mutex_init(&mutexes[i], NULL) == 0);
        data[i] = (struct philosopher_data){.number=4, .left_fork=&mutexes[i], 
            .right_fork=&mutexes[(i + 1) % num_threads]};
        assert(pthread_create(&threads[i], NULL, (void* (*)(void*))philosopher_func, (void*)&data[i]) == 0);
    }

    for (int i = 0; i < num_threads; ++i) {
        assert(pthread_join(threads[i], NULL) == 0);
    }
}

