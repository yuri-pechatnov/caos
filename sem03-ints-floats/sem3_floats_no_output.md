


```python

```

Fog A https://agner.org/optimize/optimizing_cpp.pdf c 148


```python

```


```python

```


```cpp
%%cpp lib.c
%run gcc -shared -fPIC lib.c -o lib.so # compile shared library

#include <stdint.h>

typedef union {
    double double_val;
    uint64_t uint64_val;
    struct {
        uint64_t mantissa_val : 52;
        uint64_t exp_val : 11;
        uint64_t sign_val : 1;
    };
} double_parser_t;

uint64_t get_sign(double x) {
    double_parser_t parser = {.double_val = x};
    return parser.sign_val;
}

uint64_t get_mantissa(double x) {
    double_parser_t parser = {.double_val = x};
    return parser.mantissa_val;
}

uint64_t get_exp(double x) {
    double_parser_t parser = {.double_val = x};
    return parser.exp_val;
}


```


```python
from IPython.display import display
import ctypes

lib = ctypes.CDLL("./lib.so")

for f in [lib.get_sign, lib.get_mantissa, lib.get_exp]:
    f.argtypes = [ctypes.c_double]
    f.restype = ctypes.c_uint64

    
x = -1.2345

S, M, E = lib.get_sign(x), lib.get_mantissa(x), lib.get_exp(x)
B = (1 << 10) - 1
M_bits = 52
%p x
%p S, hex(S), bin(S)
%p M, hex(M), bin(M)
%p E, hex(E), bin(E)
%p ((-1) ** S) * (2 ** (E - B)) * (1 + M / ((2 ** M_bits) - 1))


```


```python
x = -1.234e-230

S, M, E = lib.get_sign(x), lib.get_mantissa(x), lib.get_exp(x)
B = (1 << 10) - 1
M_bits = 52
%p x
%p S
%p M
%p E
%p ((-1) ** S) * (2 ** (E - B)) * (1 + M / ((2 ** M_bits) - 1))

```


```python

```


```python

```
