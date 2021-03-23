


# Мультиплексирование

<br>
<div style="text-align: right"> Спасибо <a href="https://github.com/SyrnikRebirth">Сове Глебу</a> и <a href="https://github.com/Disadvantaged">Голяр Димитрису</a> за участие в написании текста </div>
<br>


<p><a href="https://www.youtube.com/watch?v=P2VqCECx3Io&list=PLjzMm8llUm4AmU6i_hPU0NobgA4VsBowc&index=20" target="_blank">
    <h3>Видеозапись семинара</h3> 
</a></p>


[Ридинг Яковлева](https://github.com/victor-yacovlev/mipt-diht-caos/tree/master/practice/epoll)

Мультиплексирование - о чем это? Это об одновременной работе с несколькими соединениями. О том, чтобы эффективно решать задачу: вот из этих файловых дескрипторов, нужно прочитать, сразу как только станет доступно, а вот в эти записать, опять же, сразу когда будет возможность.

[Хорошая статья на хабре: select / poll / epoll: практическая разница](https://habr.com/ru/company/infopulse/blog/415259/)
В этой же статье есть плюсы и минусы `select`/`poll`/`epoll`.

[Довольно детальная статья на хабре про epoll](https://habr.com/ru/post/416669/)

Способы мультиплексирования:
* <a href="#pipelike" style="color:#856024">Для работы с пайпами и сокетами</a>
* <a href="#aio" style="color:#856024">Linux AIO</a> - одновременная запись/чтение из нескольких файлов. (К сожалению, это только с файлами работает)

<a href="#hw" style="color:#856024">Комментарии к ДЗ</a>


```python

```

#  <a name="pipelike"></a> Варианты мультиплексирования ввода из пайпов и сокетов. От наивных и не работающих до epoll c edge-triggering   

С выводом то же самое. Чтобы говорить меньше букв, поговорим только в вводе.

* <a href="#just_read" style="color:#856024">Просто обычный read</a> - то, что можно написать, когда на события не нужно реагировать максимально быстро и можно позволить себе обработать первое пришедшее событие после последнего пришедшего.
* <a href="#read_nonblock" style="color:#856024"> read из файловых дескрипторов с опцией O_NONBLOCK </a> - с неблокирующим чтением легко позволить себе постоянно пытаться читать из интересных файловых дескрипторов. Но это кушает много процессорного времени - поэтому это далеко не лучший способ.
* <a href="#select" style="color:#856024">select</a> - старая штука, но стандартизированная (POSIX), и поддерживется практически везде, где есть интернет.
  Минусы: смотри статью на хабре. Но если кратко: в многопоточных программах крайне неудобен + <a href="#select_fail" style="color:#856024">не поддерживает больше 1024 файловых дескрипторов</a> (или просто файловые дескрипторы с номерами >= 1024, тут не уверен).
* <a href="#select" style="color:#856024">poll</a> - менее старая штука, стандартизированная (POSIX.1-2001 and POSIX.1-2008).
* <a href="#epoll" style="color:#856024">epoll</a> - linux
* kqueue - FreeBSD и MacOS. Аналог epoll. Вообще для того, чтобы писать тут кроссплатформенный код, написали библиотеку [libevent](http://libevent.org/)

Посмотрим на разные способы мультиплексирования на одной задаче: прочитать все что будет записано от N подпроцессов через пайпы. Читать хочется сразу, как только произошла запись.

Разные способы мультиплексирования - разные реализации функции read_all


```cpp
%%cpp multiplexing_reader_common.h

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <errno.h>
#include <time.h>
#include <stdbool.h>
#include <stdint.h>

// log_printf - макрос для отладочного вывода, добавляющий время со старта программы, имя функции и номер строки
uint64_t start_time_msec; 
const char* log_prefix(const char* func, int line);
#define log_printf_impl(fmt, ...) { time_t t = time(0); dprintf(2, "%s: " fmt "%s", log_prefix(__FUNCTION__, __LINE__), __VA_ARGS__); }
// Format: <time_since_start> <func_name>:<line> : <custom_message>
#define log_printf(...) log_printf_impl(__VA_ARGS__, "")


#define conditional_handle_error(stmt, msg) \
    do { if (stmt) { perror(msg " (" #stmt ")"); exit(EXIT_FAILURE); } } while (0)

// Функция, реализации которой мы будем тестировать
void read_all(int* fds, int count);
```


```cpp
%%cpp multiplexing_reader_common.c

#include "multiplexing_reader_common.h"

#include <sys/syscall.h>

void  __attribute__ ((constructor)) start_time_setter() { struct timespec spec; clock_gettime(CLOCK_MONOTONIC, &spec); start_time_msec = spec.tv_sec * 1000L + spec.tv_nsec / 1000000; }

inline const char* log_prefix(const char* func, int line) {
    struct timespec spec; clock_gettime(CLOCK_MONOTONIC, &spec); int delta_msec = spec.tv_sec * 1000L + spec.tv_nsec / 1000000 - start_time_msec;
    const int max_func_len = 13; static __thread char prefix[100]; 
    sprintf(prefix, "%d.%03d %*s():%-3d [tid=%ld]", delta_msec / 1000, delta_msec % 1000, max_func_len, func, line, syscall(__NR_gettid));
    return prefix;
}
```

main тестирующей программы


```cpp
%%cpp multiplexing_reader_test.c

#include "multiplexing_reader_common.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <time.h>

const int INPUTS_COUNT = 5;

int main() {
    log_printf("Start multiplexing test\n");
    pid_t pids[INPUTS_COUNT];
    int input_fds[INPUTS_COUNT];
    // create INPUTS_COUNT subprocesses that will write to pipes with different delays
    for (int i = 0; i < INPUTS_COUNT; ++i) {
        int fds[2];
        pipe(fds);
        input_fds[i] = fds[0];
        if ((pids[i] = fork()) == 0) {
            int sleep_ms = 100 * (INPUTS_COUNT - 1 - i); 
            struct timespec t = {.tv_sec = sleep_ms / 1000, .tv_nsec = (sleep_ms % 1000) * 1000000};
            nanosleep(&t, &t);  
                
            log_printf("Send hello from %d subprocess\n", i);
            dprintf(fds[1], "Hello from %d subprocess\n", i);
            // try with EPOLL realisation
            // sleep(10);
            // dprintf(fds[1], "Hello 2 from %d subprocess\n", i);
            exit(0);
        }
        close(fds[1]);
    }
    
    // run multiplexing reading
    read_all(input_fds, INPUTS_COUNT);
     
    int status;
    for (int i = 0; i < INPUTS_COUNT; ++i) {
        close(input_fds[i]);
        assert(waitpid(pids[i], &status, 0) != -1);
    }
    log_printf("Finish multiplexing test\n");
    return 0;
}
```

## <a name="just_read"></a> Наивный read


```cpp
%%cpp multiplexing_reader_trivial.c
%run gcc multiplexing_reader_test.c multiplexing_reader_common.c multiplexing_reader_trivial.c -o trivial.exe
%MD ### Результаты чтения с помощью наивного read
%MD Видно, что чтение происходит совсем не сразу после записи, так как мультиплексирования по сути нет
%run time -p ./trivial.exe

#include "multiplexing_reader_common.h"

#include <sys/epoll.h>

void read_all(int* input_fds, int count) {
    // Работает неэффективно, так как при попытке считать из пайпа мы можем на этом надолго заблокироваться 
    // А в другом пайпе данные могут появиться, но мы их не сможем обработать сразу (заблокированы, пытаясь читать другой пайп)
    log_printf("Trivial realisation start\n");
    // Проходимся по всем файловым дескрипторам (специально выбрал плохой порядок)
    for (int i = 0; i < count; ++i) {
        char buf[100];
        int read_bytes = 0;
        while ((read_bytes = read(input_fds[i], buf, sizeof(buf))) > 0) { // Читаем файл пока он не закроется.
            buf[read_bytes] = '\0';
            log_printf("Read from %d subprocess: %s", i, buf);
        }
        conditional_handle_error(read_bytes < 0, "read error");
    }
    log_printf("Trivial realisation finish\n");
}

```


Run: `gcc multiplexing_reader_test.c multiplexing_reader_common.c multiplexing_reader_trivial.c -o trivial.exe`



### Результаты чтения с помощью наивного read



Видно, что чтение происходит совсем не сразу после записи, так как мультиплексирования по сути нет



Run: `time -p ./trivial.exe`


    0.000          main():18  [tid=69230]: Start multiplexing test
    0.000      read_all():14  [tid=69230]: Trivial realisation start
    0.001          main():31  [tid=69235]: Send hello from 4 subprocess
    0.102          main():31  [tid=69234]: Send hello from 3 subprocess
    0.217          main():31  [tid=69233]: Send hello from 2 subprocess
    0.302          main():31  [tid=69232]: Send hello from 1 subprocess
    0.415          main():31  [tid=69231]: Send hello from 0 subprocess
    0.415      read_all():21  [tid=69230]: Read from 0 subprocess: Hello from 0 subprocess
    0.415      read_all():21  [tid=69230]: Read from 1 subprocess: Hello from 1 subprocess
    0.416      read_all():21  [tid=69230]: Read from 2 subprocess: Hello from 2 subprocess
    0.416      read_all():21  [tid=69230]: Read from 3 subprocess: Hello from 3 subprocess
    0.416      read_all():21  [tid=69230]: Read from 4 subprocess: Hello from 4 subprocess
    0.416      read_all():25  [tid=69230]: Trivial realisation finish
    0.416          main():49  [tid=69230]: Finish multiplexing test
    real 0.41
    user 0.00
    sys 0.00


## <a name="read_nonblock"></a> Наивный read + NONBLOCK


```cpp
%%cpp multiplexing_reader_nonblock.c
%run gcc multiplexing_reader_test.c multiplexing_reader_common.c multiplexing_reader_nonblock.c -o nonblock.exe
%MD ### Результаты чтения с помощью наивного неблокирующего read
%MD Видно, что чтение происходит сразу после записи, **но user и system time близки к затраченному астрономическому времени**, то есть сожжено дикое количество процессорного времени
%run time -p ./nonblock.exe

#include "multiplexing_reader_common.h"

#include <fcntl.h>
#include <sys/epoll.h>

void read_all(int* input_fds, int count) {
    // Работает быстро, так как читает все что есть в "файле" на данный момент вне зависимости от того пишет ли туда кто-нибудь или нет
    // У этого метода есть большая проблема: внутри вечного цикла постоянно вызывается системное прерывание.
    // Процессорное время тратится впустую.
    log_printf("Nonblock realisation start\n");
    for (int i = 0; i < count; ++i) {
        fcntl(input_fds[i], F_SETFL, fcntl(input_fds[i], F_GETFL) | O_NONBLOCK); // Пометили дескрипторы как неблокирующие
    }
    bool all_closed = false;
    while (!all_closed) {
        all_closed = true;
        for (int i = 0; i < count; ++i) { // Проходимся по всем файловым дескрипторам
            if (input_fds[i] == -1) {
                continue;
            }
            all_closed = false;
            char buf[100];
            int read_bytes = 0;
            // Пытаемся читать пока либо не кончится файл, либо не поймаем ошибку
            while ((read_bytes = read(input_fds[i], buf, sizeof(buf))) > 0) {
                buf[read_bytes] = '\0';
                log_printf("Read from %d subprocess: %s", i, buf);
            }
            if (read_bytes == 0) { // Либо прочитали весь файл
                close(input_fds[i]);
                input_fds[i] = -1;
            } else {
                conditional_handle_error(errno != EAGAIN, "strange error"); // Либо поймали ошибку (+ проверяем, что ошибка ожидаемая)
            }
        }
    }
    log_printf("Nonblock realisation finish\n");
}

```


Run: `gcc multiplexing_reader_test.c multiplexing_reader_common.c multiplexing_reader_nonblock.c -o nonblock.exe`



### Результаты чтения с помощью наивного неблокирующего read



Видно, что чтение происходит сразу после записи, **но user и system time близки к затраченному астрономическому времени**, то есть сожжено дикое количество процессорного времени



Run: `time -p ./nonblock.exe`


    0.000          main():18  [tid=69248]: Start multiplexing test
    0.000      read_all():16  [tid=69248]: Nonblock realisation start
    0.001          main():31  [tid=69253]: Send hello from 4 subprocess
    0.001      read_all():33  [tid=69248]: Read from 4 subprocess: Hello from 4 subprocess
    0.100          main():31  [tid=69252]: Send hello from 3 subprocess
    0.101      read_all():33  [tid=69248]: Read from 3 subprocess: Hello from 3 subprocess
    0.201          main():31  [tid=69251]: Send hello from 2 subprocess
    0.201      read_all():33  [tid=69248]: Read from 2 subprocess: Hello from 2 subprocess
    0.300          main():31  [tid=69250]: Send hello from 1 subprocess
    0.300      read_all():33  [tid=69248]: Read from 1 subprocess: Hello from 1 subprocess
    0.401          main():31  [tid=69249]: Send hello from 0 subprocess
    0.401      read_all():33  [tid=69248]: Read from 0 subprocess: Hello from 0 subprocess
    0.401      read_all():43  [tid=69248]: Nonblock realisation finish
    0.401          main():49  [tid=69248]: Finish multiplexing test
    real 0.40
    user 0.21
    sys 0.18


## <a name="epoll"></a> epoll c level-triggering


```cpp
%%cpp multiplexing_reader_epoll.c
%run gcc multiplexing_reader_test.c multiplexing_reader_common.c multiplexing_reader_epoll.c -o epoll.exe
%MD ### Результаты чтения с помощью epoll c level-triggering
%MD Видно, что чтение происходит сразу после записи, а user и system time близки к 0
%run time -p ./epoll.exe

#include "multiplexing_reader_common.h"

#include <fcntl.h>
#include <sys/epoll.h>

void read_all(int* input_fds, int count) {
    // Круче предыдущего, потому что этот вариант программы не ест процессорное время ни на что
    // (в данном случае на проверку условия того, что в файле ничего нет)
    log_printf("Epoll realisation start\n");
    // Создаем epoll-объект. В случае Level Triggering события объект скорее представляет собой множество файловых дескрипторов по которым есть события. 
    // И мы можем читать это множество, вызывая epoll_wait
    // epoll_create has one legacy parameter, so I prefer to use newer function. 
    int epoll_fd = epoll_create1(0);
    // Тут мы подписываемся на события, которые будет учитывать epoll-объект, т.е. указываем события за которыми мы следим
    for (int i = 0; i < count; ++i) {
        struct epoll_event event = {
            .events = EPOLLIN | EPOLLERR | EPOLLHUP, 
            .data = {.u32 = i} // user data
        };
        epoll_ctl(epoll_fd, EPOLL_CTL_ADD, input_fds[i], &event);
    }
    int not_closed = count;
    while (not_closed > 0) {
        struct epoll_event event;
        int epoll_ret = epoll_wait(epoll_fd, &event, 1, 1000); // Читаем события из epoll-объект (то есть из множества файловых дескриптотров, по которым есть события)
        if (epoll_ret <= 0) {
            continue;
        }
        
        assert(event.events & (EPOLLIN | EPOLLHUP));
        
        int i = event.data.u32; // Получаем обратно заданную user data
        
        char buf[100];
        int read_bytes = 0;
        // Что-то прочитали из файла.
        // Так как read вызывается один раз, то если мы все не считаем, то нам придется делать это еще раз на следующей итерации большого цикла. 
        // (иначе можем надолго заблокироваться)
        // Решение: комбинируем со реализацией через O_NONBLOCK и в этом месте читаем все что доступно до самого конца
        if ((read_bytes = read(input_fds[i], buf, sizeof(buf))) > 0) {
            buf[read_bytes] = '\0';
            log_printf("Read from %d subprocess: %s", i, buf);
        } else if (read_bytes == 0) { // Файл закрылся, поэтому выкидываем его файловый дескриптор
            // Это системный вызов. Он довольно дорогой. Такая вот плата за epoll (в сравнении с poll, select)
            epoll_ctl(epoll_fd, EPOLL_CTL_DEL, input_fds[i], NULL);
            close(input_fds[i]);
            input_fds[i] = -1;
            not_closed -= 1;
        } else {
            conditional_handle_error(1, "strange error");
        }
    }
    close(epoll_fd);
    log_printf("Epoll realisation finish\n");
}

```


Run: `gcc multiplexing_reader_test.c multiplexing_reader_common.c multiplexing_reader_epoll.c -o epoll.exe`



### Результаты чтения с помощью epoll c level-triggering



Видно, что чтение происходит сразу после записи, а user и system time близки к 0



Run: `time -p ./epoll.exe`


    0.000          main():18  [tid=69266]: Start multiplexing test
    0.001      read_all():15  [tid=69266]: Epoll realisation start
    0.001          main():31  [tid=69271]: Send hello from 4 subprocess
    0.001      read_all():48  [tid=69266]: Read from 4 subprocess: Hello from 4 subprocess
    0.102          main():31  [tid=69270]: Send hello from 3 subprocess
    0.102      read_all():48  [tid=69266]: Read from 3 subprocess: Hello from 3 subprocess
    0.201          main():31  [tid=69269]: Send hello from 2 subprocess
    0.201      read_all():48  [tid=69266]: Read from 2 subprocess: Hello from 2 subprocess
    0.301          main():31  [tid=69268]: Send hello from 1 subprocess
    0.302      read_all():48  [tid=69266]: Read from 1 subprocess: Hello from 1 subprocess
    0.401          main():31  [tid=69267]: Send hello from 0 subprocess
    0.402      read_all():48  [tid=69266]: Read from 0 subprocess: Hello from 0 subprocess
    0.402      read_all():60  [tid=69266]: Epoll realisation finish
    0.402          main():49  [tid=69266]: Finish multiplexing test
    real 0.40
    user 0.00
    sys 0.00


## epoll c edge-triggering


```cpp
%%cpp multiplexing_reader_epoll_edge.c
%run gcc multiplexing_reader_test.c multiplexing_reader_common.c multiplexing_reader_epoll_edge.c -o epoll_edge.exe
%MD ### Результаты чтения с помощью epoll c edge-triggering (EPOLLET опция)
%MD Видно, что чтение происходит сразу после записи, а user и system time близки к 0
%run time -p ./epoll_edge.exe

#include "multiplexing_reader_common.h"

#include <fcntl.h>
#include <sys/epoll.h>

void read_all(int* input_fds, int count) {
    // epoll + edge triggering
    // В этом случае объект epoll уже является очередью. 
    // Ядро в него нам пишет событие каждый раз, когда случается событие, на которое мы подписались
    // А мы в дальнейшем извлекаем эти события (и в очереди их больше не будет).
    log_printf("Epoll edge-triggered realisation start\n");
    
    // sleep(1); // так можно проверить, не потеряем ли мы информацию о записанном в файловые дескрипторы, если сделаем EPOLL_CTL_ADD после записи
    int epoll_fd = epoll_create1(0);
    for (int i = 0; i < count; ++i) {
        fcntl(input_fds[i], F_SETFL, fcntl(input_fds[i], F_GETFL) | O_NONBLOCK);
        // Обратите внимание на EPOLLET
        struct epoll_event event = {
            .events = EPOLLIN | EPOLLERR | EPOLLHUP | EPOLLET, 
            .data = {.u32 = i}
        };
        epoll_ctl(epoll_fd, EPOLL_CTL_ADD, input_fds[i], &event);
    }
    int not_closed = count;
    while (not_closed > 0) {
        // У меня тут возник вопрос: а получим ли мы уведомления о файловых дескрипторах, 
        // из которых на момент EPOLL_CTL_ADD УЖЕ есть что читать?
        // Не нашел в документации, но многочисленные примеры говорят, что можно считать, что получим.
        struct epoll_event event;
        int epoll_ret = epoll_wait(epoll_fd, &event, 1, 1000);
        if (epoll_ret <= 0) {
            continue;
        }
        int i = event.data.u32;
    
        char buf[100];
        int read_bytes = 0;
        while ((read_bytes = read(input_fds[i], buf, sizeof(buf))) > 0) {
            buf[read_bytes] = '\0';
            log_printf("Read from %d subprocess: %s", i, buf);
        } 
        if (read_bytes == 0) {
            epoll_ctl(epoll_fd, EPOLL_CTL_DEL, input_fds[i], NULL);
            close(input_fds[i]);
            input_fds[i] = -1;
            not_closed -= 1;
        } else {
            conditional_handle_error(errno != EAGAIN, "strange error");
        }
    }
    close(epoll_fd);
    log_printf("Epoll edge-triggered realisation finish\n");
}

```


Run: `gcc multiplexing_reader_test.c multiplexing_reader_common.c multiplexing_reader_epoll_edge.c -o epoll_edge.exe`



### Результаты чтения с помощью epoll c edge-triggering (EPOLLET опция)



Видно, что чтение происходит сразу после записи, а user и system time близки к 0



Run: `time -p ./epoll_edge.exe`


    0.000          main():18  [tid=69284]: Start multiplexing test
    0.000      read_all():17  [tid=69284]: Epoll edge-triggered realisation start
    0.000          main():31  [tid=69289]: Send hello from 4 subprocess
    0.000      read_all():46  [tid=69284]: Read from 4 subprocess: Hello from 4 subprocess
    0.101          main():31  [tid=69288]: Send hello from 3 subprocess
    0.101      read_all():46  [tid=69284]: Read from 3 subprocess: Hello from 3 subprocess
    0.203          main():31  [tid=69287]: Send hello from 2 subprocess
    0.203      read_all():46  [tid=69284]: Read from 2 subprocess: Hello from 2 subprocess
    0.300          main():31  [tid=69286]: Send hello from 1 subprocess
    0.301      read_all():46  [tid=69284]: Read from 1 subprocess: Hello from 1 subprocess
    0.402          main():31  [tid=69285]: Send hello from 0 subprocess
    0.402      read_all():46  [tid=69284]: Read from 0 subprocess: Hello from 0 subprocess
    0.402      read_all():58  [tid=69284]: Epoll edge-triggered realisation finish
    0.402          main():49  [tid=69284]: Finish multiplexing test
    real 0.40
    user 0.00
    sys 0.00


## <a name="select"></a> select


```cpp
%%cpp multiplexing_reader_select.c
%run gcc multiplexing_reader_test.c multiplexing_reader_common.c multiplexing_reader_select.c -o select.exe
%MD ### Результаты чтения с помощью select
%MD Видно, что чтение происходит сразу после записи, а user и system time близки к 0
%run time -p ./select.exe

#include "multiplexing_reader_common.h"

#include <fcntl.h>
#include <sys/epoll.h>

void read_all(int* input_fds, int count) {
    log_printf("Select realisation start\n");

    struct timeval tv = {.tv_sec = 1, .tv_usec = 0};
    int not_closed = count;
    while (not_closed > 0) {
        int max_fd = 0;
        // Так как структура fd_set используется и на вход (какие дескрипторы обрабатывать) и на выход (из каких пришёл вывод), её надо повторно инициализировать.
        fd_set rfds;
        FD_ZERO(&rfds);
        for (int i = 0; i < count; ++i) {
            if (input_fds[i] != -1) {
                assert(input_fds[i] < 1024);
                FD_SET(input_fds[i], &rfds);
                max_fd = (input_fds[i] < max_fd) ? max_fd : input_fds[i];
            }
        }
        // аргументы: макс количество файловых дескрипторов, доступное количество на чтение, запись, ошибки, время ожидания.
        int select_ret = select(max_fd + 1, &rfds, NULL, NULL, &tv);
        conditional_handle_error(select_ret == -1, "select error");
        if (select_ret > 0) {
            for (int i = 0; i < count; ++i) {
                // Проверяем, какой дескриптор послал данные.
                if (input_fds[i] != -1 && FD_ISSET(input_fds[i], &rfds)) {
                    char buf[100];
                    int read_bytes = 0;
                    if ((read_bytes = read(input_fds[i], buf, sizeof(buf))) > 0) {
                        buf[read_bytes] = '\0';
                        log_printf("Read from %d subprocess: %s", i, buf);
                    } else if (read_bytes == 0) {
                        close(input_fds[i]);
                        input_fds[i] = -1;
                        not_closed -= 1;
                    } else {
                        conditional_handle_error(1, "strange error");
                    }
                }
            }
        }
    }
    log_printf("Select realisation finish\n");
}

```


Run: `gcc multiplexing_reader_test.c multiplexing_reader_common.c multiplexing_reader_select.c -o select.exe`



### Результаты чтения с помощью select



Видно, что чтение происходит сразу после записи, а user и system time близки к 0



Run: `time -p ./select.exe`


    0.000          main():18  [tid=69302]: Start multiplexing test
    0.001      read_all():13  [tid=69302]: Select realisation start
    0.001          main():31  [tid=69307]: Send hello from 4 subprocess
    0.001      read_all():40  [tid=69302]: Read from 4 subprocess: Hello from 4 subprocess
    0.102          main():31  [tid=69306]: Send hello from 3 subprocess
    0.102      read_all():40  [tid=69302]: Read from 3 subprocess: Hello from 3 subprocess
    0.204          main():31  [tid=69305]: Send hello from 2 subprocess
    0.204      read_all():40  [tid=69302]: Read from 2 subprocess: Hello from 2 subprocess
    0.303          main():31  [tid=69304]: Send hello from 1 subprocess
    0.303      read_all():40  [tid=69302]: Read from 1 subprocess: Hello from 1 subprocess
    0.403          main():31  [tid=69303]: Send hello from 0 subprocess
    0.403      read_all():40  [tid=69302]: Read from 0 subprocess: Hello from 0 subprocess
    0.403      read_all():52  [tid=69302]: Select realisation finish
    0.403          main():49  [tid=69302]: Finish multiplexing test
    real 0.40
    user 0.00
    sys 0.00



```python

```

# <a name="select_fail"></a> Select fail

Как-то в монорепозитории Яндекса обновили openssl...

(Суть в том, что select не поддерживает файловые дескрипторы с номерами больше 1024. Это пример на такую ошибку)


```cpp
%%cpp select_fail.c
%run gcc select_fail.c -o select_fail.exe
%run ulimit -n 1200 && ./select_fail.exe
%run gcc -DBIG_FD select_fail.c -o select_fail.exe
%run ulimit -n 1200 && ./select_fail.exe

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <time.h>
#include <sys/epoll.h>
#include <string.h>
#include <stdint.h>

// log_printf - макрос для отладочного вывода, добавляющий время со старта программы, имя функции и номер строки
uint64_t start_time_msec; void  __attribute__ ((constructor)) start_time_setter() { struct timespec spec; clock_gettime(CLOCK_MONOTONIC, &spec); start_time_msec = spec.tv_sec * 1000L + spec.tv_nsec / 1000000; }
const char* log_prefix(const char* func, int line) {
    struct timespec spec; clock_gettime(CLOCK_MONOTONIC, &spec); int delta_msec = spec.tv_sec * 1000L + spec.tv_nsec / 1000000 - start_time_msec;
    const int max_func_len = 13; static __thread char prefix[100]; 
    sprintf(prefix, "%d.%03d %*s():%-3d [tid=%ld]", delta_msec / 1000, delta_msec % 1000, max_func_len, func, line, syscall(__NR_gettid));
    return prefix;
}
#define log_printf_impl(fmt, ...) { time_t t = time(0); dprintf(2, "%s: " fmt "%s", log_prefix(__FUNCTION__, __LINE__), __VA_ARGS__); }
// Format: <time_since_start> <func_name>:<line> : <custom_message>
#define log_printf(...) log_printf_impl(__VA_ARGS__, "")

#define conditional_handle_error(stmt, msg) \
    do { if (stmt) { perror(msg " (" #stmt ")"); exit(EXIT_FAILURE); } } while (0)

#ifdef BIG_FD
const int EXTRA_FD_COUNT = 1030;
#else
const int EXTRA_FD_COUNT = 1010;
#endif

int main() {
    pid_t child_pid;
    int input_fd;
   
    {
        int fds[2];
        pipe(fds);
        input_fd = fds[0];
        if ((child_pid = fork()) == 0) {
            sleep(1);
            dprintf(fds[1], "Hello from exactly one subprocess\n");
            exit(0);
        }
        assert(child_pid > 0);
        close(fds[1]);
    }
    
    for (int i = 0; i < EXTRA_FD_COUNT; ++i) {
        input_fd = dup(input_fd); // yes, we don't bother closing file descriptors in this example
    }
    
    log_printf("Select start input_fd=%d\n", input_fd);
    
    struct timeval tv = {.tv_sec = 10, .tv_usec = 0};
    while (input_fd != -1) {
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(input_fd, &rfds);
        char secret[] = "abcdefghijklmnop";
        int select_ret = select(input_fd + 1, &rfds, NULL, NULL, &tv);
        log_printf("Secret is %s\n", secret);
        if (strcmp(secret, "abcdefghijklmnop") != 0) {
            log_printf("Hey! select is broken!\n");
        }
        conditional_handle_error(select_ret == -1, "select error");
        if (select_ret > 0) {
            assert(FD_ISSET(input_fd, &rfds));
            
            char buf[100];
            int read_bytes = 0;
            if ((read_bytes = read(input_fd, buf, sizeof(buf) - 1)) > 0) {
                buf[read_bytes] = '\0';
                log_printf("Read from child subprocess: %s", buf);
            } else if (read_bytes == 0) {
                input_fd = -1;
            } else {
                conditional_handle_error(errno != EAGAIN, "strange error");
            }
        }
    }
    
    int status;    
    assert(waitpid(child_pid, &status, 0) != -1);
    return 0;
}
```


Run: `gcc select_fail.c -o select_fail.exe`



Run: `ulimit -n 1200 && ./select_fail.exe`


    0.001          main():64  [tid=69315]: Select start input_fd=1013
    1.002          main():73  [tid=69315]: Secret is abcdefghijklmnop
    1.002          main():85  [tid=69315]: Read from child subprocess: Hello from exactly one subprocess
    1.002          main():73  [tid=69315]: Secret is abcdefghijklmnop



Run: `gcc -DBIG_FD select_fail.c -o select_fail.exe`



Run: `ulimit -n 1200 && ./select_fail.exe`


    0.001          main():64  [tid=69324]: Select start input_fd=1033
    1.002          main():73  [tid=69324]: Secret is a
    1.002          main():75  [tid=69324]: Hey! select is broken!
    1.002          main():85  [tid=69324]: Read from child subprocess: Hello from exactly one subprocess
    1.002          main():73  [tid=69324]: Secret is a
    1.002          main():75  [tid=69324]: Hey! select is broken!



```python

```

# <a name="aio"></a> Linux AIO

Медленными бывают так же диски. И у них есть особенность: они не завершаются с ошибкой EAGAIN если нет данных. А просто долго висят в операциях read, write.

Как жить? Можно делать несколько операций одновременно. И чтобы не плодить потоки (блочить каждый поток на записи/чтении) можно юзать Linux AIO

Предустановка

```bash
sudo apt-get install libaio1
sudo apt-get install libaio-dev
```

Статейки

https://github.com/littledan/linux-aio

https://oxnz.github.io/2016/10/13/linux-aio/#install


```cpp
%%cpp aio.c
%run gcc aio.c -o aio.exe -laio # обратите внимание
%run ./aio.exe
%run cat ./output_0.txt
%run cat ./output_1.txt

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <time.h>
#include <sys/epoll.h>
#include <string.h>
#include <libaio.h>  // подключаем
#include <err.h>
#include <stdint.h>

// log_printf - макрос для отладочного вывода, добавляющий время со старта программы, имя функции и номер строки
uint64_t start_time_msec; void  __attribute__ ((constructor)) start_time_setter() { struct timespec spec; clock_gettime(CLOCK_MONOTONIC, &spec); start_time_msec = spec.tv_sec * 1000L + spec.tv_nsec / 1000000; }
const char* log_prefix(const char* func, int line) {
    struct timespec spec; clock_gettime(CLOCK_MONOTONIC, &spec); int delta_msec = spec.tv_sec * 1000L + spec.tv_nsec / 1000000 - start_time_msec;
    const int max_func_len = 13; static __thread char prefix[100]; 
    sprintf(prefix, "%d.%03d %*s():%-3d [tid=%ld]", delta_msec / 1000, delta_msec % 1000, max_func_len, func, line, syscall(__NR_gettid));
    return prefix;
}
#define log_printf_impl(fmt, ...) { time_t t = time(0); dprintf(2, "%s: " fmt "%s", log_prefix(__FUNCTION__, __LINE__), __VA_ARGS__); }
// Format: <time_since_start> <func_name>:<line> : <custom_message>
#define log_printf(...) log_printf_impl(__VA_ARGS__, "")

#define conditional_handle_error(stmt, msg) \
    do { if (stmt) { perror(msg " (" #stmt ")"); exit(EXIT_FAILURE); } } while (0)

const int N_FILES = 2;

int main() {
    // подготовим контекст для асинхронных действий
    io_context_t ctx = {0};
    int io_setup_ret = io_setup(N_FILES + 10, &ctx);
    errno = -io_setup_ret;
    conditional_handle_error(io_setup_ret < 0, "Can't io_setup");
      
    // создаем какой-то список открытых файлов
    int fds[N_FILES];
    for (int i = 0; i < N_FILES; ++i) {
        char file[100];
        sprintf(file, "./output_%d.txt", i);
        fds[i] = open(file, O_WRONLY | O_CREAT, 0664);
        conditional_handle_error(fds[i] < 0, "Can't open");
        log_printf("Opened file '%s' fd %d\n", file, fds[i]);
    }
    
    // готовим батчевую запись во много файлов
    struct iocb iocb[N_FILES];
    struct iocb* iocbs[N_FILES];
    char msgs[N_FILES][100];
    for (int i = 0; i < N_FILES; ++i) {
        sprintf(msgs[i], "hello to file %d\n", i);
        // Создаём структуру для удобной записи (включает сразу дескриптор, сообщение и его длину)
        io_prep_pwrite(&iocb[i], fds[i], (void*)msgs[i], strlen(msgs[i]), 0); // Формируем запросы на запись
        // data -- для передачи дополнительной информации (в epoll такая же штуковина)
        // Конкретно здесь передаётся информация о том, в какой файл записываем
        iocb[i].data = (char*)0 + i;
        iocbs[i] = &iocb[i];
    }

    // Отправляем батч запросов на выполнение
    // Возвращает количество успешно добавленных запросов.
    int io_submit_ret = io_submit(ctx, N_FILES, iocbs);
    if (io_submit_ret != N_FILES) {
        errno = -io_submit_ret;
        log_printf("Error: %s\n", strerror(-io_submit_ret));
        warn("io_submit");
        io_destroy(ctx);
    }

    int in_fly_writings = N_FILES;
    while (in_fly_writings > 0) {
        struct io_event event;
        struct timespec timeout = {.tv_sec = 0, .tv_nsec = 500000000};
        // В этом примере получаем максимум реакцию на один запрос. Эффективнее, конечно, сразу на несколько.
        if (io_getevents(ctx, 0, 1, &event, &timeout) == 1) { // Здесь в цикле получаем реакцию на запросы
            conditional_handle_error(event.res < 0, "Can't do operation");
            int i = (char*)event.data - (char*)0;
            log_printf("%d written ok\n", i);
            close(fds[i]);
            --in_fly_writings;
            continue;
        }
        log_printf("not done yet\n");
    }
    io_destroy(ctx);

    return 0;
}
```


Run: `gcc aio.c -o aio.exe -laio # обратите внимание`



Run: `./aio.exe`


    0.000          main():55  [tid=69333]: Opened file './output_0.txt' fd 3
    0.000          main():55  [tid=69333]: Opened file './output_1.txt' fd 4
    0.000          main():90  [tid=69333]: 0 written ok
    0.000          main():90  [tid=69333]: 1 written ok



Run: `cat ./output_0.txt`


    hello to file 0



Run: `cat ./output_1.txt`


    hello to file 1



```python

```

# <a name="hw"></a> Комментарии к ДЗ

*  highload/epoll-read-fds-vector: Тупая реализация не зайдёт
<br>Контрпример: мы поочерёдно начинаем читать файлы, стартуя с 0-го. Пусть 2 файл -- это пайп, через который проверяющая система начинает посылать 100кб данных. Так как пайп не обработан сразу, то по достижении 65kb, ввод заблокируется. Чекер зависнет, не закроет нам 0-ой файл (который скорее всего пайп). И будет таймаут.
  <br>В общем задача на epoll. linux aio тут не зайдет, вопрос на подумать - почему?

* highload/epoll-read-write-socket: Возможно вам помогут факты: 
  * в epoll можно добавить файл дважды: один раз на чтение, другой раз на запись (с разной user data). 
  * вы можете переключать режим, на предмет каких событий вы слушаете файловый дескриптор
  
  
Для одновременного ожидания сигналов и событий из файловых дескрипторов можно использовать `epoll_pwait`. В этом случае нет необходимости создавать обработчики, в которых


```python

```


```python

```


```python

```


```python

```
