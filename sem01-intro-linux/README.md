



```cpp
%%cpp lib.c
%run gcc -shared -fPIC lib.c -o lib.so # compile shared library

int sum(int a, int b) {
    return a + b;
}

float sum_f(float a, float b) {
    return a + b;
}
```


Run: `gcc -shared -fPIC lib.c -o lib.so # compile shared library`



```python
!objdump -t lib.so | grep sum  # symbols in shared library
```

    0000000000000634 g     F .text	000000000000001a sum_f
    0000000000000620 g     F .text	0000000000000014 sum



```python
from IPython.display import display
import ctypes

lib = ctypes.CDLL("./lib.so")
%p lib.sum(3, 4)
%p lib.sum_f(3, 4)

lib.sum_f.restype = ctypes.c_float
%p lib.sum_f(3, 4) # with set return type

lib.sum_f.argtypes = [ctypes.c_float, ctypes.c_float]
lib.sum_f.restype = ctypes.c_float
%p lib.sum_f(3, 4) # with set return and arguments types
```


lib.sum(3, 4) = 7



lib.sum_f(3, 4) = 0



`lib.sum_f(3, 4) = 0.0`  # with set return type



`lib.sum_f(3, 4) = 7.0`  # with set return and arguments types



```python

```
