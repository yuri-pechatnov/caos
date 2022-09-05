// %%cpp structs_in_memory.c
// %run arm-linux-gnueabi-gcc -marm structs_in_memory.c -o structs_in_memory.exe
// %run qemu-arm ./structs_in_memory.exe

#include "structs_in_memory_common.h"

int main() {
    typedef struct { // максимальное выравнивание у инта, значит выравнивание структуры 4
        char c;      // 0 байт
        int i;       // 4-7 байты
        char c2;     // 8 байт
    } Obj1_t;        // 9-11 - padding байты, чтобы размер делился на выравнивание
    print_info(Obj1_t);
    print_offset(Obj1_t, c);
    print_offset(Obj1_t, i);
    print_offset(Obj1_t, c2);
    
    typedef struct { // тут все правила про выравнивание не применимы, так как указан аттрибут упаковки
        char c;
        int i;
        char c2;
    } __attribute__((packed)) Obj2_t;
    print_info(Obj2_t);
    print_offset(Obj2_t, c);
    print_offset(Obj2_t, i);
    print_offset(Obj2_t, c2);
    
    return 0;
}

