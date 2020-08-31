// %%cpp coro.cpp
// %run gcc -I ./libtask coro.cpp ./libtask/libtask.a -lpthread -o coro.exe
// %run ./coro.exe 300 100 200 1000

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <task.h>


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


const int STACK_SIZE = 32768;

Channel *c;

void delaytask(void *v)
{
    int ms = *(int*)(void*)&v;
    taskdelay(ms);
    log_printf("Task %dms is launched\n", ms);
    chansendul(c, 0);
}

void taskmain(int argc, char **argv)
{    
    c = chancreate(sizeof(unsigned long), 0);

    for(int i = 1; i < argc; i++){
        int ms = atoi(argv[i]);
        log_printf("Schedule %dms task\n", ms);
        taskcreate(delaytask, *(void**)&ms, STACK_SIZE);
    }

    for(int i = 1; i < argc; i++){
        log_printf("Some task is finished\n");
        chanrecvul(c);
    }
    taskexitall(0);
}

