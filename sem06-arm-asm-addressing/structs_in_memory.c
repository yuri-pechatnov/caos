// %%cpp structs_in_memory.c
// %run arm-linux-gnueabi-gcc -marm structs_in_memory.c -o structs_in_memory.exe
// %run qemu-arm ./structs_in_memory.exe

#include "structs_in_memory_common.h"

int main() {  
    typedef struct {     // тут пример двух структур равного размера, но с разным выравниванием
        long long a;
    } ObjS8A8;
    print_info(ObjS8A8);
    typedef struct {
        int a;
        int b;
    } ObjS8A4;
    print_info(ObjS8A4);
    
    typedef struct {    // и вот тут разное выравнивание ObjS8A8 и ObjS8A4 себя покажет
        ObjS8A8 o;
        char c;
    } Obj5_t;    
    print_info(Obj5_t); // обратите внимание на разницу с Obj6_t!

    typedef struct {
        ObjS8A4 o;
        char c;
    } Obj6_t;
    print_info(Obj6_t);
    
    typedef union {
        unsigned long long u;
        int i[3];
    } Obj7_t;
    print_info(Obj7_t); // то же самое, что и с Obj5_t
    
    return 0;
}

