

# Ints & Floats

<table width=100%> <tr>
    <th width=20%> <b>Видеозапись семинара &rarr; </b> </th>
    <th>
    <a href="https://youtu.be/E0lg8pzzR7o">
        <img src="video.png" width="320"  height="160" align="left" alt="Видео с семинара"> 
    </a>
    </th>
    <th> </th>
 </table>


[Ридинг Яковлева: Целочисленная арифметика](https://github.com/victor-yacovlev/mipt-diht-caos/tree/master/practice/integers) 
<br>[Ридинг Яковлева: Вещественная арифметика](https://github.com/victor-yacovlev/mipt-diht-caos/tree/master/practice/ieee754) 

Сегодня в программе:
* <a href="#int" style="color:#856024"> Целые числа </a>
* <a href="#float" style="color:#856024"> Вещественные числа </a>

## <a name="int"></a> Целые числа

Производя `+`, `-`, `*` со стандартными целочисленными типами в программе мы работаем $\mathbb{Z}_{2^k}$, где $k$ - количество бит в числе. Причем это верно как со знаковыми, так и беззнаковыми числами.

В процессоре для сложения знаковых и беззнаковых чисел выполняется одна и та же инструкция.


```python
k = 3 # min 0, max 7 = (1 << 3) - 1
m = (1 << k)

def normalize(x):
    return ((x % m) + m) % m

def format_n(x):
    x = normalize(x)
    return "unsigned %d, signed % 2d, bytes %s" % (x, x if x < (m >> 1) else x - m, bin(x + m)[3:])
    
for i in range(0, m):
    print("i=%d -> %s" % (i, format_n(i)))
```


```python
def show_add(a, b):
    print("%d + %d = %d" % (a, b, a + b))
    print("    (%s) + (%s) = (%s)" % (format_n(a), format_n(b), format_n(a + b)))
show_add(2, 1)
show_add(2, -1)
```


```python
def show_mul(a, b):
    print("%d * %d = %d" % (a, b, a * b))
    print("    (%s) * (%s) = (%s)" % (format_n(a), format_n(b), format_n(a * b)))
show_mul(2, 3)
show_mul(-2, -3)
show_mul(-1, -1)
```

Но есть некоторые тонкости. 

Если вы пишете код на C/C++ то компилятор считает **переполнение знакового типа UB** (undefined behavior).

А переполнение беззнакового типа - законной операцией, при которой просто отбрасываются старшие биты (или значение берется по модулю $2^k$, или просто операция производится в $\mathbb{Z}_{2^k}$ - можете выбирать более удобный для вас способ на это смотреть).


```cpp
%%cpp lib.c
%run gcc -O3 -shared -fPIC lib.c -o lib.so

int check_increment(int x) {
    return x + 1 > x;
}

int unsigned_check_increment(unsigned int x) {
    return x + 1 > x;
}
```


```python
import ctypes

int32_max = (1 << 31) - 1
uint32_max = (1 << 32) - 1

lib = ctypes.CDLL("./lib.so")
lib.check_increment.argtypes = [ctypes.c_int]
lib.unsigned_check_increment.argtypes = [ctypes.c_uint]

%p lib.check_increment(1)
%p lib.check_increment(int32_max)

%p lib.unsigned_check_increment(1)
%p lib.unsigned_check_increment(uint32_max)
```


```python
!gdb lib.so -batch -ex="disass check_increment" -ex="disass unsigned_check_increment"
```


```python
# UB санитайзер в gcc этого не ловит :|
# А вот clang молодец :)
!clang -O0 -shared -fPIC -fsanitize=undefined lib.c -o lib_ubsan.so
```


```python
%%save_file run_ub.py
%run LD_PRELOAD=$(gcc -print-file-name=libubsan.so) python3 run_ub.py

import ctypes 

int32_max = (1 << 31) - 1

lib = ctypes.CDLL("./lib_ubsan.so")
lib.check_increment.argtypes = [ctypes.c_int]

print(lib.check_increment(int32_max))
```


```python

```

Иногда хочется обрабатывать переполнения разумным образом, например, насыщением:


```cpp
%%cpp main.c
%run gcc -O3 main.c -o a.exe 
%run ./a.exe

#include <assert.h>
#include <stdint.h>

unsigned int satsum(unsigned int x, unsigned int y) {
    unsigned int z;
    // Функция, которая обрабатывает выставленный процессором флаг и возвращает его явно
    if (__builtin_uadd_overflow(x, y, &z)) {
        return -1;
    }
    return z;
}

int main() {
    assert(satsum(2000000000L, 2000000000L) == 4000000000L);
    assert(satsum(4000000000L, 4000000000L) == (unsigned int)-1);
    return 0;
}
```


```python

```

Для операций сравнения и деления целых чисел уже однозначно важно, знаковые они или нет.


```cpp
%%cpp lib2.c
%run gcc -O3 -shared -fPIC lib2.c -o lib2.so

typedef unsigned int uint;

int sum(int x, int y) { return x + y; }
uint usum(uint x, uint y) { return x + y; }

int mul(int x, int y) { return x * y; }
uint umul(uint x, uint y) { return x * y; }

int cmp(int x, int y) { return x < y; }
int ucmp(uint x, uint y) { return x < y; }

int div(int x, int y) { return x / y; }
int udiv(uint x, uint y) { return x / y; }
```


```python
# Функции sum и usum идентичны
!gdb lib2.so -batch -ex="disass sum" -ex="disass usum"
```


```python
!gdb lib2.so -batch -ex="disass mul" -ex="disass umul"
```


```python
# Функции cmp и ucmp отличаются!
!gdb lib2.so -batch -ex="disass cmp" -ex="disass ucmp"
```


```python
!gdb lib2.so -batch -ex="disass div" -ex="disass udiv"
```


```python

```

## <a name="size"></a> Про размеры int'ов и знаковость

Установка всякого-разного

Для `-m32`

`sudo apt-get install g++-multilib libc6-dev-i386`

Для `qemu-arm`

`sudo apt-get install qemu-system-arm qemu-user`

`sudo apt-get install lib32z1`

Для сборки и запуска arm:

`wget http://releases.linaro.org/components/toolchain/binaries/7.3-2018.05/arm-linux-gnueabi/gcc-linaro-7.3.1-2018.05-i686_arm-linux-gnueabi.tar.xz`

`tar xvf gcc-linaro-7.3.1-2018.05-i686_arm-linux-gnueabi.tar.xz`




```python
# Add path to compilers to PATH
import os
os.environ["PATH"] = os.environ["PATH"] + ":" + \
    "/home/pechatnov/arm/gcc-linaro-7.3.1-2018.05-i686_arm-linux-gnueabi/bin"
```


```cpp
%%cpp size.c
%// Компилируем обычным образом
%run gcc size.c -o size.exe
%run ./size.exe
%// Под 32-битную архитектуру
%run gcc -m32 size.c -o size.exe
%run ./size.exe
%// Под ARM
%run arm-linux-gnueabi-gcc -marm size.c -o size.exe
%run qemu-arm -L ~/arm/gcc-linaro-7.3.1-2018.05-i686_arm-linux-gnueabi/arm-linux-gnueabi/libc ./size.exe


#include <stdio.h>

int main() {
    printf("is char signed = %d, ", (int)((char)(-1) > 0));
    printf("sizeof(long int) = %d\n", (int)sizeof(long int));
}
```

Какой из этого можно сделать вывод? Хотите понятный тип - используйте типы с детерминированым размером и знаковостью - uint64_t и другие подобные 


```python

```


```python

```

## <a name="bit"></a> Битовые операции

`^`, `|`, `&`, `~`, `>>`, `<<`


```python
a = 0b0110

def my_bin(x, digits=4):
    m = (1 << digits)
    x = ((x % m) + m) % m
    return bin(x + m).replace('0b1', '0b')

%p my_bin(     a)  # 4-битное число
%p my_bin(    ~a)  # Его побитовое отрицание
%p my_bin(a >> 1)  # Его сдвиг вправо на 1
%p my_bin(a << 1)  # Его сдвиг влево на 1
```


```python
x = 0b0011
y = 0b1001

%p my_bin(x    )  # X
%p my_bin(y    )  # Y

%p my_bin(x | y)  # Побитовый OR
%p my_bin(x ^ y)  # Побитовый XOR
%p my_bin(x & y)  # Побитовый AND
```


```python

```

Задачки:
* Получите из числа `a` `i`-ый бит 
* Выставьте в целом числе `a` `i`-ый бит 
* Занулите в целом числе `a` `i`-ый бит 
* Инвертируйте в целом числе `a` `i`-ый бит 
* Получите биты числа `a` с `i` по `j` невключительно как беззнаковое число
* Скопируйте в биты числа `a` с `i` по `j` невключительно младшие биты числа `b` 


```python

```


```python

```

## <a name="float"></a> Вещественные числа

Давайте просто посмотрим на битовые представления вещественных чисел и найдем закономерности :)


```cpp
%%cpp stand.h
// Можно не вникать, просто печаталка битиков

#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>

//#define EXTRA_INFO // включение более подробного вывода
#if defined(EXTRA_INFO)
    #define IS_VLINE_POINT(i) (i == 63 || i == 52)
    #define DESCRIBE(d) describe_double(d)
typedef union {
    double double_val;
    struct {
        uint64_t mantissa_val : 52;
        uint64_t exp_val : 11;
        uint64_t sign_val : 1;
    };
} double_parser_t;

void describe_double(double x) {
    double_parser_t parser = {.double_val = x};
    printf("  (-1)^%d * 2^(%d) * 0x1.%013llx", 
           (int)parser.sign_val, parser.exp_val - 1023, (long long unsigned int)parser.mantissa_val);
}

#else
    #define IS_VLINE_POINT(i) 0
    #define DESCRIBE(d) (void)(d)
#endif

inline uint64_t bits_of_double(double d) {
    uint64_t result;
    memcpy(&result, &d, sizeof(result));
    return result;
}

inline void print_doubles(double* dds) {
    char line_1[70] = {0}, line_2[70] = {0}, hline[70] = {0};
    int j = 0;
    for (int i = 63; i >= 0; --i) {
        line_1[j] = (i % 10 == 0) ? ('0' + (i / 10)) : ' ';
        line_2[j] = '0' + (i % 10);
        hline[j] = '-';
        ++j;
        if (IS_VLINE_POINT(i)) {
            line_1[j] = line_2[j] = '|';
            hline[j] = '-';
            ++j;
        }
    }
    printf("Bit numbers: %s\n", line_1);
    printf("             %s  (-1)^S * 2^(E-B) * (1+M/(2^Mbits))\n", line_2);
    printf("             %s\n", hline);
    for (double* d = dds; *d; ++d) {
        printf("%10.4lf   ", *d);
        uint64_t m = bits_of_double(*d);
        for (int i = 63; i >= 0; --i) {
            printf("%d", (int)((m >> i) & 1));
            if (IS_VLINE_POINT(i)) {
                printf("|");
            }
        }
        DESCRIBE(*d);
        printf("\n");
    }
}
```

##### Посмотрим на пары чисел x и -x


```cpp
%%cpp stand.cpp
%run gcc stand.cpp -o stand.exe
%run ./stand.exe

#include "stand.h"

int main() {
    double dd[] = {1, -1, 132, -132, 3.1415, -3.1415,  0};
    print_doubles(dd);
}
```

##### Посмотрим на степени 2-ки


```cpp
%%cpp stand.cpp
%run gcc stand.cpp -o stand.exe
%run ./stand.exe

#include "stand.h"

int main() {
    double dd[] = {0.125, 0.25, 0.5, 1, 2, 4, 8, 16, 0};
    print_doubles(dd);
}
```

##### Посмотрим на числа вида $ 1 + i \cdot 2^{(-k)}$


```cpp
%%cpp stand.cpp
%run gcc stand.cpp -o stand.exe
%run ./stand.exe

#include "stand.h"

int main() {
    double t8 = 1.0 / 8;
    double dd[] = {1 + 0 * t8, 1 + 1 * t8, 1 + 2 * t8, 1 + 3 * t8, 1 + 4 * t8, 
                   1 + 5 * t8, 1 + 6 * t8, 1 + 7 * t8, 0};
    print_doubles(dd);
}
```

##### Посмотрим на числа вида $ 1 + i \cdot 2^{(-k)}$


```cpp
%%cpp stand.cpp
%run gcc stand.cpp -o stand.exe
%run ./stand.exe

#include "stand.h"

int main() {
    double eps = 1.0 / (1LL << 52);
    double dd[] = {1 + 0 * eps, 1 + 1 * eps, 1 + 2 * eps, 1 + 3 * eps, 1 + 4 * eps, 0};
    print_doubles(dd);
}
```

##### Посмотрим на существенно разные значения double


```cpp
%%cpp stand.cpp
%run gcc stand.cpp -o stand.exe
%run ./stand.exe

#include <math.h>
#include "stand.h"

int main() {
    double dd[] = {1.5, NAN, -NAN, 0.0 / 0.0, INFINITY, -INFINITY, 0};
    print_doubles(dd);
}
```


```python

```

Я надеюсь по примерам вы уловили суть. Подробнее за теорией можно в 
[Ридинг Яковлева: Вещественная арифметика](https://github.com/victor-yacovlev/mipt-diht-caos/tree/master/practice/ieee754) 


```python

```


```python

```

# Дополнение про bitcast


```cpp
%%cpp bitcast.c
%run gcc -O2 -Wall bitcast.c -o bitcast.exe
%run ./bitcast.exe

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>

uint64_t bit_cast_memcpy(double d) {
    uint64_t result;
    memcpy(&result, &d, sizeof(result)); // Железобетонный способ, но чуть сложнее для оптимизатора
    return result;
}

typedef union {
    double double_val;
    uint64_t ui64_val;
} converter_t;

uint64_t bit_cast_union(double d) {
    return ((converter_t){.double_val = d}).ui64_val; // Вроде (?) хорошее решение
}

uint64_t bit_cast_ptr(double d) {
    return *(uint64_t*)(void*)&d; // Простое, но неоднозначное решение из-за алиасинга
}

int main() {
    double d = 3.15;
    printf("%" PRId64 "\n", bit_cast_memcpy(d));
    printf("%" PRId64 "\n", bit_cast_union(d));
    printf("%" PRId64 "\n", bit_cast_memcpy(d));
}
```


```python
!gdb bitcast.exe -batch -ex="disass bit_cast_memcpy" -ex="disass bit_cast_union" -ex="disass bit_cast_ptr"
```

Я бы рекомендовал использовать в таких случаях memcpy. [Так сделано в std::bit_cast](https://en.cppreference.com/w/cpp/numeric/bit_cast)

[Про C++ алиасинг, ловкие оптимизации и подлые баги / Хабр](https://habr.com/ru/post/114117/)


```python

```


```python

```


```python

```


```python

```
