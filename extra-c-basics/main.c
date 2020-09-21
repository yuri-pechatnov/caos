// %%cpp main.c
// %run clang -std=c99 -Wall -Werror -fsanitize=address main.c -o a.exe
// %run ./a.exe 

#include <stdlib.h>
#include <assert.h>

int main() {
    int* array = calloc(10, sizeof(int));
    free(array);
    assert(array[5] == 0);
    return 0;
}

