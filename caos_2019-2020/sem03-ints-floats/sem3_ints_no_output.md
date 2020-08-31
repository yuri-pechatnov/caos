


```cpp
%%cpp lib.c
%run gcc -O3 -shared -fPIC lib.c -o lib3.so # compile shared library
%run gcc -O0 -shared -fPIC lib.c -o lib0.so # compile shared library

int check_increment(int x) {
    return x + 1 > x;
}

unsigned int unsigned_check_increment(unsigned int x) {
    return x + 1 > x;
}
```


```python
!objdump -t lib0.so | grep inc  # symbols in shared library
!objdump -t lib3.so | grep inc  # symbols in shared library
```


```python
from IPython.display import display
import ctypes

lib0 = ctypes.CDLL("./lib0.so")
lib3 = ctypes.CDLL("./lib3.so")

%p lib0.check_increment(1)
%p lib3.check_increment(1)
int32_max = (1 << 31) - 1
%p int32_max
%p lib0.check_increment(int32_max)
%p lib3.check_increment(int32_max)


uint32_max = (1 << 32) - 1
%p uint32_max
%p lib0.unsigned_check_increment(uint32_max)
%p lib3.unsigned_check_increment(uint32_max)
```


```cpp
%%cpp main.c
%run clang -O3 lib.c -c -fsanitize=undefined
%run clang -O3 main.c lib.o -o a.exe -fsanitize=undefined
%run ./a.exe

#include <assert.h>

int check_increment(int x);
unsigned int unsigned_check_increment(unsigned int x);

int main() {
    assert(unsigned_check_increment(1));
    assert(!unsigned_check_increment((1LL << 32) - 1));
    check_increment((1LL << 31) - 1);
    return 0;
}
```


```cpp
%%cpp main.c
%run gcc-7 -O3 main.c -o a.exe 
%run ./a.exe

#include <assert.h>
#include <stdint.h>

unsigned int satsum(unsigned int x, unsigned int y) {
    unsigned int z;
    if (__builtin_uadd_overflow(x, y, &z)) {
        return -1;
    }
    return z;
}

int main() {
    assert(satsum((1LL << 31) - 1, (1LL << 31) - 1) == 
        ((1LL << 31) - 1) * 2);
    assert(satsum((1LL << 31) + 1, (1LL << 31) + 1) == 
        (unsigned int)-1);
    return 0;
}
```


```python

```
