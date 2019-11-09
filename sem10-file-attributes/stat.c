
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>

int main(int argc, char *argv[])
{
    assert(argc >= 1);
    struct stat s;
    printf("Can execute: %s\n", (access(argv[1], X_OK) == 0) ? "yes" : "no");
    return 0;
}

