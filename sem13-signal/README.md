```python
# initialize magics, for multiline version look at previous notebooks
exec('\nget_ipython().run_cell_magic(\'javascript\', \'\', \'// setup cpp code highlighting\\nIPython.CodeCell.options_default.highlight_modes["text/x-c++src"] = {\\\'reg\\\':[/^%%cpp/]} ;\')\n\n# creating magics\nfrom IPython.core.magic import register_cell_magic, register_line_magic\nfrom IPython.display import display, Markdown\nimport argparse\n\n@register_cell_magic\ndef save_file(args_str, cell, line_comment_start="#"):\n    parser = argparse.ArgumentParser()\n    parser.add_argument("fname")\n    parser.add_argument("--ejudge-style", action="store_true")\n    args = parser.parse_args(args_str.split())\n    \n    cell = cell if cell[-1] == \'\\n\' or args.no_eof_newline else cell + "\\n"\n    cmds = []\n    with open(args.fname, "w") as f:\n        f.write(line_comment_start + " %%cpp " + args_str + "\\n")\n        for line in cell.split("\\n"):\n            if line.startswith("%"):\n                run_prefix = "%run "\n                assert line.startswith(run_prefix)\n                cmds.append(line[len(run_prefix):].strip())\n            else:\n                f.write((line if not args.ejudge_style else line.rstrip()) + "\\n")\n        f.write("" if not args.ejudge_style else line_comment_start + r" line without \\n")\n    for cmd in cmds:\n        display(Markdown("Run: `%s`" % cmd))\n        get_ipython().system(cmd)\n\n@register_cell_magic\ndef cpp(fname, cell):\n    save_file(fname, cell, "//")\n\n@register_cell_magic\ndef asm(fname, cell):\n    save_file(fname, cell, "//")\n    \n@register_cell_magic\ndef makefile(fname, cell):\n    assert not fname\n    save_file("makefile", cell.replace(" " * 4, "\\t"))\n        \n@register_line_magic\ndef p(line):\n    try:\n        expr, comment = line.split(" #")\n        display(Markdown("`{} = {}`  # {}".format(expr.strip(), eval(expr), comment.strip())))\n    except:\n        display(Markdown("{} = {}".format(line, eval(line))))\n    \n')
```


    <IPython.core.display.Javascript object>


# Что такое сигналы и как закрыть vim

Сигналы, которые можно послать из терминала запущенной программе:
* Ctrl-C посылает SIGINT (от interrupt), обычное действие - завершение программы.
* Ctrl-\\ посылает SIGQUIT, обычное действие - завершение с дампом памяти. **В целом срабатывает чаще чем Ctrl-C**
* Ctrl-Z посылает SIGTSTP, обычное действие - остановка процесса. То есть как SIGSTOP

Другие полезные сигналы:
* SIGKILL - безусловное убиение процесса. 
* SIGSTOP - безусловная остановка программы.
* SIGCONT - продолжение выполнения (отмена SIGSTOP)

Как убить неубиваемое?
* `killall -9 vim` или `ps aux | grep vim`, а потом уже `kill -9 <selected_pid>`. Надо заметить, что `-9` лучше писать как `-SIGKILL`, но это длиннее, так что на конкретной платформе в интерактивном режиме проще писать `-9`. А `-SIGKILL` оставить для переносимых приложений.


