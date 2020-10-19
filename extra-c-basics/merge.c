// %%cpp merge.c
// %run gcc --sanitize=address merge.c -o merge.exe
// %run ./merge.exe

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define swap(a, b) { __typeof__(a) c = (a); (a) = (b); (b) = (c); }

#define mul(a, b) ((a) * (b))

int main() {
    //n (1 + 2 / 2 + 3 / 4 + 4 / 8 + 5 / 16)
    double c = 0;
    double mul = 1.0;
    for (int i = 1; i < 10; ++i) {
        c += i / mul;
        mul *= 2;
        printf("%lf\n", c);
    }
    return 0;
}

