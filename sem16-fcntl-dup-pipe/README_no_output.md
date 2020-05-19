

# Копирование файловых дескрипторов и неименованные каналы

<br>
<div style="text-align: right"> Спасибо <a href="https://github.com/SyrnikRebirth">Сове Глебу</a> за участие в написании текста </div>
<br>
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

<a href="#hw" style="color:#856024">Комментарии к ДЗ</a>

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
        // Даже без закрытия сможет отработать (но не надо так делать, лучше всегда закрывать руками)
        execlp("ps", "ps", "aux", NULL);
        assert(0 && "Unreachable position in code if execlp succeeded");
    }
    if ((pid_2 = fork()) == 0) {
        dup2(fd[0], 0);
        close(fd[0]); // notice: close file descriptors explicitly
        close(fd[1]); // try to comment out and compare behaviour
        // ^^^ tail не завершиться пока открыт файловый дескриптор на запись в pipe (он будет ждать данных, которые он бы смог прочитать)
        execlp("tail", "tail", "-n", "4", NULL);
        assert(0 && "Unreachable position in code if execlp succeeded");
    }
    close(fd[0]); // Тут закрыли pipe, потому что он нам больше не нужен (и потому что, если не закроем, то будет ошибка как с программой tail)
    close(fd[1]);
    int status;
    assert(waitpid(pid_1, &status, 0) != -1); // [1.]
    assert(waitpid(pid_2, &status, 0) != -1);
    return 0;
}
```

1. Вопрос: почему нужно делать waitpid после close?
  <br> Ответ: потому что у нас остаётся открытый файловый дескриптор. Дочерний процесс не завершится до тех пор, пока не закроется пайп на запись. А пайп на запись не закроется, пока не закроются все соответствующие файловые дескрипторы (и не только в том же самом процессе). Соответственно, если не сделать close до waitpid, то он просто зависнет. 


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
        //dup3(fd[1], 1, O_CLOEXEC); // can compare with this
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
    close(1); // important (в следующей функции просим сделать дескриптор >= 1, поэтому тут нужно закрыть дескриптор 1, чтобы он стал доступен)
    int fd_copy = fcntl(fd, F_DUPFD, 1);
    assert(fd_copy == 1);
    // Три строчки сверху выполняют то же что и dup2(fd, 1)
    close(fd);
    printf("Redirectred 'Hello world!'");
    return 0;
}
```

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
    int ret = fcntl(fd, F_GETFD, 0); // Функция принимает 3 аргумента, поэтому обязаны записать все 3 (даже если третий не будет использоваться)
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
    describe_fd("pipe2 + O_CLOEXEC", fd[0]);

    pipe(fd);
    fcntl(fd[0], F_SETFD, fcntl(fd[0], F_GETFD, 0) | FD_CLOEXEC); //руками сделали так что у pipe есть флаг O_CLOEXEC
    describe_fd("pipe + manually set flag", fd[0]);
    return 0;
}
```


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


```python

```


```python

```

# O_NONBLOCK


```cpp
%%cpp pipe2.cpp
%run gcc pipe2.cpp -o pipe2.exe
%run ./pipe2.exe

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
#include <sched.h>
#include <time.h>
#include <errno.h>

int main() {
    int fd[2];
    pipe(fd); 
    pid_t pid_1, pid_2;
    if ((pid_1 = fork()) == 0) {
        dup2(fd[1], 1); 
        close(fd[0]); 
        close(fd[1]);
        for (int i = 0; i < 1000; ++i) {
            write(1, "X", 1);
            //sched_yield();
            struct timespec t = {.tv_sec = 0, .tv_nsec = 10000};
            nanosleep(&t, &t);  
        }
        return 0;
    }
    if ((pid_2 = fork()) == 0) {
        // no close calls here
        dup2(fd[0], 0);
        close(fd[0]); 
        close(fd[1]);
        fcntl(0, F_SETFL, fcntl(0, F_GETFL, 0) | O_NONBLOCK);
        while (true) {
            char c;
            int r = read(0, &c, 1);
            if (r > 0) {
                write(1, &c, 1);
            } else if (r < 0) {
                assert(errno == EAGAIN);
                write(1, "?", 1);
            } else {
                break;
            }
        }
        return 0;
    }
    close(fd[0]);
    close(fd[1]);
    int status;
    assert(waitpid(pid_1, &status, 0) != -1);
    assert(waitpid(pid_2, &status, 0) != -1);
    return 0;
}
```


```python

```

# <a name="hw"></a> Комментарии к ДЗ

* `inf13-0: posix/pipe/launch` - фактически разобрана, когда переизобретали `freopen`.
* `inf13-1: posix/pipe/connect-2-processes` - тоже фактически разобрана. pipe + работа с дочерними процессами. Для тестирования может быть удобно написать свои программки в виде скриптов с шебангом. [Ссылка для студентов с плохой памятью](https://andreyex.ru/operacionnaya-sistema-linux/shebang-v-bash)
* `inf13-2: posix/pipe/process-gcc-output-2` - как предыдущая, но нужно самим написать программы по разные стороны pipe. Разрешаю использовать `python -c` (типа `python -c 'print "abacaba\nasdfdsv".find("aca")'`)
* `inf13-3: posix/pipe/connect-n-processes` - просто обобщить `inf13-1`
* `inf13-4: posix/pipe/connect-n-processes-one-pipe`
  * В этой задаче важно не забывать про изначально открытые файловые дескрипторы.
  * Когда вы делаете fork и запускается новая программа, она может подгружать динамические библиотеки. И при этом могут создаваться и закрываться файловые дескрипотры. Так что, если у вас открыто 8 файловых дескрипторов, то на fork'е вы скорее всего сломаетесь.
  * Разрешаю хранить промежуточный вывод подпрограмм в памяти основного процесса.
  * В этой задаче можно немного упороться и обойтись двумя пайпами за все время существования программы. Для этого, нужно предполагать, что программы-команды не будут ретраить операцию чтения в случае ошибок. Дальше просто применяем O_NONBLOCK.

**Замечание**

Конечно, однострочники питона это очень плохо в продакшн коде, но очень полезный навык для администрирования всякого-разного.

В качестве простого примера, допустим, что вам нужно красиво отрисовать json и у вас нет для этого встроенных средств:

```bash
[00:44:00 фев 09] pechatnov@pechatnov-osx2:~
  -> echo '{"a": "b", "d": [1, 2, 3]}' | python -c 'import json, sys; json.dump(json.load(sys.stdin), sys.stdout, indent=4)'
{
    "a": "b",
    "d": [
        1,
        2,
        3
    ]
}
```

Тут же можно пофильтровать этот json по какому либо признаку.

В однострочниках есть сложности с циклами и ветвлением, но часто легко обойтись генераторами списков и "тернарными" if'ами.

Или воспользоваться exec. Но в этом случае создать файл со скриптом через vim/nano, кто-нибудь из них обычно есть в качестве встроенного редактора.


```python

```


```python

```


```python

```
