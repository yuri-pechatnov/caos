// %%cpp pthread_cancel_fail.c
// %run gcc -fsanitize=thread pthread_cancel_fail.c -lpthread -o pthread_cancel_fail.exe
// %run timeout 3 ./pthread_cancel_fail.exe  # will fail (cancelation at cancelation points)
// %run gcc -fsanitize=thread  -DASYNC_CANCEL pthread_cancel_fail.c -lpthread -o pthread_cancel_fail.exe
// %run timeout 3 ./pthread_cancel_fail.exe  # ok, async cancelation

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <pthread.h>

const char* log_prefix(const char* file, int line) {
    struct timeval tp; gettimeofday(&tp, NULL); struct tm ltime; localtime_r(&tp.tv_sec, &ltime);
    static __thread char prefix[100]; 
    size_t time_len = strftime(prefix, sizeof(prefix), "%H:%M:%S", &ltime);
    sprintf(prefix + time_len, ".%03ld %s:%d [tid=%ld]", tp.tv_usec / 1000, file, line, syscall(__NR_gettid));
    return prefix;
}

#define log_printf_impl(fmt, ...) { time_t t = time(0); dprintf(2, "%s: " fmt "%s", log_prefix(__FILE__, __LINE__), __VA_ARGS__); }
#define log_printf(...) log_printf_impl(__VA_ARGS__, "")

// thread-aware assert
#define ta_assert(stmt) if (stmt) {} else { log_printf("'" #stmt "' failed"); exit(EXIT_FAILURE); }

static void *
thread_func(void *arg)
{
    log_printf("  Thread func started\n");
    #ifdef ASYNC_CANCEL
    ta_assert(pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL) == 0); // Включаем более жесткий способ остановки потока
    #endif
    // Без опции ASYNC_CANCEL поток не может быть остановлен во время своей работы.
    while (1); // зависаем тут. В процессе явно не будет cancelation points
    log_printf("  Thread func finished\n"); 
    return NULL;
}

int main()
{
    log_printf("Main func started\n");
    pthread_t thread;
    log_printf("Thread creating\n");
    ta_assert(pthread_create(&thread, NULL, thread_func, 0) == 0);
    sleep(1);
    log_printf("Thread canceling\n");
    ta_assert(pthread_cancel(thread) == 0);
    log_printf("Thread joining\n");
    ta_assert(pthread_join(thread, NULL) == 0);
    log_printf("Thread joined\n");
    log_printf("Main func finished\n");
    return 0;
}

