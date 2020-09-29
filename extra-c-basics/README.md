


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

    UsageError: Cell magic `%%cpp` not found.



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


Run: `gcc  -std=c99 -Wall -Werror main.c -o a.exe`



Run: `./a.exe`


    i = 5, sizeof(i) = 4
    i_32 = 6, sizeof(i_32) = 4
    i_64 = 7, sizeof(i_64) = 8


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


Run: `gcc -m64 -std=c99 -Wall -Werror main.c -o a.exe`



Run: `./a.exe`


    {.x = 1.000000, .y = 100.000000}
    {.x = 4.000000, .y = 150.000000}
    {.x = -2.000000, .y = 50.000000}
    {.x = 10.000000, .y = 1000.000000}


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


Run: `gcc -m64 -std=c99 -Wall -Werror main.c -o a.exe`



Run: `./a.exe`


    Hello wotrld!
    1
    Hello world! 2
    8 2 100
    sizeof(a) = 20
    a[0] = 1, a[1] = 5, a[2] = 3, a[3] = 2, a[4] = 5, 
    a[0] = 1, a[1] = 5, a[2] = 3, a[3] = 2, a[4] = 5, 


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

    QSORT(3)                   Linux Programmer's Manual                  QSORT(3)
    
    NAME
           qsort, qsort_r - sort an array
    
    SYNOPSIS
           #include <stdlib.h>
    
           void qsort(void *base, size_t nmemb, size_t size,
                      int (*compar)(const void *, const void *));



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


Run: `gcc -m64 -std=c99 -Wall -Werror main.c -o a.exe`



Run: `./a.exe < 001.in`


    a[0] = 0, a[1] = 1, a[2] = 5, a[3] = 9, 



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


Run: `gcc -DDEBUG -m64 -std=c99 -Wall -Werror main.c -o a.exe`



Run: `./a.exe`


    a[0] = 0, a[1] = 1, a[2] = 5, a[3] = 9, 



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
%run gcc -std=c99 -Wall -Werror -fsanitize=address main.c -o a.exe
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


Run: `gcc -std=c99 -Wall -Werror -fsanitize=address main.c -o a.exe`



Run: `./a.exe < 001.in`


    READ STRING: push
       IT IS PUSH 3
    READ STRING: pop
    READ STRING: exit
    READ STRING: hello
       IT IS HELLO 6



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


Run: `gcc -std=c99 -Wall -Werror -fsanitize=address main.c -o a.exe`



Run: `./a.exe < 001.in`


    READ LINE: 'push 3
    '
       IT IS PUSH 3
    READ LINE: 'pop
    '
      GET ONLY STRING
    READ LINE: 'exit
    '
      GET ONLY STRING
    READ LINE: 'hello 6
    '
       IT IS HELLO 6
    READ LINE: '    '
      ERROR
    a.exe: main.c:34: main: Assertion `ret == 2' failed.
    Aborted (core dumped)



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


Run: `clang -std=c99 -Wall -Werror -fsanitize=address main.c -o a.exe`



Run: `./a.exe < 001.in`


    0x4d6c60
    Hello students!
    a[1] = 'e'
    c1 = a + b = Hellostudents!
    c2 = a + ' ' + b = Hello students!



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


Run: `clang -std=c99 -Wall -Werror -fsanitize=address main.c -o a.exe`



Run: `./a.exe`


    HELLO
    'A' = 65, 'a' = 97



```python

