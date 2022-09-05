// %%cpp sem_named_simple.c
// %# Обратите внимание: -lrt. Здесь нужна новая разделяемая библиотека
// %run gcc -Wall -fsanitize=thread sem_named_simple.c -lrt -lpthread -o s.exe
// %run ./s.exe 

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
#include <semaphore.h>
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
#define ta_warn_if_not(stmt) do { if (!(stmt)) { log_printf("WARNING: '" #stmt "' failed\n"); } } while (0)

// verify pthread call
#define pt_verify(pthread_call) do { int code = (pthread_call); if (code == 0) break; \
    fail_with_strerror(code, "'" #pthread_call "' failed."); } while (0)


//=============== Начало примера ======================


int main(int argc, char** argv)
{
    // создаем семафор (открываем и сразу закрываем)
    {
        sem_t* sem = sem_open("/s42", O_CREAT | O_EXCL, 0644, 1); 
        ta_verify(sem);
        ta_verify(sem_close(sem) == 0);
    }
    
    // синхронизация через семафор разных процессов
    pid_t pids[3];
    for (int i = 0; i < sizeof(pids) / sizeof(pid_t); ++i) {
        ta_verify((pids[i] = fork()) >= 0);
        if (pids[i] == 0) {
            sem_t* sem = sem_open("/s42", 0); 
            ta_verify(sem);
            sem_wait(sem);
            printf("Hello from %d\n", i); // типа межпроцессная критическая секция
            fflush(stdout);
            sem_post(sem);
            ta_verify(sem_close(sem) == 0);
            return 0;
        }
    }
    for (int i = 0; i < sizeof(pids) / sizeof(pid_t); ++i) {
        int status;
        ta_verify(waitpid(pids[i], &status, 0) != -1);
    }
    
    // удаляем семафорр
    ta_warn_if_not(sem_unlink("/s42") == 0);
    return 0;
}

