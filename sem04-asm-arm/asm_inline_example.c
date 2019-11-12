
#include <stdio.h>

int sum(int a, int b) {
    return a + b;
}


int sum2(int, int);
__asm__ (R"(
.global sum2
sum2:
    add r0, r0, r1
    bx  lr
)");


int main() {
    printf("40 + 2 = %d\n", sum(40, 2));
    printf("40 + 2 = %d\n", sum2(40, 2));
    return 0;
}

