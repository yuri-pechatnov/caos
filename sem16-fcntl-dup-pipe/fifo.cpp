// %%cpp fifo.cpp
// %run gcc fifo.cpp -o fifo.exe
// %run rm -f ./my_fifo
// %run ./fifo.exe

#ifndef _GNU_SOURCE
  #define _GNU_SOURCE
#endif
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <time.h>
#include <errno.h>

int main() {
    // Создадим два ничего не знающих друг о друге процесса
    pid_t pid_1, pid_2;
    if ((pid_1 = fork()) == 0) {
        assert(mkfifo("./my_fifo", 0644) == 0); // создаем fifo
        int fd = open("./my_fifo", O_WRONLY); // открыли fifo на запись
        assert(fd >= 0);
        char data[] = "World is just world";
        assert(write(fd, &data, strlen(data)) == strlen(data));
        close(fd);
        return 0;
    }
    if ((pid_2 = fork()) == 0) {
        int fd = -1;
        while (true) {
            fd = open("./my_fifo", O_RDONLY); // пытаемся открыть fifo
            if (fd >= 0) {
                break;
            }
            fprintf(stderr, "Failed to open fifo. Try again\n");
            struct timespec t = {.tv_sec = 0, .tv_nsec = 10000000}; // 10ms
            nanosleep(&t, &t);  
        }
        // fd - отрытый на чтение конец fifo
        int size = 0;
        char buf[100];
        // читаем из fifo и пишем прочитанное в stdout
        while ((size = read(fd, buf, sizeof(buf))) > 0) {
            assert(write(1, buf, size) == size);
        }
        assert(size == 0); // проверяем "конец файла"
        close(fd);
        return 0;
    }
    int status;
    assert(waitpid(pid_1, &status, 0) != -1);
    assert(waitpid(pid_2, &status, 0) != -1);
    return 0;
}

