// %%cpp tmp.c --ejudge-style
// %run gcc tmp.c -o tmp.exe

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main()
{
    pid_t pid = 0;
    char str[4097];
    int num = 0;
    int status = 0;
    int is_eof = scanf("%s", str);
    while (is_eof != EOF) {
        pid = fork();
        if (pid == -1) {
            fprintf(stderr, "Can't do fork\n");
        }
        if (pid == 0) {
            is_eof = scanf("%s", str);
            if (is_eof == EOF) {
                return 0;
            }
        } else {
            waitpid(pid, &status, 0);
            break;
        }
        ++num;
    }

    if (num != 0) {
        return WEXITSTATUS(status) + 1;
    }
    if (is_eof == EOF) {
        printf("0\n");
    } else {
        printf("%d\n", WEXITSTATUS(status) + 1);
    }

    return 0;
}

// line without \n