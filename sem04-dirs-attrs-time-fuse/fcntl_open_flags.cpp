// %%cpp fcntl_open_flags.cpp
// %run gcc fcntl_open_flags.cpp -o fcntl_open_flags.exe
// %run ./fcntl_open_flags.exe

#ifndef _GNU_SOURCE
  #define _GNU_SOURCE
#endif
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

void describe_fd_impl(const char* prefix, int fd) {
    if (fd < 0) { // Check that fd is valid.
        perror("open"); 
        abort();
    }
    int ret = fcntl(fd, F_GETFL, 0); // Get flags here!
    int mask = O_RDONLY | O_WRONLY | O_RDWR;
#define flag_cond_str_expanded(flag, name) ((ret & mask) == flag ? name : "")
#define flag_cond_str(flag) flag_cond_str_expanded(flag, #flag)
    printf("%55s:   %s%s%s\n", prefix, flag_cond_str(O_RDONLY), flag_cond_str(O_WRONLY), flag_cond_str(O_RDWR));
}

int main() {
#define describe(op) describe_fd_impl(#op, op)
    describe(open("fcntl_open_flags.1", O_CREAT | O_WRONLY, 0664));
    describe(open("fcntl_open_flags.2", O_CREAT | O_RDWR, 0664));
    describe(open("fcntl_open_flags.2", O_RDONLY));
    return 0;
}

