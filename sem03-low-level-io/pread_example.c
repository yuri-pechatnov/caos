// %%cpp pread_example.c
// %run gcc pread_example.c -o pread_example.exe
// %run ./pread_example.exe pread_example.txt
// %run cat pread_example.txt

#define _LARGEFILE64_SOURCE

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
        pwrite64(fd, s, sizeof(s) - 1, 0); // DIFF
        printf("Written bytes: %d\n", (int)sizeof(s) - 1);    
        size = lseek64(fd, 0, SEEK_END);
        printf("File size: %" PRIu64 "\n", size);
    }
    
    // читаем символ со 2й позиции
    char c;
    pread64(fd, &c, 1, /* offset = */ 1); // DIFF
    c = (c < '0' || c > '9') ? '0' : ((c - '0') + 1) % 10 + '0';
    
    // записываем символ в 2ю позицию
    pwrite64(fd, &c, 1, /* offset = */ 1); // DIFF
    
    close(fd);
    return 0;
}

