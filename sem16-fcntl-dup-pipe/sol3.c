// %%cpp sol3.c --ejudge-style
#include <zconf.h>
#include <stdlib.h>
#include <wait.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>

void close_pipe(int pipe_des[2]) {
    close(pipe_des[0]);
    close(pipe_des[1]);
}

void error(pid_t *pids, int count) {
#ifdef _DEBUG
    perror("");
#endif
    if (count != -1) {
        for (size_t i = 0; i != count; ++i) {
            kill(pids[i], SIGKILL);
        }
    }
    int wstatus;
    while (waitpid(-1, &wstatus, 0) > 0) {}
    _exit(1);
}

int main(int argc, char **argv) {
    if (argc == 1) {
        _exit(0);
    }

    if (argc == 2) {
        execlp(argv[1], argv[1], NULL);
        _exit(0);
    }

    int pipe_des[2];
    pid_t *pids = calloc(argc - 1, sizeof(pid_t));
    int count = 0;
    int last_out_descriptor = STDIN_FILENO;

    for (size_t i = 1; i != argc; ++i) {
        if (pipe(pipe_des) < 0) {
            error(pids, count);
        }

        pid_t child;

        if ((child = fork()) < 0) {
            error(pids, count);
        } else if (!child) { // we are in child process
            if (i == 1) { // first command
                dup2(pipe_des[1], 1);
                close_pipe(pipe_des);
            } else if (i == argc - 1) { // last command
                dup2(last_out_descriptor, 0);
                close_pipe(pipe_des);
                close(last_out_descriptor);
            } else { // others
                dup2(last_out_descriptor, 0);
                dup2(pipe_des[1], 1);
                close(last_out_descriptor);
                close(pipe_des[1]);
            }
            execlp(argv[i], argv[i], NULL);
            _exit(1);
        } else {
            if (i == 1) { // first command
                close(pipe_des[1]);
            } else if (i == argc - 1) { // last command
                close(last_out_descriptor);
                close_pipe(pipe_des);
            } else { // others
                close(last_out_descriptor);
                close(pipe_des[1]);
            }
            last_out_descriptor = pipe_des[0];
            pids[i - 1] = child;
            count++;
        }
    }

    int wstatus;
    while (waitpid(-1, &wstatus, 0) > 0) {}
    _exit(0);
}

// line without \n