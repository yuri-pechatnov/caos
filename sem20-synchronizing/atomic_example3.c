// %%cpp atomic_example3.c
// %run gcc -fsanitize=thread atomic_example3.c -lpthread -o atomic_example3.exe
// %run ./atomic_example3.exe > out.txt
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
    
    atomic_store(&x, (_Atomic int*) one);

    printf("%d\n", *(int*)atomic_load(&x));

    int* i = two;
    // изменение не пройдет, так как x = 1, а i = 2, i станет равным x
    atomic_compare_exchange_strong(&x, (_Atomic int**) &i, (_Atomic int*) three);
    printf("%d\n", *(int*)atomic_load(&x));
   
    i = one;
    // тут пройдет
    atomic_compare_exchange_strong(&x, (_Atomic int**) &i, (_Atomic int*) three);
    i = (int*) atomic_load(&x);
    printf("%d\n", *(int*)atomic_load(&x));
    return 0;
}

