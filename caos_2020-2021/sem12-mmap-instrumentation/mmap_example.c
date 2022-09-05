// %%cpp mmap_example.c
// %run gcc mmap_example.c -o mmap_example.exe
// %run echo "000" > buf.txt && ./mmap_example.exe && cat buf.txt
// %run echo "79" > buf.txt && ./mmap_example.exe && cat buf.txt
// %run echo "xxx" > buf.txt && ./mmap_example.exe && cat buf.txt
// %run echo -n "S" > buf.txt && ./mmap_example.exe && cat buf.txt

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <assert.h>

int get_page_size() {
    static int page_size = 0;
    return page_size = page_size ?: sysconf(_SC_PAGE_SIZE);
}

int upper_round_to_page_size(int sz) {
    return (sz + get_page_size() - 1) / get_page_size() * get_page_size();
}

int main() {
    printf("page size = %d\n", get_page_size());
    int fd = open("buf.txt", O_RDWR);
    struct stat s;
    assert(fstat(fd, &s) == 0);
    
    printf("file size = %d\n", (int)s.st_size);
    int old_st_size = s.st_size;
    if (s.st_size < 2) {
        const int new_size = 10;
        assert(ftruncate(fd, new_size) == 0); // изменяем размер файла
        assert(fstat(fd, &s) == 0);
        printf("new file size = %d\n", (int)s.st_size);
    }
    
    void* mapped = mmap(
        /* desired addr, addr = */ NULL, 
        /* length = */ s.st_size, 
        /* access attributes, prot = */ PROT_READ | PROT_WRITE,
        /* flags = */ MAP_SHARED,
        /* fd = */ fd,
        /* offset in file, offset = */ 0
    );
    assert(mapped != MAP_FAILED);
    assert(close(fd) == 0); // Не забываем закрывать файл
    
    char* buf = mapped;
    
    if (old_st_size != s.st_size) {
        for (int j = old_st_size; j < s.st_size; ++j) {
            buf[j] = '_';
        }
    }
    
    buf[1] = ('0' <= buf[1] && buf[1] <= '9') ? ((buf[1] - '0' + 1) % 10 + '0') : '0';
    
    assert(munmap(
        /* mapped addr, addr = */ mapped, 
        /* length = */ s.st_size
    ) == 0);
    
    return 0;
}

