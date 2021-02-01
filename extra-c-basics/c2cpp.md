



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
    new (s) stack_t;         // init_stack(s);
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
    // variant 3
    {
        stack_t* s = new stack_t[2];  
        s[0].push(123);           
        s[0].push(42);             
        assert(s[0].top() == 42);  
        s[0].pop();                
        assert(s[0].top() == 123);
        s[1].push(123);           
        s[1].push(42);             
        assert(s[1].top() == 42);  
        s[1].pop();                
        assert(s[1].top() == 123);
        delete[] s;
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
#include <new>
#include <string>

template <typename TElem>
struct stack_t {
    TElem* a;
    int sz;
    int max_sz;
    
    
    stack_t() {
        a = nullptr; 
        sz = 0;
        max_sz = 0;
    }

    ~stack_t() {
        while (sz > 0) {
            pop();
        }
        free(a);
    }
    
    void push(TElem elem)  {
        if (sz == max_sz) {
            max_sz += (max_sz == 0);
            max_sz *= 2;
            a = (TElem*)realloc((void*)a, max_sz * sizeof(TElem));
        }
        new (a + sz) TElem(elem);
        ++sz;
    }
    
    TElem top()  {
        return a[sz - 1];
    }
    
    void pop()  {
        a[--sz].~TElem();
    }
}; 

// template <typename TElem>
// struct queue_t {
//   ....
// };

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
    {
        stack_t<std::string> s;  
        s.push("Azaza");           
        s.push("Brekeke");             
        assert(s.top() == "Brekeke");  
        s.pop();                
        assert(s.top() == "Azaza");
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

    objdump: 'a.exe': No such file



```python

```

# Больше полезностей

https://ravesli.com/urok-192-std-move/

Тут вроде прилично написано про std::move

https://habr.com/ru/post/348198/
Вот тут про lvalue/rvalue и ссылочки всякие



```python

```

## Перегрузка функций


```cpp
%%cpp main.cpp
%run g++ -std=c++17 -Wall -Werror -fsanitize=address -fno-exceptions -fno-rtti main.cpp -o a.exe
%run ./a.exe 
%run objdump -t a.exe | grep sqr

#include <stdio.h>
#include <math.h>

int sqr(int a) {
    return a * a;
}

double sqr(double a) {
    return a * a;
}

int main() {
    printf("%d\n", sqr('a'));
    printf("%d\n", sqr(2));
    printf("%lf\n", sqr(3.0));
    return 0; 
}
```


Run: `g++ -std=c++17 -Wall -Werror -fsanitize=address -fno-exceptions -fno-rtti main.cpp -o a.exe`



Run: `./a.exe`


    9409
    4
    9.000000



Run: `objdump -t a.exe | grep sqr`


    00000000000011c9 g     F .text	0000000000000013              _Z3sqri
    00000000000011dc g     F .text	0000000000000018              _Z3sqrd



```python

```


```python

```

## Перегрузка арифметических операторов 


```cpp
%%cpp main.cpp
%run g++ -std=c++17 -Wall -Werror -fsanitize=address -fno-exceptions -fno-rtti main.cpp -o a.exe
%run ./a.exe 

#include <stdio.h>
#include <math.h>

struct vec_t {
    double x;
    double y;
    
    double norm() const {
        return std::sqrt(x * x + y * y);
    }
    vec_t operator+(const vec_t& b) const {
        return {this->x + b.x, this->y + b.y};
    }
    vec_t operator-() const {
        return {-x, -y};
    }
    void print() const {
        printf("{%lf, %lf}\n", x, y);
    }
};

vec_t operator*(const vec_t& a, double k) {
    return {a.x * k, a.y * k};
}

int main() {
    vec_t{1, 2}.print();
    vec_t a = {10, 20};
    vec_t b = {100, 200};
    (a + b).print();
    (-a).print();
    (a * -2).print();
    return 0; 
}
```


Run: `g++ -std=c++17 -Wall -Werror -fsanitize=address -fno-exceptions -fno-rtti main.cpp -o a.exe`



Run: `./a.exe`


    {1.000000, 2.000000}
    {110.000000, 220.000000}
    {-10.000000, -20.000000}
    {-20.000000, -40.000000}



```python

```

## Указатели, ссылки, объекты


```cpp
%%cpp main.cpp
%run g++ -std=c++17 -Wall -Werror -fsanitize=address -fno-exceptions -fno-rtti main.cpp -o a.exe
%run ./a.exe 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <new>
#include <string>

template <typename TElem>
struct stack_t {
    TElem* a;
    int sz;
    int max_sz;
    
    
    stack_t() {
        a = nullptr; 
        sz = 0;
        max_sz = 0;
    }
    stack_t(const stack_t& other): stack_t() {   
        *this = other;
    }
    stack_t(stack_t&& other): stack_t() {
        *this = std::move(other);
    }
    stack_t& operator=(const stack_t& other) {
        clear();
        for (int i = 0; i < other.sz; ++i) {
            push(other.a[i]);
        }
        return *this;
    }
    stack_t& operator=(stack_t&& other) {
        std::swap(a, other.a);
        std::swap(sz, other.sz);
        std::swap(max_sz, other.max_sz);
        other.clear();
        return *this;
    }
    
    void clear() {
        while (sz > 0) {
            pop();
        }
    }

    ~stack_t() {
        clear();
        free(a);
    }
    
    void push(TElem elem)  {
        if (sz == max_sz) {
            max_sz += (max_sz == 0);
            max_sz *= 2;
            a = (TElem*)realloc((void*)a, max_sz * sizeof(TElem));
        }
        new (a + sz) TElem(elem);
        ++sz;
    }
    
    TElem top()  {
        return a[sz - 1];
    }
    
    void pop()  {
        a[--sz].~TElem();
    }
}; 


stack_t<int> create() {
    stack_t<int> s;
    s.push(1);
    return s;
}

int main() {
    {
        stack_t<int> s;  
        s.push(123);           
        s.push(42);
        stack_t<int> s2 = s;
        
        assert(s.top() == 42);  
        s.pop();                
        assert(s.top() == 123);
        
        assert(s2.top() == 42);  
        s2.pop();                
        assert(s2.top() == 123);
    }
    {
        stack_t<int> s;  
        s.push(123);           
        s.push(42);
        stack_t<int> s2 = std::move(s);
        
        assert(s.sz == 0);
        
        assert(s2.top() == 42);  
        s2.pop();                
        assert(s2.top() == 123);
    }
    {
        stack_t<int> s;  
        s.push(123);           
        s.push(42);
        s = create();
        
        assert(s.sz == 1);
        assert(s.top() == 1);
    }
    return 0; 
}
```


Run: `g++ -std=c++17 -Wall -Werror -fsanitize=address -fno-exceptions -fno-rtti main.cpp -o a.exe`



Run: `./a.exe`





```python

```

Стандартные контейнеры
vector/queue/priority_queue/set/map/unordered_*/string

Самая нормальная документация, что я знаю тут: https://en.cppreference.com/w/


```python
  
```

# HW

Реализовать на С++ очередь (циклическую/на двух стеках/на указателях)

Опционально шаблонную

Опционально с хорошими конструкторами и операторами присваивания

Как сдавать - выложить на pastebin и прислать в лс

Дедлайн 7 февраля 23:00.



```python

```


```python

```
