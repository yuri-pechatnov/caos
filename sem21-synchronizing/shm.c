// %%cpp shm.c
// %# Обратите внимание: -lrt. Здесь нужна новая разделяемая библиотека
// %run gcc -Wall -fsanitize=thread shm.c -lrt -lpthread -o s.exe
// %run ./s.exe remove_shm /my_shm  # на случай, если прошлый запуск закончился плохо и shm не удалилась
// %run ./s.exe create_shm /my_shm
// %run ./s.exe work 1 /my_shm & PID=$! ; ./s.exe work 2 /my_shm ; wait $PID
// %run ./s.exe remove_shm /my_shm


#define _GNU_SOURCE 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdatomic.h>
#include <fcntl.h>

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
    pthread_mutex_t mutex; 
    state_t current_state; // protected by mutex
} shared_state_t;

void process_safe_func(shared_state_t* state) {
    // all function is critical section, protected by mutex
    pthread_mutex_lock(&state->mutex); // try comment lock&unlock out and look at result
    ta_verify(state->current_state == VALID_STATE);
    state->current_state = INVALID_STATE; // do some work with state. 
    sched_yield();
    state->current_state = VALID_STATE;
    pthread_mutex_unlock(&state->mutex);
}
 
shared_state_t* load_state(const char* shm_name, bool do_create) {
    // открываем / создаем объект разделяемой памяти
    // по сути это просто open, только для виртуального файла (без сохранения данных на диск + ортогональное пространство имен)
    int fd = shm_open(shm_name, O_RDWR | (do_create ? O_CREAT : 0), 0644);
    ta_verify(fd >= 0);
    ta_verify(ftruncate(fd, sizeof(shared_state_t)) == 0);
    shared_state_t* state = mmap(
        /* desired addr, addr = */ NULL, 
        /* length = */ sizeof(shared_state_t), 
        /* access attributes, prot = */ PROT_READ | PROT_WRITE, 
        /* flags = */ MAP_SHARED,
        /* fd = */ fd,
        /* offset in file, offset = */ 0
    );
    close(fd);
    
    ta_verify(state != MAP_FAILED);
    if (!do_create) {
        return state;
    }
    // create interprocess mutex
    pthread_mutexattr_t mutex_attrs;
    pt_verify(pthread_mutexattr_init(&mutex_attrs));
    // Важно!
    pt_verify(pthread_mutexattr_setpshared(&mutex_attrs, PTHREAD_PROCESS_SHARED));
    pt_verify(pthread_mutex_init(&state->mutex, &mutex_attrs));
    pt_verify(pthread_mutexattr_destroy(&mutex_attrs));
    
    state->current_state = VALID_STATE;
    return state;
}

void unload_state(shared_state_t* state) {
    ta_verify(munmap(state, sizeof(shared_state_t)) == 0);
}

int main(int argc, char** argv)
{
    ta_verify(argc >= 2);
    if (strcmp("create_shm", argv[1]) == 0) {
        log_printf("  Creating state: %s\n", argv[2]);
        unload_state(load_state(argv[2], /*do_create=*/ 1));
        log_printf("  State created\n");
    } else if (strcmp("remove_shm", argv[1]) == 0) {
        log_printf("  Removing state: %s\n", argv[2]);
        // Файлы shm существуют пока не будет вызвана unlink.
        if (shm_unlink(argv[2]) == 0) {
            log_printf("  State was removed\n");   
        } else {
            log_printf("  State was NOT removed\n");   
        }
    } else if (strcmp("work", argv[1]) == 0) {
        ta_verify(argc == 4);
        int worker = strtol(argv[2], 0, 10);
        log_printf("  Worker %d started\n", worker);
        shared_state_t* state = load_state(argv[3], /*do_create=*/ 0);
       
        for (int j = 0; j < 10000; ++j) {
            process_safe_func(state);
        }

        unload_state(state);
        log_printf("  Worker %d finished\n", worker);
    } else {
        ta_verify(0 && "unknown command");
    }
    return 0;
}

