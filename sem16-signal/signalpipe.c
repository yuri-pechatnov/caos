// %%cpp signalpipe.c
// %run gcc -g signalpipe.c -o signalpipe.exe
// %run timeout -s SIGINT 1 timeout -s SIGTERM 2  ./signalpipe.exe 

#define _GNU_SOURCE          
#include <fcntl.h>      
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/signalfd.h>
#include <errno.h>

static int signalpipe_fds[2] = {-1, -1};

static void signalpipe_handler(int signum) {
    // Если вы зараз получите умопомрачительное число сигналов, то можете переполнить буффер пайпа
    int written = write(signalpipe_fds[1], &signum, sizeof(int));
    if (written < 0) {
        if (errno != EAGAIN) {
            dprintf(2, "Strange error during writing to signal pipe");
            abort();
        }
        dprintf(2, "Pipe buffer is full, drop signal");
    } else if (written != 4) {
        dprintf(2, "Incomplete writing to signal pipe");
        abort();
    }
}

int signalpipe(int* signals) {
    pipe2(signalpipe_fds, O_CLOEXEC);
    // Делаем запись неблокирующей
    fcntl(signalpipe_fds[1], F_SETFL, fcntl(signalpipe_fds[1], F_GETFL, 0) | O_NONBLOCK);
    for (int* signal = signals; *signal; ++signal) {
        sigaction(*signal, &(struct sigaction){.sa_handler = signalpipe_handler, .sa_flags = SA_RESTART}, NULL);
    }
    return signalpipe_fds[0];
}

int main() {
    // Сводим получение сигналов к файловому дескриптору
    int signals[] = {SIGINT, SIGTERM, 0};
    int fd = signalpipe(signals);
    
    int signum;
    while (1) {
        assert(read(fd, &signum, sizeof(int)) == sizeof(int));
        printf("Got signal %d\n", signum);
        if (signum == SIGTERM) {
            printf(" ... and it is SIGTERM\n");
            break;
        }
    }
    
    // Закрывать fd (и парный к нему) не будем. 
    // Это синглтон на процесс, а при завершении процесса файловые дескрипторы сами закроются
    // При желании можно сделать, предварительно заблокировав сигналы.
    return 0;
}

