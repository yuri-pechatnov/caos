// %%cpp jmp.cpp
// %run gcc jmp.cpp -o jmp.exe
// %run ./jmp.exe

#include <stdio.h>
#include <setjmp.h>


int main() {
    int i = 0;
    jmp_buf environment;
    int jmp_ret = setjmp(environment);
    printf("i = %d, jmp_ret=%d\n", i, jmp_ret);

    if (++i < 3) {
        longjmp(environment, i * 100 + 1);
    }
    return 0;
}

