// %%cpp jmp2.cpp
// %run gcc jmp2.cpp -o jmp2.exe
// %run ./jmp2.exe

#include <stdio.h>
#include <setjmp.h>
#include <assert.h>
#include <inttypes.h>
#include <signal.h>


jmp_buf segv_catcher_jmp_buf;

void segv_catcher_handler(int signum)
{
    dprintf(2, "Got signal %d\n", signum);
//         sigset_t signals;
//         sigemptyset(&signals);
//         sigaddset(&signals, signum);
//         sigprocmask(SIG_UNBLOCK, &signals, nullptr);
    longjmp(segv_catcher_jmp_buf, 1);
}


void segv_catcher_init() {
    static uint8_t segv_catcher_stack[1024];
    stack_t ss;
    ss.ss_sp = segv_catcher_stack;
    ss.ss_flags = 0;
    ss.ss_size = sizeof(segv_catcher_stack);
    sigaltstack(&ss, NULL);
    
    struct sigaction sa{};
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_ONSTACK;
    sa.sa_handler = segv_catcher_handler;
    sigaction(SIGSEGV, &sa, NULL);
}

int segv_catcher_setjmp() {
    return setjmp(segv_catcher_jmp_buf);
}

int f(int i) {
    return f(i + 1) + 1;
}

int main() {
    segv_catcher_init();
    
    if (!segv_catcher_setjmp()) {
        printf("Call f()\n");
        f(0);
        printf("Success call of f()\n");
    } else {
        printf("Failed call of f() - segfault\n");
    }
    return 0;
}

