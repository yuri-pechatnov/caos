// %%cpp traverse_dir_2.c
// %run gcc -Wall -Werror -fsanitize=address traverse_dir_2.c -lpthread -o traverse_dir_2.exe
// %run ./traverse_dir_2.exe .

#include <stdio.h>
#include <ftw.h>
#include <assert.h>

int limit = 4;
    
int callback(const char* fpath, const struct stat* sb, int typeflag) {
    printf("%s %ld\n", fpath, sb->st_size);
    return (--limit == 0);
}
    
int main(int argc, char** argv) {
    assert(argc == 2);
    const char* dir_path = argv[1];
    ftw(dir_path, callback, 0);
    return 0;
}

