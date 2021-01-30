



```python

```

# От С к С++

Будем рассматривать минимально необходимое подмножество С++ на мой взгляд

До этого мы рассматривали подмножество языка С полностью совместимое с С++ (с 20м стандартом).

Рассмотрим на примере стека, как он может быть написан на С и на С++ и как связаны между собой реализации.


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

void push_stack(stack_t* stack, int elem) {
    if (stack->sz == stack->max_sz) {
        stack->max_sz += (stack->max_sz == 0);
        stack->max_sz *= 2;
        (*stack).a = realloc(stack->a, stack->max_sz * sizeof(int));
    }
    stack->a[stack->sz++] = elem;
}

int top_stack(stack_t* stack) {
    return stack->a[stack->sz - 1];
}

void pop_stack(stack_t* stack) {
    --stack->sz;
}


int main() {
    stack_t* s = (stack_t*)malloc(sizeof(stack_t));
    init_stack(s);
    push_stack(s, 123);
    push_stack(s, 42);
    assert(top_stack(s) == 42);
    pop_stack(s);
    assert(top_stack(s) == 123);
    destroy_stack(s);
    free(s);
    return 0;
}
```


Run: `clang -std=c99 -Wall -Werror -fsanitize=address main.c -o a.exe`



Run: `./a.exe`


1) Добавим методы


```cpp
%%cpp main.cpp
%run g++ -std=c++17 -Wall -Werror -fsanitize=address -fno-exceptions -fno-rtti main.cpp -o a.exe
%run ./a.exe 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <new> // Важно для placement new!

// Больше не нужны typedef
struct stack_t {
    // Поля такие же
    int* a;
    int sz;
    int max_sz;
    
    // Декларируем методы
    stack_t(); // конструктор
    ~stack_t(); // деструктор
    void push(int elem);
    int top();
    void pop();
}; 


// void init_stack(stack_t* stack) {
//     *stack = (stack_t){0};
// }
stack_t::stack_t() {
    this->a = nullptr; 
    this->sz = 0;
    this->max_sz = 0;
}

// void destroy_stack(stack_t* stack) {
//     free(stack->a);
// }
stack_t::~stack_t() {
    free(this->a);
}

// void push_stack(stack_t* stack, int elem) {
//     if (stack->sz == stack->max_sz) {
//         stack->max_sz += (stack->max_sz == 0);
//         stack->max_sz *= 2;
//         (*stack).a = realloc(stack->a, stack->max_sz * sizeof(int));
//     }
//     stack->a[stack->sz++] = elem;
// }
void stack_t::push(int elem) {
    if (this->sz == this->max_sz) {
        this->max_sz += (this->max_sz == 0);
        this->max_sz *= 2;
        this->a = (int*)realloc(this->a, this->max_sz * sizeof(int));
    }
    this->a[this->sz++] = elem;
}

// int top_stack(stack_t* stack) {
//     return stack->a[stack->sz - 1];
// }
int stack_t::top() {
    return this->a[this->sz - 1];
}

// void pop_stack(stack_t* stack) {
//     --stack->sz;
// }

void stack_t::pop() {
    --this->sz;
}



int main() {
    stack_t* s = (stack_t*)malloc(sizeof(stack_t));
    new ((void*)s) stack_t;  // init_stack(s);
    s->push(123);            // push_stack(s, 123);
    s->push(42);             // push_stack(s, 42);
    assert(s->top() == 42);  // assert(top_stack(s) == 42);
    s->pop();                // pop_stack(s);
    assert(s->top() == 123); // assert(top_stack(s) == 123);
    s->~stack_t();           // destroy_stack(s);
    free(s);
    return 0; 
}
```


Run: `g++ -std=c++17 -Wall -Werror -fsanitize=address -fno-exceptions -fno-rtti main.cpp -o a.exe`



Run: `./a.exe`


2) Уберем избыточность


```cpp
%%cpp main.cpp
%run g++ -std=c++17 -Wall -Werror -fsanitize=address -fno-exceptions -fno-rtti main.cpp -o a.exe
%run ./a.exe 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// Больше не нужны typedef
struct stack_t {
    // Поля такие же
    int* a;
    int sz;
    int max_sz;
    
