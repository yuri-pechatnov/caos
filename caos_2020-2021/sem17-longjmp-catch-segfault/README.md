


# goto на максималках и как при segfault отряхнуться и работать дальше



<p><a href="https://www.youtube.com/watch?v=vVfuzKuL070&list=PLjzMm8llUm4AmU6i_hPU0NobgA4VsBowc&index=18" target="_blank">
    <h3>Видеозапись семинара</h3> 
</a></p>

Сегодня будем рассматривать:
* <a href="#goto" style="color:#856024">`goto`</a>
* <a href="#jmp" style="color:#856024">`setjmp` и `longjmp`</a>
* <a href="#segfault" style="color:#856024">Как обработать segfault</a>
* <a href="#bad_code" style="color:#856024">Примеры плохого кода из домашек</a>


```python
 
```

# goto

Еще раз напомню - хорошо думайте перед использованием goto, нужен ли он вам тут или можно обойтись без него. Так как его использование сильно повышает вероятность ошибок и неправильного понимания кода читающими.


```cpp
%%cpp go.cpp
%run gcc go.cpp -o go.exe
%run ./go.exe

#include <stdio.h>

int main() {
    int i = 0;
    start:
    printf("i = %d\n", i);
    if (++i < 3) {
        goto start;
    }
    return 0;
}
```


Run: `gcc go.cpp -o go.exe`



Run: `./go.exe`


    i = 0
    i = 1
    i = 2


# <a name="jmp"></a> `setjmp` и `longjmp`


```cpp
%%cpp jmp.cpp
%run gcc jmp.cpp -o jmp.exe
%run ./jmp.exe

#include <stdio.h>
#include <setjmp.h>


int main() {
    int i = 0;
    jmp_buf environment;
    int jmp_ret = setjmp(environment);
    printf("i = %d, jmp_ret=%d\n", i, jmp_ret);

    if (++i < 3) {
        longjmp(environment, i * 100 + 1);
    }
    return 0;
}
```


Run: `gcc jmp.cpp -o jmp.exe`



Run: `./jmp.exe`


    i = 0, jmp_ret=0
    i = 1, jmp_ret=101
    i = 2, jmp_ret=201



```cpp
%%cpp jmp2.cpp
%run gcc jmp2.cpp -o jmp2.exe
%run ./jmp2.exe

#include <stdio.h>
#include <setjmp.h>

jmp_buf environment;

void f(int i) {
    if (i == 3) {
        longjmp(environment, 1);
    }
    printf("i = %d\n", i);
    f(i + 1);
    printf("After recursive call 1\n");
}

int main() {
    int i = 0;
    
    if (setjmp(environment) == 0) {
        printf("Right after setjmp\n");
        f(0);
        printf("After recursive call 1\n");
    } else {
        printf("Right after return to setjmp\n");
    }
    return 0;
}
```


Run: `gcc jmp2.cpp -o jmp2.exe`



Run: `./jmp2.exe`


    Right after setjmp
    i = 0
    i = 1
    i = 2
    Right after return to setjmp



```python

```

# <a name="segfault"></a> Как обработать segfault

SIGSEGV - интересный сигнал, при получении сегфолта он обрабатывается немедленно, что открывает интересные возможности.

При подготовке примера опирался на вот это: https://habr.com/ru/post/332626/


