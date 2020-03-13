// %%cpp lib.c
// %# `-shared` - make shared library
// %# `-fPIC` - make Positional Independant Code
// %run gcc -Wall -shared -fPIC lib.c -o libsum.so # compile shared library
// %run objdump -t libsum.so | grep sum # symbols in shared library filtered by 'sum'

int sum(int a, int b) {
    return a + b;
}

float sum_f(float a, float b) {
    return a + b;
}

