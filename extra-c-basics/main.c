// %%cpp main.c
// %run gcc -fsanitize=address -DLOGP -std=c99 -Wall -Werror main.c -o a.exe
// %run ./a.exe

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>


#if defined(LOGP)
    #define eprintf(...) fprintf(stderr, __VA_ARGS__)
#else
    #define eprintf(...) (void)42
#endif

#define logprintf_impl(fmt, line, ...) eprintf(__FILE__ ":" #line " " fmt, __VA_ARGS__)
#define logprintf_impl_2(line, fmt, ...) logprintf_impl(fmt "%s", line, __VA_ARGS__)
#define logprintf(...) logprintf_impl_2(__LINE__, __VA_ARGS__, "")


#define max(a, b) ((a) > (b) ? (a) : (b))

typedef int elem_t;
typedef elem_t (*aggregator_elem_t)(elem_t, elem_t);

typedef struct {
    elem_t* a;
    uint64_t lower_begin;
    aggregator_elem_t aggregator;
    elem_t neutral_element;
} segtree_t;

void segtree_init(segtree_t* t, int n, aggregator_elem_t aggr, elem_t nelem) {
    t->lower_begin = 1;
    while (t->lower_begin < n) {
        t->lower_begin *= 2;
    }
    t->a = calloc(t->lower_begin * 2, sizeof(elem_t));
    t->aggregator = aggr;
    t->neutral_element = nelem;
}

void segtree_destroy(segtree_t* t) {
    free(t->a);
}

void segtree_modify(segtree_t* t, uint64_t i, elem_t x) {
    i += t->lower_begin;
    t->a[i] = x;
    i /= 2;
    while (i > 0) {
        t->a[i] = t->aggregator(t->a[i * 2], t->a[i * 2 + 1]);
        i /= 2;
    }
}

elem_t segtree_get_impl(segtree_t* t, uint64_t i, uint64_t j, uint64_t v, uint64_t l, uint64_t r) {
    if (i <= l && r <= j) {
        return t->a[v];
    }
    if (r < i || j < l) {
        return t->neutral_element;
    }
    elem_t l_ans = segtree_get_impl(t, i, j, v * 2, l, (l + r) / 2);
    elem_t r_ans = segtree_get_impl(t, i, j, v * 2 + 1, (l + r) / 2 + 1, r);
    return t->aggregator(l_ans, r_ans);
}


elem_t segtree_get(segtree_t* t, uint64_t i, uint64_t j) {
    return segtree_get_impl(t, t->lower_begin + i, t->lower_begin + j, 
                            1, t->lower_begin, t->lower_begin * 2 - 1);
}

elem_t elem_max(elem_t a, elem_t b) {
    return (a > b) ? a : b;
}


elem_t elem_sum(elem_t a, elem_t b) {
    return a + b;
}

int main() {
    {
        segtree_t t;
        segtree_init(&t, 8, &elem_max, -1);
        segtree_modify(&t, 0, 0);
        segtree_modify(&t, 1, 2);
        segtree_modify(&t, 2, 1);
        segtree_modify(&t, 3, 4);
        segtree_modify(&t, 4, 3);
        segtree_modify(&t, 5, 7);
        segtree_modify(&t, 6, 5);
        segtree_modify(&t, 7, 4);
        assert(segtree_get(&t, 1, 5) == 7);
        assert(segtree_get(&t, 0, 2) == 2);
        assert(segtree_get(&t, 5, 7) == 7);
        assert(segtree_get(&t, 1, 3) == 4);
        assert(segtree_get(&t, 0, 7) == 7);
        assert(segtree_get(&t, 0, 0) == 0);

        segtree_destroy(&t);
    }
    
    {
        segtree_t t;
        segtree_init(&t, 8, &elem_sum, 0);
        segtree_modify(&t, 0, 1);
        segtree_modify(&t, 1, 2);
        segtree_modify(&t, 2, 3);
        segtree_modify(&t, 3, 4);
        assert(segtree_get(&t, 0, 3) == 10);
        assert(segtree_get(&t, 0, 0) == 1);
        assert(segtree_get(&t, 1, 2) == 5);
        assert(segtree_get(&t, 3, 3) == 4);

        segtree_destroy(&t);
    }

    return 0;
}

