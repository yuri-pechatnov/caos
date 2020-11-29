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


int check_code(int code, const char* file, int line, const char* text) {
    if (code == -1) {
        char buf[10000]; 
        sprintf(buf, "%s failed (%s:%d)", text, file, line); 
        perror(buf); 
        abort();
    }
    return code;
}
    
#define checked(call) check_code(call, __FILE__, __LINE__, #call)


int main(int argc, char *argv[])
{
    pid_t pid = checked(fork());
    if (0 == pid) {
        checked(ptrace(PTRACE_TRACEME, 0, NULL, NULL));
        checked(execvp(argv[3], argv + 3));
    }  
    srand(time(NULL));
    int always_partial = strtol(argv[1], NULL, 10);
    int enable_retryable_errors = strtol(argv[2], NULL, 10);
    
    int wstatus = 0;
    checked(waitpid(pid, &wstatus, 0));

    int reads_count = 0, writes_count = 0;
    do {
        checked(ptrace(PTRACE_SYSCALL, pid, NULL, NULL));
        checked(waitpid(pid, &wstatus, 0));
        if (WIFSTOPPED(wstatus)) {
            struct user_regs_struct state;
            checked(ptrace(PTRACE_GETREGS, pid, 0, &state));
            bool is_read = (__NR_read == state.orig_rax), is_write = (__NR_write == state.orig_rax);
            reads_count += is_read;
            writes_count += is_write;
            if (!(is_read || is_write) || (state.rdi == STDERR_FILENO) || (is_read && reads_count <= 2)) 
                continue;
              
            if (enable_retryable_errors && ((is_read && (reads_count & 1)) || (is_write && !(writes_count & 1)))) {
                unsigned long long old_rdx = state.rdx;
                unsigned long long old_rdi = state.rdi;
                state.rdx = 0;
                state.rdi = 100500; // not existing file descriptor
                checked(ptrace(PTRACE_SETREGS, pid, 0, &state));
                checked(ptrace(PTRACE_SINGLESTEP, pid, 0, 0));
                waitpid(pid, &wstatus, 0); // важно! делать wait
                state.rax = -EINTR;
                state.rdx = old_rdx;
                state.rdi = old_rdi;
                checked(ptrace(PTRACE_SETREGS, pid, 0, &state)); 
            } else if (always_partial || (random() % 3 != 0)) {
                unsigned long long old_rdx = state.rdx;
                if (state.rdx > 1) {
                    state.rdx = 1 + rand() % ((state.rdx + 4) / 5);
                    checked(ptrace(PTRACE_SETREGS, pid, 0, &state));
                } 
                checked(ptrace(PTRACE_SINGLESTEP, pid, 0, 0));
                waitpid(pid, &wstatus, 0);
                checked(ptrace(PTRACE_GETREGS, pid, 0, &state));
                // возвращаем как было, чтобы логика самой программы не поменялась
                state.rdx = old_rdx;
                checked(ptrace(PTRACE_SETREGS, pid, 0, &state)); 
            }          
        }
    } while (!WIFEXITED(wstatus));
}

