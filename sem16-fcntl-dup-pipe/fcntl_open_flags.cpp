// %%cpp fcntl_open_flags.cpp
// %run gcc fcntl_open_flags.cpp -o fcntl_open_flags.exe
// %run ./fcntl_open_flags.exe

#ifndef _GNU_SOURCE
  #define _GNU_SOURCE
#endif
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>

void describe_fd(const char* prefix, int fd) {
    int ret = fcntl(fd, F_GETFL, 0);
    
#define flag_cond_str_expanded(flag, mask, name) ((ret & (mask)) == flag ? name : "")
#define flag_cond_str_mask(flag, mask) flag_cond_str_expanded(flag, mask, #flag)
#define flag_cond_str(flag) flag_cond_str_expanded(flag, flag, #flag)
    //printf("%d\n", ret & 3);
    printf("%s: %s, %s, %s, %s, %s, %s, %s, %s, %s, %s\n", prefix
        , flag_cond_str_mask(O_RDONLY, O_RDONLY | O_WRONLY | O_RDWR)
        , flag_cond_str_mask(O_WRONLY, O_RDONLY | O_WRONLY | O_RDWR)
        , flag_cond_str_mask(O_RDWR, O_RDONLY | O_WRONLY | O_RDWR)
        , flag_cond_str(O_TRUNC)
        , flag_cond_str(O_APPEND)
        , flag_cond_str(O_CREAT)
        , flag_cond_str(O_CLOEXEC)
        , flag_cond_str(O_TMPFILE)
        , flag_cond_str(O_ASYNC)
        , flag_cond_str(O_DIRECT)
    );
}

void check_fd(int fd) {
    if (fd < 0) {
        perror("open");
        assert(fd >= 0);
    }
} 

int main() {
    describe_fd("0 (stdin)", 0);
    describe_fd("1 (stdout)", 1);
    describe_fd("2 (stderr)", 2);
    
    int f1 = open("fcntl_open_flags.1", O_CREAT|O_TRUNC|O_WRONLY, 0664); check_fd(f1);
    describe_fd("f1 O_CREAT|O_TRUNC|O_WRONLY", f1);
    
    int f2 = open("fcntl_open_flags.2", O_CREAT|O_RDWR, 0664); check_fd(f2);
    describe_fd("f2 O_CREAT|O_RDWR", f2);
    
    int f3 = open("fcntl_open_flags.2", O_WRONLY|O_APPEND); check_fd(f3);
    describe_fd("f3 O_WRONLY|O_APPEND", f3);

    int f4 = open("fcntl_open_flags.2", O_RDONLY|O_NONBLOCK|O_ASYNC|O_DIRECT); check_fd(f4);
    describe_fd("f4 O_RDONLY|O_NONBLOCK|O_ASYNC|O_DIRECT", f4);
    
    int f5 = open("./", O_TMPFILE|O_RDWR, 0664); check_fd(f5);
    describe_fd("f5 O_TMPFILE|O_RDWR", f5);
    
    int fds[2];
    pipe2(fds, O_CLOEXEC); 
    describe_fd("pipe2(fds, O_CLOEXEC)", fds[0]);
    return 0;
}

