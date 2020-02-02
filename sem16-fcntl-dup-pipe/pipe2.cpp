// %%cpp pipe2.cpp
// %run gcc pipe2.cpp -o pipe2.exe
// %run ./pipe2.exe
// %run diff pipe.cpp pipe2.cpp  | grep -v "// %" | grep -e '>' -e '<' -C 1

#ifndef _GNU_SOURCE
  #define _GNU_SOURCE
#endif
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
    pipe2(fd, O_CLOEXEC); // O_CLOEXEC - created file descriptors will be closed on exec call
    pid_t pid_1, pid_2;
    if ((pid_1 = fork()) == 0) {
        dup2(fd[1], 1); // dup2 doesn't copy O_CLOEXEC attribute
        // dup3(fd[1], 1, O_CLOEXEC); // can compare with this
        execlp("ps", "ps", "aux", NULL);
        assert(0 && "Unreachable position in code if execlp succeeded");
    }
    if ((pid_2 = fork()) == 0) {
        // no close calls here
        dup2(fd[0], 0);
        execlp("tail", "tail", "-n", "4", NULL);
        assert(0 && "Unreachable position in code if execlp succeeded");
    }
    close(fd[0]);
    close(fd[1]);
    int status;
    assert(waitpid(pid_1, &status, 0) != -1);
    assert(waitpid(pid_2, &status, 0) != -1);
    return 0;
}

