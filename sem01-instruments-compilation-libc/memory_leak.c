// %%cpp memory_leak.c

#include <stdio.h>
#include <stdlib.h>

int main() {
    malloc(16);
    return 0;
}

