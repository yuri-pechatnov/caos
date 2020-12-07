


```python
!rm ./a.exe
```


```cpp
%%cpp main.c
%run gcc -fsanitize=address -DLOGP -std=c99 -Wall -Werror main.c -o a.exe
%run ./a.exe

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


#if defined(LOGP)
    #define eprintf(...) fprintf(stderr, __VA_ARGS__)
#else
    #define eprintf(...) (void)42
#endif

#define logprintf_impl(fmt, line, ...) eprintf(__FILE__ ":" #line " " fmt, __VA_ARGS__)
#define logprintf_impl_2(line, fmt, ...) logprintf_impl(fmt "%s", line, __VA_ARGS__)
#define logprintf(...) logprintf_impl_2(__LINE__, __VA_ARGS__, "")


typedef int elem_t;

typedef struct {
    elem_t value;
    size_t prev;
    size_t next;
} node_t;

typedef struct {
    node_t* nodes;
    size_t max_index;
    size_t capacity;
    size_t free_top;
} arena_t;


void init_arena(arena_t* a) {
    a->nodes = NULL;
    a->max_index = 0;
    a->capacity = 0;
    a->free_top = -1;
}

void clear_arena(arena_t* a) {
    free(a->nodes);
    a->nodes = NULL;
    init_arena(a);
}

void reserve_arena(arena_t* a, size_t desired_size) {
    if (desired_size > a->capacity)
        a->capacity = desired_size;
    logprintf("resize to %d\n", (int)(a->capacity * sizeof(node_t)));
    a->nodes = realloc(a->nodes, a->capacity * sizeof(node_t));
}


size_t new_node_arena(arena_t* a) {
    size_t i;
    if (a->free_top != -1) {
        i = a->free_top;
        a->free_top = a->nodes[a->free_top].prev;
    } else {
        if (a->capacity == a->max_index) {
            reserve_arena(a, (a->capacity + 1) * 2);
        }
        i = a->max_index++;
    }
    return i;
}


void delete_node_arena(arena_t* a, size_t i) {
    a->nodes[i].prev = a->free_top;
    a->free_top = i;
}

typedef struct {
    arena_t* arena;
    size_t first;
    size_t last;
} list_t;

void init_list(list_t* d, arena_t* a) {
    d->arena = a;
    d->first = -1;
    d->last = -1;
}

elem_t pop_front_list(list_t* d);

void clear_list(list_t* d) {
    while (d->first != -1) {
        pop_front_list(d);
    }
}


void push_front_list(list_t* d, elem_t e) {  
    size_t i = new_node_arena(d->arena);
    d->arena->nodes[i].prev = -1;
    d->arena->nodes[i].next = d->first;
    d->arena->nodes[i].value = e;
    d->first = i;
    if (d->last == -1) {
        d->last = i;
    }
}

elem_t pop_front_list(list_t* d) {
    size_t i = d->first;
    if (i == d->last) {
        d->first = -1;
        d->last = -1;
    } else {
        d->first = d->arena->nodes[i].next;
        d->arena->nodes[d->first].prev = -1;
    }
    
    elem_t value = d->arena->nodes[i].value;
    delete_node_arena(d->arena, i);
    
    return value;
}

void print_list(list_t* d) {
    printf("[");
    for (int i = d->first; i != -1; i = d->arena->nodes[i].next) {
        printf("%d", d->arena->nodes[i].value);
        if (d->arena->nodes[i].next != -1) {
            printf(", ");
        }
    }
    printf("]\n");
}


int main() {
    arena_t a;
    
    init_arena(&a);
    reserve_arena(&a, 100);
    logprintf("x\n");
    const int N = 10;
    list_t d[N];
    for (int k = 0; k < N; ++k) {
        init_list(&d[k], &a);
        push_front_list(&d[k], 1);
        push_front_list(&d[k], 2); 
        push_front_list(&d[k], 3);
        for (int i = 0; i < 100; ++i) {
            push_front_list(&d[k], 42);
            if (i == 40) {
                for (int j = 0; j < 100; ++j) {
                    push_front_list(&d[k], 42);
                    assert(42 == pop_front_list(&d[k]));
                }
            }
            assert(42 == pop_front_list(&d[k]));
        }
        push_front_list(&d[k], 4);
    }
    
    for (int k = 0; k < N; ++k) {
        print_list(&d[k]); 
        clear_list(&d[k]);
    }

    clear_arena(&a);
    return 0;
}
```


```python

```


```cpp
%%cpp
typedef struct {
    int fd1;
    int fd2;
    void* ptr1;
    void* ptr2;
} state_t;

int work_not_finalized(state_t* s, args_t args) {
    ...
    s->fd1 = open(...);
    ...
        return -1;
    ...
            return -3;
    ...
    return 0;
}

int work(args_t args) {
    state_t state = {.fd1 = -1, .fd2 = -1, .ptr1 = NULL, .ptr2 = NULL};
    int ret_code = work_not_finalized(&state, args);
    close(state.fd1);
    close(state.fd2);
    free(state.ptr1);
    free(state.ptr2);
    return ret_code;
}



```


