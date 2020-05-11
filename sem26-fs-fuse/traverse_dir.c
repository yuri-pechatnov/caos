// %%cpp traverse_dir.c
// %run gcc -Wall -Werror -fsanitize=address traverse_dir.c -lpthread -o traverse_dir.exe
// %run ./traverse_dir.exe ..

#include <stdio.h>
#include <dirent.h>
#include <assert.h>
#include <fnmatch.h>

int main(int argc, char** argv) {
    assert(argc == 2);
    const char* dir_path = argv[1];
    DIR *pDir = opendir(dir_path);
    if (pDir == NULL) {
        fprintf(stderr, "Cannot open directory '%s'\n", dir_path);
        return 1;
    }
    int limit = 4;
    for (struct dirent *pDirent; (pDirent = readdir(pDir)) != NULL && limit > 0;) {
        // + Регулярочки
        if (fnmatch("sem2*", pDirent->d_name, 0) == 0) {
            printf("%s\n", pDirent->d_name);
            --limit;
        }
    }

    closedir(pDir);
    return 0;
}

