// %%cpp server_sol.c --ejudge-style
//%run gcc server_sol.c -o server_sol.exe
//%run ./server_sol.exe 30045

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <wait.h>
#include <sys/epoll.h>
#include <assert.h>


#define conditional_handle_error(stmt, msg) \
    do { if (stmt) { perror(msg " (" #stmt ")"); exit(EXIT_FAILURE); } } while (0)

//...

// должен работать до тех пор, пока в stop_fd не появится что-нибудь доступное для чтения
int server_main(int argc, char** argv, int stop_fd) {
    assert(argc >= 2);

    //...

    int epoll_fd = epoll_create1(0);
    {
        int fds[] = {stop_fd, socket_fd, -1};
        for (int* fd = fds; *fd != -1; ++fd) {
            struct epoll_event event = {
                .events = EPOLLIN | EPOLLERR | EPOLLHUP,
                .data = {.fd = *fd}
            };
            epoll_ctl(epoll_fd, EPOLL_CTL_ADD, *fd, &event);
        }
    }

    while (true) {
        struct epoll_event event;
        int epoll_ret = epoll_wait(epoll_fd, &event, 1, 1000); // Читаем события из epoll-объект (то есть из множества файловых дескриптотров, по которым есть события)
        if (epoll_ret <= 0) {
            continue;
        }
        if (event.data.fd == stop_fd) {
            break;
        }

        int fd = accept(socket_fd, NULL, NULL);
        // ...
        shutdown(fd, SHUT_RDWR);
        close(fd);
    }

    shutdown(socket_fd, SHUT_RDWR);
    close(socket_fd);

    return 0;
}

// Основную работу будем делать в дочернем процессе.
// А этот процесс будет принимать сигналы и напишет в пайп, когда пора останавливаться
// (Кстати, лишний процесс и пайп можно было заменить на signalf, но это менее портируемо)
int main(int argc, char** argv) {
    sigset_t full_mask;
    sigfillset(&full_mask);
    sigprocmask(SIG_BLOCK, &full_mask, NULL);

    int fds[2];
    assert(pipe(fds) == 0);

    int child_pid = fork();
    assert(child_pid >= 0);
    if (child_pid == 0) {
        close(fds[1]);
        server_main(argc, argv, fds[0]);
        return 0;
    } else {
        close(fds[0]);
        while (1) {
            siginfo_t info;
            sigwaitinfo(&full_mask, &info);
            int received_signal = info.si_signo;
            if (received_signal == SIGTERM || received_signal == SIGINT) {
                int written = write(fds[1], "X", 1);
                conditional_handle_error(written != 1, "writing failed");
                close(fds[1]);
                break;
            }
        }
        int status;
        assert(waitpid(child_pid, &status, 0) != -1);
    }
    return 0;
}

// line without \n