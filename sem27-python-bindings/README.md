


# Опрос для всех, кто зашел на эту страницу

Он не страшный, там всего два обязательных вопроса на выбор одного варианта из трёх. Извиняюсь за размер, но к сожалению студенты склонны игнорировать опросы :| 

Пытаюсь компенсировать :)

<a href="https://docs.google.com/forms/d/e/1FAIpQLSdUnBAae8nwdSduZieZv7uatWPOMv9jujCM4meBZcHlTikeXg/viewform?usp=sf_link"><img src="poll.png" width="100%"  align="left" alt="Опрос"></a>



# Python bindings

<table width=100%  > <tr>
    <th width=15%> <b>Видео с семинара &rarr; </b> </th>
    <th>
    <a href="https://youtu.be/???"><img src="video.jpg" width="320" 
   height="160" align="left" alt="Видео с семинара"></a>
    </th>
    <th> </th>
 </table>

Сегодня в программе:
* Пишем модули для python:
  * <a href="#api" style="color:#856024"> Используя Python/C API </a>
  <br> https://docs.python.org/3/c-api/index.html
  * <a href="#ctypes" style="color:#856024"> Используя ctypes </a>
  * <a href="#cython" style="color:#856024"> Используя Cython </a>
  * <a href="#pybind" style="color:#856024"> Используя Pybind </a>
* <a href="#use_interpreter" style="color:#856024"> Исползуем интерпретатор Python из C </a>
 
