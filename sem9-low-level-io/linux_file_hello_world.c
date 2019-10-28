
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>


#undef open
int main(int argc, char *argv[])
{
    (void*)open;
    
    int fd = open("linux_file_hello_world.out", O_WRONLY | O_CREAT, 0664); // попробуйте не указывать 0664
    if (fd < 0) {
        perror("Can't open file");
        return -1;
    }
    char buffer[] = "Hello world!";
    int bytes_written = write(fd, buffer, sizeof(buffer));
    if (bytes_written < 0) {
        perror("Error writing file");
        close(fd);
        return -1;
    }
    printf("Bytes written: %d (expected %d)\n", bytes_written, (int)sizeof(buffer));
    close(fd);
    return 0;
}

