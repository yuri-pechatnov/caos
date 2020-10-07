// %%cpp test_sum_v.c
// %run arm-linux-gnueabi-gcc -marm -fsanitize=address lib.c test_sum_v.c -o test_sum_v.exe
// %run qemu-arm ./test_sum_v.exe

#include <stdio.h>
#include <assert.h>

int sum_v(int* a, int* b, int n);

void test() {
    {
        int a[] = {1, 2, 3};
        int b[] = {10, 20, 30};
        sum_v(a, b, 3);
        assert(a[0] == 11);
        assert(a[1] == 22);
        assert(a[2] == 33);
    }
    {
        int a[] = {};
        int b[] = {};
        sum_v(a, b, 0);    
    }
    printf("SUCCESS\n");
}

int main() {
    test();
}

