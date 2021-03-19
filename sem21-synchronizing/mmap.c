// %%cpp mmap.c
// %run gcc -Wall -fsanitize=thread mmap.c -lpthread -o mmap.exe
// %run ./mmap.exe

#define _GNU_SOURCE 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/mman.h>
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
#define log_printf_impl(fmt, ...) { dprintf(2, "%s: " fmt "%s", log_prefix(__FUNCTION__, __LINE__), __VA_ARGS__); }
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


typedef struct {
    _Atomic int lock; 
    state_t current_state; // protected by mutex
} shared_state_t;

shared_state_t* state; // interprocess state

void sl_lock(_Atomic int* lock) { 
    int expected = 0;
    while (!atomic_compare_exchange_weak(lock, &expected, 1)) {
        expected = 0;
    }
}

void sl_unlock(_Atomic int* lock) {
    atomic_fetch_sub(lock, 1);
}

void process_safe_func() {
    // all function is critical section, protected by spinlock
    sl_lock(&state->lock);
    ta_verify(state->current_state == VALID_STATE);
    state->current_state = INVALID_STATE; // do some work with state. 
    sched_yield();
    state->current_state = VALID_STATE;
    sl_unlock(&state->lock);
}

void process_func(int process_num) 
{
    log_printf("  Process %d started\n", process_num);
    for (int j = 0; j < 10000; ++j) {
        process_safe_func();
    }
    log_printf("  Process %d finished\n", process_num);
}

 
shared_state_t* create_state() {
    shared_state_t* state = mmap(
        /* desired addr, addr = */ NULL, 
        /* length = */ sizeof(shared_state_t), 
        /* access attributes, prot = */ PROT_READ | PROT_WRITE, 
        /* flags = */ MAP_SHARED | MAP_ANONYMOUS,
        /* fd = */ -1,
        /* offset in file, offset = */ 0
    );
    ta_verify(state != MAP_FAILED);
    
    state->lock = 0;
    state->current_state = VALID_STATE;
    return state;
}

void delete_state(shared_state_t* state) {
    ta_verify(munmap(state, sizeof(shared_state_t)) == 0);
}

int main()
{
    log_printf("Main process started\n");
    state = create_state();
    const int process_count = 2;
    pid_t processes[process_count];
    for (int i = 0; i < process_count; ++i) {
        log_printf("Creating process %d\n", i);
        ta_verify((processes[i] = fork()) >= 0);
        if (processes[i] == 0) {
            process_func(i);
            exit(0);
        }
    }
    for (int i = 0; i < process_count; ++i) {
        int status;
        ta_verify(waitpid(processes[i], &status, 0) != -1);
        ta_verify(WIFEXITED(status) && WEXITSTATUS(status) == 0);
        log_printf("Process %d 'joined'\n", i);
    }
    delete_state(state);
    log_printf("Main process finished\n");
    return 0;
}

