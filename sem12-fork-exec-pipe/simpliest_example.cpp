// %%cpp simpliest_example.cpp

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <sys/wait.h>
#include <sched.h>

int main() {
    pid_t pid = fork();
//     if (pid != 0) {
//         for (int i = 0; i < 1000000; ++i) {
//             sched_yield();
//         }
//     }
    printf("Hello world! fork result (child pid) = %d, own pid = %d\n", pid, getpid()); // выполнится и в родителе и в ребенке
    
    if (pid == 0) {
        return 42; // если это дочерний процесс, то завершаемся
    }
    int status;
    pid_t w = waitpid(pid, &status, 0); // обязательно нужно дождаться, пока завершится дочерний процесс
    if (w == -1) {
        perror("waitpid");
        exit(-1);
    }
    assert(WIFEXITED(status));
    printf("Child exited with code %d\n", WEXITSTATUS(status)); // выводим код возврата дочернего процесса
    return 0;
}

