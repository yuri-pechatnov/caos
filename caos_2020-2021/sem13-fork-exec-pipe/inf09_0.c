// %%cpp inf09_0.c --ejudge-style
// %run gcc inf09_0.c -o inf09_0.exe
// %run sudo useradd tmp_user #// Создаем временного пользователя
// %// Из-под временного пользователя устанавливаем лимит и запускаем опасную программу
// %// Обратите внимание, что от имени пользователя в первую очередь запускается bash, а остальное уже запускает сам bash
// %run sudo -u tmp_user bash -c 'ulimit -u 5 ; ./inf09_0.exe'
// %run sudo userdel tmp_user #// Удаляем временного пользователя

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main()
{
    for (int i = 1; 1; ++i) {
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

// line without \n