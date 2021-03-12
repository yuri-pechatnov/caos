// %%cpp pipe2.cpp
// %run gcc pipe2.cpp -o pipe2.exe
// %run ./pipe2.exe

#ifndef _GNU_SOURCE
  #define _GNU_SOURCE
#endif
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/syscall.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

// log_printf - макрос для отладочного вывода, добавляющий время со старта программы, имя функции и номер строки
uint64_t start_time_msec; void  __attribute__ ((constructor)) start_time_setter() { struct timespec spec; clock_gettime(CLOCK_MONOTONIC, &spec); start_time_msec = spec.tv_sec * 1000L + spec.tv_nsec / 1000000; }
const char* log_prefix(const char* func, int line) {
    struct timespec spec; clock_gettime(CLOCK_MONOTONIC, &spec); int delta_msec = spec.tv_sec * 1000L + spec.tv_nsec / 1000000 - start_time_msec;
    const int max_func_len = 8; static __thread char prefix[100]; 
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


int main() {
    log_printf("Program start\n");
    int status;
    int fd[2];
    pipe2(fd, O_NONBLOCK); 
    pid_t pid_1, pid_2;
    if ((pid_1 = fork()) == 0) {
        close(fd[0]);
        const int size = 500000; // 1MB
        char* data = (char*)calloc(size, 1);
        for (int i = 0; i < size; ++i) {
            data[i] = 'A' + i * ('Z' - 'A') / size;
        }
        int written = 0;
        while (written < size) {
            int ret = write(fd[1], data + written, size - written);
            if (ret == -1) {
                ta_verify(errno == EAGAIN);
                continue;
            }
            ta_verify(ret != 0);
            log_printf("  Written %d bytes (first letter = %c)\n", ret, data[written]);
            written += ret;
        }
        free(data);
        return 0;
    }
    // Ждать pid_1 тут - глупый способ словить дедлок
    // assert(waitpid(pid_1, &status, 0) != -1);
    if ((pid_2 = fork()) == 0) {
        close(fd[1]);
        char buf[1000000];
        while (true) {
            int rd = read(fd[0], buf, sizeof(buf));
            if (rd == 0) {
                break;
            }
            if (rd == -1) {
                ta_verify(errno == EAGAIN);
                continue;
            }
            log_printf("  Read %d bytes (first letter = %c)\n", rd, buf[0]);
        }
        return 0;
    }
    close(fd[0]);
    close(fd[1]);
    log_printf("Wait subprocesses\n");
    assert(waitpid(pid_1, &status, 0) != -1);
    assert(waitpid(pid_2, &status, 0) != -1);
    log_printf("Program finished\n");
    return 0;
}

