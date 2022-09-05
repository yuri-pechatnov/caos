// %%cpp my_sum_test.c
// %run gcc -g3 -m64 -masm=intel my_sum_test.c my_sum.S -o my_sum_test.exe
// %run ./my_sum_test.exe

#include <stdint.h>
#include <stdio.h>
#include <assert.h>
    
int32_t my_sum(int32_t n, int32_t* x);

int main() {
    int32_t x[] = {100, 2, 200, 3};
    assert(my_sum(sizeof(x) / sizeof(int32_t), x) == 100 - 2 + 200 - 3);
    int32_t y[] = {100, 2, 200};
    assert(my_sum(sizeof(y) / sizeof(int32_t), y) == 100 - 2 + 200);
    int32_t z[] = {100};
    assert(my_sum(sizeof(z) / sizeof(int32_t), z) == 100);
    printf("SUCCESS");
    return 0;
}

