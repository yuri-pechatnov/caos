// %%cpp aio.c
// %run gcc aio.c -o aio.exe -laio # обратите внимание
// %run ./aio.exe
// %run cat ./output_0.txt
// %run cat ./output_1.txt

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

