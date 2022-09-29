// %%cpp fs_stat.c
// %run gcc -Wall -Werror -fsanitize=address fs_stat.c -lpthread -o fs_stat.exe
// %run ./fs_stat.exe /
// %run ./fs_stat.exe /dev/shm
// %run ./fs_stat.exe /dev

#include <stdio.h>
#include <sys/statvfs.h>
#include <assert.h>

    
int main(int argc, char** argv) {
    assert(argc == 2);
    const char* dir_path = argv[1];
    struct statvfs stat;
    statvfs(dir_path, &stat);
    
    printf("Free 1K-blocks %lu/%lu", stat.f_bavail * stat.f_bsize / 1024, stat.f_blocks * stat.f_bsize / 1024);
    return 0;
}

