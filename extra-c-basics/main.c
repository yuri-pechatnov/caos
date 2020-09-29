// %%cpp main.c
// %run clang -std=c99 -Wall -Werror -fsanitize=address main.c -o a.exe
// %run ./a.exe 

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

