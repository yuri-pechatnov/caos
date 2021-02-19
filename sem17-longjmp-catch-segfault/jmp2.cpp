// %%cpp jmp2.cpp
// %run gcc jmp2.cpp -o jmp2.exe
// %run ./jmp2.exe

#include <stdio.h>
#include <setjmp.h>

jmp_buf environment;

void f(int i) {
    if (i == 3) {
        longjmp(environment, 1);
    }
    printf("i = %d\n", i);
    f(i + 1);
    printf("After recursive call 1\n");
}

int main() {
    int i = 0;
    
    if (setjmp(environment) == 0) {
        printf("Right after setjmp\n");
        f(0);
        printf("After recursive call 1\n");
    } else {
        printf("Right after return to setjmp\n");
    }
    return 0;
}

