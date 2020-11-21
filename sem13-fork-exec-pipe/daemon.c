// %%cpp daemon.c --ejudge-style
// %run gcc daemon.c -o daemon.exe

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

void do_daemon_job(const char* mode) {
    freopen("log.txt", "wt", stdout);
    while (1) {
        printf("I'm alive, my mode is %s, my pid is %d\n", mode, (int)getpid());
        fflush(stdout);
        sleep(1);
    }
}

int main(int argc, char **argv)
{
    assert(argc == 2);
    if (strcmp(argv[1], "foreground") == 0) {
        do_daemon_job(argv[1]);
    } else if (strcmp(argv[1], "background") == 0) {
        int pid = fork(); // первый fork
        assert(pid >= 0);
        if (pid > 0) {
            int status;
            pid_t w = waitpid(pid, &status, 0);
            assert(w != -1 && WIFEXITED(status) && WEXITSTATUS(status) == 0); // проверяем, что промежуточный процесс завершился корректно
            return 0; // оригинальная программа завершается
        }
        setsid(); // создаём новый сеанс/сессию
        pid = fork(); // второй fork
        assert(pid >= 0);
        if (pid > 0) {
            // ребенка не ждем - он будет полноценным демоном, его статус после смерти получит PID 1
            return 0; // лидер новой сессии завершается
        }
        // теперь мы процесс отвязанный от всех терминалов и легко к ним не привяжемся

        close(STDIN_FILENO); // можно закрыть все стандартные дескрипторы - они больше не нужны
        close(STDOUT_FILENO);
        close(STDERR_FILENO);

        do_daemon_job(argv[1]);
    } else {
        assert(0 && "No such option");
    }
    return 0;
}

// line without \n