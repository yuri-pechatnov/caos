// %%cpp alarm_block.c
// %run gcc -g alarm_block.c -o alarm_block.exe
// %run timeout -s SIGKILL 5 ./alarm_block.exe ; echo $? # выводим так же код возврата

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

