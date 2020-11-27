



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


Run: `gcc -fsanitize=address -DLOGP -std=c99 -Wall -Werror main.c -o a.exe`



Run: `./a.exe`


    main.c:53 resize to 2400
    main.c:143 x
    [4, 3, 2, 1]
    [4, 3, 2, 1]
    [4, 3, 2, 1]
    [4, 3, 2, 1]
    [4, 3, 2, 1]
    [4, 3, 2, 1]
    [4, 3, 2, 1]
    [4, 3, 2, 1]
    [4, 3, 2, 1]
    [4, 3, 2, 1]



```python

```


```python

```


```python

```
