// %%cpp lib.c
// %run gcc -O3 -shared -fPIC lib.c -o lib.so  -Os -Wl,--gc-sections -fno-asynchronous-unwind-tables -fcf-protection=branch -mmanual-endbr 

int check_increment(int x) {
    return x + 1 > x; // Всегда ли true?
}

int unsigned_check_increment(unsigned int x) {
    return x + 1 > x; // Всегда ли true?
}

