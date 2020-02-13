// %%cpp pipe.cpp
// %run gcc pipe.cpp -o pipe.exe
// %run ./pipe.exe

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
        close(fd[0]); // notice: close file descriptors explicitly
        close(fd[1]); // try to comment out and compare behaviour
        // Даже без закрытия сможет отработать (но не надо так делать, лучше всегда закрывать руками)
        execlp("ps", "ps", "aux", NULL);
        assert(0 && "Unreachable position in code if execlp succeeded");
    }
    if ((pid_2 = fork()) == 0) {
        dup2(fd[0], 0);
        close(fd[0]); // notice: close file descriptors explicitly
        close(fd[1]); // try to comment out and compare behaviour
        // ^^^ tail не завершиться пока открыт файловый дескриптор на запись в pipe (он будет ждать данных, которые он бы смог прочитать)
        execlp("tail", "tail", "-n", "4", NULL);
        assert(0 && "Unreachable position in code if execlp succeeded");
    }
    close(fd[0]); // Тут закрыли pipe, потому что он нам больше не нужен (и потому что, если не закроем, то будет ошибка как с программой tail)
    close(fd[1]);
    int status;
    assert(waitpid(pid_1, &status, 0) != -1); // [1.]
    assert(waitpid(pid_2, &status, 0) != -1);
    return 0;
}

