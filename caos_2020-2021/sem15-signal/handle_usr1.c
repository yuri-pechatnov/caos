// %%cpp handle_usr1.c
// %run gcc -g handle_usr1.c -o handle_usr1.exe

#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>

static void handler(int sig) {}

int main() {
    // handle SIGUSR1
    sigaction(SIGUSR1,
              &(struct sigaction){
                  .sa_handler = handler, 
                  .sa_flags = SA_RESTART
              },
              NULL);
    // ignore SIGUSR2
    sigaction(SIGUSR2,
              &(struct sigaction){
                  .sa_handler = SIG_IGN, 
                  .sa_flags = SA_RESTART
              },
              NULL);
    // block SIGINT
    sigset_t mask;
    sigaddset(&mask, SIGINT);
    sigprocmask(SIG_BLOCK, &mask, NULL);
    
    pause();
    printf("Finish\n");
    return 0;
}

