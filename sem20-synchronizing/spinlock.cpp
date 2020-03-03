// %%cpp spinlock.cpp
// %run gcc spinlock.cpp -lpthread -o spinlock.exe
// %run ./spinlock.exe

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <pthread.h>
#include <atomic.h> //!

const char* log_prefix() {
    struct timeval tp; gettimeofday(&tp, NULL);
    static __thread char prefix[100];
    size_t time_len = strftime(prefix, sizeof(prefix), "%H:%M:%S", localtime(&tp.tv_sec));
    sprintf(prefix + time_len, ".%03ld [tid=%ld]", tp.tv_usec / 1000, syscall(__NR_gettid));
    return prefix;
}

#define log_printf_impl(fmt, ...) { time_t t = time(0); dprintf(2, "%s: " fmt "%s", log_prefix(), __VA_ARGS__); }
#define log_printf(...) log_printf_impl(__VA_ARGS__, "")

// thread-aware assert
#define ta_assert(stmt) if (stmt) {} else { log_printf("'" #stmt "' failed"); exit(EXIT_FAILURE); }


typedef enum {
    VALID_STATE = 0,
    INVALID_STATE = 1
} state_t;

_Atomic int lock = 0; // protects state
state_t current_state = VALID_STATE;

void sl_lock(_Atomic int* lock) {
    
}

void sl_unlock(_Atomic int* lock) {
    atomic_fetchsub(lock, 1);
}

void thread_safe_func() {
    // all function is critical section, protected by mutex
    pthread_mutex_lock(&mutex); // try comment lock&unlock out and look at result
    ta_assert(current_state == VALID_STATE);
    current_state = INVALID_STATE; // do some work with state. 
    sched_yield();
    current_state = VALID_STATE;
    pthread_mutex_unlock(&mutex);
}

// Возвращаемое значение потока (~код возврата процесса) -- любое машинное слово.
static void* thread_func(void* arg) 
{
    int i = (char*)arg - (char*)NULL;
    log_printf("  Thread %d started\n", i);
    for (int j = 0; j < 10000; ++j) {
        thread_safe_func();
    }
    log_printf("  Thread %d finished\n", i);
    return NULL;
}

int main()
{
    log_printf("Main func started\n");
    const int threads_count = 2;
    pthread_t threads[threads_count];
    for (int i = 0; i < threads_count; ++i) {
        log_printf("Creating thread %d\n", i);
        ta_assert(pthread_create(&threads[i], NULL, thread_func, (char*)NULL + i) == 0);
    }
    for (int i = 0; i < threads_count; ++i) {
        ta_assert(pthread_join(threads[i], NULL) == 0); 
        log_printf("Thread %d joined\n", i);
    }
    log_printf("Main func finished\n");
    return 0;
}

