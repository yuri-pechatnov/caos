// %%cpp multiplexing_reader_nonblock.c
// %run gcc multiplexing_reader_test.c multiplexing_reader_common.c multiplexing_reader_nonblock.c -o nonblock.exe
// %MD ### Результаты чтения с помощью наивного неблокирующего read
// %MD Видно, что чтение происходит сразу после записи, **но user и system time близки к затраченному астрономическому времени**, то есть сожжено дикое количество процессорного времени
// %run time -p ./nonblock.exe

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

