// %%cpp lseek_example.c
// %run gcc lseek_example.c -o lseek_example.exe
// %run ./lseek_example.exe lseek_example.txt
// %run cat lseek_example.txt

#define _LARGEFILE64_SOURCE // Enable xxx64 functions.

#include <assert.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

int main(int argc, char *argv[])
{   
    assert(argc >= 2);
    // O_RDWR - открытие файла на чтение и запись одновременно
    int fd = open(argv[1], O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH); 
    
    // Перемещаемся на конец файла, получаем позицию конца файла - это размер файла
    uint64_t size = lseek64(fd, 0, SEEK_END);
    
    printf("File size: %" PRIu64 "\n", size);
    
    // если размер меньше 2, то дописываем цифры
    if (size < 2) {
        const char s[] = "10";
        lseek64(fd, 0, SEEK_SET);
        write(fd, s, sizeof(s) - 1);
        printf("Written bytes: %d\n", (int)sizeof(s) - 1);    
        size = lseek64(fd, 0, SEEK_END);
        printf("File size: %" PRIu64 "\n", size);
    }
    
    // читаем символ со 2й позиции
    lseek64(fd, 1, SEEK_SET);
    char c;
    read(fd, &c, 1);
    c = (c < '0' || c > '9') ? '0' : ((c - '0') + 1) % 10 + '0';
    
    // записываем символ в 2ю позицию
    lseek64(fd, 1, SEEK_SET);
    write(fd, &c, 1);
    
    close(fd);
    return 0;
}

