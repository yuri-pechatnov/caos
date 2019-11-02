
#include <unistd.h>
#include <stdio.h>
#include <errno.h>


int read_retry(int fd, char* data, int size) {
    char* cdata = data;
    while (1) {
        int read_bytes = read(fd, cdata, size);
        if (read_bytes == 0) {
            return cdata - data;
        }
        if (read_bytes < 0) {
            if (errno == EAGAIN || errno == EINTR) {
                continue;
            } else {
                return -1;
            }
        }
        cdata += read_bytes;
        size -= read_bytes;
        if (size == 0) {
            return cdata - data;
        }
    }
}


int main(int argc, char *argv[])
{ 
    char buffer[4096];
    int bytes_read = read_retry(0, buffer, sizeof(buffer)); 
    if (bytes_read < 0) {
        perror("Error reading file");
        return -1;
    }
    int written_bytes = write(1, buffer, bytes_read);
    if (written_bytes < 0) {
        perror("Error writing file");
        return -1;
    }
    return 0;
}

