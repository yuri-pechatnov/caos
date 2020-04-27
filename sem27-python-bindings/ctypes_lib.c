// %%cpp ctypes_lib.c
// %run clang -Wall ctypes_lib.c -shared -fPIC -o ctypes_lib.so

float sum_ab(int a, float b) {
    return a + b;
}

