// %%cpp alarm_handle.cpp

#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>

static void
handler(int signum) {
    fprintf(stderr, "Get signal %d, do nothing\n", signum);
}

int main() {
    sigaction(SIGALRM,
              &(struct sigaction){
                  .sa_handler = handler, 
                  .sa_flags = SA_RESTART
              },
              NULL);
    alarm(3);
    pause();
    printf("Is this command unreachable?"); // достижима ли эта команда?
    return 0;
}

