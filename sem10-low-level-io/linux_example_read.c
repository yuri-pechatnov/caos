// %%cpp linux_example_read.c
// %run gcc linux_example_read.c -o linux_example_read.exe
// %run echo -n "Hello from file!" > linux_example_read.txt
// %run echo -n "Hello from stdin!" | ./linux_example_read.exe linux_example_read.txt

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>

int main(int argc, char *argv[])
{
    assert(argc >= 2);
    const char* file_name = argv[1];
    
    char buffer[4096];
    int bytes_read = read(STDIN_FILENO, buffer, sizeof(buffer));
    printf("From stdin: '%.*s'\n", bytes_read, buffer); // buffer not zero-terminated string after `read`!
    
    int fd = open(file_name, O_RDONLY); // O_RDWR also works 
    bytes_read = read(fd, buffer, sizeof(buffer));
    printf("From file '%s': '%.*s'\n", file_name, bytes_read, buffer); 
    close(fd);
    
    return 0;
}

