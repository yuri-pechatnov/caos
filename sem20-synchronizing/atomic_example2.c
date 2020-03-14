// %%cpp atomic_example2.c
// %run gcc -fsanitize=thread atomic_example2.c -lpthread -o atomic_example2.exe
// %run ./atomic_example2.exe > out.txt
// %run cat out.txt

#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// ПЛОХОЙ КОД!!!
_Atomic int* x;

int main(int argc, char* argv[]) {
    int data[3] = {10, 20, 30};
    int* one = data + 0;
    int* two = data + 1;
    int* three = data + 2;
    
    atomic_store(&x, one);

    printf("%d\n", *atomic_load(&x));
    
    int* i = two;
    // изменение не пройдет, так как x = 1, а i = 2, i станет равным x
    atomic_compare_exchange_strong(&x, &i, three);
    printf("%d\n", *atomic_load(&x));

    i = one;
    // тут пройдет
    atomic_compare_exchange_strong(&x, &i, three);
    printf("%d\n", *atomic_load(&x));
    return 0;
}

