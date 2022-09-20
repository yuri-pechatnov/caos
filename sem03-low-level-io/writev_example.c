// %%cpp writev_example.c
// %run gcc writev_example.c -o writev_example.exe
// %run ./writev_example.exe writev_example.txt

#include <sys/uio.h>
#include <unistd.h>

int main(int argc, char *argv[])
{   
    char str1[] = "Hello ", str2[] = "world!\n";
    struct iovec writings[] = {
        {str1, sizeof(str1) - 1},
        {str2, sizeof(str2) - 1},
    };
    writev(STDOUT_FILENO, writings, sizeof(writings) / sizeof(struct iovec));
    return 0;
}

