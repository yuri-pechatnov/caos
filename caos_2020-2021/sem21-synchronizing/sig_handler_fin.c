// %%cpp sig_handler_fin.c
// %run gcc -g -O0 sig_handler_fin.c -lpthread -o sig_handler_fin.exe
// %run # ./sig_handler_fin.exe

#include <unistd.h>
#include <stdio.h>
#include <signal.h>

volatile sig_atomic_t resource = -1;

int resource_acquire() { // здесь захватывается какой-то ресурс, который обязательно нужно освободить
    static int res = 100; ++res;
    dprintf(2, "Resource %d acquired\n", res);
    return res;
}
void resource_release(int res) { // здесь освобождается
    dprintf(2, "Resource %d released\n", res);
}

static void handler(int signum) {
    if (signum == SIGUSR1) return;
    if (resource != -1) resource_release(resource);
    _exit(0);
}

int main(int argc, char* argv[]) {
    int signals[] = {SIGINT, SIGTERM, SIGUSR1, 0};
    for (int* signal = signals; *signal; ++signal) {
        sigaction(*signal, &(struct sigaction){.sa_handler=handler}, NULL);
    }
    while (1) {
        resource = resource_acquire(); // ~ accept
        sleep(1);
        resource_release(resource); // ~ shutdown & close
    }
    return 0;
}

