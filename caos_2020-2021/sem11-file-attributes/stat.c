// %%cpp stat.c
// %run gcc stat.c -o stat.exe
// %run rm -rf tmp2 && mkdir tmp2
// %run touch tmp2/a 
// %run touch tmp2/b && chmod +x tmp2/b 
// %run ./stat.exe tmp2/a  # usual 
// %run ./stat.exe tmp2/aa  # usual
// %run ./stat.exe tmp2/b  # executable

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
    int status = access(argv[1], X_OK);
    fprintf(stderr, "Can execute: %s\n", (status == 0) ? "yes" : "no");
    if (status < 0) {
        perror("Can not execute because");
    }
    return 0;
}

