


# Сегодня будем изобретать bash

<p><a href="https://www.youtube.com/watch?v=o2MW48SsIWM&list=PLjzMm8llUm4AmU6i_hPU0NobgA4VsBowc&index=14" target="_blank">
    <h3>Видеозапись семинара</h3>
</a></p>


[Ридинг Яковлева про процессы, их свойства и состояния, fork, waitpid](https://github.com/victor-yacovlev/mipt-diht-caos/tree/master/practice/fork) 

[Ридинг Яковлева про fork & exec](https://github.com/victor-yacovlev/mipt-diht-caos/tree/master/practice/exec-rlimit-ptrace) 


Сегодня в программе:
* <a href="#fork_waitpid" style="color:#856024"> **fork & waitpid** - порождение дочерних процессов и ожидание их завершения  </a>  
* <a href="#fork_bomb" style="color:#856024"> **fork-бомба** </a> 
* <a href="#fork_daemon" style="color:#856024"> **fork -> daemon** - порождение демона </a> 
* <a href="#fork_exec" style="color:#856024"> **fork & exec** - запуск внешних программ как дочерних процессов </a>  
* <a href="#dup2" style="color:#856024"> **dup2** - изобретаем freopen </a>  
* <a href="#redirect" style="color:#856024"> **exec + dup2** - изобретаем перенаправление вывода программы в файл: `echo Hello > file.txt` </a>  
* <a href="#pipe" style="color:#856024"> **fork & exec & pipe & dup2** - изобретаем перенаправление вывода одной программы на вход другой: `cat file.txt | grep ERROR` </a> 

 
* <a href="#hw" style="color:#856024"> Комментарии к ДЗ </a> 

------

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


Про PID 1: [Хабр / systemd десять лет спустя. Историческая и техническая ретроспектива](https://habr.com/ru/post/503816/)


## <a name="fork_waitpid"></a> **fork & waitpid** - порождение дочерних процессов и ожидание их завершения

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


Run: `gcc simpliest_example.cpp -o simpliest_example.exe`



Run: `./simpliest_example.exe`


    Hello world! fork result (child pid) = 19268, own pid = 19267
    Hello world! fork result (child pid) = 0, own pid = 19268
    Child exited with code 42


## <a name="fork_bomb"></a> **fork-бомба**

С помощью вызовов форк легко написать программу, процесс которой будет бесконечно порождать свои копии, а копии в свою очередь новые копии. Такой программа при запуске быстро съест все ресурсы системы и может привести к мертвому зависанию. 

Подробнее на [Википедии](https://ru.wikipedia.org/wiki/Fork-%D0%B1%D0%BE%D0%BC%D0%B1%D0%B0)


Рекомендуется при написани и отдадке собственной форк-бомбы использовать следующий набор команд:

```
sudo useradd tmp_user # создаем пользователя
sudo passwd tmp_user  # устанавливаем пароль
su tmp_user           # логинимся под пользователя в этом окне терминала
ulimit -u 100         # ограничиваем число потоков/процессов доступное пользователю
./inf09_0.exe         # запускаем опасную программу
```

Чтобы тестировать в рамках отдельного юзера у которого ограничено число потоков, которое он может создать. Таким образом можно предотвратить эффект fork-бомбы.


```cpp
%%cpp inf09_0.c --ejudge-style
%run gcc inf09_0.c -o inf09_0.exe
%run sudo useradd tmp_user #// Создаем временного пользователя
%// Из-под временного пользователя устанавливаем лимит и запускаем опасную программу
%// Обратите внимание, что от имени пользователя в первую очередь запускается bash, а остальное уже запускает сам bash
%run sudo -u tmp_user bash -c 'ulimit -u 5 ; ./inf09_0.exe' 
%run sudo userdel tmp_user #// Удаляем временного пользователя

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


Run: `gcc inf09_0.c -o inf09_0.exe`



Run: `sudo useradd tmp_user #// Создаем временного пользователя`



\#\#\#\# `Из-под временного пользователя устанавливаем лимит и запускаем опасную программу`



\#\#\#\# `Обратите внимание, что от имени пользователя в первую очередь запускается bash, а остальное уже запускает сам bash`



Run: `sudo -u tmp_user bash -c 'ulimit -u 5 ; ./inf09_0.exe'`


    4



Run: `sudo userdel tmp_user #// Удаляем временного пользователя`



```python

```

## <a name="fork_daemon"></a> **fork -> daemon**


```cpp
%%cpp daemon.c --ejudge-style
%run gcc daemon.c -o daemon.exe

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

void do_daemon_job(const char* mode) {
    freopen("log.txt", "wt", stdout);
    while (1) {
        printf("I'm alive, my mode is %s, my pid is %d\n", mode, (int)getpid());
        fflush(stdout);
        sleep(1);
    }
}

int main(int argc, char **argv)
{
    assert(argc == 2);
    if (strcmp(argv[1], "foreground") == 0) {
        do_daemon_job(argv[1]);    
    } else if (strcmp(argv[1], "background") == 0) {
        int pid = fork(); // первый fork
        assert(pid >= 0);
        if (pid > 0) {
            int status;
            pid_t w = waitpid(pid, &status, 0); 
            assert(w != -1 && WIFEXITED(status) && WEXITSTATUS(status) == 0); // проверяем, что промежуточный процесс завершился корректно
            return 0; // оригинальная программа завершается
        }
        setsid(); // создаём новый сеанс/сессию
        pid = fork(); // второй fork
        assert(pid >= 0);
        if (pid > 0) {
            // ребенка не ждем - он будет полноценным демоном, его статус после смерти получит PID 1
            return 0; // лидер новой сессии завершается
        }
        // теперь мы процесс отвязанный от всех терминалов и легко к ним не привяжемся
        
        close(STDIN_FILENO); // можно закрыть все стандартные дескрипторы - они больше не нужны
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
        
        do_daemon_job(argv[1]);     
    } else {
        assert(0 && "No such option");
    }
    return 0;
}
```


Run: `gcc daemon.c -o daemon.exe`


**Запускаем в обычном режиме**


```python
!echo -n "" > log.txt
a = TInteractiveLauncher("tail -f log.txt")  # интерактивно выводим файл log.txt
```





<pre>
L | Process started. PID = 19414
O | I'm alive, my mode is foreground, my pid is 19419
O | I'm alive, my mode is foreground, my pid is 19419
O | I'm alive, my mode is foreground, my pid is 19419
L | Process finished. Got signal 9

</pre>





```python
#time -p --- выводит время работы команды, что следует дальше
#        timeout 3 --- запускает команду, что следует дальше, но дает ей проработать максимум 3 секунды, дальше убивает ее
#                  ./daemon.exe foreground --- собственно наша программа
!time -p timeout 3 ./daemon.exe foreground  # работает 3 секунды, пока команда timeout не убьет daemon.exe

```

    Command exited with non-zero status 124
    real 3.00
    user 0.00
    sys 0.00



```python
a.close()
```

**Запускаем в режиме демона**


```python
!echo -n "" > log.txt
a = TInteractiveLauncher("tail -f log.txt")  # интерактивно выводим файл log.txt
```





<pre>
L | Process started. PID = 19422
O | I'm alive, my mode is background, my pid is 19428
O | I'm alive, my mode is background, my pid is 19428
O | I'm alive, my mode is background, my pid is 19428
O | I'm alive, my mode is background, my pid is 19428
O | I'm alive, my mode is background, my pid is 19428
O | I'm alive, my mode is background, my pid is 19428
O | I'm alive, my mode is background, my pid is 19428
O | I'm alive, my mode is background, my pid is 19428
O | I'm alive, my mode is background, my pid is 19428
O | I'm alive, my mode is background, my pid is 19428
O | I'm alive, my mode is background, my pid is 19428
O | I'm alive, my mode is background, my pid is 19428
O | I'm alive, my mode is background, my pid is 19428
O | I'm alive, my mode is background, my pid is 19428
O | I'm alive, my mode is background, my pid is 19428
O | I'm alive, my mode is background, my pid is 19428
O | I'm alive, my mode is background, my pid is 19428
O | I'm alive, my mode is background, my pid is 19428
O | I'm alive, my mode is background, my pid is 19428
O | I'm alive, my mode is background, my pid is 19428
O | I'm alive, my mode is background, my pid is 19428
O | I'm alive, my mode is background, my pid is 19428
O | I'm alive, my mode is background, my pid is 19428
O | I'm alive, my mode is background, my pid is 19428
O | I'm alive, my mode is background, my pid is 19428
O | I'm alive, my mode is background, my pid is 19428
O | I'm alive, my mode is background, my pid is 19428
O | I'm alive, my mode is background, my pid is 19428
O | I'm alive, my mode is background, my pid is 19428
O | I'm alive, my mode is background, my pid is 19428
O | I'm alive, my mode is background, my pid is 19428
O | I'm alive, my mode is background, my pid is 19428
O | I'm alive, my mode is background, my pid is 19428
O | I'm alive, my mode is background, my pid is 19428
O | I'm alive, my mode is background, my pid is 19428
L | Process finished. Got signal 9

</pre>





```python
!time -p timeout 3 ./daemon.exe background  # завершается мгновенно
```

    real 0.00
    user 0.00
    sys 0.00



```python
!sleep 2
```


```python
# по файлу с логом находим pid демона и прибиваем его сигналом
!cat log.txt | python3 -c "import os, sys, signal; pid = int(list(sys.stdin)[0].split()[-1]) ; os.kill(pid, signal.SIGINT)"
```


```python
a.close()
```


```python

```


```python

```

## <a name="fork_exec"></a> **fork & exec** - запуск внешних программ как дочерних процессов 

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


Run: `gcc fork_exec.cpp -o fork_exec.exe`



Run: `./fork_exec.exe`


    USER         PID %CPU %MEM    VSZ   RSS TTY      STAT START   TIME COMMAND
    root           1  0.0  0.5 169056 11996 ?        Ss   ноя23   0:10 /sbin/init splash
    root           2  0.0  0.0      0     0 ?        S    ноя23   0:00 [kthreadd]
    root           3  0.0  0.0      0     0 ?        I<   ноя23   0:00 [rcu_gp]
    Child exited with code 0 
    	User time 0 sec 35884 usec
    	Sys time 0 sec 102925 usec



```python

```

## <a name="dup2"></a> **dup2** - изобретаем freopen

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


Run: `gcc fork_exec_pipe.cpp -o fork_exec_pipe.exe`



Run: `./fork_exec_pipe.exe`



Run: `echo "After program finish" && cat out.txt`


    After program finish
    Redirectred 'Hello world!'

## <a name="redirect"></a> **exec + dup2** - изобретаем перенаправление вывода программы в файл: `echo Hello > file.txt`


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


Run: `gcc redirect.cpp -o redirect.exe`



Run: `./redirect.exe out.txt   ps aux`



Run: `cat out.txt | head -n 2`


    USER       PID %CPU %MEM    VSZ   RSS TTY      STAT START   TIME COMMAND
    root         1  0.0  0.2  37752  4284 ?        Ss   Feb27   0:12 /lib/systemd/systemd --system --deserialize 21


## **fork & exec & pipe & dup2** - изобретаем перенаправление вывода одной программы на вход другой: `cat file.txt | grep ERROR`

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
    close(fd[1]);
    
    if ((pid_2 = fork()) == 0) {
        dup2(fd[0], 0);
        close(fd[0]);
        execlp("tail", "tail", "-n", "4", NULL);
        assert(0 && "Unreachable position in code if execlp succeeded");
    }
    close(fd[0]);
    
    int status;
    assert(waitpid(pid_1, &status, 0) != -1);
    assert(waitpid(pid_2, &status, 0) != -1);
    return 0;
}
```


Run: `gcc fork_exec_pipe.cpp -o fork_exec_pipe.exe`



Run: `./fork_exec_pipe.exe`


    pechatn+   19573  0.0  0.0   2608   548 pts/3    Ss+  18:41   0:00 /usr/bin/sh -c ./fork_exec_pipe.exe
    pechatn+   19574  0.0  0.0   2356   588 pts/3    S+   18:41   0:00 ./fork_exec_pipe.exe
    pechatn+   19575  0.0  0.1  20124  3396 pts/3    R+   18:41   0:00 ps aux
    pechatn+   19576  0.0  0.0  16748   592 pts/3    S+   18:41   0:00 tail -n 4



```python
!man dup2
```

    DUP(2)                     Linux Programmer's Manual                    DUP(2)
    
    NNAAMMEE
           dup, dup2, dup3 - duplicate a file descriptor
    
    SSYYNNOOPPSSIISS
           ##iinncclluuddee <<uunniissttdd..hh>>
    
           iinntt dduupp((iinntt _o_l_d_f_d));;
           iinntt dduupp22((iinntt _o_l_d_f_d,, iinntt _n_e_w_f_d));;
    
           ##ddeeffiinnee __GGNNUU__SSOOUURRCCEE             /* See feature_test_macros(7) */
           ##iinncclluuddee <<ffccnnttll..hh>>              /* Obtain O_* constant definitions */
           ##iinncclluuddee <<uunniissttdd..hh>>
    
           iinntt dduupp33((iinntt _o_l_d_f_d,, iinntt _n_e_w_f_d,, iinntt _f_l_a_g_s));;
    
    DDEESSCCRRIIPPTTIIOONN
           The  dduupp()  system  call  creates  a copy of the file descriptor _o_l_d_f_d,
           using the lowest-numbered unused descriptor for the new descriptor.
    
           After a successful return, the old and new file descriptors may be used
           interchangeably.   They  refer  to  the same open file description (see
           ooppeenn(2)) and thus share file offset and file status flags; for example,
           if the file offset is modified by using llsseeeekk(2) on one of the descrip‐
           tors, the offset is also changed for the other.
    
           The two descriptors do not share file descriptor flags  (the  close-on-
           exec  flag).  The close-on-exec flag (FFDD__CCLLOOEEXXEECC; see ffccnnttll(2)) for the
           duplicate descriptor is off.
    
       dduupp22(())
           The dduupp22() system call performs the same task as dduupp(), but instead  of
           using  the lowest-numbered unused file descriptor, it uses the descrip‐
           tor number specified in _n_e_w_f_d.  If the descriptor _n_e_w_f_d was  previously
           open, it is silently closed before being reused.
    
           The  steps  of  closing  and reusing the file descriptor _n_e_w_f_d are per‐
           formed _a_t_o_m_i_c_a_l_l_y.  This is  important,  because  trying  to  implement
           equivalent  functionality  using cclloossee(2) and dduupp() would be subject to
           race conditions, whereby _n_e_w_f_d might be reused between the  two  steps.
           Such  reuse  could  happen because the main program is interrupted by a
           signal handler that allocates a file descriptor, or because a  parallel
           thread allocates a file descriptor.
    
           Note the following points:
    
           *  If  _o_l_d_f_d  is  not a valid file descriptor, then the call fails, and
              _n_e_w_f_d is not closed.
    
           *  If _o_l_d_f_d is a valid file descriptor, and _n_e_w_f_d has the same value as
              _o_l_d_f_d, then dduupp22() does nothing, and returns _n_e_w_f_d.
    
       dduupp33(())
           dduupp33() is the same as dduupp22(), except that:
    
           *  The  caller  can  force the close-on-exec flag to be set for the new
              file descriptor by specifying OO__CCLLOOEEXXEECC in _f_l_a_g_s.  See the  descrip‐
              tion of the same flag in ooppeenn(2) for reasons why this may be useful.
    
           *  If _o_l_d_f_d equals _n_e_w_f_d, then dduupp33() fails with the error EEIINNVVAALL.
    
    RREETTUURRNN VVAALLUUEE
           On success, these system calls return the new descriptor.  On error, -1
           is returned, and _e_r_r_n_o is set appropriately.
    
    EERRRROORRSS
           EEBBAADDFF  _o_l_d_f_d isn't an open file descriptor.
    
           EEBBAADDFF  _n_e_w_f_d is out of the allowed range for file descriptors (see  the
                  discussion of RRLLIIMMIITT__NNOOFFIILLEE in ggeettrrlliimmiitt(2)).
    
           EEBBUUSSYY  (Linux  only)  This may be returned by dduupp22() or dduupp33() during a
                  race condition with ooppeenn(2) and dduupp().
    
           EEIINNTTRR  The dduupp22() or dduupp33() call was interrupted by a signal; see  ssiigg‐‐
                  nnaall(7).
    
           EEIINNVVAALL (dduupp33()) _f_l_a_g_s contain an invalid value.
    
           EEIINNVVAALL (dduupp33()) _o_l_d_f_d was equal to _n_e_w_f_d.
    
           EEMMFFIILLEE The per-process limit on the number of open file descriptors has
                  been reached (see  the  discussion  of  RRLLIIMMIITT__NNOOFFIILLEE  in  ggeettrr‐‐
                  lliimmiitt(2)).
    
    VVEERRSSIIOONNSS
           dduupp33() was added to Linux in version 2.6.27; glibc support is available
           starting with version 2.9.
    
    CCOONNFFOORRMMIINNGG TTOO
           dduupp(), dduupp22(): POSIX.1-2001, POSIX.1-2008, SVr4, 4.3BSD.
    
           dduupp33() is Linux-specific.
    
    NNOOTTEESS
           The error returned  by  dduupp22()  is  different  from  that  returned  by
           ffccnnttll((..., FF__DDUUPPFFDD, ...))  when _n_e_w_f_d is out of range.  On some systems,
           dduupp22() also sometimes returns EEIINNVVAALL like FF__DDUUPPFFDD.
    
           If _n_e_w_f_d was open, any errors that would have been reported at cclloossee(2)
           time  are lost.  If this is of concern, then—unless the program is sin‐
           gle-threaded and does not allocate file descriptors in signal handlers—
           the  correct  approach  is  _n_o_t  to  close _n_e_w_f_d before calling dduupp22(),
           because of the race condition described above.  Instead, code something
           like the following could be used:
    
               /* Obtain a duplicate of 'newfd' that can subsequently
                  be used to check for close() errors; an EBADF error
                  means that 'newfd' was not open. */
    
               tmpfd = dup(newfd);
               if (tmpfd == -1 && errno != EBADF) {
                   /* Handle unexpected dup() error */
               }
    
               /* Atomically duplicate 'oldfd' on 'newfd' */
    
               if (dup2(oldfd, newfd) == -1) {
                   /* Handle dup2() error */
               }
    
               /* Now check for close() errors on the file originally
                  referred to by 'newfd' */
    
               if (tmpfd != -1) {
                   if (close(tmpfd) == -1) {
                       /* Handle errors from close */
                   }
               }
    
    SSEEEE AALLSSOO
           cclloossee(2), ffccnnttll(2), ooppeenn(2)
    
    CCOOLLOOPPHHOONN
           This  page  is  part of release 4.04 of the Linux _m_a_n_-_p_a_g_e_s project.  A
           description of the project, information about reporting bugs,  and  the
           latest     version     of     this    page,    can    be    found    at
           http://www.kernel.org/doc/man-pages/.
    
    Linux                             2015-12-28                            DUP(2)



```python

```

# malloc fork

Один из интересных багов, которые случались в Facebook, включал в себя такую интересную комбинацию:

Если сделать malloc(1005000000), то что произойдет? Вызов завершиться, память выделится, но не совсем честно: не все страницы созданной виртуальной памяти будут иметь под собой физические страницы. Поэтому можно так "выделить" памяти больше, чем есть в системе. И пока мы как-то не проивзаимодействуем с выделенными страницами, они не будут присоединены к физической памяти.

А вот если потом сделать fork, то что будет? По идее fork не копирует сразу физические страницы в памяти, а делает их cow (copy on write), так что потребление памяти не должно измениться. 

Но оказалось, что при вызове fork вся "выделенная" память реально выделяется. То есть для этих 1005000000 выделенных байт реально ищутся страницы физической памяти. Поэтому при вызове fork всё взрывалось. 

Правда я не смог воспроизвести этот баг :)


## Загадка на потребление памяти


```cpp
%%cpp mem.c
%run gcc mem.c -o mem.exe
%run bash -c 'ulimit -v 1000000 ; ./mem.exe'
%run bash -c 'ulimit -v 1000000 ; /usr/bin/time -v ./mem.exe 2>&1 | grep resident'

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N (int)1e8
#define M (int)1e6
    
int array[N];
    
int main() {
    int* mem = malloc(N * sizeof(int));
    memset(mem, 255, M * sizeof(int));
    free(mem);
    return 0;
}
```


Run: `gcc mem.c -o mem.exe`



Run: `bash -c 'ulimit -v 1000000 ; ./mem.exe'`



Run: `bash -c 'ulimit -v 1000000 ; /usr/bin/time -v ./mem.exe 2>&1 | grep resident'`


    	Maximum resident set size (kbytes): 5024
    	Average resident set size (kbytes): 0



```python

```

## <a name="hw"></a> Комментарии к ДЗ



* Тестируйте форк-бомбу аккуратно. Например, как описано в этом ноутбуке)


```python

```
