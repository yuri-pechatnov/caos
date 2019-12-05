// %%cpp premoderate.c
// %run gcc premoderate.c -o premoderate.exe

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

#define safe_ptrace(...) { int __ret = ptrace(__VA_ARGS__); if (__ret == -1) { \
    char buf[10000]; sprintf(buf, "ptrace failed, line=%d", __LINE__); perror(buf); abort(); }}


static void
premoderate_write_syscall(pid_t pid, struct user_regs_struct state)
{
    size_t orig_buf = state.rsi;   // ecx for i386
    size_t size = state.rdx;       // rdx for i386
    char *buffer = calloc(size + sizeof(long), sizeof(*buffer));
    int val = 0;
    for (size_t i = 0; i < size; ++i) {
        buffer[i] = ptrace(PTRACE_PEEKDATA, pid, orig_buf + i, NULL);
    }
    char *bad_word;
    if (bad_word = strstr(buffer, "3")) {
         size_t offset = bad_word - buffer; // 'u' letter
         buffer[offset] = '5';                      
         size_t target_address = orig_buf + offset;
         long val;
         memcpy(&val, buffer + offset, sizeof(val));
         safe_ptrace(PTRACE_POKEDATA, pid, target_address, val);
    }
    free(buffer);
}

int main(int argc, char *argv[])
{
    pid_t pid = fork();
    if (-1 == pid) { perror("fork"); exit(1); }
    if (0 == pid) {
        safe_ptrace(PTRACE_TRACEME, 0, NULL, NULL);
        execvp(argv[1], argv + 1);
        perror("exec");
        exit(2);
    }  
    int wstatus = 0;
    struct user_regs_struct state;
    bool stop = false;
    while (!stop) {
        ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
        waitpid(pid, &wstatus, 0);
        stop = WIFEXITED(wstatus);
        if (WIFSTOPPED(wstatus)) {
            ptrace(PTRACE_GETREGS, pid, 0, &state);
            if (__NR_write == state.orig_rax) {  // orig_eax for i386
                premoderate_write_syscall(pid, state);
            }              
        }
    }  
}

