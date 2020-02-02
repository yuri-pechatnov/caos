```python
# look at tools/set_up_magics.ipynb
get_ipython().run_cell('# one_liner_str <too much code> \n')
None
```


    <IPython.core.display.Javascript object>


# Копирование файловых дескрипторов и неименованные каналы

Да, dup и pipe мы уже рассматривали в sem12. Так что по этой части будет некоторое повторение (но с большим количеством подробностей).

Сегодня будем рассматривать вызовы:
* `dup`, `dup2`, `dup3` - позволяют "скопировать" файловый дескриптор. То есть получить еще один файловый дескриптор на тот же файл/соединение. Например, так можно скопировать дескриптор файла в 1 файловый дескриптор (stdout) и потом с помощью функции printf писать в этот файл.
  * `dup` - <a href="#dup" style="color:#856024">делает какую-то копию</a>
  * `dup2` - <a href="#dup2" style="color:#856024">делает копию туда, куда вы хотите</a>
  * `dup3` - <a href="#dup3" style="color:#856024">делает купию туда, куда вы хотите + c указанными опциями</a>
* `pipe` - позволяет создать трубу(=pipe) - получить пару файловых дескрипторов. В один из них можно что-то писать, при этом оно будет становиться доступным для чтения из другого дескриптора. Можно рассматривать pipe как своеобразный файл-очередь.
  * `pipe` - <a href="#pipe" style="color:#856024">делает трубу</a>
  * `pipe2` - <a href="#pipe2" style="color:#856024">делает трубу c указанными опциями</a>
* `fcntl` - универсальная функция, которая умеет делать с открытыми файлами практически все (TODO):
  * <a href="#fcntl_fd_flags" style="color:#856024">Вытащить / установить флаги файлового дескриптора (узнать, открыт ли дескриптор на запись; выставлен ли O_CLOEXEC)</a>
  * <a href="#fcntl_dup" style="color:#856024">Исползовать вместо dup/dup2.</a>
  * Установить размер буффера для каналов.
  * Различные вещи для совместного доступа к файлам.
  * ...

