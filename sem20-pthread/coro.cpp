// %%cpp coro.cpp
// %run gcc -I ./libtask coro.cpp ./libtask/libtask.a -lpthread -o coro.exe
// %run ./coro.exe 300 100 200 1000

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <string.h>
#include <errno.h>
#include <task.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>

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


const int STACK_SIZE = 32768;

typedef struct {
    int sleep_time;
} task_args_t;

Channel *c;

void delaytask(task_args_t *args)
{
    taskdelay(args->sleep_time);
    log_printf("Task %dms is launched\n", args->sleep_time);
    chansendul(c, 0);
}

void taskmain(int argc, char **argv)
{    
    task_args_t args[argc];
    
    c = chancreate(sizeof(unsigned long), 0);

    for(int i = 1; i < argc; i++){
        args[i].sleep_time = atoi(argv[i]);
        log_printf("Schedule %dms task\n", args[i].sleep_time);
        taskcreate((void (*)(void*))delaytask, (void*)&args[i], STACK_SIZE);
    }

    for(int i = 1; i < argc; i++){
        chanrecvul(c);
        log_printf("Some task is finished\n");
    }
    taskexitall(0);
}

