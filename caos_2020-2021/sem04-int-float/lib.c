// %%cpp lib.c
// %run gcc -O3 -shared -fPIC lib.c -o lib.so

int check_increment(int x) {
    return x + 1 > x;
}

int unsigned_check_increment(unsigned int x) {
    return x + 1 > x;
}

