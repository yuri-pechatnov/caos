// %%cpp inf09_0.cpp

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>


int main() {
    for (int i = 1; true; ++i) {
        int pid = fork();
        fflush(stdout);
        if (pid < 0) {
            printf("%d\n", i);
            return 0;
        }
        if (pid != 0) {
            int status;
            assert(waitpid(pid, &status, 0) != -1);
            break;
        }
    }
    return 0;
}

