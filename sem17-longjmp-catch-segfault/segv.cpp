// %%cpp segv.cpp
// %run gcc segv.cpp -o segv.exe
// %run ./segv.exe

#include <stdio.h>
#include <setjmp.h>
#include <assert.h>
#include <inttypes.h>
#include <signal.h>


jmp_buf segv_catcher_jmp_buf;

void segv_catcher_handler(int signum)
{
    // разблокируем пришедший сигнал вручную, 
    // так как из обработчика мы не выйдем нормальным образом
    sigset_t signals;
    sigemptyset(&signals);
    sigaddset(&signals, signum);
    sigprocmask(SIG_UNBLOCK, &signals, nullptr);
    
    dprintf(2, "Got signal %d\n", signum);
    
    longjmp(segv_catcher_jmp_buf, 1);
}


void segv_catcher_init() {
    // отдельный стек для обработчика сигнала
    // иначе при получении сегфолта из-за переполнения стека 
    // обработчику сигнала будет негде работать
    static uint8_t segv_catcher_stack[65536];
    stack_t ss;
    ss.ss_sp = segv_catcher_stack;
    ss.ss_flags = 0;
    ss.ss_size = sizeof(segv_catcher_stack);
    sigaltstack(&ss, NULL);
    
    // обработчик сигнала сегфолта
    struct sigaction sa{};
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_ONSTACK;
    sa.sa_handler = segv_catcher_handler;
    sigaction(SIGSEGV, &sa, NULL);
}

int f(int i) {
    return f(i + 1) + 1;
}

int f2(int i) {
    return i > 10 ? 1 : f2(i + 1) + 1;
}

int main() {
    segv_catcher_init();
    
    if (!setjmp(segv_catcher_jmp_buf)) {
        printf("Call f()\n");
        int res = f(0);
        printf("Success call of f(0) = %d\n", res);
    } else {
        printf("Failed call of f() - segfault\n");
    }
    
    if (!setjmp(segv_catcher_jmp_buf)) {
        printf("Call f()\n");
        int res = f(0);
        printf("Success call of f(0) = %d\n", res);
    } else {
        printf("Failed call of f() - segfault\n");
    }
    
    if (!setjmp(segv_catcher_jmp_buf)) {
        printf("Call f2()\n");
        int res = f2(0);
        printf("Success call of f2(0) = %d\n", res);
    } else {
        printf("Failed call of f2() - segfault\n");
    }
    
    return 0;
}

