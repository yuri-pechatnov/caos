


# Ints & Floats

<table width=100%> <tr>
    <th width=20%> <b>Видеозапись семинара &rarr; </b> </th>
    <th>
    <a href="https://youtu.be/MTDgiATnXlc">
        <img src="video.jpg" width="320"  height="160" align="left" alt="Видео с семинара"> 
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

    i=0 -> unsigned 0, signed  0, bytes 000
    i=1 -> unsigned 1, signed  1, bytes 001
    i=2 -> unsigned 2, signed  2, bytes 010
    i=3 -> unsigned 3, signed  3, bytes 011
    i=4 -> unsigned 4, signed -4, bytes 100
    i=5 -> unsigned 5, signed -3, bytes 101
    i=6 -> unsigned 6, signed -2, bytes 110
    i=7 -> unsigned 7, signed -1, bytes 111



```python
def show_add(a, b):
    print("%d + %d = %d" % (a, b, a + b))
    print("    (%s) + (%s) = (%s)" % (format_n(a), format_n(b), format_n(a + b)))
show_add(2, 1)
show_add(2, -1)
```

    2 + 1 = 3
        (unsigned 2, signed  2, bytes 010) + (unsigned 1, signed  1, bytes 001) = (unsigned 3, signed  3, bytes 011)
    2 + -1 = 1
        (unsigned 2, signed  2, bytes 010) + (unsigned 7, signed -1, bytes 111) = (unsigned 1, signed  1, bytes 001)



```python
def show_mul(a, b):
    print("%d * %d = %d" % (a, b, a * b))
    print("    (%s) * (%s) = (%s)" % (format_n(a), format_n(b), format_n(a * b)))
show_mul(2, 3)
show_mul(-2, -3)
show_mul(-1, -1)
```

    2 * 3 = 6
        (unsigned 2, signed  2, bytes 010) * (unsigned 3, signed  3, bytes 011) = (unsigned 6, signed -2, bytes 110)
    -2 * -3 = 6
        (unsigned 6, signed -2, bytes 110) * (unsigned 5, signed -3, bytes 101) = (unsigned 6, signed -2, bytes 110)
    -1 * -1 = 1
        (unsigned 7, signed -1, bytes 111) * (unsigned 7, signed -1, bytes 111) = (unsigned 1, signed  1, bytes 001)


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


Run: `gcc -O3 -shared -fPIC lib.c -o lib.so`



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


lib.check_increment(1) = 1



lib.check_increment(int32_max) = 1



lib.unsigned_check_increment(1) = 1



lib.unsigned_check_increment(uint32_max) = 0



```python
!gdb lib.so -batch -ex="disass check_increment" -ex="disass unsigned_check_increment"
```

    Dump of assembler code for function check_increment:
       0x0000000000001100 <+0>:	endbr64 
       0x0000000000001104 <+4>:	mov    $0x1,%eax
       0x0000000000001109 <+9>:	retq   
    End of assembler dump.
    Dump of assembler code for function unsigned_check_increment:
       0x0000000000001110 <+0>:	endbr64 
       0x0000000000001114 <+4>:	xor    %eax,%eax
       0x0000000000001116 <+6>:	cmp    $0xffffffff,%edi
       0x0000000000001119 <+9>:	setne  %al
       0x000000000000111c <+12>:	retq   
    End of assembler dump.



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


