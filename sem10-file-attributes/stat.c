
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
    printf("is regular: %s\n", ((s.st_mode & S_IFMT) == S_IFREG) ? "yes" : "no"); 
    printf("is directory: %s\n", S_ISDIR(s.st_mode) ? "yes" : "no");
    printf("is symbolic link: %s\n", S_ISLNK(s.st_mode) ? "yes" : "no");
    close(fd);
    return 0;
}

