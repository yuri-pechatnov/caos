// %%cpp redirect.cpp

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>


int main(int argc, char** argv) {
    assert(argc >= 2);
    int fd = open(argv[1], O_WRONLY | O_CREAT | O_TRUNC, 0664);
    assert(fd >= 0);
    dup2(fd, 1);
    close(fd);
    execvp(argv[2], argv + 2);
    assert(0 && "Unreachable position in code if execlp succeeded");
}

