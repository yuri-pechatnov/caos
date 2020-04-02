// %%cpp signalfd.c
// %run gcc -g signalfd.c -o signalfd.exe
// %run timeout -s SIGINT 1 timeout -s SIGTERM 2  ./signalfd.exe 

#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/signalfd.h>

int main() {
    sigset_t mask;
    sigfillset(&mask);
    sigdelset(&mask, SIGCONT);
    sigprocmask(SIG_BLOCK, &mask, NULL);
    // сводим получение сигналов к файловому дескриптору
    int fd = signalfd(-1, &mask, 0);
    
    struct signalfd_siginfo fdsi;
    while (1) {
        read(fd, &fdsi, sizeof(struct signalfd_siginfo));
        printf("Got signal %d\n", fdsi.ssi_signo);
        if (fdsi.ssi_signo == SIGTERM) {
            printf(" ... and it is SIGTERM\n");
            break;
        }
    }
    return 0;
}

