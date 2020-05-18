


# PTRACE

Документация https://www.opennet.ru/base/dev/ptrace_guide.txt.html

Пример от Яковлева https://github.com/victor-yacovlev/mipt-diht-caos/tree/master/practice/exec-rlimit-ptrace


```cpp
%%cpp premoderate.c
%run gcc premoderate.c -o premoderate.exe

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
         size_t offset = bad_word - buffer; 
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
```


Run: `gcc premoderate.c -o premoderate.exe`



```python
!./premoderate.exe echo "Vasya got 3 in math"
```

    Vasya got 5 in math



```python

```

# Пример от меня, который может пригодиться при тестировании программ на надежность IO


```cpp
%%cpp run_with_unreliable_io.c
%run gcc run_with_unreliable_io.c -o run_with_unreliable_io.exe

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
```


Run: `gcc run_with_unreliable_io.c -o run_with_unreliable_io.exe`



```python
!./run_with_unreliable_io.exe 0 ./unreliable_write.exe
```

    Reliable print: Hello from C!
    Written 30 bytes by printf. errno=0, err=Success
    Hewrite: Success
    Written 2 bytes by write. errno=0, err=Success



```python
!./run_with_unreliable_io.exe 0 echo "Vasya got 3 in math"
!./run_with_unreliable_io.exe 1 echo "Vasya got 3 in math"
```

    Vasya got 3 in math
    Vaecho: write error



```python

```


```cpp
%%cpp unreliable_write.cpp
%run gcc unreliable_write.cpp -o unreliable_write.exe
%run ./unreliable_write.exe
%run ./run_with_unreliable_io.exe 0 ./unreliable_write.exe
%run ./run_with_unreliable_io.exe 1 ./unreliable_write.exe

#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

int main() {
    const char str[] = "Hello from C!\n";
    int written_p = printf("Reliable print: %s", str); fflush(stdout);
    fprintf(stderr, "Written %d bytes by printf. errno=%d, err=%s\n", written_p, errno, strerror(errno)); fflush(stderr);
  
    int written_w = write(1, str, sizeof(str));
    perror("write");
    fprintf(stderr, "Written %d bytes by write. errno=%d, err=%s\n", written_w, errno, strerror(errno)); fflush(stderr);
    
    return 0;
}
```


Run: `gcc unreliable_write.cpp -o unreliable_write.exe`



Run: `./unreliable_write.exe`


    Reliable print: Hello from C!
    Written 30 bytes by printf. errno=0, err=Success
    Hello from C!
    write: Success
    Written 15 bytes by write. errno=0, err=Success



Run: `./run_with_unreliable_io.exe 0 ./unreliable_write.exe`


    Reliable print: Hello from C!
    Written 30 bytes by printf. errno=0, err=Success
    Hello from C!
    write: Success
    Written 15 bytes by write. errno=0, err=Success



Run: `./run_with_unreliable_io.exe 1 ./unreliable_write.exe`


    Written -1 bytes by printf. errno=4, err=Interrupted system call
    Hello from C!
    write: Interrupted system call
    Written 15 bytes by write. errno=4, err=Interrupted system call



```python

```


```python

```

# Просто страшный код

Найдите ошибку и не повторяйте


```cpp
%%cpp tmp.c --ejudge-style
%run gcc tmp.c -o tmp.exe

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main()
{
    // setvbuf(stdin, NULL, _IONBF, 0); // а с этим может работать
    pid_t pid;
    int result = 0;
    while (1) {
        pid = fork();
        if (pid == 0) {
            char buffer[4097];
            int length = scanf("%s", buffer);
            return (length == EOF) ? 0 : 1;
        } else {
            int status;
            waitpid(pid, &status, 0);
            if (status == 0) {
                break;
            }
            result += WEXITSTATUS(status);
        }
    }
    printf("%d\n", result);
    return 0;
}
```


Run: `gcc tmp.c -o tmp.exe`



```python
!echo "asdf srfr" | ./tmp.exe
!echo "asdf   srfr \n sdfvf" | ./tmp.exe
```

    1
    1



```python
# в принципе даже с ненадежным io работает (если setvbuf раскомментирован)
!echo "asdf srfr" | ./run_with_unreliable_io.exe 0 ./tmp.exe
!echo "asdf   srfr \n sdfvf" | ./run_with_unreliable_io.exe 0 ./tmp.exe
```

    2
    3



```python

```
