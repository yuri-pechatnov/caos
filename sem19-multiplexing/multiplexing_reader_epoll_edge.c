// %%cpp multiplexing_reader_epoll_edge.c
// %run gcc multiplexing_reader_test.c multiplexing_reader_common.c multiplexing_reader_epoll_edge.c -o epoll_edge.exe
// %MD ### Результаты чтения с помощью epoll c edge-triggering (EPOLLET опция)
// %MD Видно, что чтение происходит сразу после записи, а user и system time близки к 0
// %run time -p ./epoll_edge.exe

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

