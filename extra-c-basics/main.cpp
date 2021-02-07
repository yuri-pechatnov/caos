// %%cpp main.cpp
// %run g++ -std=c++17 -Wall -Werror -fsanitize=address -fno-exceptions -fno-rtti main.cpp -o a.exe
// %run ./a.exe 

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
                new (new_a + i) TElem(std::move(a[i]));
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

