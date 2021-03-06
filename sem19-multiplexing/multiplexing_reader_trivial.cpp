// %%cpp multiplexing_reader_trivial.cpp
// %run g++ multiplexing_reader_test.cpp multiplexing_reader_trivial.cpp -o trivial.exe
// %run ./trivial.exe

#include "multiplexing_reader_common.h"

#include <sys/epoll.h>

void read_all(int* input_fds, int count) {
    // Работает неэффективно, так как при попытке считать из пайпа мы можем на этом надолго заблокироваться 
    // А в другом пайпе данные могут появиться, но мы их не сможем обработать сразу (заблокированы, пытаясь читать другой пайп)
    log_printf("Trivial realisation start\n");
    // Проходимся по всем файловым дескрипторам (специально выбрал плохой порядок)
    for (int i = 0; i < count; ++i) {
        char buf[100];
        int read_bytes = 0;
        while ((read_bytes = read(input_fds[i], buf, sizeof(buf))) > 0) { // Читаем файл пока он не закроется.
            buf[read_bytes] = '\0';
            log_printf("Read from %d subprocess: %s", i, buf);
        }
        conditional_handle_error(read_bytes < 0, "read error");
    }
    log_printf("Trivial realisation finish\n");
}

