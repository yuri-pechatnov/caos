// %%cpp socketpair.cpp
// %run gcc socketpair.cpp -o socketpair.exe
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
#include <errno.h>
#include <time.h>

char* extract_t(char* s) { s[19] = '\0'; return s + 10; }
#define log_printf_impl(fmt, ...) { time_t t = time(0); dprintf(2, "%s : " fmt "%s", extract_t(ctime(&t)), __VA_ARGS__); }
#define log_printf(...) log_printf_impl(__VA_ARGS__, "")

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
    union {
        int arr_fd[2]; 
        struct {
            int fd_1; // ==arr_fd[0] can change order, it will work
            int fd_2; // ==arr_fd[1]
        };
    } fds;
    assert(socketpair(AF_UNIX, SOCK_STREAM, 0, fds.arr_fd) == 0);
    
    pid_t pid_1, pid_2;
    if ((pid_1 = fork()) == 0) {
        close(fds.fd_2);
        write_smth(fds.fd_1);
        shutdown(fds.fd_1, SHUT_RDWR); // important, try to comment out and look at time
        close(fds.fd_1);
        log_printf("Writing is done\n");
        sleep(3);
        return 0;
    }
    if ((pid_2 = fork()) == 0) {
        close(fds.fd_1);
        read_all(fds.fd_2);
        shutdown(fds.fd_2, SHUT_RDWR);
        close(fds.fd_2);
        return 0;
    }
    close(fds.fd_1);
    close(fds.fd_2);
    int status;
    assert(waitpid(pid_1, &status, 0) != -1);
    assert(waitpid(pid_2, &status, 0) != -1);
    return 0;
}