```

    No manual entry for upper_case



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


Run: `clang -std=c99 -Wall -Werror -fsanitize=address main.c -o a.exe`



Run: `./a.exe`


    POP 4 (was on position 3)
    POP 3 (was on position 2)
    POP 5 (was on position 2)
    POP 2 (was on position 1)
    POP 1 (was on position 0)
    SUCCESS


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


Run: `clang -std=c99 -Wall -Werror -fsanitize=address main.c -o a.exe`



Run: `./a.exe`


    POP 4
    POP 3
    POP 5
    POP 2
    POP 1
    SUCCESS


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


Run: `clang -std=c99 -Wall -Werror -fsanitize=address main.c -o a.exe`



Run: `./a.exe`


    POP 4 (was on position 3)
    POP 3 (was on position 2)
    POP 5 (was on position 2)
    POP 2 (was on position 1)
    POP 1 (was on position 0)
    SUCCESS


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


Run: `clang -std=c99 -Wall -Werror -fsanitize=address main.c -o a.exe`



Run: `./a.exe`


    
    =================================================================
    [1m[31m==2729==ERROR: LeakSanitizer: detected memory leaks
    [1m[0m
    [1m[34mDirect leak of 40 byte(s) in 1 object(s) allocated from:
    [1m[0m    #0 0x493ba2 in calloc (/home/pechatnov/vbox/caos/extra-c-basics/a.exe+0x493ba2)
        #1 0x4c313d in main (/home/pechatnov/vbox/caos/extra-c-basics/a.exe+0x4c313d)
        #2 0x7f41fe10e0b2 in __libc_start_main /build/glibc-YYA7BZ/glibc-2.31/csu/../csu/libc-start.c:308:16
    
    SUMMARY: AddressSanitizer: 40 byte(s) leaked in 1 allocation(s).


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


Run: `clang -std=c99 -Wall -Werror -fsanitize=address main.c -o a.exe`



Run: `./a.exe`


    AddressSanitizer:DEADLYSIGNAL
    =================================================================
    [1m[31m==2746==ERROR: AddressSanitizer: SEGV on unknown address 0x6040000622e0 (pc 0x0000004c3191 bp 0x7ffdea6c0450 sp 0x7ffdea6c0430 T0)
    [1m[0m==2746==The signal is caused by a READ memory access.
        #0 0x4c3191 in main (/home/pechatnov/vbox/caos/extra-c-basics/a.exe+0x4c3191)
        #1 0x7f8e489200b2 in __libc_start_main /build/glibc-YYA7BZ/glibc-2.31/csu/../csu/libc-start.c:308:16
        #2 0x41b2ed in _start (/home/pechatnov/vbox/caos/extra-c-basics/a.exe+0x41b2ed)
    
    AddressSanitizer can not provide additional info.
    SUMMARY: AddressSanitizer: SEGV (/home/pechatnov/vbox/caos/extra-c-basics/a.exe+0x4c3191) in main
    ==2746==ABORTING


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


Run: `clang -std=c99 -Wall -Werror -fsanitize=address main.c -o a.exe`



Run: `./a.exe`


    =================================================================
    [1m[31m==2753==ERROR: AddressSanitizer: heap-use-after-free on address 0x6040000000a4 at pc 0x0000004c3199 bp 0x7ffdfa1b0400 sp 0x7ffdfa1b03f8
    [1m[0m[1m[34mREAD of size 4 at 0x6040000000a4 thread T0[1m[0m
        #0 0x4c3198 in main (/home/pechatnov/vbox/caos/extra-c-basics/a.exe+0x4c3198)
        #1 0x7fdcdaa230b2 in __libc_start_main /build/glibc-YYA7BZ/glibc-2.31/csu/../csu/libc-start.c:308:16
        #2 0x41b2ed in _start (/home/pechatnov/vbox/caos/extra-c-basics/a.exe+0x41b2ed)
    
    [1m[32m0x6040000000a4 is located 20 bytes inside of 40-byte region [0x604000000090,0x6040000000b8)
    [1m[0m[1m[35mfreed by thread T0 here:[1m[0m
        #0 0x4937ad in free (/home/pechatnov/vbox/caos/extra-c-basics/a.exe+0x4937ad)
        #1 0x4c314d in main (/home/pechatnov/vbox/caos/extra-c-basics/a.exe+0x4c314d)
        #2 0x7fdcdaa230b2 in __libc_start_main /build/glibc-YYA7BZ/glibc-2.31/csu/../csu/libc-start.c:308:16
    
    [1m[35mpreviously allocated by thread T0 here:[1m[0m
        #0 0x493ba2 in calloc (/home/pechatnov/vbox/caos/extra-c-basics/a.exe+0x493ba2)
        #1 0x4c313d in main (/home/pechatnov/vbox/caos/extra-c-basics/a.exe+0x4c313d)
        #2 0x7fdcdaa230b2 in __libc_start_main /build/glibc-YYA7BZ/glibc-2.31/csu/../csu/libc-start.c:308:16
    
    SUMMARY: AddressSanitizer: heap-use-after-free (/home/pechatnov/vbox/caos/extra-c-basics/a.exe+0x4c3198) in main
    Shadow bytes around the buggy address:
      0x0c087fff7fc0: [1m[0m00[1m[0m [1m[0m00[1m[0m [1m[0m00[1m[0m [1m[0m00[1m[0m [1m[0m00[1m[0m [1m[0m00[1m[0m [1m[0m00[1m[0m [1m[0m00[1m[0m [1m[0m00[1m[0m [1m[0m00[1m[0m [1m[0m00[1m[0m [1m[0m00[1m[0m [1m[0m00[1m[0m [1m[0m00[1m[0m [1m[0m00[1m[0m [1m[0m00[1m[0m
      0x0c087fff7fd0: [1m[0m00[1m[0m [1m[0m00[1m[0m [1m[0m00[1m[0m [1m[0m00[1m[0m [1m[0m00[1m[0m [1m[0m00[1m[0m [1m[0m00[1m[0m [1m[0m00[1m[0m [1m[0m00[1m[0m [1m[0m00[1m[0m [1m[0m00[1m[0m [1m[0m00[1m[0m [1m[0m00[1m[0m [1m[0m00[1m[0m [1m[0m00[1m[0m [1m[0m00[1m[0m
      0x0c087fff7fe0: [1m[0m00[1m[0m [1m[0m00[1m[0m [1m[0m00[1m[0m [1m[0m00[1m[0m [1m[0m00[1m[0m [1m[0m00[1m[0m [1m[0m00[1m[0m [1m[0m00[1m[0m [1m[0m00[1m[0m [1m[0m00[1m[0m [1m[0m00[1m[0m [1m[0m00[1m[0m [1m[0m00[1m[0m [1m[0m00[1m[0m [1m[0m00[1m[0m [1m[0m00[1m[0m
      0x0c087fff7ff0: [1m[0m00[1m[0m [1m[0m00[1m[0m [1m[0m00[1m[0m [1m[0m00[1m[0m [1m[0m00[1m[0m [1m[0m00[1m[0m [1m[0m00[1m[0m [1m[0m00[1m[0m [1m[0m00[1m[0m [1m[0m00[1m[0m [1m[0m00[1m[0m [1m[0m00[1m[0m [1m[0m00[1m[0m [1m[0m00[1m[0m [1m[0m00[1m[0m [1m[0m00[1m[0m
      0x0c087fff8000: [1m[31mfa[1m[0m [1m[31mfa[1m[0m [1m[35mfd[1m[0m [1m[35mfd[1m[0m [1m[35mfd[1m[0m [1m[35mfd[1m[0m [1m[35mfd[1m[0m [1m[35mfd[1m[0m [1m[31mfa[1m[0m [1m[31mfa[1m[0m [1m[0m00[1m[0m [1m[0m00[1m[0m [1m[0m00[1m[0m [1m[0m00[1m[0m [1m[0m00[1m[0m [1m[0m02[1m[0m
    =>0x0c087fff8010: [1m[31mfa[1m[0m [1m[31mfa[1m[0m [1m[35mfd[1m[0m [1m[35mfd[1m[0m[[1m[35mfd[1m[0m][1m[35mfd[1m[0m [1m[35mfd[1m[0m [1m[31mfa[1m[0m [1m[31mfa[1m[0m [1m[31mfa[1m[0m [1m[31mfa[1m[0m [1m[31mfa[1m[0m [1m[31mfa[1m[0m [1m[31mfa[1m[0m [1m[31mfa[1m[0m [1m[31mfa[1m[0m
      0x0c087fff8020: [1m[31mfa[1m[0m [1m[31mfa[1m[0m [1m[31mfa[1m[0m [1m[31mfa[1m[0m [1m[31mfa[1m[0m [1m[31mfa[1m[0m [1m[31mfa[1m[0m [1m[31mfa[1m[0m [1m[31mfa[1m[0m [1m[31mfa[1m[0m [1m[31mfa[1m[0m [1m[31mfa[1m[0m [1m[31mfa[1m[0m [1m[31mfa[1m[0m [1m[31mfa[1m[0m [1m[31mfa[1m[0m
      0x0c087fff8030: [1m[31mfa[1m[0m [1m[31mfa[1m[0m [1m[31mfa[1m[0m [1m[31mfa[1m[0m [1m[31mfa[1m[0m [1m[31mfa[1m[0m [1m[31mfa[1m[0m [1m[31mfa[1m[0m [1m[31mfa[1m[0m [1m[31mfa[1m[0m [1m[31mfa[1m[0m [1m[31mfa[1m[0m [1m[31mfa[1m[0m [1m[31mfa[1m[0m [1m[31mfa[1m[0m [1m[31mfa[1m[0m
      0x0c087fff8040: [1m[31mfa[1m[0m [1m[31mfa[1m[0m [1m[31mfa[1m[0m [1m[31mfa[1m[0m [1m[31mfa[1m[0m [1m[31mfa[1m[0m [1m[31mfa[1m[0m [1m[31mfa[1m[0m [1m[31mfa[1m[0m [1m[31mfa[1m[0m [1m[31mfa[1m[0m [1m[31mfa[1m[0m [1m[31mfa[1m[0m [1m[31mfa[1m[0m [1m[31mfa[1m[0m [1m[31mfa[1m[0m
      0x0c087fff8050: [1m[31mfa[1m[0m [1m[31mfa[1m[0m [1m[31mfa[1m[0m [1m[31mfa[1m[0m [1m[31mfa[1m[0m [1m[31mfa[1m[0m [1m[31mfa[1m[0m [1m[31mfa[1m[0m [1m[31mfa[1m[0m [1m[31mfa[1m[0m [1m[31mfa[1m[0m [1m[31mfa[1m[0m [1m[31mfa[1m[0m [1m[31mfa[1m[0m [1m[31mfa[1m[0m [1m[31mfa[1m[0m
      0x0c087fff8060: [1m[31mfa[1m[0m [1m[31mfa[1m[0m [1m[31mfa[1m[0m [1m[31mfa[1m[0m [1m[31mfa[1m[0m [1m[31mfa[1m[0m [1m[31mfa[1m[0m [1m[31mfa[1m[0m [1m[31mfa[1m[0m [1m[31mfa[1m[0m [1m[31mfa[1m[0m [1m[31mfa[1m[0m [1m[31mfa[1m[0m [1m[31mfa[1m[0m [1m[31mfa[1m[0m [1m[31mfa[1m[0m
    Shadow byte legend (one shadow byte represents 8 application bytes):
      Addressable:           [1m[0m00[1m[0m
      Partially addressable: [1m[0m01[1m[0m [1m[0m02[1m[0m [1m[0m03[1m[0m [1m[0m04[1m[0m [1m[0m05[1m[0m [1m[0m06[1m[0m [1m[0m07[1m[0m 
      Heap left redzone:       [1m[31mfa[1m[0m
      Freed heap region:       [1m[35mfd[1m[0m
      Stack left redzone:      [1m[31mf1[1m[0m
      Stack mid redzone:       [1m[31mf2[1m[0m
      Stack right redzone:     [1m[31mf3[1m[0m
      Stack after return:      [1m[35mf5[1m[0m
      Stack use after scope:   [1m[35mf8[1m[0m
      Global redzone:          [1m[31mf9[1m[0m
      Global init order:       [1m[36mf6[1m[0m
      Poisoned by user:        [1m[34mf7[1m[0m
      Container overflow:      [1m[34mfc[1m[0m
      Array cookie:            [1m[31mac[1m[0m
      Intra object redzone:    [1m[33mbb[1m[0m
      ASan internal:           [1m[33mfe[1m[0m
      Left alloca redzone:     [1m[34mca[1m[0m
      Right alloca redzone:    [1m[34mcb[1m[0m
      Shadow gap:              [1m[0mcc[1m[0m
    ==2753==ABORTING


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


Run: `gcc --sanitize=address merge.c -o merge.exe`



Run: `./merge.exe`


    1, 2, 3, 4, 5, 8, 9, 



```python

