// %%cpp tmp.c --ejudge-style
// %run gcc tmp.c -o tmp.exe

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main()
{
    // setvbuf(stdin, NULL, _IONBF, 0); // а с этим может работать
    pid_t pid;
    int result = 0;
    while (1) {
        pid = fork();
        if (pid == 0) {
            char buffer[4097];
            int length = scanf("%s", buffer);
            return (length == EOF) ? 0 : 1;
        } else {
            int status;
            waitpid(pid, &status, 0);
            if (status == 0) {
                break;
            }
            result += WEXITSTATUS(status);
        }
    }
    printf("%d\n", result);
    return 0;
}

// line without \n