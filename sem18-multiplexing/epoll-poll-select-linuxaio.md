```python
# look at tools/set_up_magics.ipynb
get_ipython().run_cell('# one_liner_str <too much code> \n')
None
```


    <IPython.core.display.Javascript object>


# Мультиплексирование

<br>
<div style="text-align: right"> Спасибо <a href="https://github.com/SyrnikRebirth">Сове Глебу</a> и <a href="https://github.com/Disadvantaged">Голяр Димитрису</a> за участие в написании текста </div>
<br>

Мультиплексирование - о чем это? Это об одновременной работе с несколькими соединениями. О том, чтобы эффективно решать задачу: вот из этих файловых дескрипторов, нужно прочитать, когда станет доступно, а вот в эти записать, опять же, когда будет возможность.

[Хорошая статья на хабре: select / poll / epoll: практическая разница](https://habr.com/ru/company/infopulse/blog/415259/)
В этой же статье есть плюсы и минусы `select`/`poll`/`epoll`.

[Довольно детальная статья на хабре про epoll](https://habr.com/ru/post/416669/)

Способы мультиплексирования:
* O_NONBLOCK
* <a href="#select" style="color:#856024">select</a> - старая штука, но стандартизированная (POSIX), и поддерживется практически везде, где есть интернет.
  Минусы: смотри статью на хабре. <a href="#select_fail" style="color:#856024">Боольшой минус select</a>
* <a href="#select" style="color:#856024">poll</a> - менее старая штука, стандартизированная (POSIX.1-2001 and POSIX.1-2008).
* <a href="#epoll" style="color:#856024">epoll</a> - linux
* kqueue - FreeBSD и MacOS. Аналог epoll. Вообще для того, чтобы писать тут кроссплатформенный код, написали библиотеку [libevent](http://libevent.org/)
* <a href="#aio" style="color:#856024">Linux AIO</a> - одновременная запись/чтение из нескольких файлов. (К сожалению, это только с файлами работает)

<a href="#hw" style="color:#856024">Комментарии к ДЗ</a>

[Ридинг Яковлева](https://github.com/victor-yacovlev/mipt-diht-caos/tree/master/practice/epoll)


```python

```

# <a name="epoll"></a> <a name="select"></a> epoll и select


```cpp
%%cpp epoll.cpp
%run gcc -DTRIVIAL_REALISATION epoll.cpp -o epoll.exe
%run ./epoll.exe
%run gcc -DNONBLOCK_REALISATION epoll.cpp -o epoll.exe
%run ./epoll.exe
%run gcc -DSELECT_REALISATION epoll.cpp -o epoll.exe
%run ./epoll.exe
%run gcc -DEPOLL_REALISATION epoll.cpp -o epoll.exe
%run ./epoll.exe
%run gcc -DEPOLL_EDGE_TRIGGERED_REALISATION epoll.cpp -o epoll.exe
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
    // create INPUTS_COUNT subprocesses that will write to pipes with different delays
    for (int i = 0; i < INPUTS_COUNT; ++i) {
        int fds[2];
        pipe(fds);
        input_fds[i] = fds[0];
        if ((pids[i] = fork()) == 0) {
            sleep(i);
            dprintf(fds[1], "Hello from %d subprocess\n", i);
            // try with EPOLL realisation
            // sleep(10);
            // dprintf(fds[1], "Hello 2 from %d subprocess\n", i);
            exit(0);
        }
        close(fds[1]);
    }
    
    #ifdef TRIVIAL_REALISATION
    // Работает неэффективно, так как при попытке считать из пайпа мы можем на этом надолго заблокироваться 
    // А в другом пайпе данные могут появиться, но мы их не сможем обработать сразу (заблокированы, пытаясь читать другой пайп)
    log_printf("Trivial realisation start\n");
    // Проходимся по всем файловым дескрипторам (специально выбрал плохой порядок)
    for (int i = INPUTS_COUNT - 1; i >= 0; --i) {
        char buf[100];
        int read_bytes = 0;
        while ((read_bytes = read(input_fds[i], buf, sizeof(buf))) > 0) { // Читаем файл пока он не закроется.
            buf[read_bytes] = '\0';
            log_printf("Read from %d subprocess: %s", i, buf);
        }
        conditional_handle_error(read_bytes < 0, "read error");
    }
    #endif
    #ifdef NONBLOCK_REALISATION
    // Работает быстро, так как читает все что есть в "файле" на данный момент вне зависимости от того пишет ли туда кто-нибудь или нет
    // У этого метода есть большая проблема: внутри вечного цикла постоянно вызывается системное прерывание.
    // Процессорное время тратится впустую.
    log_printf("Nonblock realisation start\n");
    for (int i = 0; i < INPUTS_COUNT; ++i) {
        fcntl(input_fds[i], F_SETFL, fcntl(input_fds[i], F_GETFL) | O_NONBLOCK); // Пометили дескрипторы как неблокирующие
    }
    bool all_closed = false;
    while (!all_closed) {
        all_closed = true;
        for (int i = INPUTS_COUNT - 1; i >= 0; --i) { // Проходимся по всем файловым дескрипторам
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
    #endif
    #ifdef EPOLL_REALISATION
    // Круче предыдущего, потому что этот вариант программы не ест процессорное время ни на что
    // (в данном случае на проверку условия того, что в файле ничего нет)
    log_printf("Epoll realisation start\n");
    // Создаем epoll-объект. В случае Level Triggering события объект скорее представляет собой множество файловых дескрипторов по которым есть события. 
    // И мы можем читать это множество, вызывая epoll_wait
    // epoll_create has one legacy parameter, so I prefer to use newer function. 
    int epoll_fd = epoll_create1(0);
    // Тут мы подписываемся на события, которые будет учитывать epoll-объект, т.е. указываем события за которыми мы следим
    for (int i = 0; i < INPUTS_COUNT; ++i) {
        struct epoll_event event = {
            .events = EPOLLIN | EPOLLERR | EPOLLHUP, 
            .data = {.u32 = i} // user data
        };
        epoll_ctl(epoll_fd, EPOLL_CTL_ADD, input_fds[i], &event);
    }
    int not_closed = INPUTS_COUNT;
    while (not_closed > 0) {
        struct epoll_event event;
        int epoll_ret = epoll_wait(epoll_fd, &event, 1, 1000); // Читаем события из epoll-объект (то есть из множества файловых дескриптотров, по которым есть события)
        if (epoll_ret <= 0) {
            continue;
        }
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
    #endif
    #ifdef EPOLL_EDGE_TRIGGERED_REALISATION
    // epoll + edge triggering
    // В этом случае объект epoll уже является очередью. 
    // Ядро в него нам пишет событие каждый раз, когда случается событие, на которое мы подписались
    // А мы в дальнейшем извлекаем эти события (и в очереди их больше не будет).
    log_printf("Epoll edge-triggered realisation start\n");
    
    sleep(1);
    int epoll_fd = epoll_create1(0);
    for (int i = 0; i < INPUTS_COUNT; ++i) {
        fcntl(input_fds[i], F_SETFL, fcntl(input_fds[i], F_GETFL) | O_NONBLOCK);
        // Обратите внимание на EPOLLET
        struct epoll_event event = {
            .events = EPOLLIN | EPOLLERR | EPOLLHUP | EPOLLET, 
            .data = {.u32 = i}
        };
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
        // Так как структура fd_set используется и на вход (какие дескрипторы обрабатывать) и на выход (из каких пришёл вывод), её надо повторно инициализировать.
        fd_set rfds;
        FD_ZERO(&rfds);
        for (int i = 0; i < INPUTS_COUNT; ++i) {
            if (input_fds[i] != -1) {
                FD_SET(input_fds[i], &rfds);
                ndfs = (input_fds[i] < ndfs) ? ndfs : input_fds[i] + 1;
            }
        }
        // аргументы: макс количество файловых дескрипторов, доступное количество на чтение, запись, ошибки, время ожидания.
        int select_ret = select(ndfs, &rfds, NULL, NULL, &tv);
        conditional_handle_error(select_ret == -1, "select error");
        if (select_ret > 0) {
            for (int i = 0; i < INPUTS_COUNT; ++i) {
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
    #endif
    
    int status;
    for (int i = 0; i < INPUTS_COUNT; ++i) {
        assert(waitpid(pids[i], &status, 0) != -1);
    }
    return 0;
}
```


Run: `gcc -DTRIVIAL_REALISATION epoll.cpp -o epoll.exe`



Run: `./epoll.exe`


     19:10:51 : Trivial realisation start
     19:10:55 : Read from 4 subprocess: Hello from 4 subprocess
     19:10:55 : Read from 3 subprocess: Hello from 3 subprocess
     19:10:55 : Read from 2 subprocess: Hello from 2 subprocess
     19:10:55 : Read from 1 subprocess: Hello from 1 subprocess
     19:10:55 : Read from 0 subprocess: Hello from 0 subprocess



Run: `gcc -DNONBLOCK_REALISATION epoll.cpp -o epoll.exe`



Run: `./epoll.exe`


     19:10:57 : Nonblock realisation start
     19:10:57 : Read from 0 subprocess: Hello from 0 subprocess
     19:10:58 : Read from 1 subprocess: Hello from 1 subprocess
     19:10:59 : Read from 2 subprocess: Hello from 2 subprocess
     19:11:00 : Read from 3 subprocess: Hello from 3 subprocess
     19:11:01 : Read from 4 subprocess: Hello from 4 subprocess



Run: `gcc -DSELECT_REALISATION epoll.cpp -o epoll.exe`



Run: `./epoll.exe`


     19:11:02 : Select realisation start
     19:11:02 : Read from 0 subprocess: Hello from 0 subprocess
     19:11:03 : Read from 1 subprocess: Hello from 1 subprocess
     19:11:04 : Read from 2 subprocess: Hello from 2 subprocess
     19:11:05 : Read from 3 subprocess: Hello from 3 subprocess
     19:11:06 : Read from 4 subprocess: Hello from 4 subprocess



Run: `gcc -DEPOLL_REALISATION epoll.cpp -o epoll.exe`



Run: `./epoll.exe`


     19:11:07 : Epoll realisation start
     19:11:07 : Read from 0 subprocess: Hello from 0 subprocess
     19:11:08 : Read from 1 subprocess: Hello from 1 subprocess
     19:11:09 : Read from 2 subprocess: Hello from 2 subprocess
     19:11:10 : Read from 3 subprocess: Hello from 3 subprocess
     19:11:11 : Read from 4 subprocess: Hello from 4 subprocess



Run: `gcc -DEPOLL_EDGE_TRIGGERED_REALISATION epoll.cpp -o epoll.exe`



Run: `./epoll.exe`


     19:11:12 : Epoll edge-triggered realisation start
     19:11:13 : Read from 0 subprocess: Hello from 0 subprocess
     19:11:13 : Read from 1 subprocess: Hello from 1 subprocess
     19:11:14 : Read from 2 subprocess: Hello from 2 subprocess
     19:11:15 : Read from 3 subprocess: Hello from 3 subprocess
     19:11:16 : Read from 4 subprocess: Hello from 4 subprocess



```python

```

# <a name="select_fail"></a> Select fail

Как-то в монорепозитории Яндекса обновили openssl...

(Суть в том, что select не поддерживает файловые дескрипторы с номерами больше 1024. Это пример на такую ошибку)


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


     19:10:30 : Select start input_fd=1013
     19:10:31 : Secret is abcdefghijklmnop
     19:10:31 : Read from child subprocess: Hello from exactly one subprocess
     19:10:31 : Secret is abcdefghijklmnop



Run: `gcc -DBIG_FD select_fail.cpp -o select_fail.exe`



Run: `ulimit -n 1200 && ./select_fail.exe`


     19:10:32 : Select start input_fd=1033
     19:10:33 : Secret is a
     19:10:33 : Hey! select is broken!
     19:10:33 : Read from child subprocess: Hello from exactly one subprocess
     19:10:33 : Secret is a
     19:10:33 : Hey! select is broken!



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
        fds[i] = open(file, O_WRONLY | O_CREAT, 0664);
        log_printf("open file '%s' fd %d\n", file, fds[i]);
        conditional_handle_error(fds[i] < 0, "Can't open");
        // Создаём структуру для удобной записи (включает сразу дескриптор, сообщение и его длину)
        io_prep_pwrite(&iocb[i], fds[i], (void*)msgs[i], strlen(msgs[i]), 0); // Формируем запросы на запись
        // data -- для передачи дополнительной информации (в epoll такая же штуковина)
        // Конкретно здесь передаётся информация о том, в какой файл записываем
        iocb[i].data = (char*)0 + i;
        
        iocbs[i] = &iocb[i];
    }

    // Отправляем запросы на выполнение
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


Run: `gcc aio.cpp -o aio.exe -laio # обратите внимание`



Run: `./aio.exe`


     19:10:34 : open file './output_0.txt' fd 3
     19:10:34 : open file './output_1.txt' fd 4
     19:10:34 : 0 written ok
     19:10:34 : 1 written ok



Run: `cat ./output_0.txt`


    hello to file 0



Run: `cat ./output_1.txt`


    hello to file 1



```python

```

# <a name="hw"></a> Комментарии к ДЗ

*  inf15-0: highload/epoll-read-fds-vector: Тупая реализация не зайдёт
<br>Контрпример: мы поочерёдно начинаем читать файлы, стартуя с 0-го. Пусть 2 файл -- это пайп, через который проверяющая система начинает посылать 100кб данных. Так как пайп не обработан сразу, то по достижении 65kb, ввод заблокируется. Чекер зависнет, не закроет нам 0-ой файл (который скорее всего пайп). И будет таймаут.
  <br>В общем задача на epoll. linux aio тут не зайдет, вопрос на подумать - почему?

* inf15-1: highload/epoll-read-write-socket: Возможно вам помогут факты: 
  * в epoll можно добавить файл дважды: один раз на чтение, другой раз на запись. 
  * вы можете переключать режим, на предмет каких событий вы слушаете файловый дескриптор


```python

```


```python

```


```python

```


```python

```