[Ридинг Яковлева](https://github.com/victor-yacovlev/mipt-diht-caos/tree/master/practice/fdup-pipe)


```python

```

# dup2

Возможно кто-то из вас видел вызов freopen. Вот это примерно о том же.


```cpp
%%cpp dup2.cpp
%run gcc dup2.cpp -o dup2.exe
%run ./dup2.exe
%run echo "After program finish" && cat out.txt

#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/types.h>


int main() {
    int fd = open("out.txt", O_WRONLY | O_CREAT | O_TRUNC, 0664);
    dup2(fd, 1); // redirect stdout to file
    close(fd);
    printf("Redirectred 'Hello world!'");
    return 0;
}
```


Run: `gcc dup2.cpp -o dup2.exe`



Run: `./dup2.exe`



Run: `echo "After program finish" && cat out.txt`


    After program finish
    Redirectred 'Hello world!'

# dup


```cpp
%%cpp dup.cpp
%run gcc dup.cpp -o dup.exe
%run ./dup.exe

#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/types.h>

int main() {
    int fd = dup(1);  // копируем stdout в другой дескриптор (значение дескриптора выбирается автоматически)
    dprintf(1, "Write to 1 fd.\n");
    dprintf(fd, "Write to %d fd.\n", fd);
    close(1);
    dprintf(fd, "Write to %d fd after closing 1 fd. (still to stdout)\n", fd);
    close(fd);
    return 0;
}
```


Run: `gcc dup.cpp -o dup.exe`



Run: `./dup.exe`


    Write to 1 fd.
    Write to 3 fd.
    Write to 3 fd after closing 1 fd. (still to stdout)



```python

```

## <a name="pipe"></a> pipe и dup2



```cpp
%%cpp pipe.cpp
%run gcc pipe.cpp -o pipe.exe
%run ./pipe.exe

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    int fd[2];
    pipe(fd); // fd[0] - in, fd[1] - out (like stdin=0, stdout=1)
    pid_t pid_1, pid_2;
    if ((pid_1 = fork()) == 0) {
        dup2(fd[1], 1);
        close(fd[0]); // notice: close file descriptors explicitly
        close(fd[1]); // try to comment out and compare behaviour
        execlp("ps", "ps", "aux", NULL);
        assert(0 && "Unreachable position in code if execlp succeeded");
    }
    if ((pid_2 = fork()) == 0) {
        dup2(fd[0], 0);
        close(fd[0]); // notice: close file descriptors explicitly
        close(fd[1]); // try to comment out and compare behaviour
        execlp("tail", "tail", "-n", "4", NULL);
        assert(0 && "Unreachable position in code if execlp succeeded");
    }
    close(fd[0]);
    close(fd[1]);
    int status;
    assert(waitpid(pid_1, &status, 0) != -1);
    assert(waitpid(pid_2, &status, 0) != -1);
    return 0;
}
```


Run: `gcc pipe.cpp -o pipe.exe`



Run: `./pipe.exe`


    pechatn+ 31754  0.0  0.0  19588   972 ?        S    13:07   0:00 bash -c echo 1 ; echo 2 1>&2 ; read XX ; echo "A${XX}B" 
    pechatn+ 31968  0.0  2.5 626064 51372 ?        Ssl  14:04   0:08 /usr/bin/python3 -m ipykernel_launcher -f /home/pechatnov/.local/share/jupyter/runtime/kernel-2b3b1e25-d506-477e-9a00-0bf57c5e5dd2.json
    pechatn+ 32515 22.5  3.5 655736 73384 ?        R    14:27  83:33 /usr/bin/python3 -m ipykernel_launcher -f /home/pechatnov/.local/share/jupyter/runtime/kernel-af3f74df-105a-4e78-81e2-b162e29df778.json
    pechatn+ 32518 22.5  3.5 655736 73432 ?        R    14:27  83:17 /usr/bin/python3 -m ipykernel_launcher -f /home/pechatnov/.local/share/jupyter/runtime/kernel-af3f74df-105a-4e78-81e2-b162e29df778.json



```python

```

# <a name="pipe2"></a> pipe2 и dup2


```cpp
%%cpp pipe2.cpp
%run gcc pipe2.cpp -o pipe2.exe
%run ./pipe2.exe
%run diff pipe.cpp pipe2.cpp  | grep -v "// %" | grep -e '>' -e '<' -C 1

#ifndef _GNU_SOURCE
  #define _GNU_SOURCE
#endif
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    int fd[2];
    pipe2(fd, O_CLOEXEC); // O_CLOEXEC - created file descriptors will be closed on exec call
    pid_t pid_1, pid_2;
    if ((pid_1 = fork()) == 0) {
        dup2(fd[1], 1); // dup2 doesn't copy O_CLOEXEC attribute
        // dup3(fd[1], 1, O_CLOEXEC); // can compare with this
        execlp("ps", "ps", "aux", NULL);
        assert(0 && "Unreachable position in code if execlp succeeded");
    }
    if ((pid_2 = fork()) == 0) {
        // no close calls here
        dup2(fd[0], 0);
        execlp("tail", "tail", "-n", "4", NULL);
        assert(0 && "Unreachable position in code if execlp succeeded");
    }
    close(fd[0]);
    close(fd[1]);
    int status;
    assert(waitpid(pid_1, &status, 0) != -1);
    assert(waitpid(pid_2, &status, 0) != -1);
    return 0;
}
```


Run: `gcc pipe2.cpp -o pipe2.exe`



Run: `./pipe2.exe`


    pechatn+ 31754  0.0  0.0  19588   972 ?        S    13:07   0:00 bash -c echo 1 ; echo 2 1>&2 ; read XX ; echo "A${XX}B" 
    pechatn+ 31968  0.0  2.5 626064 51372 ?        Ssl  14:04   0:08 /usr/bin/python3 -m ipykernel_launcher -f /home/pechatnov/.local/share/jupyter/runtime/kernel-2b3b1e25-d506-477e-9a00-0bf57c5e5dd2.json
    pechatn+ 32515 22.5  3.5 655736 73384 ?        R    14:27  83:33 /usr/bin/python3 -m ipykernel_launcher -f /home/pechatnov/.local/share/jupyter/runtime/kernel-af3f74df-105a-4e78-81e2-b162e29df778.json
    pechatn+ 32518 22.5  3.5 655736 73432 ?        R    14:27  83:18 /usr/bin/python3 -m ipykernel_launcher -f /home/pechatnov/.local/share/jupyter/runtime/kernel-af3f74df-105a-4e78-81e2-b162e29df778.json



Run: `diff pipe.cpp pipe2.cpp  | grep -v "// %" | grep -e '>' -e '<' -C 1`


    4a6,8
    > #ifndef _GNU_SOURCE
    >   #define _GNU_SOURCE
    > #endif
    16c20
    <     pipe(fd); // fd[0] - in, fd[1] - out (like stdin=0, stdout=1)
    ---
    >     pipe2(fd, O_CLOEXEC); // O_CLOEXEC - created file descriptors will be closed on exec call
    19,21c23,24
    <         dup2(fd[1], 1);
    <         close(fd[0]); // notice: close file descriptors explicitly
    <         close(fd[1]); // try to comment out and compare behaviour
    ---
    >         dup2(fd[1], 1); // dup2 doesn't copy O_CLOEXEC attribute
    >         // dup3(fd[1], 1, O_CLOEXEC); // can compare with this
    25a29
    >         // no close calls here
    27,28d30
    <         close(fd[0]); // notice: close file descriptors explicitly
    <         close(fd[1]); // try to comment out and compare behaviour


Этот пример работал в том числе благодаря тому, что dup2 не копирует O_CLOEXEC флаг. 
Если же вам по каким-то причинам будет нужно, чтобы он копировался, 
то можно исползовать `dup3(fd[1], 1, O_CLOEXEC)`. <a name="dup3"></a>


```python

```

# <a name="fcntl_dup"></a> fcntl: вместо dup


```cpp
%%cpp fcntl_dup.cpp
%run gcc fcntl_dup.cpp -o fcntl_dup.exe
%run ./fcntl_dup.exe
%run echo "After program finish" && cat out.txt
%run diff dup2.cpp fcntl_dup.cpp  | grep -v "// %" | grep -e '>' -e '<' -C 1

#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/types.h>


int main() {
    int fd = open("out.txt", O_WRONLY | O_CREAT | O_TRUNC, 0664);
    close(1); // important
    int fd_copy = fcntl(fd, F_DUPFD, 1);
    assert(fd_copy == 1);
    close(fd);
    printf("Redirectred 'Hello world!'");
    return 0;
}
```


Run: `gcc fcntl_dup.cpp -o fcntl_dup.exe`



Run: `./fcntl_dup.exe`



Run: `echo "After program finish" && cat out.txt`


    After program finish
    Redirectred 'Hello world!'


Run: `diff dup2.cpp fcntl_dup.cpp  | grep -v "// %" | grep -e '>' -e '<' -C 1`


    15c16,18
    <     dup2(fd, 1); // redirect stdout to file
    ---
    >     close(1); // important
    >     int fd_copy = fcntl(fd, F_DUPFD, 1);
    >     assert(fd_copy == 1);


# <a name="fcntl_fd_flags"></a> fcntl: получаем флаги дескриптора


```cpp
%%cpp fcntl_flags.cpp
%run gcc fcntl_flags.cpp -o fcntl_flags.exe
%run ./fcntl_flags.exe

#ifndef _GNU_SOURCE
  #define _GNU_SOURCE
#endif
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>

void describe_fd(const char* prefix, int fd) {
    int ret = fcntl(fd, F_GETFD, 0);
    if (ret & FD_CLOEXEC) {
        printf("%s: fd %d has CLOEXEC flag\n", prefix, fd);
    } else {
        printf("%s: fd %d doesn't have CLOEXEC flag\n", prefix, fd);
    } 
}

int main() {
    int fd[2];
    
    pipe(fd);
    describe_fd("pipe", fd[0]);

    pipe2(fd, O_CLOEXEC); 
    describe_fd("pipe2 + O_CLOEXEC: ", fd[0]);

    pipe(fd);
    fcntl(fd[0], F_SETFD, fcntl(fd[0], F_GETFD, 0) | FD_CLOEXEC);
    describe_fd("pipe + manually set flag: ", fd[0]);
    return 0;
}
```


Run: `gcc fcntl_flags.cpp -o fcntl_flags.exe`



Run: `./fcntl_flags.exe`


    pipe: fd 3 doesn't have CLOEXEC flag
    pipe2 + O_CLOEXEC: : fd 5 has CLOEXEC flag
    pipe + manually set flag: : fd 7 has CLOEXEC flag



```python

```

# fcntl: узнаем как открывали файл

На самом деле только ограниченный набор флагов можно вытащить.

Эксперимент показывает, что O_RDWR, O_RDONLY, O_WRONLY, O_APPEND, O_TMPFILE, O_ASYNC, O_DIRECT вытащить можно.


```cpp
%%cpp fcntl_open_flags.cpp
%run gcc fcntl_open_flags.cpp -o fcntl_open_flags.exe
%run ./fcntl_open_flags.exe

#ifndef _GNU_SOURCE
  #define _GNU_SOURCE
#endif
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>

void describe_fd(const char* prefix, int fd) {
    int ret = fcntl(fd, F_GETFL, 0);
    
#define flag_cond_str_expanded(flag, mask, name) ((ret & (mask)) == flag ? name : "")
#define flag_cond_str_mask(flag, mask) flag_cond_str_expanded(flag, mask, #flag)
#define flag_cond_str(flag) flag_cond_str_expanded(flag, flag, #flag)
    //printf("%d\n", ret & 3);
    printf("%s: %s, %s, %s, %s, %s, %s, %s, %s, %s, %s\n", prefix
        , flag_cond_str_mask(O_RDONLY, O_RDONLY | O_WRONLY | O_RDWR)
        , flag_cond_str_mask(O_WRONLY, O_RDONLY | O_WRONLY | O_RDWR)
        , flag_cond_str_mask(O_RDWR, O_RDONLY | O_WRONLY | O_RDWR)
        , flag_cond_str(O_TRUNC)
        , flag_cond_str(O_APPEND)
        , flag_cond_str(O_CREAT)
        , flag_cond_str(O_CLOEXEC)
        , flag_cond_str(O_TMPFILE)
        , flag_cond_str(O_ASYNC)
        , flag_cond_str(O_DIRECT)
    );
}

void check_fd(int fd) {
    if (fd < 0) {
        perror("open");
        assert(fd >= 0);
    }
} 

int main() {
    describe_fd("0 (stdin)", 0);
    describe_fd("1 (stdout)", 1);
    describe_fd("2 (stderr)", 2);
    
    int f1 = open("fcntl_open_flags.1", O_CREAT|O_TRUNC|O_WRONLY, 0664); check_fd(f1);
    describe_fd("f1 O_CREAT|O_TRUNC|O_WRONLY", f1);
    
    int f2 = open("fcntl_open_flags.2", O_CREAT|O_RDWR, 0664); check_fd(f2);
    describe_fd("f2 O_CREAT|O_RDWR", f2);
    
    int f3 = open("fcntl_open_flags.2", O_WRONLY|O_APPEND); check_fd(f3);
    describe_fd("f3 O_WRONLY|O_APPEND", f3);

    int f4 = open("fcntl_open_flags.2", O_RDONLY|O_NONBLOCK|O_ASYNC|O_DIRECT); check_fd(f4);
    describe_fd("f4 O_RDONLY|O_NONBLOCK|O_ASYNC|O_DIRECT", f4);
    
    int f5 = open("./", O_TMPFILE|O_RDWR, 0664); check_fd(f5);
    describe_fd("f5 O_TMPFILE|O_RDWR", f5);
    
    int fds[2];
    pipe2(fds, O_CLOEXEC); 
    describe_fd("pipe2(fds, O_CLOEXEC)", fds[0]);
    return 0;
}
```


Run: `gcc fcntl_open_flags.cpp -o fcntl_open_flags.exe`



Run: `./fcntl_open_flags.exe`


    0 (stdin): , , O_RDWR, , , , , , , 
    1 (stdout): , , O_RDWR, , , , , , , 
    2 (stderr): , , O_RDWR, , , , , , , 
    f1 O_CREAT|O_TRUNC|O_WRONLY: , O_WRONLY, , , , , , , , 
    f2 O_CREAT|O_RDWR: , , O_RDWR, , , , , , , 
    f3 O_WRONLY|O_APPEND: , O_WRONLY, , , O_APPEND, , , , , 
    f4 O_RDONLY|O_NONBLOCK|O_ASYNC|O_DIRECT: O_RDONLY, , , , , , , , O_ASYNC, O_DIRECT
    f5 O_TMPFILE|O_RDWR: , , O_RDWR, , , , , O_TMPFILE, , 
    pipe2(fds, O_CLOEXEC): O_RDONLY, , , , , , , , , 



```python

```


```python

```