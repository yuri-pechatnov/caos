// %%cpp program_asm_p.c
// %run arm-linux-gnueabi-gcc -marm program_asm_p.c -o program.exe
// %run qemu-arm ./program.exe ; echo 

#include <stdio.h>
#include <stdint.h>
#include <assert.h>


typedef struct {
    int8_t i8;
    int16_t i16;
} __attribute__((packed)) complicated_t;    
    
    
#define min(a, b) ((a) < (b) ? (a) : (b))

#ifdef C_MIN_IMPL
void complicated_min(complicated_t* a, complicated_t* b, complicated_t* c) {
    *c = (complicated_t){.i8 = min(a->i8, b->i8), .i16 = min(a->i16, b->i16)};
}
#else
void complicated_min(complicated_t* a, complicated_t* b, complicated_t* c);
__asm__ (R"(
.global complicated_min
complicated_min:
    ldrsb r3, [r0] // r3 = a->i8
    ldrsb ip, [r1] // ip = b->i8
    cmp r3, ip     // r3 ? ip
    movgt r3, ip   // if (r3 > ip) { r3 = ip }
    strb r3, [r2]  // c->i8 = r3
    
    ldrsh r3, [r0, #1] // r3 = a->i16
    ldrsh ip, [r1, #1] // ip = b->i16
    cmp r3, ip         // r3 ? ip
    movgt r3, ip       // if (r3 > ip) { r3 = ip }
    strh r3, [r2, #1]  // c->i16 = r3
    
    bx lr // return
)");
#endif

void test() {
    complicated_t a = {0}, b = {0}, c = {0};
    
    complicated_min(&a, &b, &c);
    assert(c.i8 == 0 && c.i16 == 0);
    
    a = (complicated_t){.i8 = -3, .i16 = -4};
    complicated_min(&a, &b, &c);
    assert(c.i8 == -3 && c.i16 == -4);
    
    b = (complicated_t){.i8 = -9, .i16 = 9};
    complicated_min(&a, &b, &c);
    assert(c.i8 == -9 && c.i16 == -4);
    
    b = (complicated_t){.i8 = 9, .i16 = -9};
    complicated_min(&a, &b, &c);
    assert(c.i8 == -3 && c.i16 == -9);
    
    printf("SUCCESS\n");
}

int main() {
    test();
    return 0;
}

