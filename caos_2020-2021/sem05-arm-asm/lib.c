// %%cpp lib.c
// %run arm-linux-gnueabi-gcc -S -Os -marm lib.c -o /dev/stdout

extern int a;
extern int r;
int r = 0;

void f() {
    ++r;
    r += a;
}

