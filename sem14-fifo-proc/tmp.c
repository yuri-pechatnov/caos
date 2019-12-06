// %%cpp tmp.c --ejudge-style
//
// Created by dgolear on 11/30/19.
//

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <zconf.h>

int main(int argc, char** argv)
{
    if (argc < 3) {
        fprintf(stderr, "Not enough arguments\n");
        return -1;
    }
    int32_t n = strtol(argv[2], NULL, 10);
    if (-1 == mkfifo(argv[1], 0664)) {
        perror("mkfifo(): ");
        return -1;
    }
    sigaction(
        SIGPIPE,
        &(struct sigaction){.sa_handler = SIG_IGN, .sa_flags = SA_RESTART},
        NULL);

    int pid;
    scanf("%d", &pid);
    kill(pid, SIGHUP);

    int fd = open(argv[1], O_RDWR);
    if (fd == -1) {
        perror("open FIFO: ");
        return -1;
    }

    int i;
    for (i = 0; i <= n; ++i) {
        int wrote;
        if (i != n) {
            wrote = dprintf(fd, "%d ", i);
        } else {
            wrote = dprintf(fd, "%d", i);
        }
        if (wrote < 0) {
            if (errno == EPIPE) {
                printf("%d\n", i);
                return 0;
            }
            perror("dprintf: ");
            return 0;
        }
    }
    printf("%d\n", i);
    close(fd);
    return 0;
}

// line without \n