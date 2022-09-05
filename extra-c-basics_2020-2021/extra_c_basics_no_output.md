

# Особенности Си

Сегодня в программе:
* <a href="#primitives" style="color:#856024"> Примитивные типы и их вывод </a>
* <a href="#structures" style="color:#856024"> Структуры </a>
* <a href="#arrays" style="color:#856024"> Массивы </a>
* <a href="#dyn_arrays" style="color:#856024"> Динамические массивы </a>

[Пиши на C как джентльмен / Хабр](https://habr.com/ru/post/325678/)

[Здесь](/sem02-instruments-compilation) в конце (дополнение про макросы) есть больше примеров про макросы, если интересно

## <a name="primitives"></a> Примитивные типы и их вывод


```cpp
%%cpp main.c
%run gcc -std=c99 -Wall -Werror main.c -o a.exe
%run ./a.exe

#include <stdio.h>

int main() {
    int i = 5;
    float f = 5.5555555555555555555555555;
    double d = 5.5555555555555555555555555;
    char c = 'X';
    char* str = "Hello world!";
    printf("i = %d\n", i);
    printf("f = %f, f = %0.20f\n", f, f); // обратите внимание на точность
    printf("d = %lf, d = %0.20lf\n", d, d);
    printf("c = %c, int(c) = %d\n", c, (int)c); // приведение типов
    printf("s = %s\n", str); // zero-terminated string
    printf("s = %.*s\n", 4, str); // string by begin and length
    printf("*str = %c, *(str + 1) = %c, str[1] = %c\n", *str, *(str + 1), str[1]);
    return 0;
}
```


```cpp
%%cpp main.c
%run gcc  -std=c99 -Wall -Werror main.c -o a.exe
%run ./a.exe

#include <stdio.h>
#include <inttypes.h>


typedef int64_t i64;

int main() {
    int i = 5;
    int32_t i_32 = 6; // -2^31 до (2^31) - 1
    i64 i_64 = 7; // -2^63 до (2^63) - 1
    // uint64_t, int8_t, ...
    
    printf("i = %" "d" ", sizeof(i) = %d\n", i, (int)sizeof(i));
    //               [1]                                 [2]                        
    printf("i_32 = %" PRId32 ", sizeof(i_32) = %d\n", i_32, (int)sizeof(i_32));
    printf("i_64 = %" PRId64 ", sizeof(i_64) = %d\n", i_64, (int)sizeof(i_64));
    // [1] - правильный спецификатор
    // [2] - каст к типу спецификатора
    return 0;
}
```

## <a name="structures"></a> Структуры


```cpp
%%cpp main.c
%run gcc -m64 -std=c99 -Wall -Werror main.c -o a.exe
%run ./a.exe

#include <stdio.h>

struct point {
    double x, y;
};


struct point point_sum(struct point a, struct point b) {
    struct point c;
    c.x = a.x + b.x;
    c.y = a.y + b.y;
    return c;
} 


typedef struct point point_t;

point_t point_sub(point_t a, point_t b) {
    a.x -= b.x;
    a.y -= b.y;
    return a;
} 

point_t point_mul(point_t a, double k) {
    return (point_t){.x = a.x * k, .y = a.y * k};
}

void point_print(point_t a) {
    printf("{.x = %lf, .y = %lf}", a.x, a.y);
}

int main() {
    point_t a = {1, 100}, b = {3, 50};
    point_print(a); printf("\n");
    
    point_print(point_sum(a, b)); printf("\n");
    point_print(point_sub(a, b)); printf("\n");
    point_print(point_mul(a, 10)); printf("\n");
    
    return 0;
}
```

## <a name="arrays"></a> Массивы


```cpp
%%cpp main.c
%run gcc -m64 -std=c99 -Wall -Werror main.c -o a.exe
%run ./a.exe

#include <stdio.h>

#define CONST 2
#define SIZE(a) (sizeof(a) / sizeof((a)[0]))

int main() {
    char* s = "Hello wotrld!";
    printf("%s\n", s);
    char s1[] = "1";
    printf("%s\n", s1);
    char s2[100] = "Hello world! 2";
    printf("%s\n", s2);
    printf("%d %d %d\n", (int)sizeof(s), (int)sizeof(s1), (int)sizeof(s2));
        
    int a[] = {1, 5, 3, 2, 5};
    printf("sizeof(a) = %d\n", (int)sizeof(a));
    for (int i = 0; i < sizeof(a) / sizeof(int); ++i) {
        printf("a[%d] = %d, ", i, a[i]);
    }
    printf("\n");
    for (int i = 0; i < SIZE(a); ++i) {
        printf("a[%d] = %d, ", i, a[i]);
    }
    printf("\n");
        
    return 0;
}
```

## <a name="dyn_arrays"></a> Динамические массивы


```python
with open("001.in", "w") as f:
    f.write('''\
4
5 1 9 0
    ''')
```


```python
!man qsort | head -n 10
```


```cpp
%%cpp main.c
%run gcc -m64 -std=c99 -Wall -Werror main.c -o a.exe
%run ./a.exe < 001.in

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>

//        void qsort(void *base, size_t nmemb, size_t size,
//                   int (*compar)(const void *, const void *));

int int_compare(const int* x, const int* y) {
    return *x - *y;
}

int main() {
    int n = 0;
    scanf("%d", &n);
    int* a = calloc(n, sizeof(int));
    for (int i = 0; i < n; ++i) {
        scanf("%d", &a[i]);
    }
    qsort(a, n, sizeof(int), (int(*)(const void *, const void *))int_compare);
    for (int i = 0; i < n; ++i) {
        printf("a[%d] = %d, ", i, a[i]);
    }
    printf("\n");
    free(a);
    return 0;
}
```


```cpp
%%cpp main.c
%run gcc -DDEBUG -m64 -std=c99 -Wall -Werror main.c -o a.exe
%run ./a.exe

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>

#define SWAP(a, b) { __typeof__(a) c = (a); (a) = (b); (b) = (c); }


void my_sort(int* a, int length) {
    // my_sort_2(a, a + length);
    for (int i = 0; i < length - 1; ++i) {
        for (int j = i + 1; j < length; ++j) {
            if (a[i] > a[j]) {
                SWAP(a[i], a[j]);
            }
        }
    }
}

void my_sort_2(int* begin, int* end) {
    // my_sort(begin, end - begin);
    for (int* i = begin; i != end; ++i) {
        for (int* j = i + 1; j != end; j++) {
            if (*i > *j) {
                SWAP(*i, *j);
            }
        }
    }
}

int main() {
    #if defined(DEBUG)
    freopen("001.in", "rt", stdin);
    #endif
    int n = 0;
    scanf("%d", &n);
    int* a = calloc(n, sizeof(int));
    for (int i = 0; i < n; ++i) {
        scanf("%d", &a[i]);
    }
    my_sort_2(a, a + n);
    for (int i = 0; i < n; ++i) {
        printf("a[%d] = %d, ", i, a[i]);
    }
    printf("\n");
    free(a);
    return 0;
}
```


```python

```

# Строки


```python
with open("001.in", "w") as f:
    f.write('''\
push 3
pop
exit
hello 6
    ''')
```


```cpp
%%cpp main.c
%run clang -std=c99 -Wall -Werror -fsanitize=address main.c -o a.exe
%run ./a.exe < 001.in

#include <stdio.h>
#include <string.h>

int main() {
    char str[100];
    while (1) {
        int ret = scanf("%s", str);
        if (ret <= 0) {
            break;
        }
        
        printf("READ STRING: %s\n", str);
        if (strcmp(str, "push") == 0) {
            int value;
            scanf("%d", &value);
            printf("   IT IS PUSH %d\n", value);
        } else if (strcmp(str, "hello") == 0) {
            int value;
            scanf("%d", &value);
            printf("   IT IS HELLO %d\n", value);
        }    
    }
    return 0;
}
```


```cpp
%%cpp main.c
%run gcc -std=c99 -Wall -Werror -fsanitize=address main.c -o a.exe
%run ./a.exe < 001.in

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h> // true, false

int main() {
    char buff[100];
 
    while (true) {
        char* line = fgets(buff, sizeof(buff), stdin);
        if (!line) {
            break;
        }
        
        printf("READ LINE: '%s'\n", line);
        
        char str[100];
        int value = 0;
        int ret = sscanf(line, "%s %d", str, &value);
        if (ret <= 0) {
            printf("  ERROR\n");
        } else if (ret == 1) {
            printf("  GET ONLY STRING\n");
        }
           
        if (strcmp(str, "push") == 0) {
            assert(ret == 2);
            printf("   IT IS PUSH %d\n", value);
        } else if (strcmp(str, "hello") == 0) {
            assert(ret == 2);
            printf("   IT IS HELLO %d\n", value);
        }    
    }
    return 0;
}
```


```python

```


```cpp
%%cpp main.c
%run clang -std=c99 -Wall -Werror -fsanitize=address main.c -o a.exe
%run ./a.exe < 001.in

#include <stdio.h>
#include <string.h>
#include <assert.h>

int main() {
    char* a = "Hello";
    char* b = "students!";
    printf("%p\n", a);
    printf("%s %s\n", a, b);
    printf("a[1] = '%c'\n", a[1]);
    
    char c1[1000];
    int i = 0;
    for (int j = 0; a[j]; ++j) 
        c1[i++] = a[j];
    for (int j = 0; b[j]; ++j) 
        c1[i++] = b[j];
    c1[i] = '\0';
    printf("c1 = a + b = %s\n", c1);
    
    char c2[1000];
    snprintf(c2, sizeof(c2), "%s %s", a, b);
    printf("c2 = a + ' ' + b = %s\n", c2);
    
    return 0;
}
```


```cpp
%%cpp main.c
%run clang -std=c99 -Wall -Werror -fsanitize=address main.c -o a.exe
%run ./a.exe 

#include <stdio.h>
#include <string.h>
#include <assert.h>


void to_upper(char* src, char* dst) {
    int i = 0;
    for (; src[i]; ++i) {
        if ('a' <= src[i] && src[i] <= 'z') {
            dst[i] = src[i] - 'a' + 'A';
        } else {
            dst[i] = src[i];
        }
    }
    dst[i] = '\0';
}

int main() {
    char* a = "Hello";
      
    char buffer[100];
    to_upper(a, buffer);
    
    printf("%s\n", buffer);
    printf("'A' = %d, 'a' = %d\n", (int)'A', (int)'a');
    
    return 0;
}
```


```python

```


```cpp
%%cpp test.h

void test() {
    stack_t stack;
    init_stack(&stack);
    push(&stack, 1);
    push(&stack, 2);
    push(&stack, 3);
    push(&stack, 4);
    
    assert(top(&stack) == 4);
    pop(&stack);
    
    assert(top(&stack) == 3);
    pop(&stack);
    
    push(&stack, 5);
    
    assert(top(&stack) == 5);
    pop(&stack);
    assert(top(&stack) == 2);
    pop(&stack);
    assert(top(&stack) == 1);
    pop(&stack);
    
    destroy_stack(&stack);
    
    printf("SUCCESS\n");
}

int main() {
    test();
    return 0;
}
```

## Стек олимпиадника


```cpp
%%cpp main.c
%run clang -std=c99 -Wall -Werror -fsanitize=address main.c -o a.exe
%run ./a.exe 

#include <stdio.h>
#include <string.h>
#include <assert.h>

typedef struct stack {
    int a[100500];
    int sz;
} stack_t;


void init_stack(stack_t* stack) {
    stack->sz = 0;
}

void destroy_stack(stack_t* stack) {}

void push(stack_t* stack, int elem) {
    stack->a[stack->sz++] = elem;
}

int top(stack_t* stack) {
    return stack->a[stack->sz - 1];
}

void pop(stack_t* stack) {
    --stack->sz;
    fprintf(stderr, "POP %d (was on position %d)\n", stack->a[stack->sz], stack->sz);
}


#include "test.h"
```

## Стек странного человека


```cpp
%%cpp main.c
%run clang -std=c99 -Wall -Werror -fsanitize=address main.c -o a.exe
%run ./a.exe 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef struct node {
    int elem;
    struct node* previous;
} node_t;



typedef struct stack {
    node_t* top;
} stack_t;


void init_stack(stack_t* stack) {
    stack->top = NULL;
}

void pop(stack_t* stack);

void destroy_stack(stack_t* stack) {
    while (stack->top) {
        pop(stack);
    }
}

void push(stack_t* stack, int elem) {
    node_t* node = calloc(1, sizeof(node_t));
    node->elem = elem;
    node->previous = stack->top;
    stack->top = node;
}

int top(stack_t* stack) {
    return stack->top->elem;
}

void pop(stack_t* stack) {
    node_t* old_top = stack->top;
    stack->top = old_top->previous;
    fprintf(stderr, "POP %d\n", old_top->elem);
    free(old_top);
}


#include "test.h"
```

## Cтек здорового человека


```cpp
%%cpp main.c
%run clang -std=c99 -Wall -Werror -fsanitize=address main.c -o a.exe
%run ./a.exe 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef struct stack {
    int* a;
    int sz;
    int max_sz;
} stack_t;


void init_stack(stack_t* stack) {
    *stack = (stack_t){0};
}

void destroy_stack(stack_t* stack) {
    free(stack->a);
}

void push(stack_t* stack, int elem) {
    if (stack->sz == stack->max_sz) {
        stack->max_sz += (stack->max_sz == 0);
        stack->max_sz *= 2;
        (*stack).a = realloc(stack->a, stack->max_sz * sizeof(int));
    }
    stack->a[stack->sz++] = elem;
}

int top(stack_t* stack) {
    return stack->a[stack->sz - 1];
}

void pop(stack_t* stack) {
    --stack->sz;
    fprintf(stderr, "POP %d (was on position %d)\n", stack->a[stack->sz], stack->sz);
}


#include "test.h"
```

# ASAN

aka address-sanitizer

Опция компилятора `-fsanitize=address`

## 1) Утечки


```cpp
%%cpp main.c
%run clang -std=c99 -Wall -Werror -fsanitize=address main.c -o a.exe
%run ./a.exe 

#include <stdlib.h>
#include <assert.h>

int main() {
    int* array = calloc(10, sizeof(int));
    assert(array[0] == 0);
    return 0;
}
```

## 2) Проезды


```cpp
%%cpp main.c
%run clang -std=c99 -Wall -Werror -fsanitize=address main.c -o a.exe
%run ./a.exe 

#include <stdlib.h>
#include <assert.h>

int main() {
    int* array = calloc(10, sizeof(int));
    assert(array[100500] == 0);
    return 0;
}
```

## 3) Использование памяти после free


```cpp
%%cpp main.c
%run clang -std=c99 -Wall -Werror -fsanitize=address main.c -o a.exe
%run ./a.exe 

#include <stdlib.h>
#include <assert.h>

int main() {
    int* array = calloc(10, sizeof(int));
    free(array);
    assert(array[5] == 0);
    return 0;
}
```

## ... и так далее


```python

```

# MergeSort


```cpp
%%cpp merge.c
%run gcc --sanitize=address merge.c -o merge.exe
%run ./merge.exe

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef int elem_t;

void merge_sort_impl(elem_t* a, int size, elem_t* buff) {
    if (size <= 1) {
        return;
    }
    int lsize = size / 2;
    int rsize = size - lsize;
    
    merge_sort_impl(a, lsize, buff);
    merge_sort_impl(a + lsize, rsize, buff);
    
    int L = 0, R = lsize;
    int k = 0;
    while (L < lsize && R < size) {
        if (a[L] < a[R]) {
            buff[k++] = a[L++];
        } else {
            buff[k++] = a[R++];    
        }
    }
    while (L < lsize) {
        buff[k++] = a[L++];
    }
    while (R < size) {
        buff[k++] = a[R++];
    }
    memcpy(a, buff, size * sizeof(elem_t));
}


void merge_sort(elem_t* a, int size) {
     if (size < 10000 / sizeof(elem_t)) {
        elem_t buff[size];
        merge_sort_impl(a, size, buff);
     } else {
        elem_t* buff = calloc(size, sizeof(elem_t));
        merge_sort_impl(a, size, buff);
        free(buff);
    }
}



int main() {
    //int a[] = {1, 5, 8, 2, 4, 9, 3};
    int a[] = {1, 5, 8, 2, 4, 9, 3};
    int a_size = sizeof(a) / sizeof(a[0]);
    
    merge_sort(a, a_size);
        
    for (int i = 0; i < a_size; ++i) {
        printf("%d, ", a[i]);
    }
    printf("\n");
    return 0;
}
```


```python

```


```python
!man calloc
```


```python

```

# Queue


```cpp
%%cpp test.h

void test() {
    queue_t q;
    init_queue(&q);
    push(&q, 1);
    push(&q, 2);
    push(&q, 3);
    push(&q, 4);
    
    assert(front(&q) == 1);
    pop_front(&q);

    assert(front(&q) == 2);
    pop_front(&q);

    assert(front(&q) == 3);
    pop_front(&q);
    
    push(&q, 5);
    
    assert(front(&q) == 4);
    pop_front(&q);

    push(&q, 6);
    
    assert(front(&q) == 5);
    pop_front(&q);
    
    destroy_queue(&q);
    
    printf("SUCCESS\n");
}

int main() {
    test();
    return 0;
}
```

# Очередь олимпиадника


```cpp
%%cpp main.c
%run clang -std=c99 -Wall -Werror -fsanitize=address main.c -o a.exe
%run ./a.exe 

#include <stdio.h>
#include <string.h>
#include <assert.h>

typedef struct queue {
    int a[100500];
    int first;
    int last; // не включительно
} queue_t;


void init_queue(queue_t* q) {
    q->first = q->last = 0;
}

void destroy_queue(queue_t* q) {}

void push(queue_t* q, int elem) {
    q->a[q->last++] = elem;
}

int front(queue_t* q) {
    return q->a[q->first];
}

void pop_front(queue_t* q) {
    ++q->first;
    fprintf(stderr, "POP %d (was on position %d)\n", q->a[q->first], q->first);
}


#include "test.h"
```

# Очередь человека любящего указатели 


```cpp
%%cpp main.c
%run clang -std=c99 -Wall -Werror -fsanitize=address main.c -o a.exe
%run ./a.exe 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef struct node {
    int elem;
    struct node* next;
} node_t;

typedef struct queue {
    node_t* first;
    node_t* last; // включительно
} queue_t;


void pop_front(queue_t* q);

void init_queue(queue_t* q) {
    q->first = NULL;
}

void destroy_queue(queue_t* q) {
    while (q->first) {
        pop_front(q);
    }
}

void push(queue_t* q, int elem) {
    node_t* new_node = calloc(1, sizeof(node_t));
    new_node->elem = elem;
    
    if (q->first) {
        q->last->next = new_node;
    } else {
        q->first = new_node;
    }
    q->last = new_node;
}

int front(queue_t* q) {
    return q->first->elem;
}

void pop_front(queue_t* q) {
    node_t* x = q->first;
    q->first = q->first->next;
    fprintf(stderr, "POP %d\n", x->elem);
    free(x);
}


#include "test.h"
```

# Циклическая очередь без расширения


```cpp
%%cpp main.c
%run clang -std=c99 -Wall -Werror -fsanitize=address main.c -o a.exe
%run ./a.exe 

#include <stdio.h>
#include <string.h>
#include <assert.h>

typedef struct queue {
    int a[1024];
    int first;
    int last; // не включительно
} queue_t;


void init_queue(queue_t* q) {
    q->first = q->last = 0;
}

void destroy_queue(queue_t* q) {}

void push(queue_t* q, int elem) {
    q->a[(q->last++) % 1024] = elem;
}

int front(queue_t* q) {
    return q->a[q->first % 1024];
}

void pop_front(queue_t* q) {
    ++q->first;
    fprintf(stderr, "POP %d (was on position %d)\n", q->a[q->first % 1024], q->first);
}


#include "test.h"
```


```cpp
%%cpp main.c
d
```

# Тестирование алгоритма (пример с семинара)


```cpp
%%cpp main.c
%run gcc -std=c99 -Wall -Werror -fsanitize=address main.c -o unused.exe
%run gcc -DDO_TEST -std=c99 -Wall -Werror -fsanitize=address main.c -o o.exe
%run ./o.exe 

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

```


```python

```


```python

```


```cpp
%%cpp main.c
%run clang -std=c99 -Wall -Werror -fsanitize=address main.c -o a.exe
%run ./a.exe 

#include <stdio.h>
#include <string.h>
#include <assert.h>

#define print_int(x) printf("%s = %d\n", #x, (x))

int main() {
    int a = 42;
    print_int(a);
    print_int(a++);
    print_int(a);
    
    int b = 42;
    print_int(b);
    print_int(++b);
    print_int(b);
}
```


```cpp
%%cpp main.c
%run clang -std=c99 -Wall -Werror -fsanitize=address main.c -o a.exe
%run ./a.exe 

#include <stdio.h>
#include <string.h>
#include <assert.h>

#define print_int(x) printf("%s = %d\n", #x, (x))

int main() {
    int d[] = {1, 10, 16, 26, 9, 6};
    for (int i = 0; i < sizeof(d) / sizeof(int); ++i) {
        char c = (d[i] < 10) ? ('0' + d[i]) : ('A' + d[i] - 10);
        printf("%c", c);
    }
    printf("\n");
}
```


```cpp
%%cpp main.c
%run clang -std=c11 -Wall -Werror -fsanitize=address main.c -o a.exe
%run ./a.exe 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


int main() {
//     int n;
//     scanf("%d", &n);
//     int d[n];
//     (void)d;

    int* a = (int*)calloc(3, sizeof(int));
    free(a);
    return 0;
}
```


```python

```


```python

```


```cpp
%%cpp merge.c
%run gcc --sanitize=address merge.c -o merge.exe
%run ./merge.exe

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define swap(a, b) { __typeof__(a) c = (a); (a) = (b); (b) = (c); }

#define mul(a, b) ((a) * (b))

int main() {
    printf("%d\n", mul(1 + 1, 1 + 1));
    int a = 40, b = 50;
    printf("%d %d\n", a, b);
    swap(a, b);
    printf("%d %d\n", a, b);
    return 0;
}
```


```cpp
%%cpp merge.c
%run gcc --sanitize=address merge.c -o merge.exe
%run ./merge.exe

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define swap(a, b) { __typeof__(a) c = (a); (a) = (b); (b) = (c); }

#define mul(a, b) ((a) * (b))

int main() {
    //n (1 + 2 / 2 + 3 / 4 + 4 / 8 + 5 / 16)
    double c = 0;
    double mul = 1.0;
    for (int i = 1; i < 10; ++i) {
        c += i / mul;
        mul *= 2;
        printf("%lf\n", c);
    }
    return 0;
}
```


```python

```

# Задача на код на Си и совсем капельку на алгоритмы


```cpp
%%cpp task.c
%run gcc -fsanitize=address task.c -o task.exe
%run ./task.exe

#include <stdio.h>
#include <string.h>
#include <assert.h>


typedef struct {
    int x, y;
} point_t;

// 1 пункт
int compare_struct(point_t* a, point_t* b) {
    // напишите компаратор такой же, какой нужен для qsort
    // используйте man qsort в консоли или поисковике
    if (a->x != b->x) 
        return a->x - b->x;
    return a->y - b->y;
}

// 2 пункт
void quadratic_sort(
    void* base, size_t array_size, size_t elem_size, 
    int (*comparator)(const void *, const void *)
) {
    // напишите квадратичную сортировку
    char* base_c = (char*)base;
    char tmp_buff[elem_size];
    for (int i = 0; i < array_size; ++i) {
        for (int j = i + 1; j < array_size; ++j) {
            char* a = base + i * elem_size;
            char* b = base + j * elem_size;
            if (comparator(a, b) > 0) {
                memcpy(tmp_buff, a, elem_size);
                memcpy(a, b, elem_size);
                memcpy(b, tmp_buff, elem_size);
            }
        }
    }
}

// 3 пункт
void do_test_1() {
    // напишите тесты на quadratic_sort с использованием структуры point_t
    {
        point_t arr[2] = {{3, 4}, {1, 2}};
        quadratic_sort(arr, sizeof(arr) / sizeof(point_t), sizeof(point_t), 
                       (int (*)(const void *, const void *))compare_struct);
        assert(arr[0].x == 1);
        assert(arr[0].y == 2);
        assert(arr[1].x == 3);
        assert(arr[1].y == 4);
    }
    {
        point_t arr[] = {{3, 4}, {1, 2}, {-1, -3}, {2, 10}, {-1, -5}};
        quadratic_sort(arr, sizeof(arr) / sizeof(point_t), sizeof(point_t), 
                       (int (*)(const void *, const void *))compare_struct);
        assert(arr[0].x == -1);
        assert(arr[0].y == -5);
        assert(arr[1].x == -1);
        assert(arr[1].y == -3);
        assert(arr[2].x == 1);
        assert(arr[2].y == 2);
        
        assert(arr[3].x == 2);
        assert(arr[3].y == 10);
        
        assert(arr[4].x == 3);
        assert(arr[4].y == 4);
    }
}


// 4 пункт
// напишите макрос, который будет создавать функцию сортировки для стандартных типов
// (использовать обычный < для сравнения)
// при этом делегировать сортировку функции quadratic_sort
#define DECLARE_SORT_FUNCTION_IMPL_2(name, type, tag)                                     \
    int compare_##tag(type* a, type* b) {                                                 \
        return (*a < *b) ? -1 : ((*a > *b) ? 1 : 0);                                      \
    }                                                                                     \
    void name(type* first, type* last) {                                                  \
        quadratic_sort(first, last - first, sizeof(*first),                               \
            (int (*)(const void *, const void *))compare_##tag);                          \
    }

#define DECLARE_SORT_FUNCTION_IMPL(name, type, tag) DECLARE_SORT_FUNCTION_IMPL_2(name, type, tag)
#define DECLARE_SORT_FUNCTION(name, type) DECLARE_SORT_FUNCTION_IMPL(name, type, __LINE__)


DECLARE_SORT_FUNCTION(sort_int, int);
DECLARE_SORT_FUNCTION(sort_double, double);

// 5 пункт
void do_test_2() {
    // протестируйте, что функция sort_int правильно работает
    {
        int a[] = {1, 3, 2, 4};
        sort_int(a, a + sizeof(a) / sizeof(int));
        assert(a[0] == 1);
        assert(a[1] == 2);
        assert(a[2] == 3);
        assert(a[3] == 4);
    }
    {
        int a[] = {3, 1};
        sort_int(a, a + sizeof(a) / sizeof(int));
        assert(a[0] == 1);
        assert(a[1] == 3);
    }
    
    {
        double a[] = {1, 3, 2, 4};
        sort_double(a, a + sizeof(a) / sizeof(double));
        assert(a[0] == 1);
        assert(a[1] == 2);
        assert(a[2] == 3);
        assert(a[3] == 4);
    }
    {
        double a[] = {3, 1};
        sort_double(a, a + sizeof(a) / sizeof(double));
        assert(a[0] == 1);
        assert(a[1] == 3);
    }
}


int main() {
    //do_test_1();
    do_test_2();
    fprintf(stderr, "SUCCESS\n");
    return 0;
}






```


```python
!man qsort
```


```python

```


```cpp
%%cpp merge.c
%run gcc --sanitize=address merge.c -o merge.exe
%run ./merge.exe

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


typedef struct {
    int sum;
    int i, j;
} result_t;

int main() {
    int a[], b[];
    
    int max_a = a[0];
    int pos_i = 0;
    
    result_t result = {.sum = a[0] + b[0], .i = 0, .j = 0};
        
    for (int j = 0; i < N; ++j) {
        if (a[i] > max_a) {
            max_a = a[j];
            pos_i = j;
        }
        result_t candidate = {.sum = max_a + b[j], .i = pos_i, .j = j};
        if (candidate.sum > result.sum) {
            result = candidate;
        }
    }
    
    print("%d %d %d", result.sum, result.i, result.j);
    
    
    
    return 0;
}
```


```cpp
%%cpp merge.c
%run gcc --sanitize=address merge.c -o merge.exe
%run ./merge.exe

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define SIZE


int f(int* a)
    
int f(int** a)
   
    
    
int* very_very_bad_g(int n) {
    int arr[n];
    for (int i = 0; i < n; ++i) {
        arr[i] = i * i;
    }
    return arr;
}

    
int* g(int n) {
    int* arr = calloc(n, sizeof(int));
    for (int i = 0; i < n; ++i) {
        arr[i] = i * i;
    }
    return arr;
}

void g(int n, int* arr) {
    for (int i = 0; i < n; ++i) {
        arr[i] = i * i;
    }
}

int main() {
    N x M
    
    int* a = calloc(N * M, sizeof(int));
    
    // NxM: 3x2
    // 12
    // 23
    // 45
    
    0 <= i < N
    0 <= j < M
    a[i][j] -> a[i * M + j] // max (N - 1) * M + (M - 1) = N * M - 1
    free(a);

    
    int** a = calloc(N, sizeof(int*));
    for (int i = 0; i < N; ++i) {
        a[i] = calloc(M, sizeof(int));
    }
    a[i][j]
    
    for (int i = 0; i < N; ++i) {
        free(a[i]);
    }
    free(a);
    
    
    int* a = calloc(N, sizeof(int));
    ...
    int* b = calloc(N, sizeof(int));
    for (int i = 0; i < N; ++i) {
        a[i] = b[i];
    }
    memcpy(a, b, N * sizeof(int));
    free(a);
    free(b);
    
    
    return 0;
}

```


```python

```


```python
int x = (y*z)%
```


```cpp
%%cpp merge.c
%run gcc --sanitize=address merge.c -o merge.exe
%run ./merge.exe

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int f() {
    return 1;
}

typedef struct {} struct_t;

int main() {
    int a = 1134234243;
    int b = 134523432;
    ((int64_t)a * (int64_t)b);
   
    
    return 0;
}
```

$$sum_{i \in 0..N} (10^i \cdot a_i)$$


```cpp
%%cpp merge.c
%run gcc -Wall -Werror --sanitize=address merge.c -o merge.exe
%run ./merge.exe

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MODULE 10

typedef struct {
    int len;
    int* digits;
} big_number_t;

big_number_t to_big_number(char* s) {
    // "1234" aka {'1', '2', '3', '4'}
    // ->
    // {4, 3, 2, 1}
    // real value = 4 * 10**0 + 3 * 10**1 + 2 * 10**2 + 1 * 10**3
    big_number_t result;
    result.len = strlen(s);
    result.digits = calloc(result.len, sizeof(int));
    for (int i = 0; i < result.len; ++i) {
        result.digits[i] = s[result.len - 1 - i] - '0'; // '0'..'9' - '0' ~ 0..9
    }
    return result;
}

void destroy_big_number(big_number_t n) {
    free(n.digits);
    n.digits = NULL;
}

big_number_t add_big_number(big_number_t a, big_number_t b) {
    if (a.len < b.len) {
        return add_big_number(b, a);
    }
    big_number_t c;
    c.len = a.len + 1;
    c.digits = calloc(c.len, sizeof(int));
    for (int i = 0; i < b.len; ++i) {
        c.digits[i] += a.digits[i] + b.digits[i];
        c.digits[i + 1] += c.digits[i] / MODULE;
        c.digits[i] %= MODULE;
    }
    for (int i = b.len; i < a.len; ++i) {
        c.digits[i] += a.digits[i];
        c.digits[i + 1] += c.digits[i] / MODULE;
        c.digits[i] %= MODULE;
    }
    while (c.len && c.digits[c.len - 1] == 0) {
        --c.len;
    }
    return c;
}

big_number_t mul_big_number(big_number_t a, big_number_t b) {
    big_number_t c;
    c.len = a.len + b.len;
    c.digits = calloc(c.len, sizeof(int));
    for (int i = 0; i < a.len; ++i) {      
        for (int j = 0; j < b.len; ++j) {
            c.digits[i + j] += a.digits[i] * b.digits[j];
        }
    }
    for (int i = 0; i + 1 < c.len; ++i) {
        c.digits[i + 1] += c.digits[i] / MODULE;
        c.digits[i] %= MODULE;
    }
    while (c.len && c.digits[c.len - 1] == 0) {
        --c.len;
    }
    return c;
}

big_number_t div_small_big_number(big_number_t a, int b, int* remainder) {
    big_number_t c;
    c.len = a.len;
    c.digits = calloc(c.len, sizeof(int));
    int r = 0;
    for (int i = a.len - 1; i >= 0; --i) {
        int cur = r * MODULE + a.digits[i];
        c.digits[i] = cur / b;
        r = cur % b;
    }
    if (remainder) {
        *remainder = r;
    }
    while (c.len && c.digits[c.len - 1] == 0) {
        --c.len;
    }
    return c;
}

void print_big_number(big_number_t n) {
    for (int i = n.len - 1; i >= 0; --i) {
        printf("%c", '0' + n.digits[i]);
    }
}


int main() {
    char* a = "999999999999999999";
    char* b = "999999999999999999";
    big_number_t a_n = to_big_number(a);
    big_number_t b_n = to_big_number(b);
    big_number_t c_n = add_big_number(a_n, b_n);
    big_number_t d_n = div_small_big_number(c_n, 11, NULL);
    print_big_number(a_n); printf("\n");
    print_big_number(b_n); printf("\n");
    print_big_number(c_n); printf("\n");
    print_big_number(d_n); printf("\n");
    
    destroy_big_number(a_n); 
    destroy_big_number(c_n); 
    destroy_big_number(b_n); 
    destroy_big_number(d_n); 
    
    return 0;
}
```


```python

```


```python
%%sql
SELECT a from B;
```


```python

```
