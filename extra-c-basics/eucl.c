// %%cpp eucl.c
// %run gcc -Wall -Werror --sanitize=address eucl.c -o eucl.exe
// %run echo "3 5 7" | ./eucl.exe

#include <stdio.h>
#include <assert.h>

typedef struct {
    int gcd;
    int x;
    int y;
} gcd_result_t;

gcd_result_t gcd(int a, int b) {
    if (a == 0) {
        return (gcd_result_t){.gcd = b, .x = 0, .y = 1};
    }
    gcd_result_t r = gcd(b % a, a);
    return (gcd_result_t){.gcd = r.gcd, .x = r.y - (b / a) * r.x, .y = r.x};
}

int main() {
    int a = 0;
    int b = 0;
    int c = 0;
    scanf("%d%d%d", &a, &b, &c);
    
    gcd_result_t r = gcd(a, b);

    if (c % r.gcd != 0) {
        printf("Impossible");
        return 0;
    } else {
        //int ka = a / r.gcd;
        int kb = b / r.gcd;
        int kc = c / r.gcd;
        fprintf(stderr, "GCD: %d * %d + %d * %d = %d\n", a, r.x, b, r.y, r.gcd);
        int x = r.x * kc;
        int y = r.y * kc;
        fprintf(stderr, "First version: %d * %d + %d * %d = %d\n", a, x, b, y, c);
        
        x = ((x % kb) + kb) % kb; 
        assert((c - x * a) % b == 0);
        y = (c - x * a) / b;
//         while (a * x2 + b * y2 != c) {
//             ++x2;
//             y2 = (c - x2 * a) / b;
//         }
        printf("%d %d", x, y);
    }
    return 0;
}

