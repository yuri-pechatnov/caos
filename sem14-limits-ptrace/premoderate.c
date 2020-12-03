// %%cpp premoderate.c
// %run gcc -m64 premoderate.c -o premoderate.exe
// %run ./premoderate.exe echo "Vasya got 3 in math and 3 in russian"

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
#include <assert.h>

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

static void premoderate_write_syscall(pid_t pid, struct user_regs_struct state) {
    size_t orig_buf = state.rsi;   
    size_t size = state.rdx;
    char* buffer = calloc(size + sizeof(size_t), sizeof(*buffer));
    for (size_t i = 0; i < size; i++) {
        *(size_t*)(buffer + i) = checked(ptrace(PTRACE_PEEKDATA, pid, orig_buf + i, NULL));
    }
    char* bad_char;
    while (bad_char = strchr(buffer, '3')) {
        *bad_char = '5';
        checked(ptrace(PTRACE_POKEDATA, pid, orig_buf + (bad_char - buffer), *(size_t*)bad_char));
    }

    free(buffer);
}

int main(int argc, char *argv[])
{
    pid_t pid = checked(fork());
    if (0 == pid) {
        checked(ptrace(PTRACE_TRACEME, 0, NULL, NULL));
        checked(execvp(argv[1], argv + 1));
    }  
    int wstatus = 0;
    checked(waitpid(pid, &wstatus, 0)); // Ждем пока дочерний процесс остановится на traceme
    do {
        checked(ptrace(PTRACE_SYSCALL, pid, NULL, NULL)); // Говорим, что будем ждать syscall и ждем
        checked(waitpid(pid, &wstatus, 0));
        if (WIFSTOPPED(wstatus)) {
            struct user_regs_struct state;
            checked(ptrace(PTRACE_GETREGS, pid, 0, &state));
            if (__NR_write == state.orig_rax) { 
                premoderate_write_syscall(pid, state);
            }              
        }
    } while (!WIFEXITED(wstatus));
}

