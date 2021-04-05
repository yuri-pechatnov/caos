


```python

```


```python

```


```cpp
%%cpp main.cpp
%run clang++ -std=c++17 -Wall -Werror -fsanitize=address main.cpp -o a.exe
%run ./a.exe 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <vector>
#include <set>

template <typename T1, typename T2>
struct TVariant {
    
    TVariant(): TVariant(T1{}) {}
    TVariant(const T1& t1): Type(1), Obj1(t1) {}
    TVariant(const T2& t2): Type(2), Obj2(t2) {}
    
    ~TVariant() {
        if (Type == 1) {
            Obj1.~T1();
        } else {
            Obj2.~T2();
        }
    }
    
    template <typename T>
    T& As() {
        if constexpr (std::is_same_v<T, T1>) {
            return Obj1;
        } else if constexpr (std::is_same_v<T, T2>) {
            return Obj2;
        } else {
            //T{} * T{} + std::vector<int>{};
            //static_assert(false, "can't compile this");
        }
    }
    
private:
    int Type;
    union {
        T1 Obj1;
        T2 Obj2;
    };
};

// {"field1": 12}
// {"field2": [1, 2, 3], "fieldX": "value"}


int main() {
    TVariant<std::vector<int>, std::set<int>> va(std::set<int>{});
    va.As<std::set<int>>().insert(1);
    //va.As<int>().insert(1);
    
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
