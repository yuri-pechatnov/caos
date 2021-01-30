// %%cpp coredump.c
// %run gcc -g coredump.c -o coredump.exe
// %run rm core # удаляем старый файл с coredump
// %run ./coredump.exe

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>

// can be replaced with 'ulimit -c unlimited' in terminal
void enable_core() {
    struct rlimit rlim;
    assert(0 == getrlimit(RLIMIT_CORE, &rlim));
    rlim.rlim_cur = rlim.rlim_max;
    assert(0 == setrlimit(RLIMIT_CORE, &rlim));
}

int f(int a) {
    if (1) {
        assert(a > 4); // тоже вызывает SIGABRT
    } else {
        if (a < 4) {
            raise(SIGABRT); // посылаем сигнал себе
        }
    }
    return 0;
}

int main(int argc, char** argv) {
    enable_core();
    return f(argc);
}

