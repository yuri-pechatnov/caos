// %%cpp run_with_unreliable_io.c
// %run gcc run_with_unreliable_io.c -o run_with_unreliable_io.exe

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdbool.h>
#include <asm/unistd.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <time.h>


#define safe_ptrace(...) { int __ret = ptrace(__VA_ARGS__); if (__ret == -1) { \
    char buf[10000]; sprintf(buf, "ptrace failed, line=%d", __LINE__); perror(buf); abort(); }}

int main(int argc, char *argv[])
{
    pid_t pid = fork();
    if (-1 == pid) { perror("fork"); exit(1); }
    if (0 == pid) {
        safe_ptrace(PTRACE_TRACEME, 0, NULL, NULL);
        execvp(argv[2], argv + 2);
        perror("exec");
        exit(2);
    }  
    srand(time(NULL));
    int enable_retryable_errors;
    sscanf(argv[1], "%d", &enable_retryable_errors);
    
    int wstatus = 0;
    waitpid(pid, &wstatus, 0);
    struct user_regs_struct state;
    bool stop = false;
    int reads_count = 0;
    while (!stop) {
        ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
        waitpid(pid, &wstatus, 0);
        stop = WIFEXITED(wstatus);
        if (WIFSTOPPED(wstatus)) {
            if (ptrace(PTRACE_GETREGS, pid, 0, &state) == -1) return 0;
            if (state.rdi != 2 && (__NR_write == state.orig_rax || __NR_read == state.orig_rax)) {  // orig_eax for i386
                if (__NR_read == state.orig_rax) {
                    ++reads_count;
                    if (reads_count <= 2) continue;
                }
                if (rand() % 3 != 0) {
                    unsigned long long old_rdx = state.rdx;
                    if (state.rdx > 1 && rand() % 2 == 0) {
                        state.rdx = 1 + rand() % ((state.rdx + 4) / 5);
                        ptrace(PTRACE_SETREGS, pid, 0, &state);
                    } 
                    ptrace(PTRACE_SINGLESTEP, pid, 0, 0);
                    waitpid(pid, &wstatus, 0);
                    ptrace(PTRACE_GETREGS, pid, 0, &state); // Вот тут был баг на семинаре (не было этой строчки), я терял возвращенное в eax значение
                    // возвращаем как было, чтобы логика самой программы не поменялась
                    state.rdx = old_rdx;
                    ptrace(PTRACE_SETREGS, pid, 0, &state); 
                } else if (enable_retryable_errors) {
                    unsigned long long old_rdx = state.rdx;
                    unsigned long long old_rdi = state.rdi;
                    state.rdx = 0;
                    state.rdi = 100500; // not existing file descriptor
                    ptrace(PTRACE_SETREGS, pid, 0, &state);
                    safe_ptrace(PTRACE_SINGLESTEP, pid, 0, 0);
                    waitpid(pid, &wstatus, 0); // важно! делать wait
                    state.rax = -EINTR;
                    state.rdx = old_rdx;
                    state.rdi = old_rdi;
                    ptrace(PTRACE_SETREGS, pid, 0, &state); 
                }
            }              
        }
    }  
}

