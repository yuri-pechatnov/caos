// %%cpp main.cpp
// %run g++ -std=c++17 -Wall -Werror -fsanitize=address -fno-exceptions -fno-rtti main.cpp -o a.exe
// %run ./a.exe 

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

