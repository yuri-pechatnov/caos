// %%cpp pipe_err.c
// %run gcc pipe_err.c -o pipe_err.exe
// %run ./pipe_err.exe ; echo  "Exit code is $?"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sched.h>
#include <time.h>
#include <errno.h>
#include <signal.h>
    
int main() {
    signal(SIGPIPE, SIG_DFL); // форсим дефолтное поведение, так как тут наследуется маска сигналов питона
    
    int size = 100000;
    int fd[2];
    pipe(fd); 

    close(fd[0]);
    char* data = (char*)calloc(size, sizeof(char));
    
    int ret = write(fd[1], data, size);
    if (ret == size) {
        printf("OK\n");
    } else {
        printf("ret = %d\n", ret);
        perror("FAILED to write");
    }
    
    free(data);
    
    int status;
    
    close(fd[1]);
    return 0;
}

