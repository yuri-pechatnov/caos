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
* SIGKILL - безусловное убиение процесса. Правда, если процесс находится в блокирующем системном вызове, то сразу он не убьется. К счастью, системный вызовы досрочно завершаются, если пришел сигнал.
* SIGSTOP - безусловная остановка программы.
* SIGCONT - продолжение выполнения (отмена SIGSTOP)

Как убить неубиваемое?
* `killall -9 vim` или `ps aux | grep vim`, а потом уже `kill -9 <selected_pid>`. Надо заметить, что `-9` лучше писать как `-SIGKILL`, но это длиннее, так что на конкретной платформе в интерактивном режиме проще писать `-9`. А `-SIGKILL` оставить для переносимых приложений.


[Ссылка на ридинг Яковлева](https://github.com/victor-yacovlev/mipt-diht-caos/tree/master/practice/signal-1)

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
!test -f ./core && gdb -ex='r' -ex="bt full" -batch ./fork_exec.exe ./core || echo "No core file :("
```

    [New LWP 24062]
    Core was generated by `./fork_exec.exe'.
    Program terminated with signal SIGABRT, Aborted.
    #0  0x00007fc6b0833428 in __GI_raise (sig=sig@entry=6) at ../sysdeps/unix/sysv/linux/raise.c:54
    54	../sysdeps/unix/sysv/linux/raise.c: No such file or directory.
    fork_exec.exe: fork_exec.cpp:21: int f(int): Assertion `a > 4' failed.
    
    Program received signal SIGABRT, Aborted.
    0x00007ffff7a42428 in __GI_raise (sig=sig@entry=6) at ../sysdeps/unix/sysv/linux/raise.c:54
    54	../sysdeps/unix/sysv/linux/raise.c: No such file or directory.
    #0  0x00007ffff7a42428 in __GI_raise (sig=sig@entry=6) at ../sysdeps/unix/sysv/linux/raise.c:54
            resultvar = 0
            pd = <optimized out>
            pid = 24067
            selftid = 24067
    #1  0x00007ffff7a4402a in __GI_abort () at abort.c:89
            save_stage = 2
            act = {__sigaction_handler = {sa_handler = 0x4, sa_sigaction = 0x4}, sa_mask = {__val = {0, 1095216660480, 140737488347056, 0, 140737354096640, 4196363, 21, 4196408, 0, 0, 140737348441404, 140737349538384, 140737349552032, 0, 140737349538384, 4196363}}, sa_flags = -134258688, sa_restorer = 0x40080b}
            sigs = {__val = {32, 0 <repeats 15 times>}}
    #2  0x00007ffff7a3abd7 in __assert_fail_base (fmt=<optimized out>, assertion=assertion@entry=0x40080b "a > 4", file=file@entry=0x4007b0 "fork_exec.cpp", line=line@entry=21, function=function@entry=0x400838 <f(int)::__PRETTY_FUNCTION__> "int f(int)") at assert.c:92
            str = 0x602010 ""
            total = 4096
    #3  0x00007ffff7a3ac82 in __GI___assert_fail (assertion=0x40080b "a > 4", file=0x4007b0 "fork_exec.cpp", line=21, function=0x400838 <f(int)::__PRETTY_FUNCTION__> "int f(int)") at assert.c:101
    No locals.
    #4  0x00000000004006f2 in f (a=1) at fork_exec.cpp:21
            __PRETTY_FUNCTION__ = "int f(int)"
    #5  0x0000000000400717 in main (argc=1, argv=0x7fffffffe218) at fork_exec.cpp:32
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


Run: `gcc -g alarm.cpp -o alarm.exe`



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

Значит нужно явно задавать обработчики сигналов (пусть даже пустые). Это можно делать через вызов signal, передавая функцию-обработчик, но так лучше не делать, так как там нестандартизированное поведение и лучше использовать более новый вызов:

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

int val_x = 0;

static void handler(int signum) {
    // Сейчас у нас есть некоторая гарантия, что обработчик будет вызван только внутри sigprocmask 
    // (ну или раньше изначального sigprocmask)
    // поэтому в случае однопоточного приложения можно использовать асинхронно-небезопасные функции
    fprintf(stderr, "Get signal %d, val_x = %d ( == 1 ?), do nothing\n", signum, val_x); 
}

int main() {
    for (int signal = 0; signal < 100; ++signal) {
        sigaction(signal,
                  &(struct sigaction)
                  {.sa_handler=handler, .sa_flags=SA_RESTART},
                  NULL);
    }
    sigset_t mask;
    sigfillset(&mask);
    sigprocmask(SIG_BLOCK, &mask, NULL); // try comment out
    sigemptyset(&mask);
    printf("pid = %d\n", getpid());
    val_x = 0;
    int res = 0;
    while (1) {
        val_x = 1;
        sigsuspend(&mask); // try comment out
        val_x = 0;
        for (int i = 0; i < 10000000; ++i) {
            res ^= i;
        }
    }
    return res;
}
```


Run: `gcc -g terminator.c -o terminator.exe`



Run: `./terminator.exe`


    pid = 25613
    Get signal 2, val_x = 1 ( == 1 ?), do nothing
    Get signal 2, val_x = 1 ( == 1 ?), do nothing
    Get signal 2, val_x = 1 ( == 1 ?), do nothing
    Get signal 2, val_x = 1 ( == 1 ?), do nothing
    ^C
    Get signal 2, val_x = 1 ( == 1 ?), do nothing



```python
!kill -2 25545
```


```python

```



# Примеры применения
* мягкая остановка SIGINT и жесткая остановка SIGKILL
* ротирование логов
