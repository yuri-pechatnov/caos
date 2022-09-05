// %%cpp sig_check.c
// %run gcc -g sig_check.c -o sig_check.exe
// %run # ./sig_check.exe 

#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>

int resource_acquire() {
    char c;
    read(0, &c, 1); // think, that here is accept-like logic (waiting external clients)
    dprintf(2, "Resource %d acquired\n", (int)c);
    return (int)c;
}
void resource_release(int res) {
    dprintf(2, "Resource %d released\n", res);
}

int main() {
    sigset_t full_mask;
    sigfillset(&full_mask);
    sigprocmask(SIG_BLOCK, &full_mask, NULL); 
    siginfo_t info;
    struct timespec timeout = {0};
    // В данном случае sigtimedwait проверяет, а не пришел ли сигнал. Работает без ожиданий
    while (sigtimedwait(&full_mask, &info, &timeout) < 0) {
        int resource = resource_acquire(); // ~ accept
        sleep(1);
        resource_release(resource); // ~ shutdown & close
    }   
    return 0;
}

