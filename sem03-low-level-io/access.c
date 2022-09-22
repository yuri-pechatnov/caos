// %%cpp access.c
// %run gcc access.c -o access.exe
// %run ./access.exe

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define PRINT_WITH_ERRNO(call) do {                                 \
    int __a_res = (call);                                           \
    char* err = ((__a_res == 0) ? "Success" : strerror(errno));     \
    printf("%s = %d, err: %s\n", #call, __a_res, err);              \
} while (0)

int main() {
    PRINT_WITH_ERRNO(access("./access.exe",    X_OK));
    PRINT_WITH_ERRNO(access("./access.c",      X_OK));
    PRINT_WITH_ERRNO(access("./access.exe",    F_OK));
    PRINT_WITH_ERRNO(access("./access788.exe", F_OK));
    PRINT_WITH_ERRNO(access("./access.exe",    W_OK));
    PRINT_WITH_ERRNO(access("/bin/bash",       W_OK));
    return 0;
}

