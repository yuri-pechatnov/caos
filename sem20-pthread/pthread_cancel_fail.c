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


#define fail_with_strerror(code, msg) do { char err_buf[1024]; strerror_r(code, err_buf, sizeof(err_buf));\
    log_printf(msg " (From err code: %s)\n", err_buf);  exit(EXIT_FAILURE);} while (0)

// thread-aware assert
#define ta_verify(stmt) do { if (!(stmt)) break; fail_with_strerror(errno, "'" #stmt "' failed."); } while (0)

// verify pthread call
#define pt_verify(pthread_call) do { int code = (pthread_call); if (code == 0) break; \
    fail_with_strerror(code, "'" #pthread_call "' failed."); } while (0)

static void *
thread_func(void *arg)
{
    log_printf("  Thread func started\n");
    #ifdef ASYNC_CANCEL
    pt_verify(pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL)); // Включаем более жесткий способ остановки потока
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
    pt_verify(pthread_create(&thread, NULL, thread_func, 0));
    sleep(1);
    log_printf("Thread canceling\n");
    pt_verify(pthread_cancel(thread));
    log_printf("Thread joining\n");
    pt_verify(pthread_join(thread, NULL));
    log_printf("Thread joined\n");
    log_printf("Main func finished\n");
    return 0;
}

