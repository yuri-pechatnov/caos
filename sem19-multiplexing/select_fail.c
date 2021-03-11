// %%cpp select_fail.c
// %run gcc select_fail.c -o select_fail.exe
// %run ulimit -n 1200 && ./select_fail.exe
// %run gcc -DBIG_FD select_fail.c -o select_fail.exe
// %run ulimit -n 1200 && ./select_fail.exe

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <time.h>
#include <sys/epoll.h>
#include <string.h>
#include <stdint.h>

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

#define conditional_handle_error(stmt, msg) \
    do { if (stmt) { perror(msg " (" #stmt ")"); exit(EXIT_FAILURE); } } while (0)

#ifdef BIG_FD
const int EXTRA_FD_COUNT = 1030;
#else
const int EXTRA_FD_COUNT = 1010;
#endif

int main() {
    pid_t child_pid;
    int input_fd;
   
    {
        int fds[2];
        pipe(fds);
        input_fd = fds[0];
        if ((child_pid = fork()) == 0) {
            sleep(1);
            dprintf(fds[1], "Hello from exactly one subprocess\n");
            exit(0);
        }
        assert(child_pid > 0);
        close(fds[1]);
    }
    
    for (int i = 0; i < EXTRA_FD_COUNT; ++i) {
        input_fd = dup(input_fd); // yes, we don't bother closing file descriptors in this example
    }
    
    log_printf("Select start input_fd=%d\n", input_fd);
    
    struct timeval tv = {.tv_sec = 10, .tv_usec = 0};
    while (input_fd != -1) {
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(input_fd, &rfds);
        char secret[] = "abcdefghijklmnop";
        int select_ret = select(input_fd + 1, &rfds, NULL, NULL, &tv);
        log_printf("Secret is %s\n", secret);
        if (strcmp(secret, "abcdefghijklmnop") != 0) {
            log_printf("Hey! select is broken!\n");
        }
        conditional_handle_error(select_ret == -1, "select error");
        if (select_ret > 0) {
            assert(FD_ISSET(input_fd, &rfds));
            
            char buf[100];
            int read_bytes = 0;
            if ((read_bytes = read(input_fd, buf, sizeof(buf) - 1)) > 0) {
                buf[read_bytes] = '\0';
                log_printf("Read from child subprocess: %s", buf);
            } else if (read_bytes == 0) {
                input_fd = -1;
            } else {
                conditional_handle_error(errno != EAGAIN, "strange error");
            }
        }
    }
    
    int status;    
    assert(waitpid(child_pid, &status, 0) != -1);
    return 0;
}

