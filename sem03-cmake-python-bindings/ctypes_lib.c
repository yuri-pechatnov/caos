// %%cpp ctypes_lib.c
// %// Делаем самую обычную динамическую библиотеку
// %run gcc -Wall ctypes_lib.c -shared -fPIC -fsanitize=address -o ctypes_lib.so

float sum_ab(int a, float b) {
    return a + b;
}

