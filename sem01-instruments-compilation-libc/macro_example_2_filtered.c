
#include <stdio.h>
#include <string.h>
#include <assert.h>

/* #VAR_NAME разворачивается в строковый литерал "VAR_NAME" */
#define print_int(i) printf(#i " = %d\n", (i));

/* Полезный макрос для вывода в поток ошибок */
#define eprintf(...) fprintf(stderr, __VA_ARGS__)

#define logprintf_impl(fmt, line, ...) eprintf(__FILE__ ":" #line " " fmt, __VA_ARGS__)
#define logprintf_impl_2(line, fmt, ...) logprintf_impl(fmt "%s", line, __VA_ARGS__)
#define logprintf(...) logprintf_impl_2(__LINE__, __VA_ARGS__, "")

#define SWAP(a, b) do { __typeof__(a) __swap_c = (a); (a) = (b); (b) = (__swap_c); } while (0)

/* Способ сделать макрос с переменным числом аргументов
 * И это единственный способ "перегрузить функцию в С" */
#define sum_2(a, b, c) ((a) + (b))
#define sum_3(a, b, c) ((a) + (b) + (c))

#define sum_impl(a, b, c, sum_func, ...) sum_func(a, b, c)

#define sum(...) sum_impl(__VA_ARGS__, sum_3, sum_2)


int main() {
    /* assert(3 > 4); */
    print_int(9 * 9 + 1);

    eprintf("It is in stderr: %d\n", 431);

    int x = 1, y = 2;
    eprintf("(x, y) = (%d, %d)\n", x, y);
    SWAP(x, y);
    eprintf("(x, y) = (%d, %d)\n", x, y);

    print_int(sum(1, 1));
    print_int(sum(1, 1, 1));
    
    eprintf("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
    
    logprintf("Before exit %s\n", "");
    return 0;
}

