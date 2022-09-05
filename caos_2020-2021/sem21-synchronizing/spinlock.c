// %%cpp spinlock.c
// %run gcc -fsanitize=thread -std=c11 spinlock.c -lpthread -o spinlock.exe
// %run ./spinlock.exe

#define _GNU_SOURCE 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <stdint.h>
#include <stdatomic.h>

// log_printf - макрос для отладочного вывода, добавляющий время со старта программы, имя функции и номер строки
uint64_t start_time_msec; void  __attribute__ ((constructor)) start_time_setter() { struct timespec spec; clock_gettime(CLOCK_MONOTONIC, &spec); start_time_msec = spec.tv_sec * 1000L + spec.tv_nsec / 1000000; }
const char* log_prefix(const char* func, int line) {
    struct timespec spec; clock_gettime(CLOCK_MONOTONIC, &spec); int delta_msec = spec.tv_sec * 1000L + spec.tv_nsec / 1000000 - start_time_msec;
    const int max_func_len = 13; static __thread char prefix[100]; 
    sprintf(prefix, "%d.%03d %*s():%-3d [tid=%ld]", delta_msec / 1000, delta_msec % 1000, max_func_len, func, line, syscall(__NR_gettid));
    return prefix;
}
#define log_printf_impl(fmt, ...) { time_t t = time(0); dprintf(2, "%s: " fmt "%s", log_prefix(__FUNCTION__, __LINE__), __VA_ARGS__); }
// Format: <time_since_start> <func_name>:<line> : <custom_message>
#define log_printf(...) log_printf_impl(__VA_ARGS__, "")


#define fail_with_strerror(code, msg) do { char err_buf[1024]; strerror_r(code, err_buf, sizeof(err_buf));\
    log_printf(msg " (From err code: %s)\n", err_buf);  exit(EXIT_FAILURE);} while (0)

// thread-aware assert
#define ta_verify(stmt) do { if (stmt) break; fail_with_strerror(errno, "'" #stmt "' failed."); } while (0)

// verify pthread call
#define pt_verify(pthread_call) do { int code = (pthread_call); if (code == 0) break; \
    fail_with_strerror(code, "'" #pthread_call "' failed."); } while (0)

//=============== Начало примера ======================

typedef enum {
    VALID_STATE = 0,
    INVALID_STATE = 1
} state_t;

_Atomic(int) lock = 0; // protects state
state_t current_state = VALID_STATE;

void sl_lock(_Atomic int* lock) { 
    int expected = 0;
    // weak отличается от strong тем, что может выдавать иногда ложный false. Но он быстрее работает.
    // atomic_compare_exchange_weak can change `expected`!
    while (!atomic_compare_exchange_weak(lock, &expected, 1)) {
        expected = 0;
    }
}

void sl_unlock(_Atomic int* lock) {
    atomic_fetch_sub(lock, 1);
}

// По сути та же функция, что и в предыдущем примере, но ипользуется spinlock вместо mutex
void thread_safe_func() { 
    // all function is critical section, protected by mutex
    sl_lock(&lock); // try comment lock&unlock out and look at result
    ta_verify(current_state == VALID_STATE);
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
        pt_verify(pthread_create(&threads[i], NULL, thread_func, (char*)NULL + i));
    }
    for (int i = 0; i < threads_count; ++i) {
        pt_verify(pthread_join(threads[i], NULL)); 
        log_printf("Thread %d joined\n", i);
    }
    log_printf("Main func finished\n");
    return 0;
}

