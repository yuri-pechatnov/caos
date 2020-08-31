// %%cpp atomic_example1.c
// %run gcc -fsanitize=thread atomic_example1.c -lpthread -o atomic_example1.exe
// %run ./atomic_example1.exe > out.txt
// %run cat out.txt

#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>

// _Atomic навешивается на `int`
_Atomic int x;

int main(int argc, char* argv[]) {
    atomic_store(&x, 1);
    printf("%d\n", atomic_load(&x));
    
    int i = 2;
    // изменение не пройдет, так как x = 1, а i = 2, i станет равным x
    atomic_compare_exchange_strong(&x, &i, 3);
    printf("%d\n", atomic_load(&x));

    // тут пройдет
    atomic_compare_exchange_strong(&x, &i, 3);
    printf("%d\n", atomic_load(&x));
    return 0;
}

