// %%cpp double_mul.c
// %run gcc -m32 -masm=intel -O3 double_mul.c -S -o double_mul.S
// %run cat double_mul.S
    
double mul(double a) { 
    return a * 13;
}

double mul2(double a, double b) { 
    return a * b;
}

