// %%cpp wcc.cpp
// %run gcc wcc.cpp -o wcc.exe
// %run # bash -c "for i in {0..1000000} ; do echo -n '1' ; done" > input.txt
// %run wc -c input.txt
// %run time ./wcc.exe 1 input.txt
// %run time ./wcc.exe 10 input.txt
// %run time ./wcc.exe 100 input.txt
// %run time ./wcc.exe 1000 input.txt
// %run time ./wcc.exe 10000 input.txt

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>

int main(int argc, char** argv) {
    assert(argc == 3);
    int buff_size = 1;
    int ret = sscanf(argv[1], "%d", &buff_size);
    assert(ret == 1);
    int fd = open(argv[2], O_RDONLY);
    assert(fd >= 0);
    char buff[buff_size];
    int result = 0;
    int cnt = 0;
    while ((cnt = read(fd, buff, buff_size)) > 0) {
        for (int i = 0; i < cnt; ++i) {
            result += buff[i];
        }
    }
    printf("CNT: %d\n", cnt);
    return 0;
}

