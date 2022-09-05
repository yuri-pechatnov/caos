

# Динамические библиотеки


<p><a href="https://www.youtube.com/watch?v=s3t8wIhj4WA&list=PLjzMm8llUm4AmU6i_hPU0NobgA4VsBowc&index=24" target="_blank">
    <h3>Видеозапись семинара</h3> 
</a></p>

[Ридинг Яковлева](https://github.com/victor-yacovlev/mipt-diht-caos/tree/master/practice/function-pointers)


Сегодня в программе:
* Создание и подгрузка динамической библиотеки
  * <a href="#create_dynlib" style="color:#856024">Создание</a>
  * Подгрузка 
    1. <a href="#load_с" style="color:#856024">При старте средствами OS (динамическая компоновка)</a> 
    <br> Вот [это](https://www.ibm.com/developerworks/ru/library/l-dynamic-libraries/) можно почитать для понимания, что в этом случае происходит.
    2. В произвольный момент времени:
      * <a href="#load_python" style="color:#856024">Из python</a> 
      * <a href="#load_с_std" style="color:#856024">Из программы на С (dlopen)</a> 
      * <a href="#load_с_mmap" style="color:#856024">Из программы на С с извращениями (mmap)</a> 
* Нетривиальный пример применения динамических библиотек
  <br> <a href="#c_interpreter" style="color:#856024">Развлекаемся и пишем простенький интерпретатор языка C (с поблочным выполнением команд)</a> 
  
  
Факты:
* Статическая линковка быстрее динамической. И при запуске программы и при непосредственно работе.
* https://agner.org/optimize/optimizing_cpp.pdf c155
* При компиляции динамической библиотеки неизвестно, где она будет располагаться в памяти, это проблема, так как загруженной библиотеке нужно понимать, где искать функции и глобальные переменные. Есть два решения проблемы, про которые можно почитать [на хабре](https://habr.com/ru/company/badoo/blog/323904/https://habr.com/ru/company/badoo/blog/323904/) (там описан один способ и есть ссылка на описание другого способа).
  
<a href="#hw" style="color:#856024">Комментарии к ДЗ</a>


# <a name="create_dynlib"></a> Создание динамической библиотеки 


```cpp
%%cpp lib.c
%MD `-shared` - создаем *разделяемую* библиотеку
%MD `-fPIC` - генерируем *позиционно-независимый код* (Positional Independant Code)
%MD **Скопилируем разделяемую библиотеку:**
%run gcc -Wall -shared -fPIC lib.c -o libsum.so
%MD **Выведем символы из скомпилированной библиотеки, фильтруя их по подстроке `sum`**
%run objdump -t libsum.so | grep sum 

int sum(int a, int b) {
    return a + b;
}

float sum_f(float a, float b) {
    return a + b;
}
```

# <a name="load_python"></a> Загрузка динамической библиотеки из python'а


```python
import ctypes

lib = ctypes.CDLL("./libsum.so")
%p lib.sum(3, 4) # По умолчанию считает типы int'ами, поэтому в этом случае все хорошо
%p lib.sum_f(3, 4) # А здесь python передает в функцию инты, а она принимает float'ы. Тут может нарушаться соглашение о вызовах и происходить что угодно

# Скажем, какие на самом деле типы в сигнатуре функции
lib.sum_f.argtypes = [ctypes.c_float, ctypes.c_float]
lib.sum_f.restype = ctypes.c_float
%p lib.sum_f(3, 4) # Теперь все работает хорошо
```

# <a name="load_с"></a> Загрузка динамической библиотеки из программы на С. Стандартными средствами, автоматически при старте программы


```cpp
%%cpp ld_exec_dynlib_func.c
%MD `-lsum` - подключаем динамическую библиотеку `libsum.so`
%MD `-L.` - во время компиляции ищем библиотеку в директории `.`
%MD `-Wl,-rpath -Wl,'$ORIGIN/'.` - говорим линкеру, чтобы он собрал программу так, чтобы при запуске она искала библиотеку в `'$ORIGIN/'.`. То есть около исполняемого файла программы
%run gcc -Wall -g ld_exec_dynlib_func.c -L. -lsum -Wl,-rpath -Wl,'$ORIGIN/.' -o ld_exec_dynlib_func.exe
%run ./ld_exec_dynlib_func.exe

#include <stdio.h>

// объявляем функции
// ~ #include "sum.h"
int sum(int a, int b);
float sum_f(float a, float b);

int main() {  
    #define p(stmt, fmt) printf(#stmt " = " fmt "\n", stmt);
    p(sum(1, 1), "%d");
    p(sum(40, 5000), "%d");
    
    p(sum_f(1, 1), "%0.2f");
    p(sum_f(4.0, 500.1), "%0.2f");
    return 0;
}
```


```python
!ldd ld_exec_dynlib_func.exe

!mkdir tmp
!cp ld_exec_dynlib_func.exe tmp/ld_exec_dynlib_func.exe
!ldd tmp/ld_exec_dynlib_func.exe
```

# <a name="load_с_std"></a> Загрузка динамической библиотеки из программы на С в произвольный момент времени, используя стандартные функции


```cpp
%%cpp stdload_exec_dynlib_func.c
%MD `-ldl` - пародоксально, но для подгрузки динамических библиотек, нужно подгрузить динамическую библиотеку
%run gcc -Wall -g stdload_exec_dynlib_func.c -ldl -o stdload_exec_dynlib_func.exe
%run ./stdload_exec_dynlib_func.exe

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <assert.h>
#include <dlfcn.h>

typedef float (*binary_float_function)(float, float);

int main() {  
    
    void *lib_handle = dlopen("./libsum.so", RTLD_NOW);
    if (!lib_handle) {
        fprintf(stderr, "dlopen: %s\n", dlerror());
        abort();
    }
   
    int (*sum)(int, int) = dlsym(lib_handle, "sum");
    binary_float_function sum_f = dlsym(lib_handle, "sum_f");
    
    #define p(stmt, fmt) printf(#stmt " = " fmt "\n", stmt);
    p(sum(1, 1), "%d");
    p(sum(40, 5000), "%d");
    
    p(sum_f(1, 1), "%0.2f");
    p(sum_f(4.0, 500.1), "%0.2f");
    
    dlclose(lib_handle);
    return 0;
}
```

# <a name="load_с_std"></a> Загрузка динамической библиотеки из программы на С в произвольный момент времени, используя mmap

В примере отсутствует парсинг elf файла, чтобы выцепить адреса функций. Поэтому они просто захардкожены

Так же не производятся релокации. В общем это чисто образовательный пример, в реальности так делать не надо :)


```cpp
%%cpp mmap_exec_dynlib_func.c
%run gcc -Wall -fsanitize=address -g mmap_exec_dynlib_func.c -o mmap_exec_dynlib_func.exe
%run ./mmap_exec_dynlib_func.exe

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <assert.h>

int main() {
    int fd = open("libsum.so", O_RDWR);
    struct stat s;
    assert(fstat(fd, &s) == 0);
    void* mapped = mmap(
        /* desired addr, addr = */ NULL, 
        /* length = */ s.st_size, 
        /* access attributes, prot = */ PROT_READ | PROT_EXEC | PROT_WRITE, // обратите внимание на PROT_EXEC
        /* flags = */ MAP_SHARED,
        /* fd = */ fd,
        /* offset in file, offset = */ 0
    );
    assert(close(fd) == 0); // Не забываем закрывать файл (при закрытии регион памяти остается доступным)
    if (mapped == MAP_FAILED) {
        perror("Can't mmap");
        return -1;
    }
 
    int (*sum)(int, int) = (void*)((char*)mapped + 0x10f9); // 0x10f9 - тот самый оффсет из objdump'a
    float (*sum_f)(float, float) = (void*)((char*)mapped + 0x1111); 
    
    #define p(stmt, fmt) printf(#stmt " = " fmt "\n", stmt);
    
    p(sum(1, 1), "%d");
    p(sum(40, 5000), "%d");
    
    p(sum_f(1, 1), "%0.2f");
    p(sum_f(4.0, 500.1), "%0.2f");

    assert(munmap(
        /* mapped addr, addr = */ mapped, 
        /* length = */ s.st_size
    ) == 0);
    return 0;
}
```

# <a name="c_interpreter"></a> Простенький интерпретатор для С

Идея такая: на каждый кусочек кода будем компилировать динамическую библиотеку, подгружать ее, и выполнять из нее функцию, в которой будет этот самый кусочек.

Взаимодействие между кусочками через глобальные переменные. (Все кусочки кода выполняются в основном процессе.)

Каждая динамическя библиотека компонуется со всеми предыдущими, чтобы видеть их глобальные переменные. Для этого же при загрузке библиотек берется опция RTLD_GLOBAL.


```python

```


```python
import os
import subprocess
import ctypes
from textwrap import dedent

uniq_counter = globals().get("uniq_counter", 0) + 1
libs = []
all_includes = []
all_variables = []


def add_includes_c(includes):
    global all_includes
    all_includes = list(set(all_includes) | set(includes.split('\n')))

    
def declare_c(declaration):
    assignment_pos = declaration.find('=')
    assignment_pos = assignment_pos if assignment_pos != -1 else len(declaration)
    decl_part = declaration[:assignment_pos].rstrip()
    var_name_begin = decl_part.rfind(' ')
    var_assignment = declaration[var_name_begin:]
    interprete_c(var_assignment, variables=[decl_part])

    
def interprete_c(code="", variables=[]):
    func_name = "lib_func_%d_%d" % (uniq_counter, len(libs))
    source_name = "./tmp/" + func_name + ".c"
    lib_name = "lib" + func_name + ".so"
    lib_file = "./tmp/" + lib_name
    includes_list = "\n".join(all_includes)
    variables_list = "; ".join("extern " + v for v in all_variables) + "; " + "; ".join(variables)
    out_file = "./tmp/" + func_name + ".out" 
    err_file = "./tmp/" + func_name + ".err" 
    lib_code = dedent('''\
        #include <stdio.h>
        {includes_list}
        {variables_list};
        void {func_name}() {{
            freopen("{err_file}", "w", stderr);
            freopen("{out_file}", "w", stdout);
            {code};
            fflush(stderr);
            fflush(stdout);
        }}
        ''').format(**locals())
    with open(source_name, "w") as f:
        f.write(lib_code)
    compile_cmd = (
        ["gcc", "-Wall", "-shared", "-fPIC", source_name, "-Ltmp"] + 
        ['-l' + lib_f for lib_f in libs] + 
        ["-Wl,-rpath", "-Wl," + os.path.join(os.getcwd(), "tmp"), "-o", lib_file]
    )
    try:
        subprocess.check_output(compile_cmd)
    except:
        print("%s\n%s" % (lib_code, " ".join(compile_cmd)))
        get_ipython().run_cell("!" + " ".join(compile_cmd))
        raise
    
    lib = ctypes.CDLL(lib_file, ctypes.RTLD_GLOBAL)  # RTLD_GLOBAL - важно! Чтобы позднее загруженные либы видели ранее загруженные
    func = lib[func_name]
    func()
    for fname, stream in [(err_file, sys.stderr), (out_file, sys.stdout)]:
        with open(fname, "r") as f:
            txt = f.read()
            if txt:
                print(txt, file=stream)
    libs.append(func_name)
    all_variables.extend(variables)
    
```


```python
interprete_c(r'''
    printf("%d", 40 + 2); 
    dprintf(2, "Hello world!");
''')
```


```python
add_includes_c('''
    #include <math.h>"
''')
interprete_c('''
    printf("%f", cos(60.0 / 180 * 3.1415))
''')
```


```python
declare_c('''
   int a = 4242
''')
```


```python
interprete_c('''
    printf("1) %d", a);
''')
interprete_c('''
    printf("2) %06d", a);
''')
interprete_c('''
    printf("3) %6d", a);
''')
interprete_c('''
    printf("4) %0.2f", (float)a);
''')
```


```python
add_includes_c('''
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <fcntl.h>
    #include <unistd.h>
''')
declare_c('''
    int fd = open("./a.txt", O_WRONLY | O_CREAT, 0644)
''')
interprete_c('''
    dprintf(fd, "Hello students! a = %d", a);
    close(fd);
    printf("a.txt written and closed!");
''')
```


```python
!cat a.txt
```

# <a name="cpp"></a> Особенности с С++

[Itanium C++ ABI](https://itanium-cxx-abi.github.io/cxx-abi/abi.html#mangling) - тут есть про манглинг


```cpp
%%cpp libsumcpp.cpp
%run g++ -std=c++11 -Wall -shared -fPIC libsumcpp.cpp -o libsumcpp.so # compile shared library
%run objdump -t libsumcpp.so | grep um

extern "C" {
    int sum_c(int a, int b) {
        return a + b;
    }
} 

int sum_cpp(int a, int b) {
    return a + b;
}

float sum_cpp_f(float a, float b) {
    return a + b;
}

class TSummer {
public:
    TSummer(int a);
    int SumA(int b);
    int SumB(int b) { return a + b; } // Обратите внимание, этой функции нет в символах [1]
    template <typename T>
    int SumC(T b) { return a + b; } // И уж тем более этой [1]
public:
    int a;
};

TSummer::TSummer(int a_arg): a(a_arg) {}
int TSummer::SumA(int b) { return a + b; } 
```

##### <a name="odr_inline"></a> Замечание про наличие символов inline-функций в объектных файлах

[1] - этих функций нет среди символов в данном запуске. Но в общем случае этого не гарантируется, так как методы класса имеют external linkage (класс не объявлен в анонимном namespace).

Но почему же их нет в таблице символов, если у них external linkage? `inline` (в данном случае неявный) позволяет определять функцию в нескольких единицах трансляции при условии, что определение будет одинаковым (смягчается требование [ODR](https://en.cppreference.com/w/cpp/language/definition)). То есть во всех единицах трансляции, где эта, функция используется, она должна быть не просто объявлена, а определена одинаковым образом. Что дает компилятору свободу для оптимизации - он может не создавать символа функции, так как этот символ все равно никому не понадобится для линковки - в других единицах трансляции все равно должно быть такое же определение функции.

<details>
<summary> Больше деталей про <code>inline</code>
    
</summary>
<p>
    
`inline` &mdash; это спецификатор [[cppref]](https://en.cppreference.com/w/cpp/language/inline) [[std.dcl.inline]](http://eel.is/c++draft/dcl.inline), используемый для объявления _inline function_ [[cppref]](https://en.cppreference.com/w/cpp/language/inline#Description)[[std.dcl.inline]](http://eel.is/c++draft/dcl.inline#2), и функции, определённые в теле класса, являются inline [[std.class.mcft]](http://eel.is/c++draft/class.mfct#1)[[std.class.friend]](http://eel.is/c++draft/class.friend#6).

</p>
</details>

[Issue по которому добавлено замечение](https://github.com/yuri-pechatnov/caos_2019-2020/issues/1)


```cpp
%%cpp use_lib_cpp.c
%run gcc -Wall -g use_lib_cpp.c -ldl -o use_lib_cpp.exe
%run ./use_lib_cpp.exe

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <assert.h>
#include <dlfcn.h>

int main() {  
    
    void *lib_handle = dlopen("./libsumcpp.so", RTLD_NOW);
    if (!lib_handle) {
        fprintf(stderr, "dlopen: %s\n", dlerror());
        abort();
    }
    
    int (*sum_c)(int, int) = dlsym(lib_handle, "sum_c");
    int (*sum)(int, int) = dlsym(lib_handle, "_Z7sum_cppii");
    float (*sum_f)(float, float) = dlsym(lib_handle, "_Z9sum_cpp_fff");
    
    #define p(stmt, fmt) printf(#stmt " = " fmt "\n", stmt);
    p(sum_c(1, 1), "%d");
    p(sum_c(40, 5000), "%d");
    
    p(sum(1, 1), "%d");
    p(sum(40, 5000), "%d");
    
    p(sum_f(1, 1), "%0.2f");
    p(sum_f(4.0, 500.1), "%0.2f");
    
    char* objStorage[100];
    void (*constructor)(void*, int) = dlsym(lib_handle, "_ZN7TSummerC1Ei");
    int (*sumA)(void*, int) = dlsym(lib_handle, "_ZN7TSummer4SumAEi");
    
    // f(1, 2, 3) --- , раздяеляет аргументы
    // (1, 3) + 3 --- , - operator, (итоговое значение 6)
    // p((1, 3) + 3, "%d"); // == 6
    p((constructor(objStorage, 10), sumA(objStorage, 1)), "%d"); // operator , - просто делает выполнеяет все команды и берет возвращаемое значение последней
    p((constructor(objStorage, 4000), sumA(objStorage, 20)), "%d"); 
    
    dlclose(lib_handle);
    return 0;
}
```


```python

```


```python

```


```python

```


```python

```


```python

```

# <a name="hw"></a> Комментарии к ДЗ

* 
*
* inf19-2-posix/dl/cpp-class-loader
<br> Задача очень интересная, советую всем сделать :)
<br>
<br> А теперь, немного информации, чтобы сделать ее было проще. Прежде всего в системе есть заголовочный файл, который можно исклюдить в решении. И вам нужно написать cpp-шник, в котором реализованы объявленные хедере функции.

<details>
<summary>interfaces.h</summary>

```cpp

#include <string>

class AbstractClass
{
    friend class ClassLoader;
public:
    explicit AbstractClass();
    ~AbstractClass();
protected:
    void* newInstanceWithSize(size_t sizeofClass);
    struct ClassImpl* pImpl;
};

template <class T>
class Class : public AbstractClass
{
public:
    T* newInstance()
    {
        size_t classSize = sizeof(T);
        void* rawPtr = newInstanceWithSize(classSize);
        return reinterpret_cast<T*>(rawPtr);
    }
};

enum class ClassLoaderError {
    NoError = 0,
    FileNotFound,
    LibraryLoadError,
    NoClassInLibrary
};


class ClassLoader
{
public:
    explicit ClassLoader();
    AbstractClass* loadClass(const std::string &fullyQualifiedName);
    ClassLoaderError lastError() const;
    ~ClassLoader();
private:
    struct ClassLoaderImpl* pImpl;
};
```
</details>

<br> Что вообще должно у вас получиться:
<br> Пусть у вас в каком-то динамической библиотеке реализован класс:

<details>
<summary> module.h </summary>

```cpp
#pragma once

class SimpleClass
{
public:
    SimpleClass();
};
```
</details>

<details>
<summary> module.cpp </summary>

```cpp
#include "module.h"

#include <iostream>

SimpleClass::SimpleClass()
{
    std::cout << "Simple Class constructor called" << std::endl;
}
```
</details>

<br> Вы хотите этот класс загрузить из этой динамической библиотеки:

<details>
<summary> main.cpp </summary>

```cpp
#include "interfaces.h"

#include "module.h"

static ClassLoader * Loader = nullptr;

int testSimpleClass()
{
    Class<SimpleClass>* c = reinterpret_cast<Class<SimpleClass>*> (
		Loader->loadClass("SimpleClass"));
    if (c) {
        SimpleClass* instance = c->newInstance(); // тут произошел аналог new SimpleClass()
        (void)instance; 
        // над уничтожением объекта в этой задаче думать не нужно
        return 0;
    }
    else {
        return 1;
    }
}


int main(int argc, char *argv[])
{
    Loader = new ClassLoader();
    int status = testSimpleClass();
    delete Loader;
    return status;
}

```
</details>



```python

```


```python

```
