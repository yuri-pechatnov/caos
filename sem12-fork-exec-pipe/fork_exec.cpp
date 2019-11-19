// %%cpp fork_exec.cpp

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<assert.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>


int main() {
    pid_t pid;
    if ((pid = fork()) == 0) {
        //execlp("ps", "ps", "aux", NULL); // also possible variant
        //execlp("echo", "echo", "Hello world from linux ECHO program", NULL);
        //execlp("sleep", "sleep", "3", NULL);
        execlp("bash", "bash", "-c", "ps aux | head -n 4", NULL);
        assert(0 && "Unreachable position in code if execlp succeeded");
    }
    int status;
    struct rusage resource_usage;
    pid_t w = wait4(pid, &status, 0, &resource_usage); // обязательно нужно дождаться, пока завершится дочерний процесс
    if (w == -1) {
        perror("waitpid");
        exit(-1);
    }
    assert(WIFEXITED(status));
    printf("Child exited with code %d \n"
           "\tUser time %ld sec %ld usec\n"
           "\tSys time %ld sec %ld usec\n", 
           WEXITSTATUS(status), 
           resource_usage.ru_utime.tv_sec,
           resource_usage.ru_utime.tv_usec,
           resource_usage.ru_stime.tv_sec,
           resource_usage.ru_stime.tv_usec); // выводим код возврата дочернего процесса
    
    return 0;
}

