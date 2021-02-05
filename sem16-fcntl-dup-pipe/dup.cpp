// %%cpp dup.cpp
// %run gcc dup.cpp -o dup.exe
// %run ./dup.exe

#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/types.h>

int main() {
    int fd = dup(1);  // копируем stdout в другой дескриптор (значение дескриптора выбирается автоматически)
    dprintf(1, "Write to 1 fd.\n");
    dprintf(fd, "Write to %d fd.\n", fd);
    close(1);
    dprintf(fd, "Write to %d fd after closing 1 fd. (still to stdout)\n", fd);
    close(fd);
    return 0;
}

