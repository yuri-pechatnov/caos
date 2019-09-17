#include <stdio.h>

#define CONST_A 123

#define mult(a, b) ((a) * (b))

#define add_prefix_aba_(w) aba_##w

#define print_int(i) printf(#i " = %d\n", (i));

#define eprintf(...) fprintf(stderr, __VA_ARGS__)

#define sum_2(a, b, c) ((a) + (b))
#define sum_3(a, b, c) ((a) + (b) + (c))

#define sum_impl(a, b, c, sum_func, ...) sum_func(a, b, c)

#define sum(...) sum_impl(__VA_ARGS__, sum_3, sum_2)

int ret_42();
float get_pi();

int main() {
    #ifdef DEBUG
        freopen("001", "rt", stdin);
    #endif

    printf("Hello world!\n");
    printf("CONST_A %d\n", CONST_A);
    printf("mult(4, 6) = %d\n", mult(2 + 2, 3 + 3));

    int aba_x = 42;
    int x = 420;
    printf("aba_x ? x = %d\n", add_prefix_aba_(x));

    print_int(9 * 9 + 1);

    eprintf("It is in stderr: %d\n", 431);

    print_int(sum(1, 1));
    print_int(sum(1, 1, 1));

    int a;
    scanf("%d", &a);
    print_int(a + a);

    print_int(ret_42());
    printf("get_pi() = %f\n", get_pi());

    return 0;
}
