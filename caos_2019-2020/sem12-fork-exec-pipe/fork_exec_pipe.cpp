// %%cpp fork_exec_pipe.cpp
// %run gcc fork_exec_pipe.cpp -o fork_exec_pipe.exe
// %run ./fork_exec_pipe.exe

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>


int main() {
    int fd[2];
    pipe(fd); // fd[0] - in, fd[1] - out (like stdin=0, stdout=1)
    pid_t pid_1, pid_2;
    if ((pid_1 = fork()) == 0) {
        dup2(fd[1], 1);
        close(fd[0]);
        close(fd[1]);
        execlp("ps", "ps", "aux", NULL);
        assert(0 && "Unreachable position in code if execlp succeeded");
    }
    if ((pid_2 = fork()) == 0) {
        dup2(fd[0], 0);
        close(fd[0]);
        close(fd[1]);
        execlp("head", "head", "-n", "4", NULL);
        assert(0 && "Unreachable position in code if execlp succeeded");
    }
    close(fd[0]);
    close(fd[1]);
    int status;
    assert(waitpid(pid_1, &status, 0) != -1);
    assert(waitpid(pid_2, &status, 0) != -1);
    return 0;
}

