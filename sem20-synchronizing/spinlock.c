// %%cpp spinlock.c
// %run gcc -fsanitize=thread -std=c11 spinlock.c -lpthread -o spinlock.exe
// %run ./spinlock.exe

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/time.h>
#include <pthread.h>
#include <stdatomic.h> //!

const char* log_prefix(const char* func, int line) {
    struct timespec spec; clock_gettime(CLOCK_REALTIME, &spec); long long current_msec = spec.tv_sec * 1000L + spec.tv_nsec / 1000000;
    static _Atomic long long start_msec_storage = -1; long long start_msec = -1; if (atomic_compare_exchange_strong(&start_msec_storage, &start_msec, current_msec)) start_msec = current_msec;
    long long delta_msec = current_msec - start_msec;
    static __thread char prefix[100]; sprintf(prefix, "%lld.%03lld %13s():%d [tid=%ld]", delta_msec / 1000, delta_msec % 1000, func, line, syscall(__NR_gettid));
    return prefix;
}
#define log_printf_impl(fmt, ...) { time_t t = time(0); dprintf(2, "%s: " fmt "%s", log_prefix(__FUNCTION__, __LINE__), __VA_ARGS__); }
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
    int expected = 0;
    // atomic_compare_exchange_weak can change `expected`!
    while (!atomic_compare_exchange_weak(lock, &expected, 1)) {
        expected = 0;
    }
}

void sl_unlock(_Atomic int* lock) {
    atomic_fetch_sub(lock, 1);
}

void thread_safe_func() {
    // all function is critical section, protected by mutex
    sl_lock(&lock); // try comment lock&unlock out and look at result
    ta_assert(current_state == VALID_STATE);
    current_state = INVALID_STATE; // do some work with state. 
    sched_yield(); // increase probability of fail of incorrect lock realisation
    current_state = VALID_STATE;
    sl_unlock(&lock);
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

