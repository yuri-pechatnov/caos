// %%cpp multiplexing_reader_epoll.c
// %run gcc multiplexing_reader_test.c multiplexing_reader_common.c multiplexing_reader_epoll.c -o epoll.exe
// %MD ### Результаты чтения с помощью epoll c level-triggering
// %MD Видно, что чтение происходит сразу после записи, а user и system time близки к 0
// %run time -p ./epoll.exe

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

