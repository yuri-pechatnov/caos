// %%cpp custom_stack.c
// %run gcc custom_stack.c -o custom_stack.exe
// %run ./custom_stack.exe 1000000


#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <inttypes.h>

    
void change_stack_size(uint64_t size, char** argv) {
    struct rlimit rlim;
    getrlimit(RLIMIT_STACK, &rlim);
    assert(RLIM_INFINITY == rlim.rlim_max || size <= rlim.rlim_max);
    
    //printf("size=%" PRIu64 " cur=%" PRIu64 "\n", size, rlim.rlim_cur);
    if (rlim.rlim_cur >= size) {
        return;
    }
    rlim.rlim_cur = size;
    setrlimit(RLIMIT_STACK, &rlim);

    pid_t pid = fork();
    assert(pid != -1);
    if (pid == 0) {
        execvp(argv[0], argv);
        perror("execvp: ");
        assert(0 && "exec");
    }
    
    int status;
    pid_t w = waitpid(pid, &status, 0);
    if (w == -1) {
        perror("waitpid");
        exit(-1);
    }
    if (WIFEXITED(status)) {
        //printf("exit status %" PRIu64 "\n", (uint64_t)WEXITSTATUS(status));
        exit(WEXITSTATUS(status));
    } else if (WIFSIGNALED(status)) {
        kill(getpid(), WTERMSIG(status));            
    }
}
    
uint64_t factorial(uint64_t n) {
    if (n <= 0) {
        return 1;
    }
    return (n % 13) * factorial(n - 1) % 13;
} 
    
int main(int argc, char** argv)
{
    assert(argc == 2);
    uint64_t n = strtoull(argv[1], NULL, 10);
    change_stack_size(n * 64 + 1000000, argv);
    printf("factorial(%" PRIu64 ") %% 13 == %" PRIu64 "\n", n, factorial(n));
}

