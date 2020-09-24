


# CMake & Python bindings

<table width=100%  > <tr>
    <th width=15%> <b>Видео с семинара &rarr; </b> </th>
    <th>
    <a href="https://youtu.be/ingtnOf69KI"><img src="video.png" width="320" 
   height="160" align="left" alt="Видео с семинара"></a>
    </th>
    <th> </th>
 </table>

Сегодня в программе:
* <a href="#cmake" style="color:#856024"> CMake что зачем и почему </a>
* Пишем модули для python (<a href="#why" style="color:#856024">а зачем это нужно?</a>):
  * <a href="#api" style="color:#856024"> Используя Python/C API </a>
  <br> Документация по api: https://docs.python.org/3/c-api/index.html
  * <a href="#ctypes" style="color:#856024"> Используя ctypes </a>
  * <a href="#cython" style="color:#856024"> Используя Cython </a>
  * <a href="#pybind" style="color:#856024"> Используя Pybind </a>
* <a href="#use_interpreter" style="color:#856024"> Исползуем интерпретатор Python из C </a>
 
[CPython на wiki](https://ru.wikipedia.org/wiki/CPython) (Не путать с Cython!)


[GIL](https://habr.com/ru/post/84629/) - почему многопоточность в pyhton это не многопоточность.

[Ссылки в python и соглашения об инкременте/декременте их счетчиков](https://pythonextensionpatterns.readthedocs.io/en/latest/refcount.html#python-terminology)

[Ридинг Яковлева про CMake](https://github.com/victor-yacovlev/mipt-diht-caos/blob/master/practice/linux_basics/cmake.md)
<br> [Ридинг Яковлева про Python](https://github.com/victor-yacovlev/mipt-diht-caos/tree/master/practice/python)

  
<a href="#hw" style="color:#856024">Комментарии к ДЗ</a>



# <a name="cmake"></a> CMake
Установка: `apt-get install cmake cmake-extras`

Решает задачу кроссплатформенной сборки

* Фронтенд для систем непосредственно занимающихся сборкой
* cmake хорошо интегрирован с многими IDE 
* CMakeLists.txt в корне дерева исходников - главный конфигурационный файл и главный индикатор того, что проект собирается с помощью cmake

Примеры:
* <a href="#cmake_simple" style="color:#856024"> Простой пример </a>
* <a href="#cmake_python" style="color:#856024"> Пример с python </a>

[Введение в CMake / Хабр](https://habr.com/ru/post/155467/)

### <a name="cmake_simple"></a> Простой пример c CMake

Подойдем к нему постепенно:
1. g++
2. make
3. cmake

### 1.


```python
!mkdir simple_cmake_example 2>&1 | grep -v "File exists" || true
```


```cpp
%%cpp simple_cmake_example/main.cpp

#include <iostream>
int main(int argc, char** argv)
{
    std::cout << "Hello, World!" << std::endl;
    return 0;
}
```


```python
!cd simple_cmake_example && g++ main.cpp -o main.exe && ./main.exe 
```

    Hello, World!


### 2.


```python
!mkdir make_example 2>&1 | grep -v "File exists" || true
!cp ./simple_cmake_example/main.cpp ./make_example/main.cpp
```


```python
%%makefile make_example/makefile

main.exe: main.cpp
    echo "Run goal main.exe"
    g++ main.cpp -o main.exe
    
run: main.exe
    ./main.exe
```


```python
!cd make_example && make run
```

    echo "Run goal main.exe"
    Run goal main.exe
    g++ main.cpp -o main.exe
    ./main.exe
    Hello, World!



```python
!rm make_example/main.exe
```

### 3.


```cmake
%%cmake simple_cmake_example/CMakeLists.txt
%run mkdir simple_cmake_example/build #// cоздаем директорию для файлов сборки
%# переходим в нее, вызываем cmake, чтобы он создал правильный Makefile
%# а затем make, который по Makefile правильно все соберет
%run cd simple_cmake_example/build && cmake ..
%run ls -la simple_cmake_example/build 
%run cd simple_cmake_example/build && make
%run ls -la simple_cmake_example/build 
%run simple_cmake_example/build/main #// запускаем собранный бинарь
%run rm -r simple_cmake_example/build

cmake_minimum_required(VERSION 2.8) # Проверка версии CMake.
                                    # Если версия установленой программы
                                    # старее указаной, произайдёт аварийный выход.
project(simple_cmake_example)

add_executable(main main.cpp)       # Создает исполняемый файл с именем main
                                    # из исходника main.cpp
```


Run: `mkdir simple_cmake_example/build #// cоздаем директорию для файлов сборки`


    mkdir: cannot create directory ‘simple_cmake_example/build’: File exists



Run: `cd simple_cmake_example/build && cmake ..`


    -- Configuring done
    -- Generating done
    -- Build files have been written to: /home/pechatnov/vbox/caos/sem03-cmake-python-bindings/simple_cmake_example/build



Run: `ls -la simple_cmake_example/build`


    total 60
    drwxrwxr-x 3 pechatnov pechatnov  4096 сен 23 22:57 .
    drwxrwxr-x 3 pechatnov pechatnov  4096 сен 23 22:57 ..
    -rw-rw-r-- 1 pechatnov pechatnov 13988 сен 17 14:18 CMakeCache.txt
    drwxrwxr-x 5 pechatnov pechatnov  4096 сен 23 22:57 CMakeFiles
    -rw-rw-r-- 1 pechatnov pechatnov  1600 сен 17 14:18 cmake_install.cmake
    -rwxrwxr-x 1 pechatnov pechatnov 17312 сен 17 14:18 main
    -rw-rw-r-- 1 pechatnov pechatnov  4952 сен 23 22:57 Makefile



Run: `cd simple_cmake_example/build && make`


    [35m[1mScanning dependencies of target main[0m
    [ 50%] [32mBuilding CXX object CMakeFiles/main.dir/main.cpp.o[0m
    [100%] [32m[1mLinking CXX executable main[0m
    [100%] Built target main



Run: `ls -la simple_cmake_example/build`


    total 60
    drwxrwxr-x 3 pechatnov pechatnov  4096 сен 23 22:57 .
    drwxrwxr-x 3 pechatnov pechatnov  4096 сен 23 22:57 ..
    -rw-rw-r-- 1 pechatnov pechatnov 13988 сен 17 14:18 CMakeCache.txt
    drwxrwxr-x 5 pechatnov pechatnov  4096 сен 23 22:57 CMakeFiles
    -rw-rw-r-- 1 pechatnov pechatnov  1600 сен 17 14:18 cmake_install.cmake
    -rwxrwxr-x 1 pechatnov pechatnov 17312 сен 23 22:57 main
    -rw-rw-r-- 1 pechatnov pechatnov  4952 сен 23 22:57 Makefile



Run: `simple_cmake_example/build/main #// запускаем собранный бинарь`


    Hello, World!



Run: `rm -r simple_cmake_example/build`



```python

```


```python

```

# Python bindings
## <a name="why"></a>  Мотивашка

Python сейчас довольно распространенный язык, широко применяемый в машинном обучении, аналитике, во всех местах, где нужно писать скрипты, которые не пишутся очевидным образом на bash...

Зачем вообще может понадобиться писать модули для Python на других языках?
1. Желание/необходимость переиспользовать библиотеку написанную на другом языке. Не будете же вы писать алгоритм, например, приведения матрицы к нормальной Жордановой форме, на всех используемых в проекте языках?
2. Скорость. Если у вас есть код на питоне, то очевидный способ его ускорить - переписать все или критичную часть на более низкоуровневом языке. Переписать большой проект за один подход крайне сложно, поэтому даже в этом случае удобно сначала переписывать отдельные модули.
3. Многопоточность. Если вы хотите эффективно распараллелить некоторый объем CPU работы, то в питоне из-за GIL вы это сделать не сможете. Так что придется писать модуль.
4. Необходимость совершить низкоуровневые действия, которые нельзя сделать из Python. Например, как-то хитро поделать системные вызовы.


```python
import copy

a = [[3]]
b = a
c = list(a)
d = copy.deepcopy(a)
a.append(4)
b[0].append(4)
print(a, b, c, d)
%p (a is b)
%p (a is c)
%p (a is d)
```

    [[3, 4], 4] [[3, 4], 4] [[3, 4]] [[3]]



(a is b) = True



(a is c) = False



(a is d) = False


## Немного про None и nullptr


```python
def f():
    pass
    # что вернет функция f?
```


```python
type(f()) # она вернет None
```




    NoneType



`None` это специальное значение в python. Оно возвращается функциями, которые `void` в терминах C. Оно используется как отсутствие значения в методе `.get` у `dict`:


```python
type({"a": 1}.get('b'))
```




    NoneType



В общем используется так же, как часто используется `NULL`/`nullptr` в С/С++.

Но при этом в Python API `None` это не `NULL`. `None` это специальный синглтон-объект который используется в качестве особого значения. (В реализации красно-черного дерева иногда выделяют специальную вершину nil, тут примерно так же).


```python

```


```python
def f(a, b):
    print(a, b)
    
f(b=2, a=3)
```

    3 2



```python
def f(*args, **kwargs):
    print(repr(args), repr(kwargs))
    
f(1, 2, 3, b=2, g=5)
```

    (1, 2, 3) {'b': 2, 'g': 5}


## <a name="api"></a> Python/C API

Пожалуй, это способ писать самые эффективные биндинги, так как этот способ самый низкоуровневый. Пишем функции для питона на C используя существующее python/c api.

Возможно у меня неправильный питон, но с адрес-санитайзером он у меня не дружит, всегда есть множественные утечки памяти. С clang вообще завести не получилось в ряде случаев. Когда получилось завести, отключаю обнаружение утечек.

https://habr.com/ru/post/469043/


```python
!python3-config --includes --ldflags
```

    -I/usr/include/python3.8 -I/usr/include/python3.8
    -L/usr/lib/python3.8/config-3.8-x86_64-linux-gnu -L/usr/lib  -lcrypt -lpthread -ldl  -lutil -lm -lm 



```cpp
%%cpp c_api_module.c
%// Собираем модуль - динамическую библиотеку. Включаем нужные пути для инклюдов и динамические библиотеки
%run gcc -Wall c_api_module.c $(python3-config --includes --ldflags) -shared -fPIC -fsanitize=address -o c_api_module.so
#define PY_SSIZE_T_CLEAN
#include <Python.h>

// Парсинг позиционных аргументов в лоб
static PyObject* func_1_(PyObject* self, PyObject* args) {
    if (PyTuple_Size(args) != 2) {
        PyErr_SetString(PyExc_TypeError, "func_ret_str args error"); // выставляем ошибку
        return NULL; // возвращаем NULL - признак ошибки
    }
    long int val_i; char *val_s;
    // l - long int, s - char*
    if (!PyArg_ParseTuple(args, "ls", &val_i, &val_s)) {
        return NULL;
    }
    printf("func1: int - %ld, string - %s\n", val_i, val_s);
    return Py_BuildValue("ls", val_i, val_s);
}

// Умный парсинг args и kwargs
static PyObject* func_2(PyObject* self, PyObject* args, PyObject* kwargs) {
    static const char* kwlist[] = {"val_i", "val_s", NULL};
    long int val_i = 0; char* val_s = ""; Py_ssize_t val_s_len = 0;
    // до | обязательные аргументы, l - long int, z# - char* + size_t
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "l|z#", (char**)kwlist, &val_i, &val_s, &val_s_len)) {
        return NULL; // ошибка уже выставлена функцией PyArg_ParseTupleAndKeywords
    }
    printf("func2: int - %ld, string - %s, string_len = %d\n", val_i, val_s, (int)val_s_len);
    return Py_BuildValue("ls", val_i, val_s);
}

// Список функций модуля
static PyMethodDef methods[] = {
    {"func_1", func_1_, METH_VARARGS, "help func_1"},
    // METH_KEYWORDS - принимает еще и именованные аргументы
    {"func_2", (PyCFunction)func_2, METH_VARARGS | METH_KEYWORDS, "help func_2"},
    {NULL, NULL, 0, NULL}
};

// Описание модуля
static struct PyModuleDef mod = {
    PyModuleDef_HEAD_INIT, "c_api_module", "Test module", -1, methods
};

// Инициализация модуля
PyMODINIT_FUNC PyInit_c_api_module(void) {
    return PyModule_Create(&mod);
}
```


Run: `gcc -Wall c_api_module.c $(python3-config --includes --ldflags) -shared -fPIC -fsanitize=address -o c_api_module.so`


Теперь заиспользуем библиотеку. Обратите внимание, что я для этого запускаю отдельный интерпретатор питона, а не делаю это просто в ячейке ноутбука.

Это из-за того, что если модуль с именем `c_api_module` был импортирован, то пусть он даже изменится - повторно его импортировать не получится. Можно каждый раз загружать его под новым именем, но это не очень удобно.

Когда будете делать домашку, учитывайте эту особенность.


```python
%%save_file api_module_example.py
%# Переменные окружения устанавливаются для корректной работы санитайзера
%run LD_PRELOAD=$(gcc -print-file-name=libasan.so) ASAN_OPTIONS=detect_leaks=0 python3 api_module_example.py | cat
import c_api_module

#print(help(c_api_module))

print(help(c_api_module.func_1))

print(c_api_module.func_1(10, "12343"))
print(c_api_module.func_2(10))
print(c_api_module.func_2(val_s="42", val_i=10))
print(c_api_module.func_2(10, val_s="42"))
```


Run: `LD_PRELOAD=$(gcc -print-file-name=libasan.so) ASAN_OPTIONS=detect_leaks=0 python3 api_module_example.py | cat`


    Help on built-in function func_1 in module c_api_module:
    
    func_1(...)
        help func_1
    
    None
    (10, '12343')
    (10, '')
    (10, '42')
    (10, '42')
    func1: int - 10, string - 12343
    func2: int - 10, string - , string_len = 0
    func2: int - 10, string - 42, string_len = 2
    func2: int - 10, string - 42, string_len = 2



```python
!echo $(clang -print-file-name=libasan.so)
```

    /usr/bin/../lib/gcc/x86_64-linux-gnu/9/libasan.so



```python
!ls /usr/bin/../lib/gcc/x86_64-linux-gnu/7.4.0/lib*so
```

    ls: cannot access '/usr/bin/../lib/gcc/x86_64-linux-gnu/7.4.0/lib*so': No such file or directory


Пример работы с более сложным типом - словариком. Без санитайзера на этот раз, чтобы хоть где-то были команды компиляции и запуска не усложненные костылями для запуска саниайзера.


```cpp
%%cpp c_api_own_type_module.c
%run clang -Wall c_api_own_type_module.c $(python3-config --includes --ldflags) -shared -fPIC -fsanitize=address -o c_api_own_type_module.so
#define PY_SSIZE_T_CLEAN
#include <Python.h>

typedef struct {
    PyObject_HEAD;
    double x, y;
} PyPoint;

PyTypeObject py_point_type = {    
    PyVarObject_HEAD_INIT(NULL, 0)
};   

PyObject* PyPoint_new(PyTypeObject* type, PyObject* args, PyObject* kwargs) {
    return (PyObject*)type->tp_alloc(type, 0);
}

void PyPoint_dealloc(PyPoint* self) {
    Py_TYPE(self)->tp_free(self);
}

int PyPoint_init(PyPoint* self, PyObject* args, PyObject* kwargs) {
    static const char* kwlist[] = {"x", "y", NULL};
    self->x = self->y = 0;
    return PyArg_ParseTupleAndKeywords(args, kwargs, "|dd", (char**)kwlist, &self->x, &self->y) != 0;
}


PyPoint* PyPoint_setfrom(PyPoint* self, PyObject* args, PyObject* kwargs) {
    static const char* kwlist[] = {"data", NULL};
    char* val_s = ""; Py_ssize_t val_s_len = 0;
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "z#", (char**)kwlist, &val_s, &val_s_len)) {
        return NULL;
    }
    sscanf(val_s, "{%lf, %lf}", &self->x, &self->y);
    Py_INCREF(self);
    return self;
}

PyPoint* PyPoint_add(PyPoint* self, PyPoint* arg) {
    if (!PyObject_IsInstance((PyObject*)arg, (PyObject*)Py_TYPE(self))) {
        PyErr_SetString(PyExc_TypeError, "not Point type");
        return NULL; 
    }
    PyPoint* result = (PyPoint*)PyPoint_new(&py_point_type, NULL, NULL);
    result->x = self->x + arg->x;
    result->y = self->y + arg->y;
    Py_INCREF(result);
    return result;
}

PyObject* PyPoint_repr(PyPoint* self)
{
    char buffer[100];
    snprintf(buffer, sizeof(buffer), "{%.1lf, %.1lf}", self->x, self->y);
    return PyUnicode_FromString(buffer);
}


void PyPoint_prepare_type() {
    PyTypeObject* o = &py_point_type;
    o->tp_name = "Point";
    o->tp_basicsize = sizeof(PyPoint);
    o->tp_dealloc = (destructor)PyPoint_dealloc;
    o->tp_repr = (reprfunc)PyPoint_repr;
    static PyNumberMethods number_methods = {
        .nb_add = (binaryfunc)PyPoint_add,
    };
    o->tp_as_number = &number_methods;
    o->tp_str = (reprfunc)PyPoint_repr;
    o->tp_flags = Py_TPFLAGS_DEFAULT;
    o->tp_doc = "Just a 2d point\n";
    static PyMethodDef methods[] = {
        {"setfrom", (PyCFunction)PyPoint_setfrom, METH_VARARGS|METH_KEYWORDS, NULL},
        {NULL, NULL, 0, NULL}
    };
    o->tp_methods = methods;
    o->tp_init = (initproc)PyPoint_init;
    o->tp_new = PyPoint_new;
    if (PyType_Ready(o) < 0) {
        Py_FatalError("Can't initialize 'Point'");
    }
}

// Инициализация модуля
PyMODINIT_FUNC PyInit_c_api_own_type_module(void) {
    static struct PyModuleDef mod_obj = {
        PyModuleDef_HEAD_INIT, "c_api_own_type_module", "Test class module", -1, NULL
    };
    PyObject* mod = PyModule_Create(&mod_obj);
    PyPoint_prepare_type();
    Py_INCREF(&py_point_type);
    PyModule_AddObject(mod, "Point", (PyObject*)&py_point_type);
    return mod;
}
```


Run: `clang -Wall c_api_own_type_module.c $(python3-config --includes --ldflags) -shared -fPIC -fsanitize=address -o c_api_own_type_module.so`



```python
%%save_file c_api_own_type_module_example.py
%run LD_PRELOAD=$(gcc -print-file-name=libasan.so) ASAN_OPTIONS=detect_leaks=0  python3 c_api_own_type_module_example.py
from c_api_own_type_module import Point

print(Point(123, 345))
print(Point(3, 4) + Point(3, 49))

a = Point(0, 0)
a.setfrom("{100, 200}")
print(a)

try:
    Point(3, 4) + 1
except Exception as e:
    print("Exception:", e)
```


Run: `LD_PRELOAD=$(gcc -print-file-name=libasan.so) ASAN_OPTIONS=detect_leaks=0  python3 c_api_own_type_module_example.py`


    {123.0, 345.0}
    {6.0, 53.0}
    {100.0, 200.0}
    Exception: not Point type



```python

```

Пример реализации собственного типа:


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
        const char* value = value_storage; 
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

    Py_RETURN_NONE; // Инкрементит счетчик ссылок None и возвращает его
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


```python

```


```python

```


```python

```

## <a name="ctypes"></a> ctypes

Способ взаимодействовать с уже существующей скомпилированной библиотекой.

Очень просто в очень простых случаях. И не очень в сложных

https://habr.com/ru/post/466499/


```cpp
%%cpp ctypes_lib.c
%// Делаем самую обычную динамическую библиотеку
%run gcc -Wall ctypes_lib.c -shared -fPIC -fsanitize=address -o ctypes_lib.so

float sum_ab(int a, float b) {
    return a + b;
}
```


Run: `gcc -Wall ctypes_lib.c -shared -fPIC -fsanitize=address -o ctypes_lib.so`



```python
%%save_file ctypes_example.py
%run LD_PRELOAD=$(gcc -print-file-name=libasan.so) ASAN_OPTIONS=detect_leaks=0 python3 ctypes_example.py

import ctypes 

ctypes_lib = ctypes.CDLL('./ctypes_lib.so')

sum_ab = ctypes_lib.sum_ab

sum_ab.restype = ctypes.c_float
sum_ab.argtypes = [ctypes.c_int, ctypes.c_float, ]

print(sum_ab(30, 1.5))
```


Run: `LD_PRELOAD=$(gcc -print-file-name=libasan.so) ASAN_OPTIONS=detect_leaks=0 python3 ctypes_example.py`


    31.5



```python

```


```python

```

## <a name="cython"></a> Cython

[Cython на wiki](https://ru.wikipedia.org/wiki/Cython)

Высокоуровневый способ связывать код на С/С++ и Python. Связка идет через промежуточный код на промежуточном языке.

По задумке (в моем понимании), cython можно использовать для написания обвязки к существующей С++ библиотеке для ее переиспользования в Python.

Но если честно, то и чистый С++ код приходится немного адаптировать под Python. Так как там есть тонкости в пробросе исключений и еще какие-то.

Получается довольно много избыточного кода и файлов.

Но помимо биндингов cython можно использовать просто для ускорения python кода. Переименовываем .py в .pyx и, магия, добавилась статическая типизация, все начало работать быстрее.

`pip3 install --user cython`

Исходный код на C. Необязательно весь в хедере, просто так проще в этом примере, а то и так файлов много будет :)


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

cython'овский хедер. В нем описывается вся провязка с C/С++, а так же объявляются классы в которых будут поля - С'шные структуры и функции, в которых можно будет использовать локальные С'шные переменные.

Этот файлик автоматически "инклюдится" в соответствующий .pyx


```python
%%save_file pairs.pxd
from libcpp.vector cimport vector
from libcpp.pair cimport pair

# "Импорты" функций из С/С++
cdef extern from "pairs.h" nogil:
    cdef cppclass TPairs:
        TPairs()
        vector[pair[int, float]] Vector
    void SortPairs(TPairs& pairs)
    void AppendPairs(TPairs& pairs, const TPairs& other)

# Объявляем класс с С++ полем
cdef class Pairs:
    cdef TPairs pairs
```

.pyx файл, в нём уже практически чистый питон, только слегка расширенный. Есть интересные ключевые слова cimport, cdef, касты объектов к плюсовым типам, но в остальном - обычный питон.


```python
%%save_file pairs.pyx
# distutils: language=c++ 
# ^^^ - обязательный комментарий
from libcpp.vector cimport vector
from libcpp.pair cimport pair

cdef class Pairs:
    def __init__(self, pairs_list=[]):
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

Скомпилируем теперь это в модуль:


```python
%%save_file cython_setup.py
%run python3 ./cython_setup.py build_ext --inplace 

from distutils.core import setup, Extension
from Cython.Distutils import build_ext

setup(
    ext_modules=[
        Extension(
            "pairs",
            sources=["pairs.pyx"],
            language="c++",
            extra_compile_args=["-std=c++17", "-Wall"]
        ),
    ], 
    cmdclass={
        'build_ext': build_ext
    }
)
```


Run: `python3 ./cython_setup.py build_ext --inplace`


    running build_ext
    cythoning pairs.pyx to pairs.cpp
    /home/pechatnov/.local/lib/python3.8/site-packages/Cython/Compiler/Main.py:369: FutureWarning: Cython directive 'language_level' not set, using 2 for now (Py2). This will change in a later release! File: /home/pechatnov/vbox/caos/sem03-cmake-python-bindings/pairs.pxd
      tree = Parsing.p_module(s, pxd, full_module_name)
    building 'pairs' extension
    creating build
    creating build/temp.linux-x86_64-3.8
    x86_64-linux-gnu-gcc -pthread -Wno-unused-result -Wsign-compare -DNDEBUG -g -fwrapv -O2 -Wall -g -fstack-protector-strong -Wformat -Werror=format-security -g -fwrapv -O2 -g -fstack-protector-strong -Wformat -Werror=format-security -Wdate-time -D_FORTIFY_SOURCE=2 -fPIC -I/usr/include/python3.8 -c pairs.cpp -o build/temp.linux-x86_64-3.8/pairs.o -std=c++17 -Wall
    x86_64-linux-gnu-g++ -pthread -shared -Wl,-O1 -Wl,-Bsymbolic-functions -Wl,-Bsymbolic-functions -Wl,-z,relro -g -fwrapv -O2 -Wl,-Bsymbolic-functions -Wl,-z,relro -g -fwrapv -O2 -g -fstack-protector-strong -Wformat -Werror=format-security -Wdate-time -D_FORTIFY_SOURCE=2 build/temp.linux-x86_64-3.8/pairs.o -o /home/pechatnov/vbox/caos/sem03-cmake-python-bindings/pairs.cpython-38-x86_64-linux-gnu.so


И заиспользуем:


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


    2.79user 0.01system 0:02.81elapsed 99%CPU (0avgtext+0avgdata 8864maxresident)k
    0inputs+0outputs (0major+828minor)pagefaults 0swaps



```python
%%save_file count_1e8_cython.py
%run time python3 ./count_1e8_cython.py

from pairs import count_1e8

count_1e8()
```


Run: `time python3 ./count_1e8_cython.py`


    1.61user 0.00system 0:01.62elapsed 99%CPU (0avgtext+0avgdata 10048maxresident)k
    0inputs+0outputs (0major+884minor)pagefaults 0swaps



```python

```

## <a name="pybind"></a> Pybind

Только С++

Лаконично по сравнению с cython, при этом так же есть неявный кастинг питонячих типов и типов из stl.

По ощущениям долго собирается, видимо количество используемого метапрограммирования сказывается

https://habr.com/ru/post/468099/

`pip3 install --user pybind11`



```cpp
%%cpp pairs_pybind.cpp

#include <vector>
#include <algorithm>
#include <sstream>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h> // неявные преобразования python-объектов и стандартных C++ классов

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
        // конструктор
        .def(py::init<std::vector<std::pair<int, float>>>(), 
             "Class constructor", py::arg("vector") = std::vector<std::pair<int, float>>{}) 
        // методы
        .def("sorted", [](TPairs pairs) { SortPairs(pairs); return pairs; })
        .def("__add__", [](TPairs a, const TPairs& b) { AppendPairs(a, b); return a; })
        .def("__repr__", [](const TPairs& p) { 
            std::stringstream ss;
            ss << "[";
            for (auto pair : p.Vector) { ss << "(" << pair.first << "," << pair.second << "),"; }
            ss << "]";
            return ss.str(); 
        })
        .def_readwrite("Vector", &TPairs::Vector) // Поле структурки как read-write property (с автоматическими конверсиями)
    ; 
};
```

Скомпилируем теперь это в модуль:


```python
%%save_file pybind_setup.py
%run python3 ./pybind_setup.py build_ext --inplace 

import pybind11
from distutils.core import setup, Extension

setup(
    ext_modules=[
        Extension(
            'pairs_pybind',                         # Имя библиотеки собранной pybind11
            ['pairs_pybind.cpp'],                   # Тестовый файлик который компилируем
            include_dirs=[pybind11.get_include()],  # Добавляем инклюды pybind11
            language='c++',                         # Указываем язык
            extra_compile_args=['-std=c++11'],      # Флаг с++11
        ),
    ],
    requires=['pybind11']
)
```


Run: `python3 ./pybind_setup.py build_ext --inplace`


    running build_ext


И заиспользуем:


```python
%%save_file test_pybind_pairs.py
%run python3 ./test_pybind_pairs.py

from pairs_pybind import Pairs

print(Pairs())
print(Pairs().Vector)
print(Pairs([(1, 2)]).Vector)
print(Pairs(vector=[(1, 2)]))
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


```python
# Сначала немного про exec и eval
print("eval вычисляет выражение в строке и возвращает его значение:", eval("1 + 1"))
print("exec выполняет код, возвращает всегда None:", exec("1 + 1"))
```

    eval вычисляет выражение в строке и возвращает его значение: 2
    exec выполняет код, возвращает всегда None: None



```python
print("Побочные эффекты могут быть как в случае eval:", eval("print('PRINT', 1 + 1)"))
print("                          так и в случае exec:", exec("print('PRINT', 2 + 2)"))
```

    PRINT 2
    Побочные эффекты могут быть как в случае eval: None
    PRINT 4
                              так и в случае exec: None



```python
try: 
    print(eval("a = 1")) # 
except Exception as e: 
    print("Это не выражение у которого можно вычислить значение:", e)
print("Но то, что вполне можно выполнить, exec справляется:", exec("b = 100500"))
print("Переменные выставленные внутри exec видны снаружи:", b)
```

    Это не выражение у которого можно вычислить значение: invalid syntax (<string>, line 1)
    Но то, что вполне можно выполнить, exec справляется: None
    Переменные выставленные внутри exec видны снаружи: 100500



```python
# А еще можно явно указывать, какие locals и globals будут внутри exec/eval
#    expression globals         locals        
eval("A1 + B2", {"A1": 100000}, {"B2": 500})
```




    100500




```python
custom_locals = {"D": 1000}
exec("A = D + 50", {}, custom_locals)
custom_locals
```




    {'D': 1000, 'A': 1050}



И, наконец, код про встраивание python:


```python
!python3 -V
```

    Python 3.8.2



```cpp
%%cpp use_interpreter.c
%run clang -Wall use_interpreter.c $(python3-config --includes --ldflags) -lpython3.8 -fsanitize=address -o use_interpreter.exe
%run ASAN_OPTIONS=detect_leaks=0 ./use_interpreter.exe
#include <Python.h>

#define EXEC_PREFIX "$ "

int main() {
    Py_Initialize();
    PyObject* locals = PyDict_New();
    // Для PyEval_GetBuiltins не нужно делать Py_DECREF, так как возвращается borrowed reference: https://docs.python.org/3/c-api/reflection.html
    // Подробнее про подсчет ссылок: https://pythonextensionpatterns.readthedocs.io/en/latest/refcount.html#new-references
    PyObject* globals = PyDict_Copy(PyEval_GetBuiltins()); // Нам же нужно, чтобы функция print сразу была определена?

    typedef struct { int is_exec_cmd; const char* line; } cmd_t;
    #define EVAL(cmd) {0, cmd}
    #define EXEC(cmd) {1, cmd}

    const cmd_t cmds[] = {
        EVAL("40 + 2"),
        EVAL("print(1)"),
        EXEC("a = 40 + 2"),
        EXEC("b = 5 + 5"),
        EXEC("print(a * b)"),
        EXEC("a + b"),
        EXEC(
            "for i in range(3):"                    "\n"
            "    print('i = %d' % i, end=', ')"     "\n"
            "print()"                               "\n"
        ),
        EVAL("&"),
    };
    
    for (const cmd_t* cmd = cmds; cmd != cmds + sizeof(cmds) / sizeof(cmd_t); ++cmd) {
        PyObject* result = cmd->is_exec_cmd ? 
            PyRun_String(cmd->line, Py_file_input, globals, locals) : // exec
            PyRun_String(cmd->line, Py_eval_input, globals, locals);  // eval
        if (result) {
            PyObject_Print(result, stdout, 0); printf("\n"); // печать python-объекта (print(obj))
            Py_DECREF(result);
        } else {
            // Не забываем, что python-функции возвращают None если нормально завершаются без return и исключений
            // При этом None это специальный синглтон. То есть != NULL. 
            // А вот если функция вернула NULL, то это значит, что кинуто исключение
            PyErr_PrintEx(0); // печать исключения
            PyErr_Clear();
        }
        
    }
    Py_DECREF(locals);
    Py_DECREF(globals);
    Py_Finalize();
}
```


Run: `clang -Wall use_interpreter.c $(python3-config --includes --ldflags) -lpython3.8 -fsanitize=address -o use_interpreter.exe`



Run: `ASAN_OPTIONS=detect_leaks=0 ./use_interpreter.exe`


    42
    1
    None
    None
    None
    420
    None
    None
    i = 0, i = 1, i = 2, 
    None
      File "<string>", line 1
        &
        ^
    SyntaxError: unexpected EOF while parsing



```python

```

### <a name="cmake_python"></a> Пример c CMake и Python


```python
!mkdir python_cmake_example 2>&1 | grep -v "File exists" || true
!cp ./use_interpreter.c ./python_cmake_example/use_interpreter.c
```


```cmake
%%cmake python_cmake_example/CMakeLists.txt
%run mkdir python_cmake_example/build
%run cd python_cmake_example/build && cmake .. && make  
%run python_cmake_example/build/example
%run rm -r python_cmake_example/build 

cmake_minimum_required(VERSION 2.8) 

project(python_cmake_example)

add_executable(example use_interpreter.c)

find_package(PythonLibs 3.6 REQUIRED)
include_directories(${PYTHON_INCLUDE_DIRS})
target_link_libraries(example ${PYTHON_LIBRARIES})
```


Run: `mkdir python_cmake_example/build`



Run: `cd python_cmake_example/build && cmake .. && make`


    -- The C compiler identification is GNU 9.3.0
    -- The CXX compiler identification is GNU 9.3.0
    -- Check for working C compiler: /usr/bin/cc
    -- Check for working C compiler: /usr/bin/cc -- works
    -- Detecting C compiler ABI info
    -- Detecting C compiler ABI info - done
    -- Detecting C compile features
    -- Detecting C compile features - done
    -- Check for working CXX compiler: /usr/bin/c++
    -- Check for working CXX compiler: /usr/bin/c++ -- works
    -- Detecting CXX compiler ABI info
    -- Detecting CXX compiler ABI info - done
    -- Detecting CXX compile features
    -- Detecting CXX compile features - done
    -- Found PythonLibs: /usr/lib/x86_64-linux-gnu/libpython3.8.so (found suitable version "3.8.2", minimum required is "3.6") 
    -- Configuring done
    -- Generating done
    -- Build files have been written to: /home/pechatnov/vbox/caos/sem03-cmake-python-bindings/python_cmake_example/build
    [35m[1mScanning dependencies of target example[0m
    [ 50%] [32mBuilding C object CMakeFiles/example.dir/use_interpreter.c.o[0m
    [100%] [32m[1mLinking C executable example[0m
    [100%] Built target example



Run: `python_cmake_example/build/example`


    42
    1
    None
    None
    None
    420
    None
    None
    i = 0, i = 1, i = 2, 
    None
      File "<string>", line 1
        &
        ^
    SyntaxError: unexpected EOF while parsing



Run: `rm -r python_cmake_example/build`



```python

```


```python

```

# <a name="hw"></a> Комментарии к ДЗ

* Python 3.x, то есть документацию по python/c api стоит смотреть для этой версии
* Если вы чувствуете в себе на это силы, то следите за ссылками, делайте Py_DECREF когда необходимо, и не делайте когда не надо :)
* В задаче про перемножение матриц считайте все в double. При возвращении в питон отдавайте питонячий float.
* В задаче про декодирование можно считать пароль строкой, остальное - байтами.


```python

```


```python

```
