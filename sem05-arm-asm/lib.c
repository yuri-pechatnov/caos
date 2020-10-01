// %%cpp lib.c
// %run arm-linux-gnueabi-gcc -S -Os -marm lib.c -o /dev/stdout | grep -v eabi

int strange_function(int n, int a) {
    if (n < 5) {
        return 5 - n;
    }
    return n * a + 41000000;
}

