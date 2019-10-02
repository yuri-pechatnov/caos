
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

typedef struct {
    char c;
    int i;
    char c2;
} Obj1_t;

typedef struct {
    char c;
    int i;
    char c2;
} __attribute__((packed)) Obj2_t;

typedef struct {
    char c8;
    uint64_t u64;
} Obj3_t;

#define print_int(x) printf(#x " = %d\n", (int)x)

#define print_offset(type, field) {\
    type o; \
    printf("Shift of ." #field " in " #type ": %d\n", (int)((void*)&o.field - (void*)&o)); \
}

int main() {
    print_int(sizeof(Obj1_t));
    
    print_offset(Obj1_t, c);
    print_offset(Obj1_t, i);
    print_offset(Obj1_t, c2);
    
    print_int(sizeof(Obj2_t));
    print_offset(Obj2_t, c);
    print_offset(Obj2_t, i);
    print_offset(Obj2_t, c2);
    
    print_int(sizeof(Obj3_t));
    print_offset(Obj3_t, u64);
    print_offset(Obj3_t, c8);
    return 0;
}

