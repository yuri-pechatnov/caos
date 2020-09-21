

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
