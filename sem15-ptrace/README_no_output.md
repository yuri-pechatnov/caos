

# Сегодня будем изобретать bash
Чтобы делать основную магию bash нам понадобятся три вызова
* `fork` - (перевод: вилка) позволяет процессу раздвоиться. Это единственный способ создания новых процессов в linux
* `exec` - позволяет процессу запустить другую программу в своем теле. То есть остается тот же процесс (тот же pid), те же открытые файлы, еще что-то общее, но исполняется код из указанного исполняемого файла. Прямо начиная с функции _start
* `pipe` - позволяет создать трубу(=pipe) - получить пару файловых дескрипторов. В один из них можно что-то писать, при этом оно будет становиться доступным для чтения из другого дескриптора. Можно рассматривать pipe как своеобразный файл-очередь.
* `dup2` - позволяет "скопировать" файловый дескриптор. То есть получить еще один файловый дескриптор на тот же файл/соединение. Например, так можно скопировать дескриптор файла в 1 файловый дескриптор и потом с помощью функции printf писать в этот файл.

* `wait` - прозволяет дождаться дочерних процессов и получить их код возврата. Так же прекращает их жизнь в качестве зомби.

Изобретать будет такую магию:
* Запуск сторонней программы
* `./program arg1 arg2 > out.txt` то есть оператор `>` из bash
* `./program1 arg1_1 arg1_2 | ./program2 arg2_1 arg2_2` то есть оператор `|` из bash


# fork

`man fork`, `man waitpid`

Простейший пример: клонируем себя, и в оригинале дожидаемся, пока копия завершится, потом тоже завершаемся.


```cpp
%%cpp simpliest_example.cpp
%run gcc simpliest_example.cpp -o simpliest_example.exe
%run ./simpliest_example.exe

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <sys/wait.h>
#include <sched.h>

int main() {
    pid_t pid = fork();
//     if (pid != 0) {
//         for (int i = 0; i < 1000000; ++i) {
//             sched_yield();
//         }
//     }
    printf("Hello world! fork result (child pid) = %d, own pid = %d\n", pid, getpid()); // выполнится и в родителе и в ребенке
    
    if (pid == 0) {
        return 42; // если это дочерний процесс, то завершаемся
    }
    int status;
    pid_t w = waitpid(pid, &status, 0); // обязательно нужно дождаться, пока завершится дочерний процесс
    if (w == -1) {
        perror("waitpid");
        exit(-1);
    }
    assert(WIFEXITED(status));
    printf("Child exited with code %d\n", WEXITSTATUS(status)); // выводим код возврата дочернего процесса
    return 0;
}
```

## fork-бомба

С помощью вызовов форк легко написать программу, процесс которой будет бесконечно порождать свои копии, а копии в свою очередь новые копии. Такой программа при запуске быстро съест все ресурсы системы и может привести к мертвому зависанию. 

