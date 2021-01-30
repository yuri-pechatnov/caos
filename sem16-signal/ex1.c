// %%cpp ex1.c

#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>

int f() {
    return 42;
}

int axa(int a, int b, ...) {
    printf("axa %d %d %d\n", a, b, *(&b + 1));
}

int main(int argc, char** argv) {
    int a = 42;
    printf("%d %d\n", 5, a);
    axa(100, 200);
    
    union {
        double d;
        unsigned long long b;
    } u = {1.0};
    
    u.b ^= 1ull << 52;
    printf("u.d = %lf\n", u.d);
    
    return a + 4;
}

