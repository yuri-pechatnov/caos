// %%cpp pthread_stack_size.c
// %MD ### Обычный стек 8мб, активно его не используем
// %run gcc -fsanitize=thread pthread_stack_size.c -lpthread -o pthread_stack_size.exe
// %run ./pthread_stack_size.exe 
// %MD ### Маленький стек 16кб, активно его не используем
// %run gcc -fsanitize=thread -DMY_STACK_SIZE=16384 pthread_stack_size.c -lpthread -o pthread_stack_size.exe
// %run ./pthread_stack_size.exe 
// %MD Во второй раз (VM delta size) не 16кб потому что имеются накладные расходы. Но оно меньше примерно на 8 MB
// %MD ### Обычный стек 8мб, активно его используем
// %run gcc -fsanitize=thread -DUSE_STACK=7000000 pthread_stack_size.c -lpthread -o pthread_stack_size.exe
// %run ./pthread_stack_size.exe 

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/resource.h>
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
#define ta_verify(stmt) do { if (stmt) break; fail_with_strerror(errno, "'" #stmt "' failed."); } while (0)

// verify pthread call
#define pt_verify(pthread_call) do { int code = (pthread_call); if (code == 0) break; \
    fail_with_strerror(code, "'" #pthread_call "' failed."); } while (0)



long int get_maxrss() {
    struct rusage usage;
    ta_verify(getrusage(RUSAGE_SELF, &usage) == 0);
    return usage.ru_maxrss;
}

const char* run_and_get_output(const char* bash_cmd) {
    int fds[2];
    pipe(fds);
    int pid = fork();
    if (pid == 0) {
        dup2(fds[1], 1);
        close(fds[0]); close(fds[1]);
        execlp("bash", "bash", "-c", bash_cmd, NULL);
        ta_verify(0 && "unreachable");
    }
    close(fds[1]);
    static __thread char buffer[100];
    int size = 0, rd = 0;
    while ((rd = read(fds[0], buffer, sizeof(buffer) - size)) != 0) {
        if (rd > 0) {
            size += rd;
        }
    }
    buffer[size] = 0;
    return buffer;
}

long int get_vm_usage() {  
    char cmd1[10000];
    sprintf(cmd1, "cat /proc/%d/status | grep VmData", getpid());
    const char* vm_usage_s = run_and_get_output(cmd1);
    long int vm_usage;
    sscanf(vm_usage_s, "VmData: %ld kB", &vm_usage);
    return vm_usage;
}

static void *
thread_func(void *arg)
{
    #ifdef USE_STACK
    char a[USE_STACK];
    for (int i = 2; i < sizeof(a); ++i) {
        a[i] = a[i - 1] ^ a[i - 2] * 19;
    }   
    log_printf("  Thread func started. Trash=%d\n", a[sizeof(a) - 1]); // Предотвращаем оптимизацию
    #else
    log_printf("  Thread func started\n");
    #endif
    sleep(2);
    log_printf("  Thread func finished\n"); 
    return NULL;
}

int main()
{
    double initial_rss = (double)get_maxrss() / 1000;
    double initial_vm_size = (double)get_vm_usage() / 1000;
    log_printf("Main func started. Initial RSS = %0.1lf MB, initial VM usage = %0.1lf MB\n", 
               initial_rss, initial_vm_size);
    pthread_t thread;
    pthread_attr_t thread_attr; 
    pt_verify(pthread_attr_init(&thread_attr)); // Атрибуты нужно инициализировать
    #ifdef MY_STACK_SIZE
    pt_verify(pthread_attr_setstacksize(&thread_attr, MY_STACK_SIZE)); // В структуру сохраняем размер стека
    #endif
    pt_verify(pthread_create(&thread, &thread_attr, thread_func, 0));
    pt_verify(pthread_attr_destroy(&thread_attr)); // И уничтожить
    sleep(1);
    double current_rss = (double)get_maxrss() / 1000;
    double current_vm_size = (double)get_vm_usage() / 1000;
    
    log_printf("Thread working. RSS = %0.1lf MB, delta RSS = %0.1lf MB\n", 
               current_rss, current_rss - initial_rss);
    log_printf("Thread working. VM size = %0.1lf MB, VM delta size = %0.1lf MB (!)\n", 
               current_vm_size, current_vm_size - initial_vm_size); 
    
    pt_verify(pthread_join(thread, NULL));
    log_printf("Main func finished\n");
    return 0;
}

