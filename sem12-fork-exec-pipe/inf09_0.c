// %%cpp inf09_0.c --ejudge-style

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
    pid_t process_id = 0;
    int status;
    int process_counter = 1;

    process_id = fork();
    int a = waitpid(process_id, &status, 0);
    printf("pid=%d, fres=%d, res=%d\n", getpid(), process_id, a);
    char buf[1200];
    sprintf(buf, "pid=%d ", getpid());
    perror(buf);

    return 0;
}

// line without \n