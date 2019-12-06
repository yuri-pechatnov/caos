```python
get_ipython().run_cell_magic('javascript', '', '// setup cpp code highlighting\nIPython.CodeCell.options_default.highlight_modes["text/x-c++src"] = {\'reg\':[/^%%cpp/]} ;')

# creating magics
from IPython.core.magic import register_cell_magic, register_line_magic
from IPython.display import display, Markdown, HTML
import argparse
from subprocess import Popen, PIPE
import random
import sys

@register_cell_magic
def save_file(args_str, cell, line_comment_start="#"):
    parser = argparse.ArgumentParser()
    parser.add_argument("fname")
    parser.add_argument("--ejudge-style", action="store_true")
    args = parser.parse_args(args_str.split())
    
    cell = cell if cell[-1] == '\n' or args.no_eof_newline else cell + "\n"
    cmds = []
    with open(args.fname, "w") as f:
        f.write(line_comment_start + " %%cpp " + args_str + "\n")
        for line in cell.split("\n"):
            line_to_write = (line if not args.ejudge_style else line.rstrip()) + "\n"
            if line.startswith("%"):
                run_prefix = "%run "
                assert line.startswith(run_prefix)
                cmds.append(line[len(run_prefix):].strip())
                f.write(line_comment_start + " " + line_to_write)
            else:
                f.write(line_to_write)
        f.write("" if not args.ejudge_style else line_comment_start + r" line without \n")
    for cmd in cmds:
        display(Markdown("Run: `%s`" % cmd))
        get_ipython().system(cmd)

@register_cell_magic
def cpp(fname, cell):
    save_file(fname, cell, "//")

@register_cell_magic
def asm(fname, cell):
    save_file(fname, cell, "//")
    
@register_cell_magic
def makefile(fname, cell):
    assert not fname
    save_file("makefile", cell.replace(" " * 4, "\t"))
        
@register_line_magic
def p(line):
    try:
        expr, comment = line.split(" #")
        display(Markdown("`{} = {}`  # {}".format(expr.strip(), eval(expr), comment.strip())))
    except:
        display(Markdown("{} = {}".format(line, eval(line))))
        
def show_file(file, clear_at_begin=False):
    if clear_at_begin:
        get_ipython().system("truncate --size 0 " + file)
    obj = file.replace('.', '_').replace('/', '_') + "_obj"
    display(HTML('''
        <script type=text/javascript>
        function refresh__OBJ__()
        {
            var elem = document.getElementById("__OBJ__");
            if (elem) {
                var xmlhttp=new XMLHttpRequest();
                xmlhttp.onreadystatechange=function()
                {
                    var elem2 = document.getElementById("__OBJ__1");
                    if (xmlhttp.readyState==4 && xmlhttp.status==200)
                    {
                        elem2.innerText = xmlhttp.responseText;
                        // console.log(xmlhttp.responseText);
                        return xmlhttp.responseText;
                    }
                }
                xmlhttp.open("GET", elem.data, true);
                xmlhttp.send();    
                elem.hidden = "hidden";
                window.setTimeout("refresh__OBJ__()", 300); 
            }
        }
        window.setTimeout("refresh__OBJ__()", 300); 
        </script>
        <div id="__OBJ__1"></div>
        <div><object id="__OBJ__" data="__FILE__", hidden="hidden"></object></div>
        '''.replace("__OBJ__", obj)
           .replace("__FILE__", file)))
    
def bash_popen_terminate_all():
    for p in globals().get("bash_popen_list", []):
        print("Terminate pid=" + str(p.pid), file=sys.stderr)
        p.terminate()
        globals()["bash_popen_list"] = []

bash_popen_terminate_all()  

def bash_popen(cmd):
    original_cmd = cmd
    h = "tmp/" + str(random.randint(0, 1e18))
    
    stdout = open(h + ".out.html", "wb")
    display(Markdown("**STDOUT** (interactive)"))
    show_file(h + ".out.html", clear_at_begin=True)
    
    stdout = open(h + ".err.html", "wb")
    display(Markdown("**STDERR** (interactive)"))
    show_file(h + ".err.html", clear_at_begin=True)
    
    fin_file = h + ".fin.html"
    cmd = "echo 'Process started!' > " + fin_file + "; " + cmd + " ; echo \"Process finished! code=$?\" >> " + fin_file
    display(Markdown("**RUN LOG** (interactive, `" + original_cmd + "`)"))
    show_file(h + ".fin.html", clear_at_begin=True)
    
    p = Popen(["bash", "-c", cmd], stdin=PIPE, stdout=stdout, stderr=stdout)
    bash_popen_list.append(p)
    return p


@register_line_magic
def bash_async(line):
    bash_popen(line)
```


    <IPython.core.display.Javascript object>


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

Пример от меня, который может пригодиться при тестировании программ на надежность IO


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
    echo: write error



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


    RWritten -1 bytes by printf. errno=4, err=Interrupted system call
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



```cpp
%%cpp tmp.c --ejudge-style
%run gcc tmp.c -o tmp.exe

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main()
{
    pid_t pid = 0;
    char str[4097];
    int num = 0;
    int status = 0;
    int is_eof = scanf("%s", str);
    while (is_eof != EOF) {
        pid = fork();
        if (pid == -1) {
            fprintf(stderr, "Can't do fork\n");
        }
        if (pid == 0) {
            is_eof = scanf("%s", str);
            if (is_eof == EOF) {
                return 0;
            }
        } else {
            waitpid(pid, &status, 0);
            break;
        }
        ++num;
    }

    if (num != 0) {
        return WEXITSTATUS(status) + 1;
    }
    if (is_eof == EOF) {
        printf("0\n");
    } else {
        printf("%d\n", WEXITSTATUS(status) + 1);
    }

    return 0;
}
```


Run: `gcc tmp.c -o tmp.exe`



```python
# в принципе даже с ненадежным io работает (если setvbuf раскомментирован)
!echo "asdf srfr" | ./run_with_unreliable_io.exe 0 ./tmp.exe
!echo "asdf   srfr \n sdfvf " | ./run_with_unreliable_io.exe 0 ./tmp.exe
!python3 -c "print('sfdfv ' * 255)" | ./tmp.exe
```

    2
    3
    255



```python
print("fsf " * 255)
```

    fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf fsf 



```python

```
