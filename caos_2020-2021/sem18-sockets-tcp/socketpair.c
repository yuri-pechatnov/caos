// %%cpp socketpair.c
// %run gcc socketpair.c -o socketpair.exe
// %run ./socketpair.exe

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/syscall.h>
#include <errno.h>
#include <time.h>
#include <stdatomic.h>
#include <stdbool.h>

// log_printf - макрос для отладочного вывода, добавляющий время с первого использования, имя функции и номер строки
const char* log_prefix(const char* func, int line) {
    struct timespec spec; clock_gettime(CLOCK_REALTIME, &spec); long long current_msec = spec.tv_sec * 1000L + spec.tv_nsec / 1000000;
    static _Atomic long long start_msec_storage = -1; long long start_msec = -1; if (atomic_compare_exchange_strong(&start_msec_storage, &start_msec, current_msec)) start_msec = current_msec;
    long long delta_msec = current_msec - start_msec; const int max_func_len = 10;
    static __thread char prefix[100]; sprintf(prefix, "%lld.%03lld %*s():%d    ", delta_msec / 1000, delta_msec % 1000, max_func_len, func, line); sprintf(prefix + max_func_len + 13, "[tid=%ld]", syscall(__NR_gettid));
    return prefix;
}
#define log_printf_impl(fmt, ...) { time_t t = time(0); dprintf(2, "%s: " fmt "%s", log_prefix(__FUNCTION__, __LINE__), __VA_ARGS__); }
// Format: <time_since_start> <func_name>:<line> : <custom_message>
#define log_printf(...) log_printf_impl(__VA_ARGS__, "")


#define SWAP(a, b) { __typeof__(a) c = (a); (a) = (b); (b) = (c); }


void write_smth(int fd) {
    for (int i = 0; i < 1000; ++i) {
        write(fd, "X", 1);
        struct timespec t = {.tv_sec = 0, .tv_nsec = 10000};
        nanosleep(&t, &t);  
    }
}

void read_all(int fd) {
    int bytes = 0;
    while (true) {
        char c;
        int r = read(fd, &c, 1);
        if (r > 0) {
            bytes += r;
        } else if (r < 0) {
            assert(errno == EAGAIN);
        } else {
            break;
        }
    }
    log_printf("Read %d bytes\n", bytes);
}

int main() {
    log_printf("Start\n");
    int fd[2];
    assert(socketpair(AF_UNIX, SOCK_STREAM, 0, fd) == 0); //socketpair создает пару соединенных сокетов(по сути pipe)
    SWAP(fd[0], fd[1]); // can change order, it will work
    
    pid_t pid_1, pid_2;
    if ((pid_1 = fork()) == 0) {
        close(fd[1]);
        write_smth(fd[0]);
        shutdown(fd[0], SHUT_RDWR); // Важное дополнение к close в случае потоковых сокетов
        close(fd[0]);
        log_printf("Writing is done\n");
        sleep(1);
        return 0;
    }
    if ((pid_2 = fork()) == 0) {
        close(fd[0]);
        read_all(fd[1]);
        shutdown(fd[1], SHUT_RDWR);
        close(fd[1]);
        return 0;
    }
    close(fd[0]);
    close(fd[1]);
    int status;
    assert(waitpid(pid_1, &status, 0) != -1);
    assert(waitpid(pid_2, &status, 0) != -1);
    return 0;
}

