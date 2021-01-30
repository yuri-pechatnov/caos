



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

    usage: ipykernel_launcher.py [-h] [--ejudge-style] fname
    ipykernel_launcher.py: error: the following arguments are required: fname



    An exception has occurred, use %tb to see the full traceback.


    SystemExit: 2




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


Run: `gcc -fsanitize=address -DLOGP -std=c99 -Wall -Werror main.c -o a.exe`



Run: `./a.exe`



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


Run: `gcc -fsanitize=address -DLOGP -std=c99 -Wall -Werror main.c -o a.exe`


    [01m[Kmain.c:[m[K In function ‘[01m[Kmain[m[K’:
    [01m[Kmain.c:107:24:[m[K [01;31m[Kerror: [m[Kexpected expression before ‘[01m[K)[m[K’ token
      107 |     segtree_init(&t, 8,[01;31m[K)[m[K;
          |                        [01;31m[K^[m[K
    [01m[Kmain.c:107:5:[m[K [01;31m[Kerror: [m[Ktoo many arguments to function ‘[01m[Ksegtree_init[m[K’
      107 |     [01;31m[Ksegtree_init[m[K(&t, 8,);
          |     [01;31m[K^~~~~~~~~~~~[m[K
    [01m[Kmain.c:37:6:[m[K [01;36m[Knote: [m[Kdeclared here
       37 | void [01;36m[Ksegtree_init[m[K(segtree_t* t, int n) {
          |      [01;36m[K^~~~~~~~~~~~[m[K



Run: `./a.exe`


    main.c:116 segtree_get_max(&t, 1, 5) = 7



```python
!man calloc

```

    MALLOC(3)                  Linux Programmer's Manual                 MALLOC(3)
    
    NNAAMMEE
           malloc, free, calloc, realloc - allocate and free dynamic memory
    
    SSYYNNOOPPSSIISS
           ##iinncclluuddee <<ssttddlliibb..hh>>
    
           vvooiidd **mmaalllloocc((ssiizzee__tt _s_i_z_e));;
           vvooiidd ffrreeee((vvooiidd _*_p_t_r));;
           vvooiidd **ccaalllloocc((ssiizzee__tt _n_m_e_m_b,, ssiizzee__tt _s_i_z_e));;
           vvooiidd **rreeaalllloocc((vvooiidd _*_p_t_r,, ssiizzee__tt _s_i_z_e));;
           vvooiidd **rreeaallllooccaarrrraayy((vvooiidd _*_p_t_r,, ssiizzee__tt _n_m_e_m_b,, ssiizzee__tt _s_i_z_e));;
    
       Feature Test Macro Requirements for glibc (see ffeeaattuurree__tteesstt__mmaaccrrooss(7)):
    
           rreeaallllooccaarrrraayy():
               Since glibc 2.29:
                   _DEFAULT_SOURCE
               Glibc 2.28 and earlier:
                   _GNU_SOURCE
    
    DDEESSCCRRIIPPTTIIOONN
           The mmaalllloocc() function allocates _s_i_z_e bytes and returns a pointer to the
           allocated memory.  _T_h_e _m_e_m_o_r_y _i_s _n_o_t _i_n_i_t_i_a_l_i_z_e_d.  If _s_i_z_e is  0,  then
           mmaalllloocc()  returns either NULL, or a unique pointer value that can later
           be successfully passed to ffrreeee().
    
           The ffrreeee() function frees the memory space pointed  to  by  _p_t_r,  which
           must  have  been  returned by a previous call to mmaalllloocc(), ccaalllloocc(), or
           rreeaalllloocc().  Otherwise, or if _f_r_e_e_(_p_t_r_) has already been called  before,
           undefined behavior occurs.  If _p_t_r is NULL, no operation is performed.
    
           The  ccaalllloocc()  function allocates memory for an array of _n_m_e_m_b elements
           of _s_i_z_e bytes each and returns a pointer to the allocated memory.   The
           memory  is  set  to zero.  If _n_m_e_m_b or _s_i_z_e is 0, then ccaalllloocc() returns
           either NULL, or a unique pointer value that can later  be  successfully
           passed to ffrreeee().  If the multiplication of _n_m_e_m_b and _s_i_z_e would result
           in integer overflow, then ccaalllloocc() returns an error.  By  contrast,  an
           integer  overflow  would  not be detected in the following call to mmaall‐‐
           lloocc(), with the result that an incorrectly sized block of memory  would
           be allocated:
    
               malloc(nmemb * size);
    
           The  rreeaalllloocc() function changes the size of the memory block pointed to
           by _p_t_r to _s_i_z_e bytes.  The contents will be unchanged in the range from
           the start of the region up to the minimum of the old and new sizes.  If
           the new size is larger than the old size, the added memory will _n_o_t  be
           initialized.   If  _p_t_r  is  NULL,  then  the call is equivalent to _m_a_l_‐
           _l_o_c_(_s_i_z_e_), for all values of _s_i_z_e; if _s_i_z_e is equal to zero, and _p_t_r is
           not  NULL,  then  the  call  is equivalent to _f_r_e_e_(_p_t_r_).  Unless _p_t_r is
           NULL, it must have been returned by an earlier call to  mmaalllloocc(),  ccaall‐‐
           lloocc(),  or rreeaalllloocc().  If the area pointed to was moved, a _f_r_e_e_(_p_t_r_) is
           done.
    
           The rreeaallllooccaarrrraayy() function  changes  the  size  of  the  memory  block
           pointed  to  by  _p_t_r to be large enough for an array of _n_m_e_m_b elements,
           each of which is _s_i_z_e bytes.  It is equivalent to the call
    
                   realloc(ptr, nmemb * size);
    
           However, unlike that rreeaalllloocc() call, rreeaallllooccaarrrraayy() fails safely in the
           case  where the multiplication would overflow.  If such an overflow oc‐
           curs, rreeaallllooccaarrrraayy() returns NULL, sets _e_r_r_n_o to EENNOOMMEEMM, and leaves the
           original block of memory unchanged.
    
    RREETTUURRNN VVAALLUUEE
           The  mmaalllloocc()  and ccaalllloocc() functions return a pointer to the allocated
           memory, which is suitably aligned for any  built-in  type.   On  error,
           these functions return NULL.  NULL may also be returned by a successful
           call to mmaalllloocc() with a _s_i_z_e of zero, or by a successful call  to  ccaall‐‐
           lloocc() with _n_m_e_m_b or _s_i_z_e equal to zero.
    
           The ffrreeee() function returns no value.
    
           The rreeaalllloocc() function returns a pointer to the newly allocated memory,
           which is suitably aligned for any built-in type, or NULL if the request
           failed.   The returned pointer may be the same as _p_t_r if the allocation
           was not moved (e.g., there was room to expand the allocation in-place),
           or different from _p_t_r if the allocation was moved to a new address.  If
           _s_i_z_e was equal to 0, either NULL or a pointer suitable to be passed  to
           ffrreeee() is returned.  If rreeaalllloocc() fails, the original block is left un‐
           touched; it is not freed or moved.
    
           On success, the rreeaallllooccaarrrraayy() function returns a pointer to the  newly
           allocated  memory.   On failure, it returns NULL and the original block
           of memory is left untouched.
    
    EERRRROORRSS
           ccaalllloocc(), mmaalllloocc(), rreeaalllloocc(), and rreeaallllooccaarrrraayy()  can  fail  with  the
           following error:
    
           EENNOOMMEEMM Out  of  memory.  Possibly, the application hit the RRLLIIMMIITT__AASS or
                  RRLLIIMMIITT__DDAATTAA limit described in ggeettrrlliimmiitt(2).
    
    AATTTTRRIIBBUUTTEESS
           For an  explanation  of  the  terms  used  in  this  section,  see  aatt‐‐
           ttrriibbuutteess(7).
    
           ┌─────────────────────┬───────────────┬─────────┐
           │IInntteerrffaaccee            │ AAttttrriibbuuttee     │ VVaalluuee   │
           ├─────────────────────┼───────────────┼─────────┤
           │mmaalllloocc(), ffrreeee(),    │ Thread safety │ MT-Safe │
           │ccaalllloocc(), rreeaalllloocc()  │               │         │
           └─────────────────────┴───────────────┴─────────┘
    CCOONNFFOORRMMIINNGG TTOO
           mmaalllloocc(), ffrreeee(), ccaalllloocc(), rreeaalllloocc(): POSIX.1-2001, POSIX.1-2008, C89,
           C99.
    
           rreeaallllooccaarrrraayy() is a nonstandard extension that first appeared in  Open‐
           BSD 5.6 and FreeBSD 11.0.
    
    NNOOTTEESS
           By  default,  Linux  follows  an optimistic memory allocation strategy.
           This means that when mmaalllloocc() returns non-NULL there  is  no  guarantee
           that  the  memory  really  is available.  In case it turns out that the
           system is out of memory, one or more processes will be  killed  by  the
           OOM   killer.    For   more   information,   see   the  description  of
           _/_p_r_o_c_/_s_y_s_/_v_m_/_o_v_e_r_c_o_m_m_i_t___m_e_m_o_r_y and _/_p_r_o_c_/_s_y_s_/_v_m_/_o_o_m___a_d_j in pprroocc(5), and
           the   Linux  kernel  source  file  _D_o_c_u_m_e_n_t_a_t_i_o_n_/_v_m_/_o_v_e_r_c_o_m_m_i_t_-_a_c_c_o_u_n_t_‐
           _i_n_g_._r_s_t.
    
           Normally, mmaalllloocc() allocates memory from the heap, and adjusts the size
           of the heap as required, using ssbbrrkk(2).  When allocating blocks of mem‐
           ory larger than MMMMAAPP__TTHHRREESSHHOOLLDD bytes, the glibc mmaalllloocc() implementation
           allocates  the  memory  as  a  private anonymous mapping using mmmmaapp(2).
           MMMMAAPP__TTHHRREESSHHOOLLDD is 128 kB by  default,  but  is  adjustable  using  mmaall‐‐
           lloopptt(3).   Prior  to Linux 4.7 allocations performed using mmmmaapp(2) were
           unaffected by the RRLLIIMMIITT__DDAATTAA resource limit;  since  Linux  4.7,  this
           limit is also enforced for allocations performed using mmmmaapp(2).
    
           To avoid corruption in multithreaded applications, mutexes are used in‐
           ternally to protect the memory-management data structures  employed  by
           these  functions.   In a multithreaded application in which threads si‐
           multaneously allocate and free memory, there could  be  contention  for
           these  mutexes.   To scalably handle memory allocation in multithreaded
           applications, glibc creates additional _m_e_m_o_r_y _a_l_l_o_c_a_t_i_o_n _a_r_e_n_a_s if  mu‐
           tex  contention  is  detected.   Each arena is a large region of memory
           that is internally allocated by the system (using bbrrkk(2)  or  mmmmaapp(2)),
           and managed with its own mutexes.
    
           SUSv2 requires mmaalllloocc(), ccaalllloocc(), and rreeaalllloocc() to set _e_r_r_n_o to EENNOOMMEEMM
           upon failure.  Glibc assumes that this is done (and the glibc  versions
           of  these routines do this); if you use a private malloc implementation
           that does not set _e_r_r_n_o, then certain library routines may fail without
           having a reason in _e_r_r_n_o.
    
           Crashes  in  mmaalllloocc(), ccaalllloocc(), rreeaalllloocc(), or ffrreeee() are almost always
           related to heap corruption, such as overflowing an allocated  chunk  or
           freeing the same pointer twice.
    
           The  mmaalllloocc()  implementation is tunable via environment variables; see
           mmaalllloopptt(3) for details.
    
    SSEEEE AALLSSOO
           vvaallggrriinndd(1), bbrrkk(2), mmmmaapp(2), aallllooccaa(3), mmaalllloocc__ggeett__ssttaattee(3),
           mmaalllloocc__iinnffoo(3), mmaalllloocc__ttrriimm(3), mmaalllloocc__uussaabbllee__ssiizzee(3), mmaalllloopptt(3),
           mmcchheecckk(3), mmttrraaccee(3), ppoossiixx__mmeemmaalliiggnn(3)
    
           For details of the GNU C library implementation, see
           ⟨https://sourceware.org/glibc/wiki/MallocInternals⟩.
    
    CCOOLLOOPPHHOONN
           This page is part of release 5.05 of the Linux _m_a_n_-_p_a_g_e_s project.  A
           description of the project, information about reporting bugs, and the
           latest version of this page, can be found at
           https://www.kernel.org/doc/man-pages/.
    
    GNU                               2020-02-09                         MALLOC(3)



```python

```


```python

```


```python

```
