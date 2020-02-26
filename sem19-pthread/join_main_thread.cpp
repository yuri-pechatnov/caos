// %%cpp join_main_thread.cpp
// %run gcc join_main_thread.cpp -lpthread -o join_main_thread.exe
// %run timeout 3 ./join_main_thread.exe ; echo "Exit code: $?"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <pthread.h>

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

pthread_t main_thread;

static void* thread_func(void* arg)
{
    log_printf("  Thread func started\n");
  
    log_printf("  Main thread joining\n");
    ta_assert(pthread_join(main_thread, NULL) == 0);
    log_printf("  Main thread joined\n");

    log_printf("  Thread func finished\n");

    _exit(42);
}

int main()
{
    log_printf("Main func started\n");
    main_thread = pthread_self();
    
    pthread_t thread;
    log_printf("Thread creating\n");
    ta_assert(pthread_create(&thread, NULL, thread_func, 0) == 0);
    
    pthread_exit(NULL);
}