```cpp
%%cpp segv.cpp
%run gcc segv.cpp -o segv.exe
%run ./segv.exe

#include <stdio.h>
#include <setjmp.h>
#include <assert.h>
#include <inttypes.h>
#include <signal.h>


jmp_buf segv_catcher_jmp_buf;

void segv_catcher_handler(int signum)
{
    // разблокируем пришедший сигнал вручную, 
    // так как из обработчика мы не выйдем нормальным образом
    sigset_t signals;
    sigemptyset(&signals);
    sigaddset(&signals, signum);
    sigprocmask(SIG_UNBLOCK, &signals, nullptr);
    
    dprintf(2, "Got signal %d\n", signum);
    
    longjmp(segv_catcher_jmp_buf, 1);
}


void segv_catcher_init() {
    // отдельный стек для обработчика сигнала
    // иначе при получении сегфолта из-за переполнения стека 
    // обработчику сигнала будет негде работать
    static uint8_t segv_catcher_stack[65536];
    stack_t ss;
    ss.ss_sp = segv_catcher_stack;
    ss.ss_flags = 0;
    ss.ss_size = sizeof(segv_catcher_stack);
    sigaltstack(&ss, NULL);
    
    // обработчик сигнала сегфолта
    struct sigaction sa{};
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_ONSTACK;
    sa.sa_handler = segv_catcher_handler;
    sigaction(SIGSEGV, &sa, NULL);
}

int f(int i) {
    return f(i + 1) + 1;
}

int f2(int i) {
    return i > 10 ? 1 : f2(i + 1) + 1;
}

int main() {
    segv_catcher_init();
    
    if (!setjmp(segv_catcher_jmp_buf)) {
        printf("Call f()\n");
        int res = f(0);
        printf("Success call of f(0) = %d\n", res);
    } else {
        printf("Failed call of f() - segfault\n");
    }
    
    if (!setjmp(segv_catcher_jmp_buf)) {
        printf("Call f()\n");
        int res = f(0);
        printf("Success call of f(0) = %d\n", res);
    } else {
        printf("Failed call of f() - segfault\n");
    }
    
    if (!setjmp(segv_catcher_jmp_buf)) {
        printf("Call f2()\n");
        int res = f2(0);
        printf("Success call of f2(0) = %d\n", res);
    } else {
        printf("Failed call of f2() - segfault\n");
    }
    
    return 0;
}
```


Run: `gcc segv.cpp -o segv.exe`



Run: `./segv.exe`


    Call f()
    Got signal 11
    Failed call of f() - segfault
    Call f()
    Got signal 11
    Failed call of f() - segfault
    Call f2()
    Success call of f2(0) = 12


Таким же образом можно перехватывать SIGBUS, SIGFPE, SIGILL


```python

```


```python

```

# <a name="bad_code"></a> Примеры плохого кода из домашек


```cpp
%%cpp ex0

volatile sig_atomic_t sigIntAmount = 0;
volatile sig_atomic_t sigTermAmount = 0;
void controller(int signal)
{
    if (signal == SIGTERM) {
        sigTermAmount++;
    }
    ...
}

int main()
{
    struct sigaction action;
    sigaction(SIGINT, ...controller...);
    sigaction(SIGTERM, ...controller...);
    printf("%d\n", getpid());
    fflush(stdout);
    while (sigTermAmount == 0) { pause();}
    printf("%d\n", sigIntAmount);
    return 0;
}

```


```python

```


```cpp
%%cpp ex1

sigaction(SIGALRM, ...trivial_handler...);
alarm(time_in_seconds);
int status;
pid_t daughter_process_pid = waitpid(pid, &status, 0);
```

В следующем примере все корректно будет работать. 
Но написано что-то сильно странное, найдите что.


```cpp
%%cpp ex2

mkfifo(fifo_name, 0666);
kill(proc_to_inform, SIGHUP);

int fd = -1;
while (1) {
    fd = open(fifo_name, O_WRONLY);
    if (fd >= 0) {
        break;
    }
}
```

Отрывок из обработчика сигнала. Найдите странность


```cpp
%%cpp ex3
int size = snprintf (buffer, sizeof (buffer), "%d\n", counter);
write (1, buffer, size);
fflush (stdout);
```


```python

```


```python
volatile sig_atomic_t counter = 0;   // counter of incoming sigints

void handle_exit(int signum) {
    fflush(stdout);
    exit(0);
}

void handle_sigusr(int signum) {
    if (signum == SIGUSR1) {
        counter++;
    } else {
        counter*= -1;
    }
    printf("%d\n", counter);
    fflush(stdout);
}


int main() {
    sigaction(SIGINT, ...);
    sigaction(SIGTERM, ...);
    sigaction(SIGUSR1, ...);
    sigaction(SIGUSR2, ...);

    printf("%d\n", getpid());
    fflush(stdout);

    scanf("%d", &counter);

    while (1) {
        pause();
    }
    return 0;
}
```


```cpp
%%cpp ex5

void controller(int signal)
{
    exit();
}

int main()
{
    sigaction(SIGINT, ...controller...);

    scanf("%d", &answer);
    printf("%d\n", getpid());
    fflush(stdout);
    
    while (signalAmount == 0) {
        pause();
    }
    return 0;
}
```


```python

```


```python

```


```python

```


```python

```
