// %%cpp lib.c
// %run gcc -shared -fPIC lib.c -o lib.so # compile shared library

int sum(int a, int b) {
    return a + b;
}

float sum_f(float a, float b) {
    return a + b;
}

