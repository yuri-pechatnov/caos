

# ARM ASM ADDRESSING

<table width=100%> <tr>
    <th width=20%> <b>Видеозапись семинара &rarr; </b> </th>
    <th>
    <a href="https://www.">
        <img src="vide.jpg" width="320"  height="160" align="left" alt="Видео с семинара"> 
    </a>
    </th>
    <th> </th>
 </table>

[Ридинг Яковлева: Адресация данных в памяти и использование библиотечных функций](https://github.com/victor-yacovlev/mipt-diht-caos/tree/master/practice/arm_globals_plt)

Сегодня в программе:
* <a href="#array" style="color:#856024"> Пример работы с массивом из ассемблера </a>
* <a href="#ldr_str" style="color:#856024">  Разные варианты STR, LDR </a>
* <a href="#placement" style="color:#856024"> Размещение структур в памяти </a>
* <a href="#byte" style="color:#856024">  Загрузка и сохранение в память 1/2/4/8 байтных целых чисел </a>
* <a href="#placement" style="color:#856024"> Размещение структур в памяти </a>




```python

```


```python
import os

linaro_download_dir = os.path.expanduser("~/arm/gcc-linaro-7.3.1-2018.05-i686_arm-linux-gnueabi")

gcc_linaro_path = os.path.join(linaro_download_dir, "bin")
if gcc_linaro_path not in os.environ["PATH"]:
    os.environ["PATH"] += ":" + gcc_linaro_path

os.environ["QEMU_LD_PREFIX"] = os.path.join(linaro_download_dir, "arm-linux-gnueabi/libc")    
```

# <a name="array"></a> Пример работы с массивом из ассемблера


```cpp
%%cpp lib.c
%run arm-linux-gnueabi-gcc -c -Os -fno-asynchronous-unwind-tables -marm lib.c -o lib.o
%run arm-linux-gnueabi-objdump -D lib.o | grep '<sum_v>' -A 14

int sum_v(int* a, int* b, int n) {
    for (int i = 0; i < n; ++i) {
        a[i] += b[i];
    }
}
```


```python

```


```cpp
%%cpp test_sum_v.c
%run arm-linux-gnueabi-gcc -marm -fsanitize=address lib.c test_sum_v.c -o test_sum_v.exe
%run qemu-arm ./test_sum_v.exe

#include <stdio.h>
#include <assert.h>

int sum_v(int* a, int* b, int n);

void test() {
    {
        int a[] = {1, 2, 3};
        int b[] = {10, 20, 30};
        sum_v(a, b, 3);
        assert(a[0] == 11);
        assert(a[1] == 22);
        assert(a[2] == 33);
    }
    {
        int a[] = {};
        int b[] = {};
        sum_v(a, b, 0);    
    }
    printf("SUCCESS\n");
}

int main() {
    test();
}
```

### Вариант 1: причесанный дизассемблер


```cpp
%%cpp lib.S
%run arm-linux-gnueabi-gcc -marm -fsanitize=address lib.S test_sum_v.c -o test_sum_v.exe
%run qemu-arm ./test_sum_v.exe

.global sum_v
sum_v:
    mov r3, #0
    cmp r3, r2
    blt long_path // if (else block)
        bx lr
    long_path:    // if (first block)
        push {lr}
        while_start: // while
            ldr ip, [r0, r3, lsl #2] // C-style: ip = *(r0 + (r3 << 2))
            ldr lr, [r1, r3, lsl #2]
            add ip, ip, lr
            str ip, [r0, r3, lsl #2] // C-style: *(r0 + (r3 << 2)) = ip
            add r3, r3, #1
            cmp r3, r2
            blt while_start
        pop {pc}
```

### Вариант 2: c нуля, придерживаясь сишной структуры кода


```cpp
%%cpp lib.S
%run arm-linux-gnueabi-gcc -marm -fsanitize=address lib.S test_sum_v.c -o test_sum_v.exe
%run qemu-arm ./test_sum_v.exe

.global sum_v
sum_v:
    // C-style: r0 = a, r1 = b, r3 = n
    mov r3, #4 // sizeof(int)
    mla r2, r2, r3, r0 // C-style: r3 = (a + n)
    while_start: // while
        cmp r0, r2
        beq while_end
            ldr ip, [r0] // C-style: ip = *r0
            ldr r3, [r1] // C-style: r3 = *r1
            add r3, r3, ip // C-style: r3 += ip
            str r3, [r0] // C-style: *r0 = r3
            add r0, r0, #4
            add r1, r1, #4
        b while_start
    while_end: // while end
    bx lr
```

### Вариант 3: оптимизируем


```cpp
%%cpp lib.S
%run arm-linux-gnueabi-gcc -marm -fsanitize=address lib.S test_sum_v.c -o test_sum_v.exe
%run qemu-arm ./test_sum_v.exe

.global sum_v
sum_v:
    // C-style: r0 = a, r1 = b, r3 = n
    add r2, r0, r2, lsl #2 // C-style: r2 = r0 + (r2 << 2)
    while_start: // while
        cmp r0, r2
        beq while_end
            ldr ip, [r0] // C-style: ip = *r0
            ldr r3, [r1], #4 // C-style: r3 = *r1, r1 += 4
            add r3, r3, ip // C-style: r3 += ip
            str r3, [r0], #4 // C-style: *r0 = r3, r0 += 4
        b while_start
    while_end: // while end
    bx lr
```

# <a name="ldr_str"></a> Разные варианты STR, LDR:


```cpp
%%cpp program_asm_p.c
%run arm-linux-gnueabi-gcc -marm program_asm_p.c -o program.exe
%run qemu-arm ./program.exe ; echo 

#include <stdio.h>
#include <assert.h>

int* load_1(int* a, int* b, int n);
__asm__ (R"(
.global load_1
load_1:
    ldr r3, [r0] // r3 = *r1
    str r3, [r1] // *b = r3
    bx lr
)");

int* load_2(int* a, int* b, int n);
__asm__ (R"(
.global load_2
load_2:
    ldr r3, [r0, r2, lsl #2] // r3 = *(r1 + r0 * 4)
    str r3, [r1] // *b = r3
    bx lr
)");

int* load_3(int* a, int* b, int n);
__asm__ (R"(
.global load_3
load_3:
    ldr r3, [r0, r2, lsl #2]! // r1 += r0 * 4; r3 = *r1
    str r3, [r1] // *b = r3
    bx lr
)");

int* load_4(int* a, int* b, int n);
__asm__ (R"(
.global load_4
load_4:
    ldr r3, [r0], r2, lsl #2 // r3 = *r1; r1 += r0 * 4
    str r3, [r1] // *b = r3
    bx lr
)");


int main() {
    int a[] = {10, 20, 30};
    int res;
    
    // Просто загрузить
    // ldr r3, [r0]
    assert(load_1(a, &res, 0) == a);
    assert(res == 10);
    
    // Загрузить по адресу со смещением
    // ldr r3, [r0, r2, lsl #2]
    assert(load_2(a, &res, 0) == a);
    assert(res == 10);
    assert(load_2(a, &res, 1) == a);
    assert(res == 20);
    
    // Загрузить по префикс-инкрементнутому адресу
    // ldr r3, [r0, r2, lsl #2]!
    assert(load_3(a, &res, 0) == a);
    assert(res == 10);
    assert(load_3(a, &res, 1) == a + 1); // (!)
    assert(res == 20);
    
    // Загрузить по постфикс-инкрементнутому адресу
    // ldr r3, [r0], r2, lsl #2
    assert(load_4(a, &res, 0) == a);
    assert(res == 10);
    assert(load_4(a, &res, 1) == a + 1); // (!)
    assert(res == 10); // (!)
        
    printf("SUCCESS\n");
    
    return 0;
}

```


```python

```

# <a name="placement"></a> Размещение структур в памяти

Не все всегда расположено очевидным образом: для более эффективного выполнения программы бывает выгодно выровненное расположение объектов в памяти, например считывать ui64 из памяти выгоднее, если адрес делится на 8.

Примерные правила:
* выравнивание (то, на что адрес должен делиться) равно размеру для простых арифметических типов (указатели тоже здесь)
* в union берется максимум для выравнивания (и максимум из размеров округенный вверх, чтобы делиться на выравнивание)
* в struct члены располагаются в том порядке, в котором указаны. Выравнивание структуры - максимум выравниваний. Каждый член располагается так, чтобы удовлетворять собственному выравниванию. Итоговый размер структуры делится на выравнивание структуры. С учетом этого размер струкуры минимизируется.

Для экспериментов можно использовать `sizeof()` и `_Alignof()`, чтобы получить размер и выравнивание.


```cpp
%%cpp structs_in_memory_common.h

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#define print_int(x) printf(#x " = %d\n", (int)x)

#define print_info(x) printf("%10s: size = %d, alignment = %d\n", #x, sizeof(x), _Alignof(x))

#define print_offset(type, field) {\
    type o; \
    printf("  %10s: shift of ." #field " is %d\n", #type, (int)((void*)&o.field - (void*)&o)); \
}
```


```cpp
%%cpp structs_in_memory.c
%run arm-linux-gnueabi-gcc -marm structs_in_memory.c -o structs_in_memory.exe
%run qemu-arm ./structs_in_memory.exe

#include "structs_in_memory_common.h"

int main() {
    print_info(char);
    print_info(short);
    print_info(int);
    print_info(long long);
    print_info(double); 
    return 0;
}
```


```cpp
%%cpp structs_in_memory.c
%run arm-linux-gnueabi-gcc -marm structs_in_memory.c -o structs_in_memory.exe
%run qemu-arm ./structs_in_memory.exe

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
```


```cpp
%%cpp structs_in_memory.c
%run arm-linux-gnueabi-gcc -marm structs_in_memory.c -o structs_in_memory.exe
%run qemu-arm ./structs_in_memory.exe

#include "structs_in_memory_common.h"

int main() {
    
    typedef struct {  // максимальное выравнивание члена - 8, так что и у всей структуры такое же
        char c8;      // 0 байт 
        uint64_t u64; // 8-15 байты
    } Obj3_t;         // всего 16 байт, выравнивание 8
    print_info(Obj3_t);
    print_offset(Obj3_t, c8);
    print_offset(Obj3_t, u64);
    
    
    typedef struct {
        char c8;
        char c8_1;
        char c8_2;
    } Obj4_t;
    print_info(Obj4_t);
    print_offset(Obj4_t, c8);
    print_offset(Obj4_t, c8_1);
    print_offset(Obj4_t, c8_2);
    
    return 0;
}
```


```cpp
%%cpp structs_in_memory.c
%run arm-linux-gnueabi-gcc -marm structs_in_memory.c -o structs_in_memory.exe
%run qemu-arm ./structs_in_memory.exe

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
```


```python

```


```python

```


```python

```


```python

```

## <a name="byte"></a> Загрузка и сохранение в память 1/2/4/8 байтных целых чисел


```cpp
%%cpp lib.c
%run arm-linux-gnueabi-gcc -S -Os -fno-asynchronous-unwind-tables -marm lib.c -o /dev/stdout

#include <stdint.h>

typedef struct {
    uint8_t u8;
    uint16_t u16;
    uint32_t u32;
    uint64_t u64;
}  __attribute__((packed)) complicated_t;    
    
    
int parse(complicated_t* a, uint8_t* du8, uint16_t* du16, uint32_t* du32, uint64_t* du64) {
    *du8 = a->u8;
    *du16 = a->u16;
    *du32 = a->u32;
    *du64 = a->u64;
}

```


```cpp
%%cpp lib.c
%run arm-linux-gnueabi-gcc -S -Os -fno-asynchronous-unwind-tables -marm lib.c -o /dev/stdout

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

```

**Наблюдение:** в знаковом и беззнаковом случаях для сохранения используется одна и та же команда. А для загрузки разная.


```python

```

### Поэлементный минимум полей нетривиальной структуры.


```cpp
%%cpp program_asm_p.c
%run arm-linux-gnueabi-gcc -marm program_asm_p.c -o program.exe
%run qemu-arm ./program.exe ; echo 

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
    *c = (complicated_t){min(a->i8, b->i8), min(a->i16, b->i16)};
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

```


```python

```


```python

```


```python

```
