// %%cpp epoll.cpp
// %run gcc -DTRIVIAL_REALISATION epoll.cpp -o epoll.exe
// %run ./epoll.exe
// %run gcc -DNONBLOCK_REALISATION epoll.cpp -o epoll.exe
// %run ./epoll.exe
// %run gcc -DSELECT_REALISATION epoll.cpp -o epoll.exe
// %run ./epoll.exe
// %run gcc -DEPOLL_REALISATION epoll.cpp -o epoll.exe
// %run ./epoll.exe
// %run gcc -DEPOLL_EDGE_TRIGGERED_REALISATION epoll.cpp -o epoll.exe
// %run ./epoll.exe

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

