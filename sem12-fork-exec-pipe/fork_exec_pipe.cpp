// %%cpp fork_exec_pipe.cpp

#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/types.h>


int main() {
    int fd = open("out.txt", O_WRONLY | O_CREAT, 0664);
    dup2(fd, 1); // redirect stdout to file
    close(fd);
    printf("Redirectred 'Hello world!'");
    return 0;
}

