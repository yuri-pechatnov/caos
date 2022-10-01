// %%cpp stat.c
// %run gcc stat.c -o stat.exe
// %run rm -rf tmp && mkdir tmp && touch tmp/a && ln -s ./a tmp/a_link && mkdir tmp/dir
// %run for i in tmp/a tmp/dir tmp/a_link ; do (printf "%10s    " $i ; ./stat.exe < $i); done

#include "stat.h"
#include <assert.h>

int main(int argc, char *argv[])
{   
    struct stat s;
    fstat(0, &s); // get stat for stdin
    describe_stat_st_mode(&s); 
    return 0;
}

