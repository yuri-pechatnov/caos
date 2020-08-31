// %%cpp strange_example.c
// %run gcc strange_example.c -o strange_example.exe
// %run echo "Hello world!" > a.txt
// %run ./strange_example.exe 5< a.txt > strange_example.out
// %run cat strange_example.out

#include <unistd.h>
#include <stdio.h>

int main(int argc, char *argv[])
{ 
    char buffer[4096];
    int bytes_read = read(5, buffer, sizeof(buffer)); 
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

