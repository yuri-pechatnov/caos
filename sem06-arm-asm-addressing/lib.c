// %%cpp lib.c
// %run arm-linux-gnueabi-gcc -S -Os -fno-asynchronous-unwind-tables -marm lib.c -o /dev/stdout

#include <stdint.h>

typedef struct {
    int8_t i8;
    int16_t i16;
    int32_t i32;
    int64_t i64;
} __attribute__((packed)) complicated_t;    
    
    
int parse(complicated_t* a, int8_t* di8, int16_t* di16, int32_t* di32, int64_t* di64) {
    *di8 = a->i8;
    *di16 = a->i16;
    *di32 = a->i32;
    *di64 = a->i64;
}

