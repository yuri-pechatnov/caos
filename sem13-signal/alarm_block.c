// %%cpp alarm_block.c

#include <unistd.h>
#include <stdio.h>
#include <signal.h>

int main() {
    signal(SIGALRM, SIG_IGN);
    alarm(3);
    pause();
    printf("Is this command unreachable?"); // достижима ли эта команда?
    return 0;
}

