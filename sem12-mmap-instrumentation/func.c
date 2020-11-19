// %%cpp func.c
// %run gcc -g -fPIC func.c -c -o func.o 
// %run objdump -F -d func.o | grep my_func -A 15

int my_func(int a, int b) {
    return a + b + 1;
}

