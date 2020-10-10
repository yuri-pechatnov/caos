// %%cpp main.c
// %run gcc -std=c99 -Wall -Werror -fsanitize=address main.c -o unused.exe
// %run gcc -DDO_TEST -std=c99 -Wall -Werror -fsanitize=address main.c -o o.exe
// %run ./o.exe 

#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <stdint.h>
#include <inttypes.h>


typedef int elem_t;

void slow_sort(elem_t* first, elem_t* last) {
    for (elem_t* i = first; i != last; ++i) {
        for (elem_t* j = i + 1; j != last; ++j) {
            if (*i > *j) {
                elem_t tmp = *i;
                *i = *j;
                *j = tmp;
            }
        }
    }
}

//#define fprintf(...) ;

void quick_sort(elem_t* first, elem_t* last) {
    if (last - first < 2) {
        return;
    }

    // for (elem_t* i = first; i != last; ++i) {
    //     fprintf(stderr, "%d ", *i);
    // }
    // fprintf(stderr, "\n");

    elem_t pivot = *(first + (last - first) / 2);
    // fprintf(stderr, "pivot = %d\n", pivot);

    elem_t* L = first;
    elem_t* R = last - 1;

    while (L <= R) {
        while (*L < pivot && L <= R) ++L;
        while (*R > pivot && L <= R) --R;
        if (L <= R) {
            elem_t tmp = *L;
            *L = *R;
            *R = tmp;
            ++L;
            --R;
        }
    }


    // fprintf(stderr, "sub sort L %d-%d\n", 0, R + 1 - first);
    // for (elem_t* i = first; i != R + 1; ++i) {
    //     fprintf(stderr, "%d ", *i);
    // }
    // fprintf(stderr, "\n");
    quick_sort(first, R + 1);
    // fprintf(stderr, "sub sort R %d-%d\n", L - first, last - first);
    // for (elem_t* i = L; i != last; ++i) {
    //     fprintf(stderr, "%d ", *i);
    // }
    // fprintf(stderr, "\n");
    quick_sort(L, last);
}

#ifdef DO_TEST

uint64_t get_ms() {
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);
    return (uint64_t)spec.tv_sec * 1000 + spec.tv_nsec / 1.0e6;
}

void test() {
    {
        int a[] = {3, 2, 1};
        slow_sort(a, a + sizeof(a) / sizeof(int));
        assert(a[0] == 1);
        assert(a[1] == 2);
        assert(a[2] == 3);
    }
    fprintf(stderr, "success 1\n"); fflush(stderr);
    {
        int a[] = {3};
        slow_sort(a, a + sizeof(a) / sizeof(int));
        assert(a[0] == 3);
    }
    fprintf(stderr, "success 2\n"); fflush(stderr);
    {
        int a[] = {1};
        slow_sort(a, a);
    }
    fprintf(stderr, "success 3\n"); fflush(stderr);
    for (int counter = 0; counter < 1000; counter++) {
        // fprintf(stderr, "%d =================== \n", counter);
        int n = 1 + rand() % 4;
        int a[n];
        for (int i = 0; i < n; ++i) {
            a[i] = rand() % 10;
        }
        int b[n];
        int c[n];
        memcpy(b, a, n * sizeof(int));
        memcpy(c, a, n * sizeof(int));

        slow_sort(b, b + n);
        quick_sort(c, c + n);

        bool good = true;
        for (int i = 0; i < n; ++i) {
            if (b[i] != c[i]) {
                good = false;
            }
        }
        if (!good) {
            printf("test: ");
            for (int i = 0; i < n; ++i) printf("%d ", a[i]);
            printf("\n");
            printf("expected: ");
            for (int i = 0; i < n; ++i) printf("%d ", b[i]);
            printf("\n");
            printf("actual: ");
            for (int i = 0; i < n; ++i) printf("%d ", c[i]);
            printf("\n");
            break;
        }
    }
    fprintf(stderr, "success 4\n"); fflush(stderr);
    {
        int n = 1000000;
        int* a = calloc(n, sizeof(int));
        for (int i = 0; i < n; ++i) {
            a[i] = rand() % n;
        }
        uint64_t start = get_ms();
        quick_sort(a, a + n);
        printf("Long sort: %" PRIu64 " ms\n", get_ms() - start);
        for (int i = 0; i + 1 < n; ++i) {
            assert(a[i] <= a[i + 1]);
        }
        free(a);
    }

    {
        int n = 1000000;
        int* a = calloc(n, sizeof(int));
        for (int i = 0; i < n; ++i) {
            a[i] = i;
        }
        uint64_t start = get_ms();
        quick_sort(a, a + n);
        printf("Already sorted long sort: %" PRIu64 " ms\n", get_ms() - start);
        for (int i = 0; i + 1 < n; ++i) {
            assert(a[i] <= a[i + 1]);
        }
        free(a);
    }

    {
        int n = 1000000;
        int* a = calloc(n, sizeof(int));
        for (int i = 0; i < n; ++i) {
            a[i] = 0;
        }
        uint64_t start = get_ms();
        quick_sort(a, a + n);
        printf("Zero long sort: %" PRIu64 " ms\n", get_ms() - start);
        for (int i = 0; i + 1 < n; ++i) {
            assert(a[i] <= a[i + 1]);
        }
        free(a);
    }

    {
        int n = 1000000;
        int* a = calloc(n, sizeof(int));
        for (int i = 0; i < n; ++i) {
            a[i] = n - i;
        }
        uint64_t start = get_ms();
        quick_sort(a, a + n);
        printf("Descending sorted long sort: %" PRIu64 " ms\n", get_ms() - start);
        for (int i = 0; i + 1 < n; ++i) {
            assert(a[i] <= a[i + 1]);
        }
        free(a);
    }
}
#endif

int main() {
#ifdef DO_TEST
    test();
    return 0;
#endif

    int n;
    scanf("%d", &n);
    int* a = calloc(n, sizeof(int));
    for (int i = 0; i < n; ++i) {
        scanf("%d", &a[i]);
    }
    quick_sort(a, a + n);

    for (int i = 0; i < n; ++i) {
        printf("%d ", a[i]);
    }
    free(a);
    return 0;
}

