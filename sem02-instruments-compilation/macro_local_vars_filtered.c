
#include <stdio.h>
#include <stdlib.h>

struct defer_record {
    struct defer_record* previous;
    void (*func) (void*);
    void* arg;
};

#define _EXECUTE_DEFERRED(to) do { \
    while (last_defer_record != to) { \
        last_defer_record->func(last_defer_record->arg); \
        last_defer_record = last_defer_record->previous; \
    } \
} while (0)

#define DEFER(func, arg) \
    struct defer_record defer_record_##__LINE__ = {last_defer_record, func, arg}; \
    last_defer_record = &defer_record_##__LINE__;


#define DEFER_FRIENDLY_BLOCK(...) { \
    struct defer_record* first_defer_record = last_defer_record; \
    { \
        struct defer_record* last_defer_record = first_defer_record; \
        __VA_ARGS__ \
        _EXECUTE_DEFERRED(first_defer_record); \
    } \
} 

#define DEFER_FRIENDLY_FUNCTION_BLOCK(...) { \
    struct defer_record* last_defer_record = NULL; \
    DEFER_FRIENDLY_BLOCK(__VA_ARGS__); \
} 

#define DEFER_FRIENDLY_BREAKABLE_BLOCK(...) { \
    struct defer_record* first_breakable_defer_record = last_defer_record; \
    DEFER_FRIENDLY_BLOCK(__VA_ARGS__) \
} 

#define DEFER_FRIENDLY_RETURN(value) do { \
    _EXECUTE_DEFERRED(NULL); \
    return value; \
} while (0)

#define DEFER_FRIENDLY_BREAK do { \
    _EXECUTE_DEFERRED(first_breakable_defer_record); \
    break; \
} while (0)



void func(int i) DEFER_FRIENDLY_FUNCTION_BLOCK(
    void* data = malloc(145);
    DEFER(free, data);
    void* data2 = malloc(14);
    DEFER(free, data2);
    if (i % 10 == 0) {
        DEFER_FRIENDLY_RETURN(0);
    }
)

int main() DEFER_FRIENDLY_FUNCTION_BLOCK(
        
    void* data = malloc(145);
    DEFER(free, data);
    
    DEFER_FRIENDLY_BLOCK(    
        void* data = malloc(145);
        DEFER(free, data);
    )
    
    for (int i = 0; i < 100; ++i) DEFER_FRIENDLY_BREAKABLE_BLOCK(    
        void* data = malloc(145);
        DEFER(free, data);
        if (i % 10 == 0) {
            DEFER_FRIENDLY_BREAK;
        }    
    )
    
    DEFER_FRIENDLY_RETURN(0);
)