[Ссылка на ридинг Яковлева](https://github.com/victor-yacovlev/mipt-diht-caos/tree/master/practice/signal-1)
и на [вторую его часть](https://github.com/victor-yacovlev/mipt-diht-caos/blob/master/practice/signal-2/README.md)

[Пост на хабре](https://habr.com/ru/post/141206/)

**Все это дело (сигналы) плохо сочетается с потоками**

<details>
<summary>Доставка сигналов в случае, когда есть несколько потоков</summary>
  <p>

Сигнал поступает в нить, если он не должен игнорироваться. Доставка сигналов в процессах с несколькими нитями подчиняется следующим правилам:

* Если по сигналу следует завершить, остановить или продолжить целевую нить, то при обработке сигнала соответственно завершается, останавливается или возобновляется весь процесс (а следовательно, все его нити). Это означает, что программы с одной нитью можно переработать в программы с несколькими нитями, не изменяя в них видимую сторону обработки сигналов.
Рассмотрим пользовательскую команду с несколькими нитями, например, команду grep. Пользователь может запустить эту команду из оболочки и затем попытаться прервать ее выполнение, передав соответствующий сигнал командой kill. Очевидно, что этот сигнал прервет весь процесс, в котором выполняется команда grep.

* Сигналы, соответствующие конкретной нити и отправленные с помощью функций pthread_kill или raise, передаются в эту нить. Если эта нить заблокировала доставку данного сигнала, то сигнал переходит в состояние ожидания на уровне нити, пока доставка не будет разблокирована. Если выполнение нити завершилось раньше доставки сигнала, то сигнал будет проигнорирован.
* Сигналы, соответствующие процессу и отправленные, например, с помощью функции kill, передаются только одной нити процесса. Если одна или несколько нитей вызвали функцию sigwait, то сигнал передается ровно в одну из них. В противном случае сигнал передается ровно в одну нить из числа тех нитей, которые не блокировали его доставку. Если нитей, удовлетворяющих этим условиям, нет, то сигнал переходит в состояние ожидания на уровне процесса до тех пор, пока какая-либо нить не вызовет функцию sigwait с указанием этого сигнала или пока доставка не будет разблокирована.


Если ожидающий сигнал (на уровне нити или процесса) должен игнорироваться, то он игнорируется.  
  
[Источник](http://www.regatta.cs.msu.su/doc/usr/share/man/info/ru_RU/a_doc_lib/aixprggd/genprogc/signal_mgmt.htm)

**Как с этим жить?** Принимать сигналы только в одном потоке и вызывать хендлеры в контексте выполнения потока. Тогда можно будет не беспокоиться о async-signal safety и ограничиться thread safety.

  </p>
</details>


## Как сигналы приходят в программу?

Когда одна программу отправляет сигнал другой, то этот сигнал записывается в атрибуты программы получателя. Если это обычный сигнал, то проставится бит в маске ожидающих доставки сигналов (SigPnd), если сигнал реального времени, то запишется в очередь сигналов. 

Сама программа-получатель из своего кода не взаимодействует с маской ожидающих сигналов или очередью сигналов. С ними взаимодействует ядро системы. Оно же обрабатывает (или не обрабатывает, если доставка сигналов заблокирована SigBlk) эти сигналы либо действиями по умолчанию, либо игнорированем (SigIgn), либо **останавливая выполнение программы в произвольный для нее момент** и вызывая в ней обработчик сигнала в отдельном контексте (на отдельном стеке вызовов функций).

Отсюда вытекает **требование к асинхронной безопасности обработчиков сигналов**. Например, если основная программа может быть прервана на обработку сигнала в момент вызова printf, и в обработчике тоже используется эта функция, то есть шанс, что в вас все взорвется (испортится глобальный стейт функции printf и, возможно, еще что-то) или намертво зависнет (если в основной программе был взят lock, и теперь снова пытается взяться уже из обработчика сигнала).

Бороться с этим можно тремя способами: 
  1. Писать честные асинхронно-безопасные обработчики (взаимодействие с остальной частью программы только через  sig_atomic_t переменные и системные вызовы).
  1. Использовать `sigsuspend` (чтобы обработчики могли выполняться, только пока выполняется sigsuspend)
  1. Использовать `signalfd`. С ней можно обрабатывать сигналы в контексте основного потока программы, соответственно никаких требований к асинхронной безопасности. Но это linux-специфичное решение.

## Что значит завершение с дампом памяти?


```cpp
%%cpp coredump.c
%run gcc -g coredump.c -o coredump.exe
%run rm core # удаляем старый файл с coredump
%run ./coredump.exe

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>

// can be replaced with 'ulimit -c unlimited' in terminal
void enable_core() {
    struct rlimit rlim;
    assert(0 == getrlimit(RLIMIT_CORE, &rlim));
    rlim.rlim_cur = rlim.rlim_max;
    assert(0 == setrlimit(RLIMIT_CORE, &rlim));
}

int f(int a) {
    if (1) {
        assert(a > 4); // тоже вызывает SIGABRT
    } else {
        if (a < 4) {
            raise(SIGABRT); // посылаем сигнал себе
        }
    }
    return 0;
}

int main(int argc, char** argv) {
    enable_core();
    return f(argc);
}
```


Run: `gcc -g coredump.c -o coredump.exe`



Run: `rm core # удаляем старый файл с coredump`



Run: `./coredump.exe`


    coredump.exe: coredump.c:21: f: Assertion `a > 4' failed.
    Aborted (core dumped)



```python
!test -f ./core && gdb -ex='r' -ex="bt full" -batch ./coredump.exe ./core || echo "No core file :("
```

    [New LWP 10276]
    Core was generated by `./coredump.exe'.
    Program terminated with signal SIGABRT, Aborted.
    #0  0x00007fed74d9a428 in __GI_raise (sig=sig@entry=6) at ../sysdeps/unix/sysv/linux/raise.c:54
    54	../sysdeps/unix/sysv/linux/raise.c: No such file or directory.
    coredump.exe: coredump.c:21: f: Assertion `a > 4' failed.
    
    Program received signal SIGABRT, Aborted.
    0x00007ffff7a42428 in __GI_raise (sig=sig@entry=6) at ../sysdeps/unix/sysv/linux/raise.c:54
    54	../sysdeps/unix/sysv/linux/raise.c: No such file or directory.
    #0  0x00007ffff7a42428 in __GI_raise (sig=sig@entry=6) at ../sysdeps/unix/sysv/linux/raise.c:54
            resultvar = 0
            pd = <optimized out>
            pid = 10289
            selftid = 10289
    #1  0x00007ffff7a4402a in __GI_abort () at abort.c:89
            save_stage = 2
            act = {__sigaction_handler = {sa_handler = 0x4, sa_sigaction = 0x4}, sa_mask = {__val = {0, 1095216660480, 140737488347056, 0, 140737354096640, 4196355, 21, 4196380, 0, 0, 140737348441404, 140737349538384, 140737349552032, 0, 140737349538384, 4196355}}, sa_flags = -134258688, sa_restorer = 0x400803}
            sigs = {__val = {32, 0 <repeats 15 times>}}
    #2  0x00007ffff7a3abd7 in __assert_fail_base (fmt=<optimized out>, assertion=assertion@entry=0x400803 "a > 4", file=file@entry=0x4007a8 "coredump.c", line=line@entry=21, function=function@entry=0x40081c <__PRETTY_FUNCTION__.3343> "f") at assert.c:92
            str = 0x602010 ""
            total = 4096
    #3  0x00007ffff7a3ac82 in __GI___assert_fail (assertion=0x400803 "a > 4", file=0x4007a8 "coredump.c", line=21, function=0x40081c <__PRETTY_FUNCTION__.3343> "f") at assert.c:101
    No locals.
    #4  0x00000000004006f2 in f (a=1) at coredump.c:21
            __PRETTY_FUNCTION__ = "f"
    #5  0x000000000040071c in main (argc=1, argv=0x7fffffffe218) at coredump.c:32
    No locals.


# alarm - таймер с использованием сигналов

Системный вызов `alarm` запускает таймер, по истечении которого процесс сам себе отправит сигнал `SIGALRM`.

```
unsigned int alarm(unsigned int seconds);
```


```cpp
%%cpp alarm.c
%run gcc -g alarm.c -o alarm.exe
%run ./alarm.exe ; echo $? # выводим так же код возврата

#include <unistd.h>
#include <stdio.h>

int main() {
    alarm(3);
    pause();
    printf("Is this command unreachable?"); // достижима ли эта команда?
    return 0;
}
```


Run: `gcc -g alarm.c -o alarm.exe`



Run: `./alarm.exe ; echo $? # выводим так же код возврата`


    Alarm clock
    142


# signal
В прошлом примере мы заметили, что использование сигналов без их обработки имеет ограниченную ценность. 

Поэтому рассмотрим способ для начала блокировать сигналы.


```cpp
%%cpp alarm_block.c
%run gcc -g alarm_block.c -o alarm_block.exe
%run ./alarm_block.exe ; echo $? # выводим так же код возврата

#include <unistd.h>
#include <stdio.h>
#include <signal.h>

int main() {
    signal(SIGALRM, SIG_IGN);
    alarm(3);
    pause();
    printf("Is this command unreachable?"); // достижима ли эта команда?
    return 0;
}
```


Run: `gcc -g alarm_block.c -o alarm_block.exe`



Run: `./alarm_block.exe ; echo $? # выводим так же код возврата`


    ^C


Упс, но теперь сигнал вообще до нас не доходит.

Это бывает довольно полезно, например, если мы хотим сделать программу устойчивой к прерываниям через Ctrl-C. Но в остальных случаях это не помогает.

Значит нужно явно задавать обработчики сигналов (пусть даже пустые). Это можно делать через вызов signal, передавая функцию-обработчик, но так лучше не делать, так как там нестандартизированное поведение и лучше использовать более новый вызов sigaction. Впрочем, особенности вызова signal стоит знать, если вы вдруг на него наткнетесь.

# sigaction

Позволяет установить функцию-обработчик на сигнал. Функция-обработчик должна быть async-signal safe. То есть ее вызов должен быть безопасен в любой момент выполнения основного кода программы. Это условие сильнее чем thread-safe.


```cpp
%%cpp alarm_handle.c
%run gcc -g alarm_handle.c -o alarm_handle.exe
%run ./alarm_handle.exe ; echo $? # выводим так же код возврата

#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>

static void handler(int signum) {
    static char buffer[100];
    int size = snprintf(buffer, sizeof(buffer), "Get signal %d, do nothing\n", signum);
    write(2, buffer, size); // можно использовать системные вызовы, они async-signal safe
    // fprintf(stderr, "Get signal %d, do nothing\n", signum); // А вот это уже использовать нелья
}

int main() {
    sigaction(SIGALRM,
              &(struct sigaction){
                  .sa_handler = handler, 
                  .sa_flags = SA_RESTART
              },
              NULL);
    alarm(1);
    pause();
    printf("Is this command unreachable?\n"); // достижима ли эта команда?
    return 0;
}
```


Run: `gcc -g alarm_handle.c -o alarm_handle.exe`



Run: `./alarm_handle.exe ; echo $? # выводим так же код возврата`


    Get signal 14, do nothing
    Is this command unreachable?
    0


# Делаем программу-терминатора

По умолчанию все сигналы обрабатываются немедленно, но это может вызвать гонку данных и поэтому неудобно. К счастью, есть способ приостановить обработку сигналов до поры до времени (заблокировать сигнал), а потом, когда захочется, выполнить обработчики.

# sigprocmask, sigsuspend
sigprocmask позволяет выбрать сигналы, которые будут заблокированы. sigsuspend позволяет подождать, пока придут определенные сигналы (он как бы разблокирует сигналы, подождет пока что-то придет, потом снова заблокирует).

В следующем примере нужно вручную из терминала посылать сигналы в запущенный процесс (`kill -SIGINT <pid>`)


```cpp
%%cpp terminator.c
%run gcc -g terminator.c -o terminator.exe
%run ./terminator.exe 

#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>

int inside_sigsuspend = 0;

static void handler(int signum) {
    // Сейчас у нас есть некоторая гарантия, что обработчик будет вызван только внутри sigprocmask 
    // (ну или раньше изначального sigprocmask)
    // поэтому в случае однопоточного приложения можно использовать асинхронно-небезопасные функции
    fprintf(stderr, "Get signal %d, inside_sigsuspend = %d ( == 1 ?), do nothing\n", 
            signum, inside_sigsuspend);  
}

int main() {
    sigset_t mask;
    sigfillset(&mask);
    sigprocmask(SIG_BLOCK, &mask, NULL); // try comment out
    
    for (int signal = 0; signal < 100; ++signal) {
        sigaction(signal,
                  &(struct sigaction) {
                      .sa_handler=handler, 
                      .sa_flags=SA_RESTART, 
                      // этот параметр говорит, что во время вызова обработчика сигнала
                      // будут заблокированы сигналы указанные в маске (то есть все)
                      .sa_mask=mask 
                  },
                  NULL);
    }
    
    sigemptyset(&mask);
    printf("pid = %d\n", getpid());
    
    int res = 0;
    
    raise(SIGINT);
    raise(SIGCHLD);
    raise(SIGCHLD);
    
    while (1) {
        inside_sigsuspend = 1;
        sigsuspend(&mask); // try comment out
        inside_sigsuspend = 0;
        for (int i = 0; i < 10000000; ++i) {
            res ^= i;
        }
    }
    return res;
}
```


Run: `gcc -g terminator.c -o terminator.exe`



Run: `./terminator.exe`


    pid = 28790
    Get signal 2, inside_sigsuspend = 1 ( == 1 ?), do nothing
    Get signal 17, inside_sigsuspend = 1 ( == 1 ?), do nothing
    ^C
    Get signal 2, inside_sigsuspend = 1 ( == 1 ?), do nothing



```python
!kill -2 25545
```

    /bin/sh: 1: kill: No such process
    


# Ping-pong


```cpp
%%cpp pipo.c
%run gcc -g pipo.c -o pipo.exe
%run ./pipo.exe 

#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>

sig_atomic_t last_signal = 0;

static void handler(int signum) {
    last_signal = signum;  // что плохо с таким обработчиком?
}

int main() {
    sigset_t mask;
    sigfillset(&mask);
    sigprocmask(SIG_BLOCK, &mask, NULL);
    
    int signals[] = {SIGUSR1, SIGINT, 0};
    for (int* signal = signals; *signal; ++signal) {
        sigaction(*signal, &(struct sigaction){.sa_handler=handler, .sa_flags=SA_RESTART, .sa_mask=mask}, NULL);
    }
    
    sigemptyset(&mask);
    
    int parent_pid = getpid();
    
    int child_pid = fork();
    assert(child_pid >= 0);
    if (child_pid == 0) {
        while (1) {
            sigsuspend(&mask);
            if (last_signal) {
                if (last_signal == SIGUSR1) {
                    printf("Child process: Pong\n"); fflush(stdout);
                    kill(parent_pid, SIGUSR1);
                } else {
                    printf("Child process finish\n"); fflush(stdout);
                    return 0;
                }
                last_signal = 0;
            }
        }
    } else {
        for (int i = 0; i < 3; ++i) {
            printf("Parent process: Ping\n"); fflush(stdout);
            kill(child_pid, SIGUSR1);
            while (1) {
                sigsuspend(&mask);
                if (last_signal) { last_signal = 0; break; }
            }
        }
        printf("Parent process: Request child finish\n"); fflush(stdout);
        kill(child_pid, SIGINT); 
        int status;
        waitpid(child_pid, &status, 0);
    }
    return 0;
}
```


Run: `gcc -g pipo.c -o pipo.exe`



Run: `./pipo.exe`


    Parent process: Ping
    Child process: Pong
    Parent process: Ping
    Child process: Pong
    Parent process: Ping
    Child process: Pong
    Parent process: Request child finish
    Child process finish



```python

```

# Сигналы реального времени. 
Они передаются через очередь, а не через маску, как обычные.


```cpp
%%cpp sigqueue.c
%run gcc -g sigqueue.c -o sigqueue.exe
%run ./sigqueue.exe 

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>

sig_atomic_t last_signal = 0;

static void handler(int signum) {
    if (signum == SIGUSR1) {
        printf("Child process: got SIGUSR1\n"); fflush(stdout);
    } else if (signum == SIGINT) {
        printf("Child process: got SIGINT, finish\n"); fflush(stdout);
        exit(0);
    } else {
        printf("Child process: got SIGRTMIN\n"); fflush(stdout);
    }
}

int main() {
    assert(SIGRTMIN < SIGRTMAX);
    sigset_t mask;
    sigfillset(&mask);
    sigprocmask(SIG_BLOCK, &mask, NULL);
    
    int signals[] = {SIGUSR1, SIGINT, SIGRTMIN, 0};
    for (int* signal = signals; *signal; ++signal) {
        sigaction(*signal, &(struct sigaction){.sa_handler=handler, .sa_flags=SA_RESTART, .sa_mask=mask}, NULL);
    }
    
    sigemptyset(&mask);
    
    int parent_pid = getpid();
    int child_pid = fork();
    assert(child_pid >= 0);
    if (child_pid == 0) {
        while (1) { sigsuspend(&mask); }
    } else {
        for (int i = 0; i < 10; ++i) 
            assert(kill(child_pid, SIGUSR1) == 0);
        for (int i = 0; i < 10; ++i)
            assert(sigqueue(child_pid, SIGRTMIN, (union sigval){0}) == 0);
        sleep(1);
        printf("Parent process: Request child finish with SIGINT\n"); fflush(stdout);
        kill(child_pid, SIGINT); 
        int status;
        waitpid(child_pid, &status, 0);
    }
    return 0;
}
```


Run: `gcc -g sigqueue.c -o sigqueue.exe`



Run: `./sigqueue.exe`


    Child process: got SIGUSR1
    Child process: got SIGRTMIN
    Child process: got SIGRTMIN
    Child process: got SIGRTMIN
    Child process: got SIGRTMIN
    Child process: got SIGRTMIN
    Child process: got SIGRTMIN
    Child process: got SIGRTMIN
    Child process: got SIGRTMIN
    Child process: got SIGRTMIN
    Child process: got SIGRTMIN
    Parent process: Request child finish with SIGINT
    Child process: got SIGINT, finish


# Ping-pong c sigqueue и доп. информацией


```cpp
%%cpp pipoqu.c
%run gcc -g pipoqu.c -o pipoqu.exe
%run ./pipoqu.exe 

#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>

sig_atomic_t last_signal = 0;
sig_atomic_t last_signal_value = 0;

// через info принимаем дополнительный int
static void handler(int signum, siginfo_t* info, void* ucontext) {
    last_signal = signum; 
    last_signal_value = info->si_value.sival_int; // сохраняем переданное число
}

int main() {
    sigset_t mask;
    sigfillset(&mask);
    sigprocmask(SIG_BLOCK, &mask, NULL);
    
    int signals[] = {SIGUSR1, SIGINT, 0};
    for (int* signal = signals; *signal; ++signal) {
        // обратите внимание, что хендлер теперь принимает больше аргументов
        // и записывается в другое поле
        // и еще есть флаг SA_SIGINFO, говорящий, что именно такой хендлер будет использоваться
        sigaction(*signal, &(struct sigaction){
            .sa_sigaction = handler, .sa_flags = SA_RESTART | SA_SIGINFO, .sa_mask=mask}, NULL);
    }
    
    sigemptyset(&mask);
    
    int parent_pid = getpid();
    
    int child_pid = fork();
    assert(child_pid >= 0);
    if (child_pid == 0) {
        while (1) {
            sigsuspend(&mask);
            if (last_signal) {
                if (last_signal == SIGUSR1) {
                    printf("Child process: Pong (get %d, send %d)\n", last_signal_value, last_signal_value * 2); 
                    fflush(stdout);
                    // вместе с сигналом передаем число
                    sigqueue(parent_pid, SIGUSR1, (union sigval) {.sival_int = last_signal_value * 2 });
                } else {
                    printf("Child process finish\n"); fflush(stdout);
                    return 0;
                }
                last_signal = 0;
            }
        }
    } else {
        int child_response = 10;
        for (int i = 0; i < 3; ++i) {
            printf("Parent process: Ping (got %d, send %d)\n", child_response, child_response + 1); fflush(stdout);
            sigqueue(child_pid, SIGUSR1, (union sigval) {.sival_int = child_response + 1 });
            while (!last_signal) {
                sigsuspend(&mask); 
            }
            last_signal = 0;
            child_response = last_signal_value;
        }
        printf("Parent process: Request child finish\n"); fflush(stdout);
        kill(child_pid, SIGINT); 
        int status;
        waitpid(child_pid, &status, 0);
    }
    return 0;
}
```


Run: `gcc -g pipoqu.c -o pipoqu.exe`



Run: `./pipoqu.exe`


    Parent process: Ping (got 10, send 11)
    Child process: Pong (get 11, send 22)
    Parent process: Ping (got 22, send 23)
    Child process: Pong (get 23, send 46)
    Parent process: Ping (got 46, send 47)
    Child process: Pong (get 47, send 94)
    Parent process: Request child finish
    Child process finish


# Ping-pong c sigqueue и sigwaitinfo доп. информацией


```cpp
%%cpp pipoquwa.c
%run gcc -g pipoquwa.c -o pipoquwa.exe
%run ./pipoquwa.exe 

#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    sigset_t full_mask;
    sigfillset(&full_mask);
    sigprocmask(SIG_BLOCK, &full_mask, NULL); 
    
    int parent_pid = getpid();
    
    int child_pid = fork();
    assert(child_pid >= 0);
    if (child_pid == 0) {
        while (1) {
            siginfo_t info;
            sigwaitinfo(&full_mask, &info); // вместо sigsuspend и обработчика
            int received_signal = info.si_signo;
            int received_value = info.si_value.sival_int;
            if (received_signal == SIGUSR1) {
                printf("Child process: Pong (get %d, send %d)\n", received_value, received_value * 2); 
                fflush(stdout);
                // вместе с сигналом передаем число
                sigqueue(parent_pid, SIGUSR1, (union sigval) {.sival_int = received_value * 2 });
            } else {
                printf("Child process finish\n"); fflush(stdout);
                return 0;
            }
        }
    } else {
        int child_response = 100;
        for (int i = 0; i < 3; ++i) {
            printf("Parent process: Ping (got %d, send %d)\n", child_response, child_response + 1); fflush(stdout);
            sigqueue(child_pid, SIGUSR1, (union sigval) {.sival_int = child_response + 1 });
            
            siginfo_t info;
            sigwaitinfo(&full_mask, &info);
            child_response = info.si_value.sival_int;
        }
        printf("Parent process: Request child finish\n"); fflush(stdout);
        kill(child_pid, SIGINT); 
        int status;
        waitpid(child_pid, &status, 0);
    }
    return 0;
}
```


Run: `gcc -g pipoquwa.c -o pipoquwa.exe`



Run: `./pipoquwa.exe`


    Parent process: Ping (got 100, send 101)
    Child process: Pong (get 101, send 202)
    Parent process: Ping (got 202, send 203)
    Child process: Pong (get 203, send 406)
    Parent process: Ping (got 406, send 407)
    Child process: Pong (get 407, send 814)
    Parent process: Request child finish
    Child process finish



```python

```



# Примеры применения
* мягкая остановка SIGINT и жесткая остановка SIGKILL
* ротирование логов


```python

```


```python

```

Вопросы для подготовки к контрольной:
* Что тут не так? Что произойдет? (x86 32-bit)

```c
int desired_fd = 4;
printf("We are to open file at %d fd. Yeah really at %d fd\n", desired_fd, desired_fd);
int fd = open("file.txt", O_WRONLY | O_CREAT | O_TRUNC);
dup2(fd, desired_fd);
```
* Страничная память: все что знаете
* Жизненный цикл процесса: все что знаете
* TLB-кеш, что это?
* Какая память релаьно копируется при вызове fork()?
* Файлы в linux, файловые системы и все около того.
* Как изменится число?

```c
union {
    double d;
    unsigned long long b;
} u = {1.0};

u.b ^= 1ull << 52;
printf("u.d = %lf\n", u.d);
```


```python

```


```python

```


```python

```