```


```python
!man calloc
```

    MALLOC(3)                  Linux Programmer's Manual                 MALLOC(3)
    
    NAME
           malloc, free, calloc, realloc - allocate and free dynamic memory
    
    SYNOPSIS
           #include <stdlib.h>
    
           void *malloc(size_t size);
           void free(void *ptr);
           void *calloc(size_t nmemb, size_t size);
           void *realloc(void *ptr, size_t size);
           void *reallocarray(void *ptr, size_t nmemb, size_t size);
    
       Feature Test Macro Requirements for glibc (see feature_test_macros(7)):
    
           reallocarray():
               Since glibc 2.29:
                   _DEFAULT_SOURCE
               Glibc 2.28 and earlier:
                   _GNU_SOURCE
    
    DESCRIPTION
           The malloc() function allocates size bytes and returns a pointer to the
           allocated memory.  The memory is not initialized.  If size is  0,  then
           malloc()  returns either NULL, or a unique pointer value that can later
           be successfully passed to free().
    
           The free() function frees the memory space pointed  to  by  ptr,  which
           must  have  been  returned by a previous call to malloc(), calloc(), or
           realloc().  Otherwise, or if free(ptr) has already been called  before,
           undefined behavior occurs.  If ptr is NULL, no operation is performed.
    
           The  calloc()  function allocates memory for an array of nmemb elements
           of size bytes each and returns a pointer to the allocated memory.   The
           memory  is  set  to zero.  If nmemb or size is 0, then calloc() returns
           either NULL, or a unique pointer value that can later  be  successfully
           passed to free().  If the multiplication of nmemb and size would result
           in integer overflow, then calloc() returns an error.  By  contrast,  an
           integer  overflow  would  not be detected in the following call to mal‐
           loc(), with the result that an incorrectly sized block of memory  would
           be allocated:
    
               malloc(nmemb * size);
    
           The  realloc() function changes the size of the memory block pointed to
           by ptr to size bytes.  The contents will be unchanged in the range from
           the start of the region up to the minimum of the old and new sizes.  If
           the new size is larger than the old size, the added memory will not  be
           initialized.   If  ptr  is  NULL,  then  the call is equivalent to mal‐
           loc(size), for all values of size; if size is equal to zero, and ptr is
           not  NULL,  then  the  call  is equivalent to free(ptr).  Unless ptr is
           NULL, it must have been returned by an earlier call to  malloc(),  cal‐
           loc(),  or realloc().  If the area pointed to was moved, a free(ptr) is
           done.
    
           The reallocarray() function  changes  the  size  of  the  memory  block
           pointed  to  by  ptr to be large enough for an array of nmemb elements,
           each of which is size bytes.  It is equivalent to the call
    
                   realloc(ptr, nmemb * size);
    
           However, unlike that realloc() call, reallocarray() fails safely in the
           case  where the multiplication would overflow.  If such an overflow oc‐
           curs, reallocarray() returns NULL, sets errno to ENOMEM, and leaves the
           original block of memory unchanged.
    
    RETURN VALUE
           The  malloc()  and calloc() functions return a pointer to the allocated
           memory, which is suitably aligned for any  built-in  type.   On  error,
           these functions return NULL.  NULL may also be returned by a successful
           call to malloc() with a size of zero, or by a successful call  to  cal‐
           loc() with nmemb or size equal to zero.
    
           The free() function returns no value.
    
           The realloc() function returns a pointer to the newly allocated memory,
           which is suitably aligned for any built-in type, or NULL if the request
           failed.   The returned pointer may be the same as ptr if the allocation
           was not moved (e.g., there was room to expand the allocation in-place),
           or different from ptr if the allocation was moved to a new address.  If
           size was equal to 0, either NULL or a pointer suitable to be passed  to
           free() is returned.  If realloc() fails, the original block is left un‐
           touched; it is not freed or moved.
    
           On success, the reallocarray() function returns a pointer to the  newly
           allocated  memory.   On failure, it returns NULL and the original block
           of memory is left untouched.
    
    ERRORS
           calloc(), malloc(), realloc(), and reallocarray()  can  fail  with  the
           following error:
    
           ENOMEM Out  of  memory.  Possibly, the application hit the RLIMIT_AS or
                  RLIMIT_DATA limit described in getrlimit(2).
    
    ATTRIBUTES
           For an  explanation  of  the  terms  used  in  this  section,  see  at‐
           tributes(7).
    
           ┌─────────────────────┬───────────────┬─────────┐
           │Interface            │ Attribute     │ Value   │
           ├─────────────────────┼───────────────┼─────────┤
           │malloc(), free(),    │ Thread safety │ MT-Safe │
           │calloc(), realloc()  │               │         │
           └─────────────────────┴───────────────┴─────────┘
    CONFORMING TO
           malloc(), free(), calloc(), realloc(): POSIX.1-2001, POSIX.1-2008, C89,
           C99.
    
           reallocarray() is a nonstandard extension that first appeared in  Open‐
           BSD 5.6 and FreeBSD 11.0.
    
    NOTES
           By  default,  Linux  follows  an optimistic memory allocation strategy.
           This means that when malloc() returns non-NULL there  is  no  guarantee
           that  the  memory  really  is available.  In case it turns out that the
           system is out of memory, one or more processes will be  killed  by  the
           OOM   killer.    For   more   information,   see   the  description  of
           /proc/sys/vm/overcommit_memory and /proc/sys/vm/oom_adj in proc(5), and
           the   Linux  kernel  source  file  Documentation/vm/overcommit-account‐
           ing.rst.
    
           Normally, malloc() allocates memory from the heap, and adjusts the size
           of the heap as required, using sbrk(2).  When allocating blocks of mem‐
           ory larger than MMAP_THRESHOLD bytes, the glibc malloc() implementation
           allocates  the  memory  as  a  private anonymous mapping using mmap(2).
           MMAP_THRESHOLD is 128 kB by  default,  but  is  adjustable  using  mal‐
           lopt(3).   Prior  to Linux 4.7 allocations performed using mmap(2) were
           unaffected by the RLIMIT_DATA resource limit;  since  Linux  4.7,  this
           limit is also enforced for allocations performed using mmap(2).
    
           To avoid corruption in multithreaded applications, mutexes are used in‐
           ternally to protect the memory-management data structures  employed  by
           these  functions.   In a multithreaded application in which threads si‐
           multaneously allocate and free memory, there could  be  contention  for
           these  mutexes.   To scalably handle memory allocation in multithreaded
           applications, glibc creates additional memory allocation arenas if  mu‐
           tex  contention  is  detected.   Each arena is a large region of memory
           that is internally allocated by the system (using brk(2)  or  mmap(2)),
           and managed with its own mutexes.
    
           SUSv2 requires malloc(), calloc(), and realloc() to set errno to ENOMEM
           upon failure.  Glibc assumes that this is done (and the glibc  versions
           of  these routines do this); if you use a private malloc implementation
           that does not set errno, then certain library routines may fail without
           having a reason in errno.
    
           Crashes  in  malloc(), calloc(), realloc(), or free() are almost always
           related to heap corruption, such as overflowing an allocated  chunk  or
           freeing the same pointer twice.
    
           The  malloc()  implementation is tunable via environment variables; see
           mallopt(3) for details.
    
    SEE ALSO
           valgrind(1), brk(2), mmap(2), alloca(3), malloc_get_state(3),
           malloc_info(3), malloc_trim(3), malloc_usable_size(3), mallopt(3),
           mcheck(3), mtrace(3), posix_memalign(3)
    
           For details of the GNU C library implementation, see
           ⟨https://sourceware.org/glibc/wiki/MallocInternals⟩.
    
    COLOPHON
           This page is part of release 5.05 of the Linux man-pages project.  A
           description of the project, information about reporting bugs, and the
           latest version of this page, can be found at
           https://www.kernel.org/doc/man-pages/.
    
    GNU                               2020-02-09                         MALLOC(3)



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


Run: `clang -std=c99 -Wall -Werror -fsanitize=address main.c -o a.exe`



Run: `./a.exe`


    POP 2 (was on position 1)
    POP 3 (was on position 2)
    POP 4 (was on position 3)
    POP 5 (was on position 4)
    POP 6 (was on position 5)
    SUCCESS


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


Run: `clang -std=c99 -Wall -Werror -fsanitize=address main.c -o a.exe`



Run: `./a.exe`


    POP 1
    POP 2
    POP 3
    POP 4
    POP 5
    POP 6
    SUCCESS


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


Run: `clang -std=c99 -Wall -Werror -fsanitize=address main.c -o a.exe`



Run: `./a.exe`


    POP 2 (was on position 1)
    POP 3 (was on position 2)
    POP 4 (was on position 3)
    POP 5 (was on position 4)
    POP 6 (was on position 5)
    SUCCESS



```python

```
