// %%cpp linux_file_hello_world.c
// %run gcc linux_file_hello_world.c -o linux_file_hello_world.exe
// %run ./linux_file_hello_world.exe
// %run cat linux_file_hello_world.out

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

int main(int argc, char *argv[])
{   
    int fd = open("linux_file_hello_world.out", O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH); 
    // S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH == 0664
    // попробуйте не указывать 0664   
    // (ошибка такая же как в printf("%d");)
    // для справки `man 2 open`
     
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