[CPython на wiki](https://ru.wikipedia.org/wiki/CPython)

[Cython](https://ru.wikipedia.org/wiki/Cython)

[Ридинг Яковлева](---)
  
  
<a href="#hw" style="color:#856024">Комментарии к ДЗ</a>



## <a name="api"></a> Python/C API

Пожалуй, это способ писать самые эффективные биндинги, так как этот способ самый низкоуровневый. Пишем функции для питона на C используя существующее python/c api.

https://habr.com/ru/post/469043/


```cpp
%%cpp c_api_module.c
%run clang -Wall c_api_module.c $(python3-config --includes --ldflags) -shared -fPIC -o c_api_module.so
#include <Python.h>

// Парсинг позиционных аргументов в лоб
static PyObject* func_1(PyObject* self, PyObject* args) {
    if (PyTuple_Size(args) != 2) {
        PyErr_SetString(PyExc_TypeError, "func_ret_str args error");
        return NULL;
    }
    int val_i; char *val_s;
    // i - long int, s - char*
    PyArg_ParseTuple(args, "is", &val_i, &val_s);
    printf("func1: int - %d, string - %s\n", val_i, val_s);
    return Py_BuildValue("is", val_i, val_s);
}

// Умный парсинг args и kwargs
static PyObject* func_2(PyObject* self, PyObject* args, PyObject* kwargs) {
    static const char* kwlist[] = {"val_i", "val_s", NULL};
    int val_i = 0; char* val_s = ""; size_t val_s_len = 0;
    // до | обязательные аргументы, i - long int, z# - char* + size_t
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "i|z#", (char**)kwlist, &val_i, &val_s, &val_s_len)) {
        return NULL;
    }
    printf("func2: int - %d, string - %s, string_len = %zu\n", val_i, val_s, val_s_len);
    return Py_BuildValue("is", val_i, val_s);
}

// Список функций модуля
static PyMethodDef methods[] = {
    {"func_1", func_1, METH_VARARGS, "func_1"},
    // METH_KEYWORDS - принимает еще и именованные аргументы
    {"func_2", (PyCFunction)func_2, METH_VARARGS | METH_KEYWORDS, "func_2"},
    {NULL, NULL, 0, NULL}
};

// Описание модуля
static struct PyModuleDef module = {
    PyModuleDef_HEAD_INIT, "c_api_module", "Test module", -1, methods
};

// Инициализация модуля
PyMODINIT_FUNC PyInit_c_api_module(void) {
    PyObject* mod = PyModule_Create(&module);
    return mod;
}
```


Run: `clang -Wall c_api_module.c $(python3-config --includes --ldflags) -shared -fPIC -o c_api_module.so`



```python
%%save_file api_module_example.py
%# LD_PRELOAD=$(gcc -print-file-name=libasan.so) 
%run python3 api_module_example.py
import c_api_module

print(c_api_module.func_1(10, "12343"))

print(c_api_module.func_2(10))
print(c_api_module.func_2(val_i=10, val_s="42"))
print(c_api_module.func_2(10, val_s="42"))
```


Run: `python3 api_module_example.py`


    func1: int - 10, string - 12343
    (10, '12343')
    func2: int - 10, string - , string_len = 0
    (10, '')
    func2: int - 10, string - 42, string_len = 2
    (10, '42')
    func2: int - 10, string - 42, string_len = 2
    (10, '42')



```bash
%%bash
LD_PRELOAD=$(clang -print-file-name=libclang_rt.asan-x86_64.so)
```

    libclang_rt.asan-x86_64.so



```python

```


```cpp
%%cpp c_api_module_2.c
%run clang -Wall c_api_module_2.c $(python3-config --includes --ldflags) -shared -fPIC -o c_api_module_2.so
#include <Python.h>

static PyObject* print_dict(PyObject* self, PyObject* args, PyObject* kwargs) {
    static const char* kwlist[] = {"d", NULL};
    PyObject* d;
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O", (char**)kwlist, &d)) {
        return NULL;
    }
    Py_ssize_t ppos = 0;
    PyObject* pkey; PyObject* pvalue;
    while (PyDict_Next(d, &ppos, &pkey, &pvalue)) {
        const char* key = PyUnicode_AsUTF8(pkey);
        if (!key) return NULL;
        char value_storage[20];
        char* value = value_storage; 
        if (PyLong_Check(pvalue)) {
            sprintf(value_storage, "%lld", PyLong_AsLongLong(pvalue));
        } else {
            value = PyUnicode_AsUTF8(pvalue);
            if (!value) return NULL;
        }
        
        printf("%s -> %s\n", key, value);
    }
    printf("\n");
    fflush(stdout);

    Py_RETURN_NONE;
}

// Список функций модуля
static PyMethodDef methods[] = {
    {"print_dict", (PyCFunction)print_dict, METH_VARARGS | METH_KEYWORDS, "print_dict"},
    {NULL, NULL, 0, NULL}
};

// Описание модуля
static struct PyModuleDef module = {
    PyModuleDef_HEAD_INIT, "c_api_module_2", "Test module", -1, methods
};

// Инициализация модуля
PyMODINIT_FUNC PyInit_c_api_module_2(void) {
    PyObject* mod = PyModule_Create(&module);
    return mod;
}
```


Run: `clang -Wall c_api_module_2.c $(python3-config --includes --ldflags) -shared -fPIC -o c_api_module_2.so`



```python
%%save_file c_api_module_2_example.py
%run python3 c_api_module_2_example.py
import c_api_module_2

c_api_module_2.print_dict({"key1": "value1"})
c_api_module_2.print_dict({
    "key1": "value1",
    "key2": 42,
})
```


Run: `python3 c_api_module_2_example.py`


    key1 -> value1
    
    key1 -> value1
    key2 -> 42
    



```python

```

## <a name="ctypes"></a> ctypes

Способ взаимодействовать с уже существующей скомпилированной библиотекой.

Очень просто в очень простых случаях. И не очень в сложных

https://habr.com/ru/post/466499/


```cpp
%%cpp ctypes_lib.c
%run clang -Wall ctypes_lib.c -shared -fPIC -o ctypes_lib.so

float sum_ab(int a, float b) {
    return a + b;
}
```


Run: `clang -Wall ctypes_lib.c -shared -fPIC -o ctypes_lib.so`



```python
%%save_file ctypes_example.py
%run python3 ctypes_example.py

import ctypes 

ctypes_lib = ctypes.CDLL('./ctypes_lib.so')

sum_ab = ctypes_lib.sum_ab

sum_ab.restype = ctypes.c_float
sum_ab.argtypes = [ctypes.c_int, ctypes.c_float, ]

print(sum_ab(30, 1.5))
```


Run: `python3 ctypes_example.py`


    31.5



```python

```


```python

```

## <a name="cython"></a> Cython

Высокоуровневый способ связывать код на С/С++ и Python. Связка идет через промежуточный код на промежуточном языке.

По задумке (в моем понимании), cython можно использовать для написания обвязки к существующей С++ библиотеке для ее переиспользования в Python.

Но если честно, то и чистый С++ код приходится немного адаптировать под Python. Так как там есть тонкости в пробросе исключений и еще какие-то.

Получается довольно много избыточного кода и файлов.

Но помимо биндингов cython можно использовать просто для ускорения python кода. Переименовываем .py в .pyx и, магия, добавилась статическая типизация, все начало работать быстрее.

`pip3 install --user cython`


```cpp
%%cpp pairs.h
#pragma once
#include <vector>
#include <algorithm>

struct TPairs {
    std::vector<std::pair<int, float>> Vector;
};

inline void SortPairs(TPairs& pairs) {
    std::sort(pairs.Vector.begin(), pairs.Vector.end());
}

inline void AppendPairs(TPairs& pairs, const TPairs& other) {
    pairs.Vector.insert(pairs.Vector.end(), other.Vector.begin(), other.Vector.end());
}
```


```python
%%save_file pairs.pxd
from libcpp.vector cimport vector
from libcpp.pair cimport pair

cdef extern from "pairs.h" nogil:
    cdef cppclass TPairs:
        TPairs()
        vector[pair[int, float]] Vector
    void SortPairs(TPairs& pairs)
    void AppendPairs(TPairs& pairs, const TPairs& other)
    
cdef class Pairs:
    cdef TPairs pairs
    cdef init_cpp_from_python(self, pairs_list)
```


```python
%%save_file pairs.pyx
# distutils: language=c++
from libcpp.vector cimport vector
from libcpp.pair cimport pair

cdef class Pairs:
    def __init__(self, pairs_list=[]):
        self.init_cpp_from_python(pairs_list)
        
    cdef init_cpp_from_python(self, pairs_list):
        for val_i, val_f in pairs_list:
            self.pairs.Vector.push_back(pair[int, float](val_i, val_f))
            
    def sorted(self):
        sorted_pairs = Pairs()
        sorted_pairs.pairs = self.pairs
        SortPairs(sorted_pairs.pairs)
        return sorted_pairs
    
    def __add__(self, other):
        assert isinstance(other, Pairs)
        sum_pairs = Pairs()
        sum_pairs.pairs = (<Pairs>self).pairs
        AppendPairs(sum_pairs.pairs, (<Pairs>other).pairs)
        return sum_pairs
    
    def __repr__(self):
        return repr(self.pairs.Vector)

# это к примеру о том, что .pyx быстрее работает, чем .py
def count_1e8():
    for i in range(int(1e8)):
        pass
```


```python
%%save_file cython_setup.py
%run python3 ./cython_setup.py build_ext --inplace 

from distutils.core import setup
from Cython.Build import cythonize
import os

os.environ['CFLAGS'] = os.environ.get('CFLAGS', '') + ' -O3 -Wall -std=c++17'

setup(
    ext_modules = cythonize("pairs.pyx")
)
```


Run: `python3 ./cython_setup.py build_ext --inplace`


    running build_ext



```python
%%save_file test_pairs.py
%run python3 ./test_pairs.py

from pairs import Pairs

print(Pairs([(1, 2)]))
print((Pairs([(1, 2), (3, 10)]) + Pairs([(2, -1), (4, -10)])).sorted())
```


Run: `python3 ./test_pairs.py`


    [(1, 2.0)]
    [(1, 2.0), (2, -1.0), (3, 10.0), (4, -10.0)]


**Про то, что .pyx быстрее, чем .py**


```python
%%save_file count_1e8_native.py
%run time python3 ./count_1e8_native.py

def count_1e8():
    for i in range(int(1e8)):
        pass
    
count_1e8()
```


Run: `time python3 ./count_1e8_native.py`


    1.93user 0.02system 0:01.99elapsed 97%CPU (0avgtext+0avgdata 8776maxresident)k
    0inputs+0outputs (0major+939minor)pagefaults 0swaps



```python
%%save_file count_1e8_cython.py
%run time python3 ./count_1e8_cython.py

from pairs import count_1e8

count_1e8()
```


Run: `time python3 ./count_1e8_cython.py`


    1.14user 0.00system 0:01.19elapsed 96%CPU (0avgtext+0avgdata 9704maxresident)k
    0inputs+0outputs (0major+979minor)pagefaults 0swaps



```python

```

## <a name="pybind"></a> Pybind

https://habr.com/ru/post/468099/

`pip3 install --user pybind11`



```python

```


```cpp
%%cpp pairs_pybind.cpp

#include <vector>
#include <algorithm>
#include <sstream>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/operators.h>

struct TPairs {
    std::vector<std::pair<int, float>> Vector;
};

inline void SortPairs(TPairs& pairs) {
    std::sort(pairs.Vector.begin(), pairs.Vector.end());
}

inline void AppendPairs(TPairs& pairs, const TPairs& other) {
    pairs.Vector.insert(pairs.Vector.end(), other.Vector.begin(), other.Vector.end());
}

// -------------------

namespace py = pybind11;

PYBIND11_MODULE(pairs_pybind, m) {
    py::class_<TPairs>(m, "Pairs")
        .def(py::init<std::vector<std::pair<int, float>>>(), 
             "Class constructor", py::arg("vector") = std::vector<std::pair<int, float>>{}) 
        .def("sorted", [](TPairs pairs) { SortPairs(pairs); return pairs; })
        .def("__add__", [](TPairs a, const TPairs& b) { AppendPairs(a, b); return a; })
        .def("__repr__", [](const TPairs& p) { 
            std::stringstream ss;
            ss << "[";
            for (auto pair : p.Vector) { ss << "(" << pair.first << "," << pair.second << "),"; }
            ss << "]";
            return ss.str(); 
        })
        .def_readwrite("Vector", &TPairs::Vector) ; 
};
```


```python
%%save_file pybind_setup.py
%run python3 ./pybind_setup.py build_ext --inplace 

import pybind11
from distutils.core import setup, Extension

setup(
    ext_modules=[
        Extension(
            'pairs_pybind', # имя библиотеки собранной pybind11
            ['pairs_pybind.cpp'], # Тестовый файлик который компилируем
            include_dirs=[pybind11.get_include()],  # не забываем добавить инклюды pybind11
            language='c++', # Указываем язык
            extra_compile_args=['-std=c++11'], # флаг с++11
        ),
    ],
    requires=['pybind11']
)
```


Run: `python3 ./pybind_setup.py build_ext --inplace`


    running build_ext
    building 'pairs_pybind' extension
    x86_64-linux-gnu-gcc -pthread -DNDEBUG -g -fwrapv -O2 -Wall -Wstrict-prototypes -g -fstack-protector-strong -Wformat -Werror=format-security -Wdate-time -D_FORTIFY_SOURCE=2 -fPIC -I/home/pechatnov/.local/lib/python3.5/site-packages/pybind11/include -I/usr/include/python3.5m -c pairs_pybind.cpp -o build/temp.linux-x86_64-3.5/pairs_pybind.o -std=c++11
    [01m[Kcc1plus:[m[K [01;35m[Kwarning: [m[Kcommand line option ‘[01m[K-Wstrict-prototypes[m[K’ is valid for C/ObjC but not for C++
    x86_64-linux-gnu-g++ -pthread -shared -Wl,-O1 -Wl,-Bsymbolic-functions -Wl,-Bsymbolic-functions -Wl,-z,relro -Wl,-Bsymbolic-functions -Wl,-z,relro -g -fstack-protector-strong -Wformat -Werror=format-security -Wdate-time -D_FORTIFY_SOURCE=2 build/temp.linux-x86_64-3.5/pairs_pybind.o -o /home/pechatnov/vbox/caos_2019-2020/sem27-python-bindings/pairs_pybind.cpython-35m-x86_64-linux-gnu.so



```python
%%save_file test_pybind_pairs.py
%run python3 ./test_pybind_pairs.py

from pairs_pybind import Pairs

print(Pairs())
print(Pairs().Vector)
print(Pairs([(1, 2)]).Vector)
print(Pairs([(1, 2)]))
print(Pairs([(1, 2), (2, 1)]).sorted())

print((Pairs([(1, 2), (3, 10)]) + Pairs([(2, -1), (4, -10)])).sorted())
```


Run: `python3 ./test_pybind_pairs.py`


    []
    []
    [(1, 2.0)]
    [(1,2),]
    [(1,2),(2,1),]
    [(1,2),(2,-1),(3,10),(4,-10),]



```python

```

## <a name="use_interpreter"></a> Используем интерпретатор Python из C

https://habr.com/ru/post/466181/


Если вам понадобилось встраивать себе интерпретатор питона, то скорее всего в вашей жизни что-то пошло не так.

Но если прям пришлось, то всё-таки:


```cpp
%%cpp use_interpreter.c
%run clang -Wall use_interpreter.c $(python3-config --includes --ldflags) -o use_interpreter.exe
%run ./use_interpreter.exe
#include <Python.h>


int main() {
    Py_Initialize();
    PyObject* locals = PyDict_New();
    PyObject* globals = PyDict_Copy(PyEval_GetBuiltins()); 
    
    const char* exec_prefix = "$ ";
    const char* cmds[] = {
        "40 + 2",
        "print(1)",
        "$ a = 40 + 2",
        "$ b = 5 + 5",
        "print(a * b)",
        "a + b",
        "&",
        NULL
    };
    
    PyRun_SimpleString("print('Hello!')");
    for (const char** line = cmds; *line; ++line) {
        PyObject* result = (strncmp(line[0], exec_prefix, strlen(exec_prefix)) == 0) ?
            PyRun_String(line[0] + strlen(exec_prefix), Py_file_input, globals, locals) :
            PyRun_String(line[0], Py_eval_input, globals, locals);
        if (result) {
            PyObject_Print(result, stdout, 0); printf("\n");
        } else {
            PyErr_PrintEx(0);
            PyErr_Clear();
        }
        
    }
    Py_Finalize();
}

```


Run: `clang -Wall use_interpreter.c $(python3-config --includes --ldflags) -o use_interpreter.exe`



Run: `./use_interpreter.exe`


    Hello!
    42
    1
    None
    None
    None
    420
    None
    52
      File "<string>", line 1
        &
        ^
    SyntaxError: unexpected EOF while parsing



```python

```


```python

```

# <a name="hw"></a> Комментарии к ДЗ

* Python 3.x, то есть документацию по python/c api стоит смотреть для этой версии
*


```python

```


```python

```
