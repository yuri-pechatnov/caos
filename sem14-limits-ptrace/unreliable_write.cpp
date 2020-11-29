// %%cpp unreliable_write.cpp
// %run gcc unreliable_write.cpp -o unreliable_write.exe
// %run ./unreliable_write.exe
// %run ./run_with_unreliable_io.exe 1 0 ./unreliable_write.exe
// %run ./run_with_unreliable_io.exe 1 1 ./unreliable_write.exe

#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

int main() {
    const char str[] = "Hello from C!\n";
    int written_p = printf("Reliable print: %s", str); fflush(stdout);
    fprintf(stderr, "Written %d bytes by printf. errno=%d, err=%s\n", written_p, errno, strerror(errno)); fflush(stderr);
  
    int written_w = write(1, str, sizeof(str));
    perror("write");
    fprintf(stderr, "Written %d bytes by write. errno=%d, err=%s\n", written_w, errno, strerror(errno)); fflush(stderr);
    
    return 0;
}

