// %%cpp sem_named.c
// %# Обратите внимание: -lrt. Здесь нужна новая разделяемая библиотека
// %run gcc -Wall -fsanitize=thread sem_named.c -lrt -lpthread -o s.exe
// %run ./s.exe work 1 /s42 & PID=$! ; ./s.exe work 2 /s42 ; wait $PID
// %run ./s.exe cleanup /s42 # необязательная команда. Будет работать и без нее

#define _GNU_SOURCE 
#include <stdio.h> 
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <pthread.h>
#include <string.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>

const char* log_prefix(const char* file, int line) {
    struct timeval tp; gettimeofday(&tp, NULL); struct tm ltime; localtime_r(&tp.tv_sec, &ltime);
    static __thread char prefix[100]; 
    size_t time_len = strftime(prefix, sizeof(prefix), "%H:%M:%S", &ltime);
    sprintf(prefix + time_len, ".%03ld %s:%3d [pid=%d]", tp.tv_usec / 1000, file, line, getpid());
    return prefix;
}

#define log_printf_impl(fmt, ...) { dprintf(2, "%s: " fmt "%s", log_prefix(__FILE__, __LINE__), __VA_ARGS__); }
#define log_printf(...) log_printf_impl(__VA_ARGS__, "")

// process-aware assert
#define pa_warn_if_not(stmt) if (stmt) {} else { log_printf("WARNING: '" #stmt "' failed\n"); }
#define pa_assert(stmt) if (stmt) {} else { log_printf("'" #stmt "' failed\n"); exit(EXIT_FAILURE); }

typedef enum {
    VALID_STATE = 0,
    INVALID_STATE = 1
} state_t;


typedef struct {
    pthread_mutex_t mutex; 
    state_t current_state; // protected by mutex
} shared_state_t;

void process_safe_func(shared_state_t* state) {
    // all function is critical section, protected by mutex
    pthread_mutex_lock(&state->mutex); // try comment lock&unlock out and look at result
    pa_assert(state->current_state == VALID_STATE);
    state->current_state = INVALID_STATE; // do some work with state. 
    sched_yield();
    state->current_state = VALID_STATE;
    pthread_mutex_unlock(&state->mutex);
}
 
shared_state_t* load_state(const char* shm_name, bool do_create) {
    // открываем / создаем объект разделяемой памяти
    int fd = shm_open(shm_name, O_RDWR | (do_create ? O_CREAT : 0), 0644);
    pa_assert(ftruncate(fd, sizeof(shared_state_t)) == 0);
    shared_state_t* state = mmap(
        /* desired addr, addr = */ NULL, 
        /* length = */ sizeof(shared_state_t), 
        /* access attributes, prot = */ PROT_READ | PROT_WRITE, 
        /* flags = */ MAP_SHARED,
        /* fd = */ fd,
        /* offset in file, offset = */ 0
    );
    pa_assert(state != MAP_FAILED);
    if (!do_create) {
        return state;
    }
    // create interprocess mutex
    pthread_mutexattr_t mutex_attrs;
    pa_assert(pthread_mutexattr_init(&mutex_attrs) == 0);
    // Важно!
    pa_assert(pthread_mutexattr_setpshared(&mutex_attrs, PTHREAD_PROCESS_SHARED) == 0);
    pa_assert(pthread_mutex_init(&state->mutex, &mutex_attrs) == 0);
    pa_assert(pthread_mutexattr_destroy(&mutex_attrs) == 0);
    
    state->current_state = VALID_STATE;
    return state;
}

void unload_state(shared_state_t* state) {
    pa_assert(munmap(state, sizeof(shared_state_t)) == 0);
}

shared_state_t* process_safe_init_and_load(const char* name) {
    // succeeded only for first process. This process will initalize state
    sem_t* init_semaphore = sem_open(
        name, O_CREAT | O_EXCL, 0644, 0);
    if (init_semaphore != SEM_FAILED) {
        // initializing branch for initializing process
        shared_state_t* state = load_state(name, /*do_create=*/ 1);
        sem_post(init_semaphore);
        return state;
    } else {
        // branch for processes waiting initialisation
        init_semaphore = sem_open(name, 0);
        pa_assert(init_semaphore != SEM_FAILED);
        sem_wait(init_semaphore); // wait finish if initializing process
        sem_post(init_semaphore);
        return load_state(name, /*do_create=*/ 0);
    }
}

int main(int argc, char** argv)
{
    pa_assert(argc >= 2);
    if (strcmp("cleanup", argv[1]) == 0) {
        log_printf("  Cleanup sem and shm: %s\n", argv[2]);
        pa_warn_if_not(shm_unlink(argv[2]) == 0);
        pa_warn_if_not(sem_unlink(argv[2]) == 0);
        log_printf("  State created\n");
    } else if (strcmp("work", argv[1]) == 0) {
        pa_assert(argc >= 3);
        int worker = strtol(argv[2], 0, 10);
        log_printf("  Worker %d started\n", worker);
        
        shared_state_t* state = process_safe_init_and_load(argv[3]);
       
        for (int j = 0; j < 10000; ++j) {
            process_safe_func(state);
        }
     
        unload_state(state);
        log_printf("  Worker %d finished\n", worker);
    } else {
        pa_assert(0 && "unknown command")
    }
    return 0;
}

