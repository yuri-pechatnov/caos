// %%cpp memory_leak.cpp

#include<stdlib.h>
#include<stdio.h>

int main() {
    printf("d");
    fflush(stdout);
    malloc(16000000);
}

