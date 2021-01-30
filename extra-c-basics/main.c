// %%cpp main.c
// %run clang -std=c99 -Wall -Werror -fsanitize=address main.c -o a.exe
// %run ./a.exe 

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

