
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
    printf("Linux by printf");
    char linux_str[] = "Linux by write\n";
    write(1, linux_str, sizeof(linux_str));
    if (argc < 2) {
        printf("Need at least 2 arguments\n");
        return 1;
    }
    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        perror("Can't open file");
        return -1;
    }
    
    char buffer[4096];
    int bytes_read = read(fd, buffer, sizeof(buffer));
    if (bytes_read < 0) {
        perror("Error reading file");
        close(fd);
        return -1;
    }
    char buffer2[4096];
    int written_bytes = snprintf(buffer2, sizeof(buffer2), "Bytes read: %d\n'''%s'''\n", bytes_read, buffer);
    write(1, buffer2, written_bytes);
    close(fd);
    return 0;
}