Run: `LD_PRELOAD=$(gcc -print-file-name=libubsan.so) python3 run_ub.py`


    [1mlib.c:5:14:[1m[31m runtime error: [1m[0m[1msigned integer overflow: 2147483647 + 1 cannot be represented in type 'int'[1m[0m
    0



```python

```




    9




```cpp
%%cpp code_sample
// воображаемая ситуация, когда переполнение нежелательно
isize = 100000
n, m = 100000
for (int i = 0; i < isize && i < saturation_multiplication(n, m); ++i) {
    
}
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
        return ~0u;
    }
    return z;
}

int main() {
    assert(satsum(2000000000L, 2000000000L) == 4000000000L);
    assert(satsum(4000000000L, 4000000000L) == (unsigned int)-1);
    return 0;
}
```


Run: `gcc -O3 main.c -o a.exe`



Run: `./a.exe`



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


Run: `gcc -O3 -shared -fPIC lib2.c -o lib2.so`



```python
# Функции sum и usum идентичны
!gdb lib2.so -batch -ex="disass sum" -ex="disass usum"
```

    Dump of assembler code for function sum:
       0x0000000000001100 <+0>:	endbr64 
       0x0000000000001104 <+4>:	lea    (%rdi,%rsi,1),%eax
       0x0000000000001107 <+7>:	retq   
    End of assembler dump.
    Dump of assembler code for function usum:
       0x0000000000001110 <+0>:	endbr64 
       0x0000000000001114 <+4>:	lea    (%rdi,%rsi,1),%eax
       0x0000000000001117 <+7>:	retq   
    End of assembler dump.



```python
!gdb lib2.so -batch -ex="disass mul" -ex="disass umul"
```

    Dump of assembler code for function mul:
       0x0000000000001120 <+0>:	endbr64 
       0x0000000000001124 <+4>:	mov    %edi,%eax
       0x0000000000001126 <+6>:	imul   %esi,%eax
       0x0000000000001129 <+9>:	retq   
    End of assembler dump.
    Dump of assembler code for function umul:
       0x0000000000001130 <+0>:	endbr64 
       0x0000000000001134 <+4>:	mov    %edi,%eax
       0x0000000000001136 <+6>:	imul   %esi,%eax
       0x0000000000001139 <+9>:	retq   
    End of assembler dump.



```python
# Функции cmp и ucmp отличаются!
!gdb lib2.so -batch -ex="disass cmp" -ex="disass ucmp"
```

    Dump of assembler code for function cmp:
       0x0000000000001140 <+0>:	endbr64 
       0x0000000000001144 <+4>:	xor    %eax,%eax
       0x0000000000001146 <+6>:	cmp    %esi,%edi
       0x0000000000001148 <+8>:	setl   %al
       0x000000000000114b <+11>:	retq   
    End of assembler dump.
    Dump of assembler code for function ucmp:
       0x0000000000001150 <+0>:	endbr64 
       0x0000000000001154 <+4>:	xor    %eax,%eax
       0x0000000000001156 <+6>:	cmp    %esi,%edi
       0x0000000000001158 <+8>:	setb   %al
       0x000000000000115b <+11>:	retq   
    End of assembler dump.



```python
!gdb lib2.so -batch -ex="disass div" -ex="disass udiv"
```

    Dump of assembler code for function div:
       0x0000000000001160 <+0>:	endbr64 
       0x0000000000001164 <+4>:	mov    %edi,%eax
       0x0000000000001166 <+6>:	cltd   
       0x0000000000001167 <+7>:	idiv   %esi
       0x0000000000001169 <+9>:	retq   
    End of assembler dump.
    Dump of assembler code for function udiv:
       0x0000000000001170 <+0>:	endbr64 
       0x0000000000001174 <+4>:	mov    %edi,%eax
       0x0000000000001176 <+6>:	xor    %edx,%edx
       0x0000000000001178 <+8>:	div    %esi
       0x000000000000117a <+10>:	retq   
    End of assembler dump.



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


\#\#\#\# `Компилируем обычным образом`



Run: `gcc size.c -o size.exe`



Run: `./size.exe`


    is char signed = 0, sizeof(long int) = 8



\#\#\#\# `Под 32-битную архитектуру`



Run: `gcc -m32 size.c -o size.exe`



Run: `./size.exe`


    is char signed = 0, sizeof(long int) = 4



\#\#\#\# `Под ARM`



Run: `arm-linux-gnueabi-gcc -marm size.c -o size.exe`



Run: `qemu-arm -L ~/arm/gcc-linaro-7.3.1-2018.05-i686_arm-linux-gnueabi/arm-linux-gnueabi/libc ./size.exe`


    is char signed = 1, sizeof(long int) = 4


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


`my_bin(     a) = 0b0110`  # 4-битное число



`my_bin(    ~a) = 0b1001`  # Его побитовое отрицание



`my_bin(a >> 1) = 0b0011`  # Его сдвиг вправо на 1



`my_bin(a << 1) = 0b1100`  # Его сдвиг влево на 1



```python
x = 0b0011
y = 0b1001

%p my_bin(x    )  # X
%p my_bin(y    )  # Y

%p my_bin(x | y)  # Побитовый OR
%p my_bin(x ^ y)  # Побитовый XOR
%p my_bin(x & y)  # Побитовый AND
```


`my_bin(x    ) = 0b0011`  # X



`my_bin(y    ) = 0b1001`  # Y



`my_bin(x | y) = 0b1011`  # Побитовый OR



`my_bin(x ^ y) = 0b1010`  # Побитовый XOR



`my_bin(x & y) = 0b0001`  # Побитовый AND


Задачки:
1. Получите из числа `a` `i`-ый бит 
2. Выставьте в целом числе `a` `i`-ый бит 
3. Занулите в целом числе `a` `i`-ый бит 
4. Инвертируйте в целом числе `a` `i`-ый бит 
5. Получите биты числа `a` с `i` по `j` невключительно как беззнаковое число
6. Скопируйте в биты числа `a` с `i` по `j` невключительно младшие биты числа `b` 

<details> <summary> Решения с семинара </summary>
<pre> <code> 
1. (a >> i) & 1u
2. a | (1u << i)
3. a & ~(1u << i)
4. a ^ (1u << i)
5. (a >> i) & ((1u << (j - i)) - 1u)
6. 
i = 2, j = 5
       xxx
a=0b00011000
         yyy
b=0b01010101
    
m = (1u << (j - i)) - 1u
(a & ~(m << i)) | ((b & m) << i)
</code> </pre>
</details>

## <a name="float"></a> Вещественные числа

Давайте просто посмотрим на битовые представления вещественных чисел и найдем закономерности :)


```cpp
%%cpp stand.h
// Можно не вникать, просто печаталка битиков

#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>

#define EXTRA_INFO // включение более подробного вывода
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


Run: `gcc stand.cpp -o stand.exe`



Run: `./stand.exe`


    Bit numbers:  |  6        | 5         4         3         2         1         0
                 3|21098765432|1098765432109876543210987654321098765432109876543210  (-1)^S * 2^(E-B) * (1+M/(2^Mbits))
                 ------------------------------------------------------------------
        1.0000   0|01111111111|0000000000000000000000000000000000000000000000000000  (-1)^0 * 2^(0) * 0x1.0000000000000
       -1.0000   1|01111111111|0000000000000000000000000000000000000000000000000000  (-1)^1 * 2^(0) * 0x1.0000000000000
      132.0000   0|10000000110|0000100000000000000000000000000000000000000000000000  (-1)^0 * 2^(7) * 0x1.0800000000000
     -132.0000   1|10000000110|0000100000000000000000000000000000000000000000000000  (-1)^1 * 2^(7) * 0x1.0800000000000
        3.1415   0|10000000000|1001001000011100101011000000100000110001001001101111  (-1)^0 * 2^(1) * 0x1.921cac083126f
       -3.1415   1|10000000000|1001001000011100101011000000100000110001001001101111  (-1)^1 * 2^(1) * 0x1.921cac083126f


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


Run: `gcc stand.cpp -o stand.exe`



Run: `./stand.exe`


    Bit numbers:  |  6        | 5         4         3         2         1         0
                 3|21098765432|1098765432109876543210987654321098765432109876543210  (-1)^S * 2^(E-B) * (1+M/(2^Mbits))
                 ------------------------------------------------------------------
        0.1250   0|01111111100|0000000000000000000000000000000000000000000000000000  (-1)^0 * 2^(-3) * 0x1.0000000000000
        0.2500   0|01111111101|0000000000000000000000000000000000000000000000000000  (-1)^0 * 2^(-2) * 0x1.0000000000000
        0.5000   0|01111111110|0000000000000000000000000000000000000000000000000000  (-1)^0 * 2^(-1) * 0x1.0000000000000
        1.0000   0|01111111111|0000000000000000000000000000000000000000000000000000  (-1)^0 * 2^(0) * 0x1.0000000000000
        2.0000   0|10000000000|0000000000000000000000000000000000000000000000000000  (-1)^0 * 2^(1) * 0x1.0000000000000
        4.0000   0|10000000001|0000000000000000000000000000000000000000000000000000  (-1)^0 * 2^(2) * 0x1.0000000000000
        8.0000   0|10000000010|0000000000000000000000000000000000000000000000000000  (-1)^0 * 2^(3) * 0x1.0000000000000
       16.0000   0|10000000011|0000000000000000000000000000000000000000000000000000  (-1)^0 * 2^(4) * 0x1.0000000000000


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


Run: `gcc stand.cpp -o stand.exe`



Run: `./stand.exe`


    Bit numbers:  |  6        | 5         4         3         2         1         0
                 3|21098765432|1098765432109876543210987654321098765432109876543210  (-1)^S * 2^(E-B) * (1+M/(2^Mbits))
                 ------------------------------------------------------------------
        1.0000   0|01111111111|0000000000000000000000000000000000000000000000000000  (-1)^0 * 2^(0) * 0x1.0000000000000
        1.1250   0|01111111111|0010000000000000000000000000000000000000000000000000  (-1)^0 * 2^(0) * 0x1.2000000000000
        1.2500   0|01111111111|0100000000000000000000000000000000000000000000000000  (-1)^0 * 2^(0) * 0x1.4000000000000
        1.3750   0|01111111111|0110000000000000000000000000000000000000000000000000  (-1)^0 * 2^(0) * 0x1.6000000000000
        1.5000   0|01111111111|1000000000000000000000000000000000000000000000000000  (-1)^0 * 2^(0) * 0x1.8000000000000
        1.6250   0|01111111111|1010000000000000000000000000000000000000000000000000  (-1)^0 * 2^(0) * 0x1.a000000000000
        1.7500   0|01111111111|1100000000000000000000000000000000000000000000000000  (-1)^0 * 2^(0) * 0x1.c000000000000
        1.8750   0|01111111111|1110000000000000000000000000000000000000000000000000  (-1)^0 * 2^(0) * 0x1.e000000000000


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


Run: `gcc stand.cpp -o stand.exe`



Run: `./stand.exe`


    Bit numbers:  |  6        | 5         4         3         2         1         0
                 3|21098765432|1098765432109876543210987654321098765432109876543210  (-1)^S * 2^(E-B) * (1+M/(2^Mbits))
                 ------------------------------------------------------------------
        1.0000   0|01111111111|0000000000000000000000000000000000000000000000000000  (-1)^0 * 2^(0) * 0x1.0000000000000
        1.0000   0|01111111111|0000000000000000000000000000000000000000000000000001  (-1)^0 * 2^(0) * 0x1.0000000000001
        1.0000   0|01111111111|0000000000000000000000000000000000000000000000000010  (-1)^0 * 2^(0) * 0x1.0000000000002
        1.0000   0|01111111111|0000000000000000000000000000000000000000000000000011  (-1)^0 * 2^(0) * 0x1.0000000000003
        1.0000   0|01111111111|0000000000000000000000000000000000000000000000000100  (-1)^0 * 2^(0) * 0x1.0000000000004


##### Посмотрим на существенно разные значения double


```cpp
%%cpp stand.cpp
%run gcc stand.cpp -o stand.exe
%run ./stand.exe

#include <math.h>
#include "stand.h"

int main() {
    double dd[] = {1.5, 100, NAN, -NAN, 0.0 / 0.0, INFINITY, -INFINITY, 0};
    print_doubles(dd);
}
```


Run: `gcc stand.cpp -o stand.exe`



Run: `./stand.exe`


    Bit numbers:  |  6        | 5         4         3         2         1         0
                 3|21098765432|1098765432109876543210987654321098765432109876543210  (-1)^S * 2^(E-B) * (1+M/(2^Mbits))
                 ------------------------------------------------------------------
        1.5000   0|01111111111|1000000000000000000000000000000000000000000000000000  (-1)^0 * 2^(0) * 0x1.8000000000000
      100.0000   0|10000000101|1001000000000000000000000000000000000000000000000000  (-1)^0 * 2^(6) * 0x1.9000000000000
           nan   0|11111111111|1000000000000000000000000000000000000000000000000000  (-1)^0 * 2^(1024) * 0x1.8000000000000
          -nan   1|11111111111|1000000000000000000000000000000000000000000000000000  (-1)^1 * 2^(1024) * 0x1.8000000000000
          -nan   1|11111111111|1000000000000000000000000000000000000000000000000000  (-1)^1 * 2^(1024) * 0x1.8000000000000
           inf   0|11111111111|0000000000000000000000000000000000000000000000000000  (-1)^0 * 2^(1024) * 0x1.0000000000000
          -inf   1|11111111111|0000000000000000000000000000000000000000000000000000  (-1)^1 * 2^(1024) * 0x1.0000000000000



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
    converter_t conv;
    conv.double_val = d;
    return conv.ui64_val;
    //return ((converter_t){.double_val = d}).ui64_val; // Вроде (?) хорошее решение
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


Run: `gcc -O2 -Wall bitcast.c -o bitcast.exe`


    [01m[Kbitcast.c:[m[K In function ‘[01m[Kbit_cast_ptr[m[K’:
    [01m[Kbitcast.c:29:13:[m[K [01;35m[Kwarning: [m[Kdereferencing type-punned pointer will break strict-aliasing rules [[01;35m[K-Wstrict-aliasing[m[K]
       29 |     return *[01;35m[K(uint64_t*)(void*)&d[m[K; // Простое, но неоднозначное решение из-за алиасинга
          |             [01;35m[K^~~~~~~~~~~~~~~~~~~~[m[K



Run: `./bitcast.exe`


    4614275588213125939
    4614275588213125939
    4614275588213125939



```python
!gdb bitcast.exe -batch -ex="disass bit_cast_memcpy" -ex="disass bit_cast_union" -ex="disass bit_cast_ptr"
```

    Dump of assembler code for function bit_cast_memcpy:
       0x00000000000011c0 <+0>:	endbr64 
       0x00000000000011c4 <+4>:	movq   %xmm0,%rax
       0x00000000000011c9 <+9>:	retq   
    End of assembler dump.
    Dump of assembler code for function bit_cast_union:
       0x00000000000011d0 <+0>:	endbr64 
       0x00000000000011d4 <+4>:	movq   %xmm0,%rax
       0x00000000000011d9 <+9>:	retq   
    End of assembler dump.
    Dump of assembler code for function bit_cast_ptr:
       0x00000000000011e0 <+0>:	endbr64 
       0x00000000000011e4 <+4>:	movq   %xmm0,%rax
       0x00000000000011e9 <+9>:	retq   
    End of assembler dump.


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
