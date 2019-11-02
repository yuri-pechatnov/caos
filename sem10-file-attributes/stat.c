
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <pwd.h>
#include <stdio.h>
#include <assert.h>

int main(int argc, char *argv[])
{
    struct stat s;
    fstat(0, &s);
    struct passwd *pw = getpwuid(s.st_uid);
    assert(pw);
    printf("%s\n", pw->pw_name);
    return 0;
}