    // Декларируем методы
    stack_t(); // конструктор
    ~stack_t(); // деструктор
    void push(int elem);
    int top();
    void pop();
}; 


// stack_t::stack_t() {
//     this->a = nullptr; 
//     this->sz = 0;
//     this->max_sz = 0;
// }
stack_t::stack_t() {
    a = nullptr; 
    sz = 0;
    max_sz = 0;
}


// stack_t::~stack_t() {
//     free(this->a);
// }
stack_t::~stack_t() {
    free(a);
}

// void stack_t::push(int elem) {
//     if (this->sz == this->max_sz) {
//         this->max_sz += (this->max_sz == 0);
//         this->max_sz *= 2;
//         this->a = (int*)realloc(this->a, this->max_sz * sizeof(int));
//     }
//     this->a[this->sz++] = elem;
// }
void stack_t::push(int elem) {
    if (sz == max_sz) {
        max_sz += (max_sz == 0);
        max_sz *= 2;
        a = (int*)realloc(a, max_sz * sizeof(int));
    }
    a[sz++] = elem;
}

// int stack_t::top() {
//     return this->a[this->sz - 1];
// }
int stack_t::top() {
    return a[sz - 1];
}

// void stack_t::pop() {
//     --this->sz;
// }
void stack_t::pop() {
    --sz;
}

int main() {
    // variant 1
    {
        stack_t* s = new stack_t; // stack_t* s = (stack_t*)malloc(sizeof(stack_t));
                                  // new ((void*)s) stack_t;  
        s->push(123);           
        s->push(42);             
        assert(s->top() == 42);  
        s->pop();                
        assert(s->top() == 123);
        delete s;                 // s->~stack_t();
                                  // free(s);
    }
    // variant 2
    {
        stack_t s; // new ((void*)s) stack_t;  
        s.push(123);           
        s.push(42);             
        assert(s.top() == 42);  
        s.pop();                
        assert(s.top() == 123);
        // s->~stack_t(); (at the end of scope)
    }
    return 0; 
}
```


Run: `g++ -std=c++17 -Wall -Werror -fsanitize=address -fno-exceptions -fno-rtti main.cpp -o a.exe`



Run: `./a.exe`


3) Ещё подужмем


```cpp
%%cpp main.cpp
%run g++ -std=c++17 -Wall -Werror -fsanitize=address -fno-exceptions -fno-rtti main.cpp -o a.exe
%run ./a.exe 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

struct stack_t {
    int* a;
    int sz;
    int max_sz;
    
    stack_t() {
        a = nullptr; 
        sz = 0;
        max_sz = 0;
    }

    ~stack_t() {
        free(a);
    }
    
    void push(int elem)  {
        if (sz == max_sz) {
            max_sz += (max_sz == 0);
            max_sz *= 2;
            a = (int*)realloc(a, max_sz * sizeof(int));
        }
        a[sz++] = elem;
    }
    
    int top()  {
        return a[sz - 1];
    }
    
    void pop()  {
        --sz;
    }
}; 

int main() {
    stack_t s;  
    s.push(123);           
    s.push(42);             
    assert(s.top() == 42);  
    s.pop();                
    assert(s.top() == 123);
    return 0; 
}
```


Run: `g++ -std=c++17 -Wall -Werror -fsanitize=address -fno-exceptions -fno-rtti main.cpp -o a.exe`



Run: `./a.exe`


4) Добавим шаблонов


```cpp
%%cpp main.cpp
%run g++ -std=c++17 -Wall -Werror -fsanitize=address -fno-exceptions -fno-rtti main.cpp -o a.exe
%run ./a.exe 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