```python

```

# Универсальное дерево отрезков


```cpp
%%cpp main.c
%run gcc -fsanitize=address -DLOGP -std=c99 -Wall -Werror main.c -o a.exe
%run ./a.exe

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
```


```python

```


```python

```


```python

```


```python

```


```python

```


```python

```

# Дерево отрезков с присваиванием на отрезке


```cpp
%%cpp main.c
%run gcc -fsanitize=address -DLOGP -std=c99 -Wall -Werror main.c -o a.exe
%run ./a.exe

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>


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

typedef struct {
    elem_t value;
    bool do_propagate;
} segtree_node_t;

typedef struct {
    segtree_node_t* a;
    uint64_t lower_begin;
} segtree_t;

void segtree_init(segtree_t* t, int n) {
    t->lower_begin = 1;
    while (t->lower_begin < n) {
        t->lower_begin *= 2;
    }
    t->a = calloc(t->lower_begin * 2, sizeof(segtree_node_t));
}

void segtree_destroy(segtree_t* t) {
    free(t->a);
}

void segtree_push_down(segtree_t* t, uint64_t v) {
    if (t->a[v].do_propagate && v < t->lower_begin) {
        t->a[v * 2].value = t->a[v].value;
        t->a[v * 2 + 1].value = t->a[v].value;
        t->a[v * 2].do_propagate = true;
        t->a[v * 2 + 1].do_propagate = true;
    }
    t->a[v].do_propagate = false;
}

void segtree_modify_impl(segtree_t* t, uint64_t i, uint64_t j, elem_t x, uint64_t v, uint64_t l, uint64_t r) {
    if (r < i || j < l) {
        return;
    }
    if (i <= l && r <= j) {
        t->a[v].value = x;
        t->a[v].do_propagate = true;
        return;
    }

    segtree_push_down(t, v);
    segtree_modify_impl(t, i, j, x, v * 2, l, (l + r) / 2);
    segtree_modify_impl(t, i, j, x, v * 2 + 1, (l + r) / 2 + 1, r);
    t->a[v].value = max(t->a[v * 2].value, t->a[v * 2 + 1].value);
}

void segtree_modify_segment(segtree_t* t, uint64_t i, uint64_t j, elem_t x) {
    segtree_modify_impl(t, t->lower_begin + i, t->lower_begin + j, x,
                        1, t->lower_begin, t->lower_begin * 2 - 1);
    
}

void segtree_modify(segtree_t* t, uint64_t i, elem_t x) {
    segtree_modify_segment(t, i, i, x);
}

elem_t segtree_get_max_impl(segtree_t* t, uint64_t i, uint64_t j, uint64_t v, uint64_t l, uint64_t r) {
    if (r < i || j < l) {
        return -1;
    }
    segtree_push_down(t, v);
    if (i <= l && r <= j) {
        return t->a[v].value;
    }
    elem_t l_ans = segtree_get_max_impl(t, i, j, v * 2, l, (l + r) / 2);
    elem_t r_ans = segtree_get_max_impl(t, i, j, v * 2 + 1, (l + r) / 2 + 1, r);
    return max(l_ans, r_ans);
}


elem_t segtree_get_max(segtree_t* t, uint64_t i, uint64_t j) {
    return segtree_get_max_impl(t, t->lower_begin + i, t->lower_begin + j, 
                                1, t->lower_begin, t->lower_begin * 2 - 1);
}


int main() {
    segtree_t t;
    segtree_init(&t, 8);
    segtree_modify(&t, 0, 0);
    segtree_modify(&t, 1, 2);
    segtree_modify(&t, 2, 1);
    segtree_modify(&t, 3, 4);
    segtree_modify(&t, 4, 3);
    segtree_modify(&t, 5, 7);
    segtree_modify(&t, 6, 5);
    segtree_modify(&t, 7, 4);
    logprintf("segtree_get_max(&t, 1, 5) = %d\n", segtree_get_max(&t, 1, 5));
    assert(segtree_get_max(&t, 1, 5) == 7);
    assert(segtree_get_max(&t, 0, 2) == 2);
    assert(segtree_get_max(&t, 5, 7) == 7);
    assert(segtree_get_max(&t, 1, 3) == 4);
    assert(segtree_get_max(&t, 0, 7) == 7);
    assert(segtree_get_max(&t, 0, 0) == 0);
    
    segtree_modify_segment(&t, 1, 5, 8);
    assert(segtree_get_max(&t, 1, 5) == 8);
    assert(segtree_get_max(&t, 0, 2) == 8);
    assert(segtree_get_max(&t, 5, 7) == 8);
    assert(segtree_get_max(&t, 1, 3) == 8);
    assert(segtree_get_max(&t, 0, 7) == 8);
    assert(segtree_get_max(&t, 0, 0) == 0);
    assert(segtree_get_max(&t, 6, 6) == 5);
    assert(segtree_get_max(&t, 7, 7) == 4);
    
    
    segtree_destroy(&t);
    return 0;
}
```


```python
!man calloc

```


```python

```


```python

```


```python

```
