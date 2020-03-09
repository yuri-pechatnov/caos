// %%cpp more_than_4.c
// %run arm-linux-gnueabi-gcc -marm more_than_4.c -O2 -S -o more_than_4.s
// %run cat more_than_4.s | grep -v "^\\s*\\." | grep -v "^\\s*@"

int mega_sum(int a1, int a2, int a3, int a4, int a5, int a6) {
    return a1 + a2 + a3 + a4 + a5 + a6;
}

