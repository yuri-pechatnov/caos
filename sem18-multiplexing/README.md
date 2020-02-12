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

Способы мультиплексирования:
* O_NONBLOCK
* <a href="#select" style="color:#856024">select</a> - старая штука, но стандартизированная (POSIX)
* <a href="#epoll" style="color:#856024">epoll</a> - linux
* kqueue - FreeBSD и MacOS

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
    // create subprocesses that will write to pipes with delays
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
    log_printf("Trivial realisation start\n");
    // lets consider worst order
    for (int i = INPUTS_COUNT - 1; i >= 0; --i) {
        char buf[100];
        int read_bytes = 0;
        while ((read_bytes = read(input_fds[i], buf, sizeof(buf))) > 0) {
            buf[read_bytes] = '\0';
            log_printf("Read from %d subprocess: %s", i, buf);
        }
        conditional_handle_error(read_bytes < 0, "read error");
    }
    #endif
    #ifdef NONBLOCK_REALISATION
    log_printf("Nonblock realisation start\n");
    for (int i = 0; i < INPUTS_COUNT; ++i) {
        fcntl(input_fds[i], F_SETFL, fcntl(input_fds[i], F_GETFL) | O_NONBLOCK);
    }
    bool all_closed = false;
    while (!all_closed) {
        all_closed = true;
        for (int i = INPUTS_COUNT - 1; i >= 0; --i) {
            if (input_fds[i] == -1) {
                continue;
            }
            all_closed = false;
            char buf[100];
            int read_bytes = 0;
            while ((read_bytes = read(input_fds[i], buf, sizeof(buf))) > 0) {
                buf[read_bytes] = '\0';
                log_printf("Read from %d subprocess: %s", i, buf);
            }
            if (read_bytes == 0) {
                close(input_fds[i]);
                input_fds[i] = -1;
            } else {
                conditional_handle_error(errno != EAGAIN, "strange error");
            }
        }
    }
    #endif
    #ifdef EPOLL_REALISATION
    log_printf("Epoll realisation start\n");
    int epoll_fd = epoll_create1(0); // epoll_create has one legacy parameter, so I prefer to use newer function
    for (int i = 0; i < INPUTS_COUNT; ++i) {
        struct epoll_event event = {.events = EPOLLIN | EPOLLERR | EPOLLHUP, .data = {.u32 = i}};
        epoll_ctl(epoll_fd, EPOLL_CTL_ADD, input_fds[i], &event);
    }
    int not_closed = INPUTS_COUNT;
    while (not_closed > 0) {
        struct epoll_event event;
        int epoll_ret = epoll_wait(epoll_fd, &event, 1, 1000);
        if (epoll_ret <= 0) {
            continue;
        }
        int i = event.data.u32;
        
        char buf[100];
        int read_bytes = 0;
        if ((read_bytes = read(input_fds[i], buf, sizeof(buf))) > 0) {
            buf[read_bytes] = '\0';
            log_printf("Read from %d subprocess: %s", i, buf);
        } else if (read_bytes == 0) {
            epoll_ctl(epoll_fd, EPOLL_CTL_DEL, input_fds[i], &event);
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


Run: `gcc -DTRIVIAL_REALISATION epoll.cpp -o epoll.exe`



Run: `./epoll.exe`


     00:56:52 : Trivial realisation start
     00:56:56 : Read from 4 subprocess: Hello from 4 subprocess
     00:56:56 : Read from 3 subprocess: Hello from 3 subprocess
     00:56:56 : Read from 2 subprocess: Hello from 2 subprocess
     00:56:56 : Read from 1 subprocess: Hello from 1 subprocess
     00:56:56 : Read from 0 subprocess: Hello from 0 subprocess



Run: `gcc -DNONBLOCK_REALISATION epoll.cpp -o epoll.exe`



Run: `./epoll.exe`


     00:56:56 : Nonblock realisation start
     00:56:56 : Read from 0 subprocess: Hello from 0 subprocess
     00:56:57 : Read from 1 subprocess: Hello from 1 subprocess
     00:56:58 : Read from 2 subprocess: Hello from 2 subprocess
     00:56:59 : Read from 3 subprocess: Hello from 3 subprocess
     00:57:00 : Read from 4 subprocess: Hello from 4 subprocess



Run: `gcc -DSELECT_REALISATION epoll.cpp -o epoll.exe`



Run: `./epoll.exe`


     00:57:01 : Select realisation start
     00:57:01 : Read from 0 subprocess: Hello from 0 subprocess
     00:57:02 : Read from 1 subprocess: Hello from 1 subprocess
     00:57:03 : Read from 2 subprocess: Hello from 2 subprocess
     00:57:04 : Read from 3 subprocess: Hello from 3 subprocess
     00:57:05 : Read from 4 subprocess: Hello from 4 subprocess



Run: `gcc -DEPOLL_REALISATION epoll.cpp -o epoll.exe`



Run: `./epoll.exe`


     00:57:07 : Epoll realisation start
     00:57:07 : Read from 0 subprocess: Hello from 0 subprocess
     00:57:08 : Read from 1 subprocess: Hello from 1 subprocess
     00:57:09 : Read from 2 subprocess: Hello from 2 subprocess
     00:57:10 : Read from 3 subprocess: Hello from 3 subprocess
     00:57:11 : Read from 4 subprocess: Hello from 4 subprocess



```python

```


```python

```


```python

```


```python

```


```python

```


```python

```


```python

```


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
