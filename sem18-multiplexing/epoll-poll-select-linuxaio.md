```python
# look at tools/set_up_magics.ipynb
get_ipython().run_cell('# one_liner_str <too much code> \n')
None
```


    <IPython.core.display.Javascript object>


# Мультиплексирование

<br>
<div style="text-align: right"> Спасибо <a href="https://github.com/">пока никому :(</a> за участие в написании текста </div>
<br>

Мультиплексирование - о чем это? Это об одновременной работе с несколькими соединениями. О том, чтобы эффективно решать задачу: вот из этих файловых дескрипторов, нужно прочитать, когда станет доступно, а вот в эти записать, опять же, когда будет возможность.

[Хорошая статья на хабре: select / poll / epoll: практическая разница](https://habr.com/ru/company/infopulse/blog/415259/)
В этой же статье есть плюсы и минусы `select`/`poll`/`epoll`.

[Довольно детальная статья на хабре про epoll](https://habr.com/ru/post/416669/)

Способы мультиплексирования:
* O_NONBLOCK
* <a href="#select" style="color:#856024">select</a> - старая штука, но стандартизированная (POSIX).
  Минусы: смотри статью на хабре. <a href="#select_fail" style="color:#856024">Боольшой минус select</a>
* <a href="#epoll" style="color:#856024">epoll</a> - linux
* kqueue - FreeBSD и MacOS
* <a href="#aio" style="color:#856024">Linux AIO</a> - одновременная запись/чтение из нескольких файлов. (К сожалению, это только с файлами работает)

<a href="#hw" style="color:#856024">Комментарии к ДЗ</a>

[Ридинг Яковлева](https://github.com/victor-yacovlev/mipt-diht-caos/tree/master/practice/epoll)


```python

```

# <a name="epoll"></a> <a name="select"></a> epoll и select


```cpp
%%cpp epoll.cpp
%run gcc -DEPOLL_EDGE_TRIGGERED_REALISATION epoll.cpp -o epoll.exe
%run ./epoll.exe
%run gcc -DTRIVIAL_REALISATION epoll.cpp -o epoll.exe
%run ./epoll.exe
%run gcc -DNONBLOCK_REALISATION epoll.cpp -o epoll.exe
%run ./epoll.exe
%run gcc -DSELECT_REALISATION epoll.cpp -o epoll.exe
%run ./epoll.exe
%run gcc -DEPOLL_REALISATION epoll.cpp -o epoll.exe
%run ./epoll.exe

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <time.h>
#include <sys/epoll.h>

char* extract_t(char* s) { s[19] = '\0'; return s + 10; }
#define log_printf_impl(fmt, ...) { time_t t = time(0); dprintf(2, "%s : " fmt "%s", extract_t(ctime(&t)), __VA_ARGS__); }
#define log_printf(...) log_printf_impl(__VA_ARGS__, "")

#define conditional_handle_error(stmt, msg) \
    do { if (stmt) { perror(msg " (" #stmt ")"); exit(EXIT_FAILURE); } } while (0)

const int INPUTS_COUNT = 5;

int main() {
    pid_t pids[INPUTS_COUNT];
    int input_fds[INPUTS_COUNT];
    // create subprocesses that will write to pipes with delays. По сути создаем INPUTS_COUNT кол-во подпроцессов
    for (int i = 0; i < INPUTS_COUNT; ++i) {
        int fds[2];
        pipe(fds);
        input_fds[i] = fds[0];
        if ((pids[i] = fork()) == 0) {
            sleep(i);
            dprintf(fds[1], "Hello from %d subprocess\n", i);
            exit(0);
        }
        close(fds[1]);
    }
    
    #ifdef TRIVIAL_REALISATION
    log_printf("Trivial realisation start\n"); //Работает медленно, так как при считывании мы ждем когда закончится запись в файл, чтобы все прочитать
    // lets consider worst order
    for (int i = INPUTS_COUNT - 1; i >= 0; --i) { //Проходимся по всем файловым дескрипторам
        char buf[100];
        int read_bytes = 0;
        while ((read_bytes = read(input_fds[i], buf, sizeof(buf))) > 0) {  //Читаем файл
            buf[read_bytes] = '\0';
            log_printf("Read from %d subprocess: %s", i, buf);
        }
        conditional_handle_error(read_bytes < 0, "read error");
    }
    #endif
    #ifdef NONBLOCK_REALISATION
    log_printf("Nonblock realisation start\n"); //Работает быстро, так как читает все что есть файле на данный момент вне зависимости от того пишет ли туда кто-нибудь
    for (int i = 0; i < INPUTS_COUNT; ++i) {
        fcntl(input_fds[i], F_SETFL, fcntl(input_fds[i], F_GETFL) | O_NONBLOCK); //Пометили дескрипторы как неблокирующие
    }
    bool all_closed = false;
    while (!all_closed) {  //Пока не все закрыто
        all_closed = true;
        for (int i = INPUTS_COUNT - 1; i >= 0; --i) { //Проходимся по всем файловым дескрипторам
            if (input_fds[i] == -1) {
                continue;
            }
            all_closed = false;
            char buf[100];
            int read_bytes = 0;
            while ((read_bytes = read(input_fds[i], buf, sizeof(buf))) > 0) { //Пытаемся читать пока либо не коничтся файл, либо не поймаем ошибку
                buf[read_bytes] = '\0';
                log_printf("Read from %d subprocess: %s", i, buf);
            }
            if (read_bytes == 0) { //Либо прочитали весь файл
                close(input_fds[i]);
                input_fds[i] = -1;
            } else {
                conditional_handle_error(errno != EAGAIN, "strange error"); //Либо поймали ошибку
            }
        }
    }
    #endif
    #ifdef EPOLL_REALISATION
    log_printf("Epoll realisation start\n"); //Круче предыдущего, потому что этот вариант программы не ест процессорное время ни на что(в данном случае на проверку условия того что в файле ничего нет)
    int epoll_fd = epoll_create1(0); // epoll_create has one legacy parameter, so I prefer to use newer function. СОздали объект "очереди"
    for (int i = 0; i < INPUTS_COUNT; ++i) { //Тут мы заполнили очередь, т.е. указали события за которыми мы следим, на остальные события нам наплевать
        struct epoll_event event = {.events = EPOLLIN | EPOLLERR | EPOLLHUP, .data = {.u32 = i}};
        epoll_ctl(epoll_fd, EPOLL_CTL_ADD, input_fds[i], &event);
    }
    int not_closed = INPUTS_COUNT;
    while (not_closed > 0) {
        struct epoll_event event;
        int epoll_ret = epoll_wait(epoll_fd, &event, 1, 1000); //Извлекаем собыитя из очереди
        if (epoll_ret <= 0) {
            continue;
        }
        int i = event.data.u32;
        
        char buf[100];
        int read_bytes = 0;
        if ((read_bytes = read(input_fds[i], buf, sizeof(buf))) > 0) { //Что-то прочитали из файла. Так как read вызывается один раз, то если мы все не считаем, то нам придется делать это еще раз на следующей итерации большого цикла. Решение: комбинируем со старой реализацией и в этом месте читаем все что доступно до самого конца
            buf[read_bytes] = '\0';
            log_printf("Read from %d subprocess: %s", i, buf);
        } else if (read_bytes == 0) {  // Файл закрылся, поэтому выкидываем его файловый дескриптор
            epoll_ctl(epoll_fd, EPOLL_CTL_DEL, input_fds[i], NULL);
            close(input_fds[i]);
            input_fds[i] = -1;
            not_closed -= 1;
        } else {
            conditional_handle_error(errno != EAGAIN, "strange error");
        }
    }
    close(epoll_fd);
    #endif
    #ifdef EPOLL_EDGE_TRIGGERED_REALISATION
    log_printf("Epoll edge-triggered realisation start\n");
    
    sleep(1);
    int epoll_fd = epoll_create1(0);
    for (int i = 0; i < INPUTS_COUNT; ++i) {
        fcntl(input_fds[i], F_SETFL, fcntl(input_fds[i], F_GETFL) | O_NONBLOCK);
        // Обратите внимание на EPOLLET
        struct epoll_event event = {.events = EPOLLIN | EPOLLERR | EPOLLHUP | EPOLLET, .data = {.u32 = i}};
        epoll_ctl(epoll_fd, EPOLL_CTL_ADD, input_fds[i], &event);
    }
    int not_closed = INPUTS_COUNT;
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
    #endif
    #ifdef SELECT_REALISATION
    log_printf("Select realisation start\n"); 

    
    struct timeval tv = {.tv_sec = 1, .tv_usec = 0};
    int not_closed = INPUTS_COUNT;
    while (not_closed > 0) {
        int ndfs = 0;
        fd_set rfds;
        FD_ZERO(&rfds);
        for (int i = 0; i < INPUTS_COUNT; ++i) {
            if (input_fds[i] != -1) {
                FD_SET(input_fds[i], &rfds);
                ndfs = (input_fds[i] < ndfs) ? ndfs : input_fds[i] + 1;
            }
        }
        int select_ret = select(ndfs, &rfds, NULL, NULL, &tv);
        conditional_handle_error(select_ret == -1, "select error");
        if (select_ret > 0) {
            for (int i = 0; i < INPUTS_COUNT; ++i) {
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
                        conditional_handle_error(errno != EAGAIN, "strange error");
                    }
                }
            }
        }
    }
    #endif
    
    int status;
    for (int i = 0; i < INPUTS_COUNT; ++i) {
        assert(waitpid(pids[i], &status, 0) != -1);
    }
    return 0;
}
```


Run: `gcc -DEPOLL_EDGE_TRIGGERED_REALISATION epoll.cpp -o epoll.exe`


    [01m[Kepoll.cpp:[m[K In function ‘[01m[Kint main()[m[K’:
    [01m[Kepoll.cpp:131:106:[m[K [01;35m[Kwarning: [m[Knarrowing conversion of ‘[01m[Ki[m[K’ from ‘[01m[Kint[m[K’ to ‘[01m[Kuint32_t {aka unsigned int}[m[K’ inside { } [[01;35m[K-Wnarrowing[m[K]
     vent = {.events = EPOLLIN | EPOLLERR | EPOLLHUP | EPOLLET, .data = {.u32 = i}[01;35m[K}[m[K;
                                                                                  [01;35m[K^[m[K



Run: `./epoll.exe`


     10:25:42 : Epoll edge-triggered realisation start
     10:25:43 : Read from 0 subprocess: Hello from 0 subprocess
     10:25:43 : Read from 1 subprocess: Hello from 1 subprocess
     10:25:44 : Read from 2 subprocess: Hello from 2 subprocess
     10:25:45 : Read from 3 subprocess: Hello from 3 subprocess
     10:25:46 : Read from 4 subprocess: Hello from 4 subprocess



Run: `gcc -DTRIVIAL_REALISATION epoll.cpp -o epoll.exe`



Run: `./epoll.exe`


     10:25:46 : Trivial realisation start
     10:25:50 : Read from 4 subprocess: Hello from 4 subprocess
     10:25:50 : Read from 3 subprocess: Hello from 3 subprocess
     10:25:50 : Read from 2 subprocess: Hello from 2 subprocess
     10:25:50 : Read from 1 subprocess: Hello from 1 subprocess
     10:25:50 : Read from 0 subprocess: Hello from 0 subprocess



Run: `gcc -DNONBLOCK_REALISATION epoll.cpp -o epoll.exe`



Run: `./epoll.exe`


     10:25:50 : Nonblock realisation start
     10:25:50 : Read from 0 subprocess: Hello from 0 subprocess
     10:25:51 : Read from 1 subprocess: Hello from 1 subprocess
     10:25:52 : Read from 2 subprocess: Hello from 2 subprocess
     10:25:53 : Read from 3 subprocess: Hello from 3 subprocess
     10:25:54 : Read from 4 subprocess: Hello from 4 subprocess



Run: `gcc -DSELECT_REALISATION epoll.cpp -o epoll.exe`



Run: `./epoll.exe`


     10:25:55 : Select realisation start
     10:25:55 : Read from 0 subprocess: Hello from 0 subprocess
     10:25:56 : Read from 1 subprocess: Hello from 1 subprocess
     10:25:57 : Read from 2 subprocess: Hello from 2 subprocess
     10:25:58 : Read from 3 subprocess: Hello from 3 subprocess
     10:25:59 : Read from 4 subprocess: Hello from 4 subprocess



Run: `gcc -DEPOLL_REALISATION epoll.cpp -o epoll.exe`


    [01m[Kepoll.cpp:[m[K In function ‘[01m[Kint main()[m[K’:
    [01m[Kepoll.cpp:95:96:[m[K [01;35m[Kwarning: [m[Knarrowing conversion of ‘[01m[Ki[m[K’ from ‘[01m[Kint[m[K’ to ‘[01m[Kuint32_t {aka unsigned int}[m[K’ inside { } [[01;35m[K-Wnarrowing[m[K]
     ll_event event = {.events = EPOLLIN | EPOLLERR | EPOLLHUP, .data = {.u32 = i}[01;35m[K}[m[K;
                                                                                  [01;35m[K^[m[K



Run: `./epoll.exe`


     10:25:59 : Epoll realisation start
     10:25:59 : Read from 0 subprocess: Hello from 0 subprocess
     10:26:00 : Read from 1 subprocess: Hello from 1 subprocess
     10:26:01 : Read from 2 subprocess: Hello from 2 subprocess
     10:26:02 : Read from 3 subprocess: Hello from 3 subprocess
     10:26:03 : Read from 4 subprocess: Hello from 4 subprocess



```python

```

# <a name="select_fail"></a> Select fail

Как-то в монорепозитории Яндекса обновили openssl...(Суть в том, что select не поддерживает файловые дескрипторы с номерами больше 1024. Это пример на такую ошибку)


```cpp
%%cpp select_fail.cpp
%run gcc select_fail.cpp -o select_fail.exe
%run ulimit -n 1200 && ./select_fail.exe
%run gcc -DBIG_FD select_fail.cpp -o select_fail.exe
%run ulimit -n 1200 && ./select_fail.exe

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <time.h>
#include <sys/epoll.h>
#include <string.h>

char* extract_t(char* s) { s[19] = '\0'; return s + 10; }
#define log_printf_impl(fmt, ...) { time_t t = time(0); dprintf(2, "%s : " fmt "%s", extract_t(ctime(&t)), __VA_ARGS__); }
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


Run: `gcc select_fail.cpp -o select_fail.exe`



Run: `ulimit -n 1200 && ./select_fail.exe`


     01:13:05 : Select start input_fd=1013
     01:13:06 : Secret is abcdefghijklmnop
     01:13:06 : Read from child subprocess: Hello from exactly one subprocess
     01:13:06 : Secret is abcdefghijklmnop



Run: `gcc -DBIG_FD select_fail.cpp -o select_fail.exe`



Run: `ulimit -n 1200 && ./select_fail.exe`


     01:13:07 : Select start input_fd=1033
     01:13:08 : Secret is a
     01:13:08 : Hey! select is broken!
     01:13:08 : Read from child subprocess: Hello from exactly one subprocess
     01:13:08 : Secret is a
     01:13:08 : Hey! select is broken!



```python

```

    1024


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
%%cpp aio.cpp
%run gcc aio.cpp -o aio.exe -laio # обратите внимание
%run ./aio.exe
%run cat ./output_0.txt
%run cat ./output_1.txt

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <time.h>
#include <sys/epoll.h>
#include <string.h>
#include <libaio.h>  // подключаем
#include <err.h>

char* extract_t(char* s) { s[19] = '\0'; return s + 10; }
#define log_printf_impl(fmt, ...) { time_t t = time(0); dprintf(2, "%s : " fmt "%s", extract_t(ctime(&t)), __VA_ARGS__); }
#define log_printf(...) log_printf_impl(__VA_ARGS__, "")

#define conditional_handle_error(stmt, msg) \
    do { if (stmt) { perror(msg " (" #stmt ")"); exit(EXIT_FAILURE); } } while (0)

const int N_FILES = 2;

int main() {
    io_context_t ctx = {0};
    int io_setup_ret = io_setup(N_FILES + 10, &ctx);
    errno = -io_setup_ret;
    conditional_handle_error(io_setup_ret < 0, "Can't io_setup");
        
    struct iocb iocb[N_FILES];
    struct iocb * iocbs[N_FILES];
    char msgs[N_FILES][100];
    int fds[N_FILES];
    for (int i = 0; i < N_FILES; ++i) {
        sprintf(msgs[i], "hello to file %d\n", i);
        char file[100];
        sprintf(file, "./output_%d.txt", i);
        fds[i] = open(file, O_WRONLY | O_CREAT);
        log_printf("open file '%s' fd %d\n", file, fds[i]);
        conditional_handle_error(fds[i] < 0, "Can't open");
        io_prep_pwrite(&iocb[i], fds[i], (void*)msgs[i], strlen(msgs[i]), 0); //Формируем запросы на запись
        iocb[i].data = (char*)0 + i; //Трансформируем число в адресс
        
        iocbs[i] = &iocb[i];
    }

    int io_submit_ret = io_submit(ctx, N_FILES, iocbs);  //Отправили запросы в ядро
    if (io_submit_ret != N_FILES) {
        errno = -io_submit_ret;
        log_printf("Error: %s\n", strerror(-io_submit_ret));
        warn("io_submit");
        io_destroy(ctx);
    }

    int in_fly_writings = N_FILES;
    while (in_fly_writings > 0) { //Здесь в цикле получаем реакцию на запросы
        struct io_event event;
        struct timespec timeout = {.tv_sec = 0, .tv_nsec = 500000000};
        if (io_getevents(ctx, 0, 1, &event, &timeout) == 1) {
            conditional_handle_error(event.res < 0, "Can't do operation");
            int i = (char*)event.data - (char*)0;
            log_printf("%d written ok\n", i);
            close(fds[i]);
            --in_fly_writings;
            continue;
        }
        printf("not done yet\n");
    }
    io_destroy(ctx);

    return 0;
}
```


Run: `gcc aio.cpp -o aio.exe -laio # обратите внимание`


    [01m[Kaio.cpp:19:10:[m[K [01;31m[Kfatal error: [m[Klibaio.h: Нет такого файла или каталога
     #include [01;31m[K<libaio.h>[m[K  // подключаем
              [01;31m[K^~~~~~~~~~[m[K
    compilation terminated.



Run: `./aio.exe`


    /bin/sh: 1: ./aio.exe: not found



Run: `cat ./output_0.txt`


    cat: ./output_0.txt: Нет такого файла или каталога



Run: `cat ./output_1.txt`


    cat: ./output_1.txt: Нет такого файла или каталога



```python

```

# <a name="hw"></a> Комментарии к ДЗ

* d


```python

```


```python

```


```python

```


```python

```
