// %%cpp macro_example_2.c
// %run gcc macro_example_2.c -o macro_example_2.exe
// %run ./macro_example_2.exe

#include <stdio.h>

// #VAR_NAME разворачивается в строковый литерал "VAR_NAME"
#define print_int(i) printf(#i " = %d\n", (i));

// Полезный макрос для вывода в поток ошибок
#define eprintf(...) fprintf(stderr, __VA_ARGS__)

// Способ сделать макрос с переменным числом аргументов
#define sum_2(a, b, c) ((a) + (b))
#define sum_3(a, b, c) ((a) + (b) + (c))

#define sum_impl(a, b, c, sum_func, ...) sum_func(a, b, c)

#define sum(...) sum_impl(__VA_ARGS__, sum_3, sum_2)


int main() {
    print_int(9 * 9 + 1);

    eprintf("It is in stderr: %d\n", 431);

    print_int(sum(1, 1));
    print_int(sum(1, 1, 1));

    return 0;
}

