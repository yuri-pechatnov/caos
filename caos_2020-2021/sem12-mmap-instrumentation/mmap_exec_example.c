// %%cpp mmap_exec_example.c
// %run gcc -g mmap_exec_example.c -o mmap_exec_example.exe
// %run ./mmap_exec_example.exe

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <assert.h>

int main() {
    int fd = open("func.o", O_RDWR);
    struct stat s;
    assert(fstat(fd, &s) == 0);
    
    printf("file size = %d\n", (int)s.st_size);
    
    void* mapped = mmap(
        /* desired addr, addr = */ NULL, 
        /* length = */ s.st_size, 
        /* access attributes, prot = */ PROT_READ | PROT_EXEC, // обратите внимание на PROT_EXEC
        /* flags = */ MAP_PRIVATE,
        /* fd = */ fd,
        /* offset in file, offset = */ 0
    );
    assert(close(fd) == 0); // Не забываем закрывать файл (при закрытии регион памяти остается доступным)
    if (mapped == MAP_FAILED) {
        perror("Can't mmap");
        return -1;
    }
 
    int (*func)(int, int) = (void*)((char*)mapped + 0x40); // 0x40 - тот самый оффсет
   
    printf("func(1, 1) = %d\n", func(1, 1));
    printf("func(10, 100) = %d\n", func(10, 100));
    printf("func(40, 5000) = %d\n", func(40, 5000));
    
    assert(munmap(
        /* mapped addr, addr = */ mapped, 
        /* length = */ s.st_size
    ) == 0);
    return 0;
}

