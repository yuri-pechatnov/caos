// %%cpp main.c
// %run gcc  -Wall -Werror -fsanitize=address main.c -o a.exe
// %run ./a.exe 
// %run objdump -t a.exe | grep sqr

#include <stdio.h>
#include <math.h>

int sqr(int a) {
    return a * a;
}

// double sqr(double a) {
//     return a * a;
// }

int main() {
    printf("%d\n", sqr(2));
//     printf("%lf\n", sqr(3.0));
    return 0; 
}

