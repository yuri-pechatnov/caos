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

    0000000000000684 g     F .text	000000000000001a              sum_f
    0000000000000670 g     F .text	0000000000000014              sum



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
    lib.sum_f(3, 4) # with set return type = 0.0
    lib.sum_f(3, 4) # with set return and arguments types = 7.0



```python
!jupyter nbconvert cpp_run.ipynb
```

    [NbConvertApp] Converting notebook cpp_run.ipynb to html
    [NbConvertApp] Writing 281542 bytes to cpp_run.html



```python

```
