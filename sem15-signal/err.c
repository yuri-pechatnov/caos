// %%cpp err.c
// %run gcc -g err.c -o err.exe
// %run timeout -s SIGINT 3 timeout -s SIGUSR1 1 ./err.exe # через секунду SIGUSR1, через 3 секунды SIGKILL

#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>

volatile sig_atomic_t must_exit = 0;

static void handler(int signum) {
    must_exit = 1;
    dprintf(2, "Catch signal\n");
}

int main() {
    sigaction(SIGUSR1, &(struct sigaction){.sa_handler = handler, .sa_flags = SA_RESTART}, NULL);
    dprintf(2, "Start\n");
    while (!must_exit) {
        sleep(2); // сигнал SIGUSR1 может прийти и быть обработанным тут
                  // вместо sleep тут могло быть обычное переключение контекста на другой процесс и обратно
        pause(); // и тогда вот тут все зависнет навсегда (до SIGINT)
    }
    printf("Stopped normally"); // до сюда не дошло, аварийно завершились SIGINT-ом
    return 0;
}