template <typename T>
struct stack_t {
    T* a;
    int sz;
    int max_sz;
    
    stack_t() {
        a = nullptr; 
        sz = 0;
        max_sz = 0;
    }

    ~stack_t() {
        free(a);
    }
    
    void push(T elem)  {
        if (sz == max_sz) {
            max_sz += (max_sz == 0);
            max_sz *= 2;
            a = (T*)realloc(a, max_sz * sizeof(T));
        }
        a[sz++] = elem;
    }
    
    T top()  {
        return a[sz - 1];
    }
    
    void pop()  {
        --sz;
    }
}; 

int main() {
    {
        stack_t<int> s;  
        s.push(123);           
        s.push(42);             
        assert(s.top() == 42);  
        s.pop();                
        assert(s.top() == 123);
    }
    {
        stack_t<char> s;  
        s.push('A');           
        s.push('Z');             
        assert(s.top() == 'Z');  
        s.pop();                
        assert(s.top() == 'A');
    }
    return 0; 
}
```


Run: `g++ -std=c++17 -Wall -Werror -fsanitize=address -fno-exceptions -fno-rtti main.cpp -o a.exe`



Run: `./a.exe`



```python

```


```python

```

Немного больше о связи С++ и С


```cpp
%%cpp main.cpp
%run g++ -std=c++17 -Wall -Werror -fsanitize=address -fno-exceptions -fno-rtti main.cpp -o a.exe
%run ./a.exe 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

struct stack_t {
    int* a;
    int sz;
    int max_sz;
    
    stack_t();
    ~stack_t();
    void push(int elem);
    int top();
    void pop();
}; 

// Если упрощенно, то код в этом блоке компилится как сишный, но при этом он связан с окружающим плюсовым
extern "C" {
    void _ZN7stack_tC1Ev(stack_t* s) {
        s->a = nullptr; 
        s->sz = 0;
        s->max_sz = 0;
    }
    
    void _ZN7stack_tD1Ev(stack_t* s) {
        free(s->a);
    }
    
    void _ZN7stack_t4pushEi(stack_t* s, int elem) {
        if (s->sz == s->max_sz) {
            s->max_sz += (s->max_sz == 0);
            s->max_sz *= 2;
            s->a = (int*)realloc(s->a, s->max_sz * sizeof(int));
        }
        s->a[s->sz++] = elem;
    }
    void _ZN7stack_t3popEv(stack_t* s) {
        --s->sz;
    }
    int _ZN7stack_t3topEv(stack_t* s) {
        return s->a[s->sz - 1];
    }
}

int main() {
    stack_t s;  
    s.push(123);           
    s.push(42);             
    assert(s.top() == 42);  
    s.pop();                
    assert(s.top() == 123);
    return 0; 
}
```


Run: `g++ -std=c++17 -Wall -Werror -fsanitize=address -fno-exceptions -fno-rtti main.cpp -o a.exe`



Run: `./a.exe`



```python
!objdump -t a.exe | grep stack_t
```

    00000000000015d2  w    F .text	00000000000001eb              _ZN7stack_t4pushEi
    000000000000187a  w    F .text	0000000000000057              _ZN7stack_t3popEv
    00000000000017be  w    F .text	00000000000000bb              _ZN7stack_t3topEv
    00000000000014ca  w    F .text	00000000000000c3              _ZN7stack_tC1Ev
    00000000000014ca  w    F .text	00000000000000c3              _ZN7stack_tC2Ev
    000000000000158e  w    F .text	0000000000000043              _ZN7stack_tD1Ev
    000000000000158e  w    F .text	0000000000000043              _ZN7stack_tD2Ev



```python

```


```python

```


```python

```

Перегрузка функций


```python

```

Перегрузка арифметических операторов 


```python

```

Стандартные контейнеры
vector/queue/priority_queue/set/map/unordered_*/


```python

```


```python

```


```python

```


```python

```
