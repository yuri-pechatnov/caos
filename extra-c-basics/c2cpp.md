



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
            // На самом деле так нельзя в общем случае
            // не все объекты хорошо перенесут изменение своего адреса в памяти
            // a = (TElem*)realloc((void*)a, max_sz * sizeof(TElem));
            TElem* new_a = (TElem*)malloc(max_sz * sizeof(TElem));
            for (int i = 0; i < sz; ++i) {
                new (new_a + i) TElem(std::move(a[i])); // move-конструктором безопасно перемещаем объект в новую память
                a[i].~TElem();
            }
            a = new_a;
            
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
    {
        stack_t<stack_t<int>> s;  
        s.push(create());           
        s.push(create());
        s.pop();
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

# Задачки про порядок вызова конструкторов и деструкторов


```python

```


```cpp
%%cpp common.h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <new>
#include <string>

#define eprintf(...) fprintf(stderr, __VA_ARGS__)
#define logprintf_impl(fmt, line, ...) eprintf(__FILE__ ":" #line " " fmt, __VA_ARGS__)
#define logprintf_impl_2(fmt, line, ...) logprintf_impl(fmt, line, __VA_ARGS__)
#define logprintf(fmt, ...) logprintf_impl_2(fmt, __LINE__, __VA_ARGS__)


// Выделяем на sizeof(size_t) байт больше, чтобы явно сохранить размер выделяемого блока
void* operator new(size_t sz) {
    void* ptr = malloc(sz);
    printf("allocate %d bytes, addr=%p\n", (int)sz, (void*)ptr);
    return ptr;
}
// А здесь удаляем этот расширенный блок и выводим сохраненный размер
void operator delete(void* ptr, size_t sz) noexcept
{
    printf("deallocate %d bytes, addr=%p\n", (int)sz, ptr);
    free(ptr);
}

// размер структур совпадает с последней цифрой названия
struct obj4 {
    char data[4];
    obj4(obj4&&) { printf("construct (move) ojb4\n"); }
    obj4& operator=(obj4&&) { printf("assign (move) ojb4\n");  return *this; }
    obj4() { printf("construct ojb4\n"); }
    ~obj4() { printf("destruct ojb4\n"); }
};

struct obj5 {
    char data[5];
    obj5() { printf("construct ojb5\n"); }
    ~obj5() { printf("destruct ojb5\n"); }
};

struct obj10 {
    obj5 o5;
    obj4 o4;
    char data[1];

    obj10() { printf("construct ojb10\n"); }
    ~obj10() { printf("destruct ojb10\n"); }
};

struct obj20 {
    obj4* o4;
    obj5 o5;
    char data[7];
    obj20() { printf("construct ojb20\n"); o4 = new obj4; printf("end of construct ojb20\n"); }
    ~obj20() { printf("destruct ojb20\n"); delete o4; printf("end of destruct ojb20\n"); }
};

```


```python

```


```cpp
%%cpp main.cpp
%run g++ -std=c++17 -Wall -Werror -fsanitize=address -fno-exceptions -fno-rtti main.cpp -o a.exe
%run ./a.exe 

#include "common.h"

int main() {
    obj4 o4; // constructor
    return 0; 
    // destructor
}
```


Run: `g++ -std=c++17 -Wall -Werror -fsanitize=address -fno-exceptions -fno-rtti main.cpp -o a.exe`


    In file included from [01m[Kmain.cpp:5[m[K:
    [01m[Kcommon.h:[m[K In member function ‘[01m[Kobj4& obj4::operator=(obj4&&)[m[K’:
    [01m[Kcommon.h:33:63:[m[K [01;31m[Kerror: [m[Kno return statement in function returning non-void [[01;31m[K-Werror=return-type[m[K]
       33 |     obj4& operator=(obj4&&) { printf("assign (move) ojb4\n"); [01;31m[K}[m[K
          |                                                               [01;31m[K^[m[K
          |                                                               [32m[Kreturn *this;[m[K
    cc1plus: all warnings being treated as errors



Run: `./a.exe`


    allocate 24 bytes, addr=0x603000000010
    construct ojb5
    construct ojb20
    allocate 4 bytes, addr=0x602000000010
    construct ojb4
    end of construct ojb20
    destruct ojb20
    destruct ojb4
    deallocate 4 bytes, addr=0x602000000010
    end of destruct ojb20
    destruct ojb5
    deallocate 24 bytes, addr=0x603000000010



```python

```


```cpp
%%cpp main.cpp
%run g++ -std=c++17 -Wall -Werror -fsanitize=address -fno-exceptions -fno-rtti main.cpp -o a.exe
%run ./a.exe 

#include "common.h"

int main() {
    obj4* o4 = new obj4; // allocate, constructor obj4
    delete o4; // destructor, deallocate
    return 0; 
}
```


Run: `g++ -std=c++17 -Wall -Werror -fsanitize=address -fno-exceptions -fno-rtti main.cpp -o a.exe`


    In file included from [01m[Kmain.cpp:5[m[K:
    [01m[Kcommon.h:[m[K In member function ‘[01m[Kobj4& obj4::operator=(obj4&&)[m[K’:
    [01m[Kcommon.h:33:63:[m[K [01;31m[Kerror: [m[Kno return statement in function returning non-void [[01;31m[K-Werror=return-type[m[K]
       33 |     obj4& operator=(obj4&&) { printf("assign (move) ojb4\n"); [01;31m[K}[m[K
          |                                                               [01;31m[K^[m[K
          |                                                               [32m[Kreturn *this;[m[K
    cc1plus: all warnings being treated as errors



Run: `./a.exe`


    allocate 24 bytes, addr=0x603000000010
    construct ojb5
    construct ojb20
    allocate 4 bytes, addr=0x602000000010
    construct ojb4
    end of construct ojb20
    destruct ojb20
    destruct ojb4
    deallocate 4 bytes, addr=0x602000000010
    end of destruct ojb20
    destruct ojb5
    deallocate 24 bytes, addr=0x603000000010



```python

```


```cpp
%%cpp main.cpp
%run g++ -std=c++17 -Wall -Werror -fsanitize=address -fno-exceptions -fno-rtti main.cpp -o a.exe
%run ./a.exe 

#include "common.h"

int main() {
    obj10 o10;
    return 0; 
}
```


Run: `g++ -std=c++17 -Wall -Werror -fsanitize=address -fno-exceptions -fno-rtti main.cpp -o a.exe`



Run: `./a.exe`


    construct ojb5
    construct ojb4
    construct ojb10
    destruct ojb10
    destruct ojb4
    destruct ojb5



```python

```


```cpp
%%cpp main.cpp
%run g++ -std=c++17 -Wall -Werror -fsanitize=address -fno-exceptions -fno-rtti main.cpp -o a.exe
%run ./a.exe 

#include "common.h"

int main() {
    obj20 o20;
    return 0; 
}
```


Run: `g++ -std=c++17 -Wall -Werror -fsanitize=address -fno-exceptions -fno-rtti main.cpp -o a.exe`



Run: `./a.exe`


    construct ojb5
    construct ojb20
    allocate 4 bytes, addr=0x602000000010
    construct ojb4
    end of construct ojb20
    destruct ojb20
    destruct ojb4
    deallocate 4 bytes, addr=0x602000000010
    end of destruct ojb20
    destruct ojb5



```cpp
%%cpp main.cpp
%run g++ -std=c++17 -Wall -Werror -fsanitize=address -fno-exceptions -fno-rtti main.cpp -o a.exe
%run ./a.exe 

#include "common.h"

int main() {
    obj20* o20 = new obj20;
    delete o20;
    return 0; 
}
```


Run: `g++ -std=c++17 -Wall -Werror -fsanitize=address -fno-exceptions -fno-rtti main.cpp -o a.exe`



Run: `./a.exe`


    allocate 24 bytes, addr=0x603000000010
    construct ojb5
    construct ojb20
    allocate 4 bytes, addr=0x602000000010
    construct ojb4
    end of construct ojb20
    destruct ojb20
    destruct ojb4
    deallocate 4 bytes, addr=0x602000000010
    end of destruct ojb20
    destruct ojb5
    deallocate 24 bytes, addr=0x603000000010



```cpp
%%cpp main.cpp
%run g++ -std=c++17 -Wall -Werror -fsanitize=address -fno-exceptions -fno-rtti main.cpp -o a.exe
%run ./a.exe 

#include "common.h"

int main() {
    obj4 o4_1;
    obj4 o4_2;
    std::swap(o4_1, o4_2);
    return 0; 
}
```


Run: `g++ -std=c++17 -Wall -Werror -fsanitize=address -fno-exceptions -fno-rtti main.cpp -o a.exe`



Run: `./a.exe`


    construct ojb4
    construct ojb4
    construct (move) ojb4
    assign (move) ojb4
    assign (move) ojb4
    destruct ojb4
    destruct ojb4
    destruct ojb4



```python

```


```cpp
%%cpp lib.h

// void print42();
#include <stdio.h>

inline void print42() {
    printf("42\n");
}

template <typename T>
T min(T a, T b);
```


```cpp
%%cpp lib.cpp

#include "lib.h"
// #include <stdio.h>

// void print42() {
//     printf("42\n");
// }

template <typename T>
T min(T a, T b) {
    return (a > b) ? b : a;
}

// int f(int a, int b) {
//     return min(a, b);
// }

template 
int min<int> (int a, int b);
```


```cpp
%%cpp main.cpp
%run g++ main.cpp lib.cpp -o main.exe
%run ./main.exe

#include "lib.h"

int main() {
    print42();
    printf("%d\n", min(10, 30));
    return 0;
}
```


Run: `g++ main.cpp lib.cpp -o main.exe`



Run: `./main.exe`


    42
    10



```python

```


```cpp
%%cpp main.cpp
%run g++ main.cpp lib.cpp -o main.exe
%run echo "10.1 20.2" | ./main.exe

#include <stdio.h>
#include <iostream>
#include <iomanip>

struct point_t {
    double x;
    double y;

    point_t operator-(const point_t& b) const {
        return point_t{.x = x - b.x, .y = y - b.y};
    }
    
    point_t operator*(double k) const {
        return {.x = x * k, .y = y * k};
    }
    
    static point_t read(FILE* file) {
        point_t p;
        fscanf(file, "%lf%lf", &p.x, &p.y); 
        return p;
    }
    void write(FILE* file) const {
        fprintf(file, "{.x = %lf, .y = %lf}", x, y);
    }
};

std::istream& operator>>(std::istream& in, point_t& p) {
    return in >> p.x >> p.y;
}

std::ostream& operator<<(std::ostream& out, const point_t& p) {
    return out << "{" << std::fixed << std::setprecision(3) << p.x << ", " << p.y << "}";
}

int main() {
    //(point_t::read(stdin) * 2).write(stdout);
    point_t p;
    std::cin >> p;
    std::cout << (p * 2);
    return 0;
}

```


Run: `g++ main.cpp lib.cpp -o main.exe`



Run: `echo "10.1 20.2" | ./main.exe`


    {20.200, 40.400}


```cpp
%%cpp main.cpp
%run g++ main.cpp lib.cpp -o main.exe
%run ./main.exe

#include <stdio.h>
#include <iostream>
#include <memory>
#include <cassert>

template <typename T>
struct uniq_ptr {
    T* ptr;
    
    uniq_ptr() {
        ptr = nullptr;
    }
    
    explicit uniq_ptr(T* p) {
        ptr = p;
    }
    
    uniq_ptr(uniq_ptr<T>&& t) {
        ptr = t.ptr;
        t.ptr = nullptr;
    }
    
    uniq_ptr<T>& operator=(uniq_ptr<T>&& t) {
        reset();
        ptr = t.ptr;
        t.ptr = nullptr;
        return *this;
    }
    
    T* operator->() {
        assert(ptr && "*null");
        return ptr;
    }
    
    T& operator*() {
        assert(ptr && "*null");
        return *ptr;
    }
    
    void reset() {
        if (ptr) {
            delete ptr;
        }
    }
    
    ~uniq_ptr() {
        reset();
    }
};


struct obj_t {
    obj_t() { printf("construct obj_t\n"); }
    void touch() { printf("is being touched\n"); }
    ~obj_t() { printf("destruct obj_t\n"); }
};


uniq_ptr<obj_t> create_obj() {
    uniq_ptr<obj_t> obj(new obj_t);
    printf("0\n");
    return obj;
}

int main() {
    obj_t* t = NULL;
    free(t);
    uniq_ptr<obj_t> obj = create_obj();
   
    obj.ptr->touch();
    obj->touch();
    (*obj).touch();
    printf("2\n");
    
    return 0;
}

```


Run: `g++ main.cpp lib.cpp -o main.exe`



Run: `./main.exe`


    construct obj_t
    0
    is being touched
    is being touched
    is being touched
    2
    main.exe: main.cpp:35: T* uniq_ptr<T>::operator->() [with T = obj_t]: Assertion `ptr && "*null"' failed.
    Aborted (core dumped)



```python

```


```python

```


```cpp
%%cpp main.cpp
%run g++ main.cpp lib.cpp -o main.exe
%run ./main.exe

#include <stdio.h>
#include <iostream>
#include <memory>



struct obj_t {
    obj_t() { printf("construct obj_t\n"); }
    void touch() { printf("is being touched\n"); }
    ~obj_t() { printf("destruct obj_t\n"); }
};


int main() {
    obj_t* t = NULL;
    free(t);
    std::shared_ptr<obj_t> obj(new obj_t);
    std::shared_ptr<obj_t> obj2 = obj;
    obj->touch();
    (*obj).touch();
    printf("0\n");
    obj.reset();
    printf("1\n");
    obj2.reset();
    printf("2\n");
    return 0;
}

```


Run: `g++ main.cpp lib.cpp -o main.exe`



Run: `./main.exe`


    construct obj_t
    is being touched
    is being touched
    0
    1
    destruct obj_t
    2



```python

```


```cpp
%%cpp main.cpp
%run g++ main.cpp lib.cpp -o main.exe
%run ./main.exe

#include <stdio.h>
#include <iostream>
#include <memory>
#include <vector>
#include <algorithm>


int main() {
    std::vector<int> v;
    v.push_back(1);
    v[0];
    v.resize(2);
    v.push_back(2);
    v.push_back(5);
    v.pop_back();
    std::sort(v.begin(), v.end(), std::greater<>());
    std::sort(v.begin(), v.end(), [](int a, int b) {
        return a > b;
    });

    v.reserve(100);
    
    for (int* i = v.data(); i != v.data() + v.size(); ++i) {
        printf("%d ", *i);
    }
    printf("\n");
    
    for (std::vector<int>::iterator i = v.begin(); i != v.end(); ++i) {
        printf("%d ", *i);
    }
    printf("\n");
    
    for (auto i = v.begin(); i != v.end(); ++i) {
        printf("%d ", *i);
    }
    printf("\n");
    
    int a[232];
    
    for (auto i = std::begin(v); i != std::end(v); ++i) {
        printf("%d ", *i);
    }
    printf("\n");
    
        
    {
        auto&& __x = v;
        for (auto __i = std::begin(__x); __i != std::end(__x); ++__i) {
            auto& x = *__i;
            printf("%d ", x);
        }
    }
    printf("\n");


    for (int x : v) {
        printf("%d ", x);
    }
    printf("\n");
    
    return 0;
}

```


Run: `g++ main.cpp lib.cpp -o main.exe`



Run: `./main.exe`


    2 1 0 
    2 1 0 
    2 1 0 
    2 1 0 
    2 1 0 
    2 1 0 



```cpp
%%cpp main.cpp
%run g++ main.cpp lib.cpp -o main.exe
%run ./main.exe

#include <stdio.h>
#include <iostream>
#include <memory>
#include <vector>

namespace my {
    void f();
    
    void g();
}


namespace my {
    void f() {
        printf("X\n");
    }
    
    void g() {
        printf("Y\n");
        f();
    }
}


int main() {
    using my::g;
    g();
    return 0;
}

```


Run: `g++ main.cpp lib.cpp -o main.exe`



Run: `./main.exe`


    Y
    X



```python

```


```cpp
%%cpp main.cpp
%run g++ main.cpp lib.cpp -o main.exe
%run ./main.exe

#include <stdio.h>
#include <iostream>
#include <memory>
#include <set>


int main() {
    std::set<int> v;
    
    v.insert(4);
    v.insert(4);
    v.insert(5);
    
    if (v.count(4)) {
        printf("4 in set\n");
    }
    
    
    {
        auto it = v.find(4);
        if (it != v.end()) {
            printf("4 in set\n");
        }
    }
    



    for (int x : v) {
        printf("%d ", x);
    }
    printf("\n");
    
   
    return 0;
}

```


Run: `g++ main.cpp lib.cpp -o main.exe`



Run: `./main.exe`


    4 in set
    4 in set
    4 5 



```cpp
%%cpp main.cpp
%run g++ -std=c++17 main.cpp lib.cpp -o main.exe
%run ./main.exe

#include <stdio.h>
#include <iostream>
#include <memory>
#include <map>


int main() {
    std::map<int, int> v;
    
    v[0] = 0;
    v[1] = 10;
    v[2] = 20;
    v[10] = 100;
    
    printf("v[10] = %d\n", v[10]);
    
    if (v.count(4)) {
        printf("4 in set\n");
    }
   
    {
        auto [iter, success] = v.try_emplace(6, 60);
        if (success) {
            printf("6 added to set\n");
        } else {
            printf("6 NOT added to set\n");
        }
    }
   
    for (auto& [key, value] : v) {
        printf("(%d %d) ", key, value);
    }
    printf("\n");
    
   
    return 0;
}

```


Run: `g++ -std=c++17 main.cpp lib.cpp -o main.exe`



Run: `./main.exe`


    v[10] = 100
    6 added to set
    (0 0) (1 10) (2 20) (6 60) (10 100) 



```cpp
%%cpp main.cpp
%run g++ -std=c++17 main.cpp lib.cpp -o main.exe
%run ./main.exe

#include <stdio.h>
#include <iostream>
#include <memory>
#include <unordered_map>


int main() {
    std::unordered_map<int, int> v;
    
    v[0] = 0;
    v[1] = 10;
    v[2] = 20;
    v[10] = 100;
    
    printf("v[10] = %d\n", v[10]);
    
    if (v.count(4)) {
        printf("4 in set\n");
    }
   
    {
        auto [iter, success] = v.try_emplace(6, 60);
        if (success) {
            printf("6 added to set\n");
        } else {
            printf("6 NOT added to set\n");
        }
    }
   
    for (auto& [key, value] : v) {
        printf("(%d %d) ", key, value);
    }
    printf("\n");
    
   
    return 0;
}

```


Run: `g++ -std=c++17 main.cpp lib.cpp -o main.exe`



Run: `./main.exe`


    v[10] = 100
    6 added to set
    (6 60) (10 100) (2 20) (1 10) (0 0) 



```python

```


```python

```


```cpp
%%cpp main.cpp
%run g++ -std=c++17 main.cpp lib.cpp -o main.exe
%run ./main.exe

#include <stdio.h>
#include <iostream>
#include <memory>
#include <vector>
#include <unordered_map>


int main() {
    std::vector<std::string> names = {
        "x",
        "y",
        "x",
        "y",
        "z",
    };

    std::unordered_map<std::string, int> name_to_index(names.size());
    
    std::vector<int> indexes;
    indexes.reserve(names.size());
    
    for (auto& name : names) {
        auto [iter, success] = name_to_index.try_emplace(
            name, name_to_index.size());
        indexes.push_back(iter->second);
    }
   
    for (auto i : indexes) {
        printf("%d ", i);
    }
    printf("\n");
    
   
    return 0;
}

```


Run: `g++ -std=c++17 main.cpp lib.cpp -o main.exe`



Run: `./main.exe`


    0 1 0 1 2 



```cpp
%%cpp main.cpp
%run g++ -std=c++17 main.cpp lib.cpp -o main.exe
%run ./main.exe

#include <stdio.h>
#include <iostream>
#include <memory>
#include <vector>
#include <unordered_map>


template <typename T>
struct array2d_t {
    T* arr;
    int n;
    int m;
    
    array2d_t(int n_, int m_) {
        n = n_;
        m = m_;
        arr = new T[n * m];
        for (int i = 0; i < n * m; ++i) {
            arr[i] = {};
        }
    }
    
    T* operator[](int i) {
        return arr + i * m;
    }
    
    ~array2d_t() {
        delete[] arr;
    }
    
};


int main() {
    array2d_t<int> arr(5, 5);
    arr[3][3] = 142;
    
    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 5; ++j) {
            printf("%d ", arr[i][j]);
        }
        printf("\n");
    }
   
    return 0;
}

```


Run: `g++ -std=c++17 main.cpp lib.cpp -o main.exe`



Run: `./main.exe`


    0 0 0 0 0 
    0 0 0 0 0 
    0 0 0 0 0 
    0 0 0 142 0 
    0 0 0 0 0 



```cpp
%%cpp main.cpp
%run clang++ -std=c++17 -Wall -Werror -fsanitize=address main.cpp -o a.exe
%run ./a.exe 

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <variant>


template <typename T>
struct TErrorOr {
    std::variant<std::string, T> Variant;
    
    TErrorOr() {
        Variant.template emplace<0>("not defined");
    }
    TErrorOr(T value) {
        Variant.template emplace<1>(std::move(value));
    }
    
    bool IsOk() const {
        return Variant.index() == 1;
    }
    
    T& Value() {
        return std::get<1>(Variant);
    }
    
    std::string& Error() {
        return std::get<0>(Variant);
    }
};

template <typename T>
TErrorOr<T> CreateError(std::string str) {
    TErrorOr<T> err;
    err.Variant.template emplace<0>(std::move(str));
    return err;
}


TErrorOr<int> f(int a) {
    if (a > 40000)
        return CreateError<int>("a too big");
    return a * a; 
}

TErrorOr<int> f2(int a, int b) {
    TErrorOr<int> a2 = f(a);
    if (!a2.IsOk()) {
        return a2;
    }
    TErrorOr<int> b2 = f(b);
    if (!b2.IsOk()) {
        return b2;
    }
    if ((int64_t)a + b > 2000000000) {
        return CreateError<int>("a + b too big");
    }
    return a2.Value() + b2.Value();
}

int main() {
    {
        int a = 0;
        TErrorOr<int> x = f(a);
        if (!x.IsOk()) {
            printf("Error %s\n", x.Error().c_str());
        } else {
            printf("Success, res = %d\n", x.Value());    
        }
    }
    {
        int a = 1000;
        int b = 100000;
        TErrorOr<int> x = f2(a, b);
        if (!x.IsOk()) {
            printf("Error %s\n", x.Error().c_str());
        } else {
            printf("Success, res = %d\n", x.Value());    
        }
    }
    return 0;
}
```


Run: `clang++ -std=c++17 -Wall -Werror -fsanitize=address main.cpp -o a.exe`



Run: `./a.exe`


    Success, res = 0
    Error a too big



```python

```


```cpp
%%cpp main.cpp
%run clang++ -std=c++17 -Wall -Werror -fsanitize=address main.cpp -o a.exe
%run ./a.exe 

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <exception>
#include <stdexcept>


int f(int a) {
    if (a > 40000)
        throw std::runtime_error("a too big");
    return a * a; 
}

int f2(int a, int b) {
    int64_t res = f(a) + f(b);
    if (res > 2000000000) {
        throw std::runtime_error("a + b too big");
    }
    return res;
}

int main() {
    try {
        int a = 1000000;
        int x = f(a);
        printf("Success, res = %d\n", x);  
    } catch (const std::exception& e) {
        printf("Error: %s\n", e.what());  
    }
    return 0;
}
```


Run: `clang++ -std=c++17 -Wall -Werror -fsanitize=address main.cpp -o a.exe`



Run: `./a.exe`


    Error: a too big



```cpp
%%cpp main.cpp
%run clang++ -std=c++17 -Wall -Werror -fsanitize=address main.cpp -o a.exe
%run ./a.exe 

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <exception>
#include <stdexcept>


void f() {
    std::vector<int> x;
    throw std::runtime_error("XXX");
    std::vector<int> y;
}

int main() {
    std::vector<int> a;
    try {
        std::vector<int> b;
        f();
        std::vector<int> c;  
    } catch (const std::exception& e) {
        std::vector<int> d; 
    }
    return 0;
}
```


```python
swap для кастомных типов
```


```python

```


```cpp
%%cpp main.cpp
%run clang++ -std=c++20 -Wall -Werror -fsanitize=address main.cpp -o a.exe
%run ./a.exe 

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <exception>
#include <stdexcept>


struct A {
   // bool operator==(const A&) const = default;
};

int main() {
    A{} == A{};
    return 0;
}
```


Run: `clang++ -std=c++20 -Wall -Werror -fsanitize=address main.cpp -o a.exe`


    [1mmain.cpp:17:9: [0m[0;1;31merror: [0m[1minvalid operands to binary expression ('A' and 'A')[0m
        A{} == A{};
    [0;1;32m    ~~~ ^  ~~~
    [0m1 error generated.



Run: `./a.exe`



```python

```
