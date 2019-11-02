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


Run: `gcc -O3 -shared -fPIC lib.c -o lib3.so # compile shared library`



Run: `gcc -O0 -shared -fPIC lib.c -o lib0.so # compile shared library`



```python
!objdump -t lib0.so | grep inc  # symbols in shared library
!objdump -t lib3.so | grep inc  # symbols in shared library
```

    0000000000000698 g     F .text	0000000000000018              unsigned_check_increment
    0000000000000680 g     F .text	0000000000000018              check_increment
    0000000000000690 g     F .text	000000000000000c              unsigned_check_increment
    0000000000000680 g     F .text	0000000000000006              check_increment



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

    lib0.check_increment(1) = 1
    lib3.check_increment(1) = 1
    int32_max = 2147483647
    lib0.check_increment(int32_max) = 0
    lib3.check_increment(int32_max) = 1
    uint32_max = 4294967295
    lib0.unsigned_check_increment(uint32_max) = 0
    lib3.unsigned_check_increment(uint32_max) = 0



```python
%%cpp main.c
%run gcc -O3 lib.c -c --sanitize=undefined
%run gcc -O3 main.c lib.o -o a.exe --sanitize=undefined
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


Run: `gcc -O3 lib.c -c --sanitize=undefined`



Run: `gcc -O3 main.c lib.o -o a.exe --sanitize=undefined`



Run: `./a.exe`


    [1mlib.c:3:14:[1m[31m runtime error: [1m[0m[1msigned integer overflow: 2147483647 + 1 cannot be represented in type 'int'[1m[0m



```python
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


Run: `gcc-7 -O3 main.c -o a.exe`



Run: `./a.exe`



```python

```
