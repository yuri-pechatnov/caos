// %%cpp stat.c
// %run gcc stat.c -o stat.exe
// %run rm -rf tmp && mkdir tmp && touch tmp/a && ln -s ./a tmp/a_link && mkdir tmp/dir
// %run for i in tmp/a tmp/dir tmp/a_link ; do (printf "%10s    " $i ; ./stat.exe $i); done

#define _GNU_SOURCE // need for O_PATH

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>

int main(int argc, char *argv[])
{   
    assert(argc == 2);
    struct stat s;
    int fd = open(argv[1], O_RDONLY | O_NOFOLLOW | O_PATH);
    assert(fd >= 0);
    fstat(fd, &s); 
    printf("is regular: %s    ", ((s.st_mode & S_IFMT) == S_IFREG) ? "yes" : "no "); // can use predefined mask
    printf("is directory: %s    ", S_ISDIR(s.st_mode) ? "yes" : "no "); // or predefined macro
    printf("is symbolic link: %s\n", S_ISLNK(s.st_mode) ? "yes" : "no ");
    close(fd);
    return 0;
}

