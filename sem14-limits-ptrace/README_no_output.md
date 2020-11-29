

# Лимиты в linux и отладка программ с помощью PTRACE

<p><a href="https://www.youtube.com/watch?v=???" target="_blank">
    <h3>Видеозапись семинара</h3> 
</a></p>


[Ридинг Яковлева](https://github.com/victor-yacovlev/mipt-diht-caos/tree/master/practice/exec-rlimit-ptrace) 

Сегодня в программе:
* <a href="#limits" style="color:#856024"> Лимиты в linux </a>  
* <a href="#ptrace" style="color:#856024"> Отладка программ с помощью PTRACE </a>  



## <a name="limits"></a> Лимиты в linux


```python
!ulimit 
```


```python
!ulimit -a
```

1. RLIMIT_CPU - лимит на процессорное время, которое доступно процессу
  <br> `ulimit -t 1` - запретить процессам тратить больще 1 секунды процессорного времени.
1. RLIMIT_FSIZE - лимит на увеличение размера файлов.
  <br> `ulimit -f 100000` - ограничить максимальный размер создаваемого файла в 100 MB
1. RLIMIT_DATA - лимит на размер сегмента данных (глобальные переменные + куча).
  <br> `ulimit -d 48000` - ограничить максимальный размер сегмента данных в 48 MB
1. RLIMIT_STACK - лимит на размер стека ~ просто размер стека.
  <br> `ulimit -s 16384` - установить размер стека в 8 MB
1. RLIMIT_CORE - лимит на размер coredump'ов
  <br> `ulimit -c 0` - запрещаем создавать core файлы
  <br> `ulimit -c unlimited` - разрешаем неограниченно создавать core файлы (*****)
1. RLIMIT_RSS - лимит на потребление оперативной памяти (реально подгруженной в RAM). Работает только в определенных версиях linux. Так что не стоит закладываться на это.
1. RLIMIT_NPROC - лимит на количество процессов/потоков запущенных от екущего пользователя.
  <br> `ulimit -u 64` - запретить создавать больше 64 потоков/процессов.
1. RLIMIT_NOFILE - лимит на количество одновременно открытых файлов
  <br> `ulimit -n 5` - запретить открывать больше 5 файлов.
1. RLIMIT_AS - лимит на потребление виртуальной памяти.
  <br> `ulimit -v 100000` - ограничить размер используемой виртуальной памяти в 100 MB
...


```cpp
%%cpp custom_stack.c
%run gcc custom_stack.c -o custom_stack.exe
%run ./custom_stack.exe 1000000


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
```


```python

```


```python

```

## <a name="ptrace"></a> Отладка программ с помощью PTRACE


Документация https://www.opennet.ru/base/dev/ptrace_guide.txt.html



```cpp
%%cpp premoderate.c
%run gcc -m64 premoderate.c -o premoderate.exe
%run ./premoderate.exe echo "Vasya got 3 in math and 3 in russian"

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

static void premoderate_write_syscall(pid_t pid, struct user_regs_struct state)
{
    size_t orig_buf = state.rsi;   
    size_t size = state.rdx;
    char* buffer = calloc(size + sizeof(size_t), sizeof(*buffer));
    for (size_t i = 0; i < size; i += sizeof(size_t)) {
        *(size_t*)(buffer + i) = checked(ptrace(PTRACE_PEEKDATA, pid, orig_buf + i, NULL));
    }
    *(size_t*)(buffer + size - 1) = checked(ptrace(PTRACE_PEEKDATA, pid, orig_buf + size - 1, NULL));
    
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
```


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
```


```python
!./run_with_unreliable_io.exe 1 0 echo "123 456 789 012 345 678 90"
!./run_with_unreliable_io.exe 1 1 echo "123 456 789 012 345 678 90"
```


```python

```


```cpp
%%cpp unreliable_write.cpp
%run gcc unreliable_write.cpp -o unreliable_write.exe
%run ./unreliable_write.exe
%run ./run_with_unreliable_io.exe 1 0 ./unreliable_write.exe
%run ./run_with_unreliable_io.exe 1 1 ./unreliable_write.exe

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


```python

```


```python

```

# Просто страшный код

Найдите ошибку и не повторяйте


```cpp
%%cpp tmp.c --ejudge-style
%run gcc tmp.c -o tmp.exe
%run echo "asdf srfr" | ./tmp.exe
%run echo "asdf   srfr \n sdfvf" | ./tmp.exe

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


```python

```
