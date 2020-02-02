// %%cpp fcntl_dup.cpp
// %run gcc fcntl_dup.cpp -o fcntl_dup.exe
// %run ./fcntl_dup.exe
// %run echo "After program finish" && cat out.txt
// %run diff dup2.cpp fcntl_dup.cpp  | grep -v "// %" | grep -e '>' -e '<' -C 1

#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/types.h>


int main() {
    int fd = open("out.txt", O_WRONLY | O_CREAT | O_TRUNC, 0664);
    close(1); // important
    int fd_copy = fcntl(fd, F_DUPFD, 1);
    assert(fd_copy == 1);
    close(fd);
    printf("Redirectred 'Hello world!'");
    return 0;
}

