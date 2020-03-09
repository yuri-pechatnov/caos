// %%cpp alarm.c
// %run gcc -g alarm.c -o alarm.exe
// %run timeout -s SIGKILL 5 ./alarm.exe ; echo $? # выводим так же код возврата

#include <unistd.h>
#include <stdio.h>

int main() {
    alarm(3);
    pause();
    printf("Is this command unreachable?"); // достижима ли эта команда?
    return 0;
}