Подробнее на [Википедии](https://ru.wikipedia.org/wiki/Fork-%D0%B1%D0%BE%D0%BC%D0%B1%D0%B0)

# fork + exec

`man exec`, `man wait4`

О том как гуглить непонятные структуры: struct timeval linux 


```cpp
%%cpp fork_exec.cpp
%run gcc fork_exec.cpp -o fork_exec.exe
%run ./fork_exec.exe

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>


int main() {
    pid_t pid;
    if ((pid = fork()) == 0) {
        //execlp("ps", "ps", "aux", NULL); // also possible variant
        //execlp("echo", "echo", "Hello world from linux ECHO program", NULL);
        //execlp("sleep", "sleep", "3", NULL);
        execlp("bash", "bash", "-c", "ps aux | head -n 4", NULL);
        assert(0 && "Unreachable position in code if execlp succeeded");
    }
    int status;
    struct rusage resource_usage;
    pid_t w = wait4(pid, &status, 0, &resource_usage); // обязательно нужно дождаться, пока завершится дочерний процесс
    if (w == -1) {
        perror("waitpid");
        exit(-1);
    }
    assert(WIFEXITED(status));
    printf("Child exited with code %d \n"
           "\tUser time %ld sec %ld usec\n"
           "\tSys time %ld sec %ld usec\n", 
           WEXITSTATUS(status), 
           resource_usage.ru_utime.tv_sec,
           resource_usage.ru_utime.tv_usec,
           resource_usage.ru_stime.tv_sec,
           resource_usage.ru_stime.tv_usec); // выводим код возврата дочернего процесса + еще полезную информацию
    
    return 0;
}
```


```python

```

# dup2

Возможно кто-то из вас видел вызов freopen. Вот это примерно о том же.


```cpp
%%cpp fork_exec_pipe.cpp
%run gcc fork_exec_pipe.cpp -o fork_exec_pipe.exe
%run ./fork_exec_pipe.exe
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

# fork + exec + dup2
Реализуем перенаправление вывода программы в файл. (Оператор `>` из bash)


```cpp
%%cpp redirect.cpp
%run gcc redirect.cpp -o redirect.exe
%run ./redirect.exe out.txt   ps aux
%run cat out.txt | head -n 2

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>


int main(int argc, char** argv) {
    assert(argc >= 2);
    int fd = open(argv[1], O_WRONLY | O_CREAT | O_TRUNC, 0664);
    assert(fd >= 0);
    dup2(fd, 1);
    close(fd);
    execvp(argv[2], argv + 2);
    assert(0 && "Unreachable position in code if execlp succeeded");
}
```

# fork + exec + pipe + dup2

`man 2 pipe`, `man dup2`

Реализуем логику пайпа / оператора `|` из bash: запуск двух программ и перенаправление вывода одной на ввод другой.


```cpp
%%cpp fork_exec_pipe.cpp
%run gcc fork_exec_pipe.cpp -o fork_exec_pipe.exe
%run ./fork_exec_pipe.exe

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
        close(fd[0]);
        close(fd[1]);
        execlp("ps", "ps", "aux", NULL);
        assert(0 && "Unreachable position in code if execlp succeeded");
    }
    if ((pid_2 = fork()) == 0) {
        dup2(fd[0], 0);
        close(fd[0]);
        close(fd[1]);
        execlp("head", "head", "-n", "4", NULL);
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


```python
!man dup2
```

# inf09-0

Рекомендуется при отладке задачи использовать следующий набор команд:

```
sudo useradd tmp_user # создаем пользователя
sudo passwd tmp_user  # устанавливаем пароль
su tmp_user           # логинимся под пользователя в этом окне терминала
ulimit -u 100         # ограничиваем число потоков доступное пользователю
./inf09_0.exe         # запускаем опасную программу
```

Чтобы тестировать в рамках отдельного юзера у которого ограничено число потоков, которое он может создать. Таким образом можно предотвратить эффект fork-бомбы.


```cpp
%%cpp inf09_0.c --ejudge-style
%run gcc inf09_0.c -o inf09_0.exe

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main()
{
    for (int i = 1; 1; ++i) {
        int pid = fork();
        fflush(stdout);
        if (pid < 0) {
            printf("%d\n", i);
            return 0;
        }
        if (pid != 0) {
            int status;
            assert(waitpid(pid, &status, 0) != -1);
            break;
        }
    }
    return 0;
}
```


```python

```

# malloc fork

Один из интересных багов, которые случались в Facebook, включал в себя такую интересную комбинацию:

Если сделать malloc(1005000000), то что произойдет? Вызов завершиться, память выделится, но не совсем честно: не все страницы созданной виртуальной памяти будут иметь под собой физические страницы. Поэтому можно так "выделить" памяти больше, чем есть в системе. И пока мы как-то не проивзаимодействуем с выделенными страницами, они не будут присоединены к физической памяти.

А вот если потом сделать fork, то что будет? По идее fork не копирует сразу физические страницы в памяти, а делает их cow (copy on write), так что потребление памяти не должно измениться. 

Но оказалось, что при вызове fork вся "выделенная" память реально выделяется. То есть для этих 1005000000 выделенных байт реально ищутся страницы физической памяти. Поэтому при вызове fork всё взрывалось. 



```python

```
