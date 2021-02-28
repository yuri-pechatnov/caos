// %%cpp multiplexing_reader_select.c
// %run gcc multiplexing_reader_test.c multiplexing_reader_common.c multiplexing_reader_select.c -o select.exe
// %MD ### Результаты чтения с помощью select
// %MD Видно, что чтение происходит сразу после записи, а user и system time близки к 0
// %run time -p ./select.exe

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

