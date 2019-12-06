```javascript
%%javascript
// setup cpp code highlighting
IPython.CodeCell.options_default.highlight_modes["text/x-c++src"] = {'reg':[/^%%cpp/]} ;
```


    <IPython.core.display.Javascript object>



```python
# creating magics
from IPython.core.magic import register_cell_magic, register_line_magic
from IPython.display import display, Markdown

@register_cell_magic
def cpp(fname, cell):
    cell = cell if cell[-1] == '\n' else cell + "\n"
    cmds = []
    with open(fname, "w") as f:
        for line in cell.split("\n"):
            if line.startswith("%"):
                run_prefix = "%run "
                assert line.startswith(run_prefix)
                cmds.append(line[len(run_prefix):].strip())
            else:
                f.write(line + "\n")
    for cmd in cmds:
        display(Markdown("Run: `%s`" % cmd))
        get_ipython().system(cmd)

@register_line_magic
def p(line):
    print("{} = {}".format(line, eval(line)))
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


Run: `gcc -shared -fPIC lib.c -o lib.so # compile shared library`



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

    x = -1.2345
    S, hex(S), bin(S) = (1, '0x1', '0b1')
    M, hex(M), bin(M) = (1056094112618381, '0x3c083126e978d', '0b11110000001000001100010010011011101001011110001101')
    E, hex(E), bin(E) = (1023, '0x3ff', '0b1111111111')
    ((-1) ** S) * (2 ** (E - B)) * (1 + M / ((2 ** M_bits) - 1)) = -1.2345



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

    x = -1.234e-230
    S = 1
    M = 888918597024966
    E = 259
    ((-1) ** S) * (2 ** (E - B)) * (1 + M / ((2 ** M_bits) - 1)) = -1.234e-230



```python

```


```python

```
