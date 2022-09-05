


```python

```


```cpp
%%cpp main.cpp
%run clang++ -Wall -Werror -fsanitize=address main.cpp -o a.exe
%run time -p ./a.exe 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <vector>


int main() {
    std::vector<int> a;
    a.reserve(10000000);
    for (int i = 0; i < 10'000'000; ++i) {
        a.push_back(i);
    }
}
```


```cpp
%%cpp main.cpp
%run clang++ -Wall -Werror -fsanitize=address main.cpp -o a.exe
%run time -p ./a.exe 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <deque>


int main() {
    std::deque<int> a;
    for (int i = 0; i < 10'000'000; ++i) {
        a.push_back(i);
    }
}
```


```python

```


```cpp
%%cpp main.cpp
%run clang++ -Wall -Werror -fsanitize=address main.cpp -o a.exe
%run time -p ./a.exe 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <vector>


int main() {
    std::vector<int> a;
    a.reserve(10000000);
    for (int i = 0; i < 10'000'000; ++i) {
        a.push_back(i);
    }
}
```


```cpp
%%cpp main.cpp
%run clang++ -Wall -Werror -fsanitize=address main.cpp -o a.exe
%run time -p ./a.exe 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <vector>


int main(int argc, char** argv) {
    std::vector<int> a;
    a.reserve(10000000);
    for (int i = 0; i < 100'000'000; ++i) {
        a.push_back(i % argc);
    }
}
```


```cpp
%%cpp main.cpp
%run clang++ -Wall -Werror -fsanitize=address main.cpp -o a.exe
%run time -p ./a.exe 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <vector>


int main(int argc, char** argv) {
    std::vector<int> a;
    a.reserve(10000000);
    for (int i = 0; i < 100'000'000; ++i) {
        a.push_back(i & argc);
    }
}
```


```cpp
%%cpp main.cpp
%run clang++ -Wall -Werror -fsanitize=address main.cpp -o a.exe
%run ./a.exe 

#include <iostream>
#include <vector>
#include <memory>
#include <functional>


template <typename TFunction>
void PrintMap(const std::vector<int>& v, TFunction f) {
    for (int i = 0; i < v.size(); ++i) {
        std::cout << f(v[i]) << std::endl;
    }
    std::cout << std::endl;
}


struct TFuncImpl {
    virtual int operator()(int a) const = 0;
    virtual ~TFuncImpl() = default;
};

template <typename TFunctor>
struct TFuncImpl2 : TFuncImpl {
    TFunctor F;
    TFuncImpl2(TFunctor&& f): F(std::move(f)) {}
    int operator()(int a) const override {
        return F(a);
    }
}

struct TFunc {
    std::unique_ptr<TFuncImpl> FuncImpl;
        
    template <typename TFunctor>
    TFunc(TFunctor f) {
        FuncImpl = std::make_unique<TFuncImpl2<TFunctor>>(std::move(f));
    }
    
    int operator()(int a) const { return (*FuncImpl)(a); }
};



void PrintMap2(const std::vector<int>& v, const TFunc& f) {
    for (int i = 0; i < v.size(); ++i) {
        std::cout << f(v[i]) << std::endl;
    }
    std::cout << std::endl;
}


void PrintMap3(const std::vector<int>& v, const std::function<int(int)>& f) {
    for (int i = 0; i < v.size(); ++i) {
        std::cout << f(v[i]) << std::endl;
    }
    std::cout << std::endl;
}

int F2(int a) { return a * a; }

int main(int argc, char** argv) {
    PrintMap({1, 2, 3}, F2);
    
    struct {
        int x;
        
        int operator()(int a) {
            return a + x;
        }
    } f3 {.x = argc};
    
    PrintMap({1, 2, 3}, f3);
    
    PrintMap({1, 2, 3}, [argc](int a) { return a + argc; });
    
    PrintMap2({1, 2, 3}, [argc](int a) { return a + argc; });
    
    PrintMap3({1, 2, 3}, [argc](int a) { return a + argc; });
    
}
```


```python

```


```cpp
%%cpp main.cpp
%run clang++ -fno-rtti -Wall -Werror -fsanitize=address main.cpp -o a.exe
%run ./a.exe 

#include <iostream>
#include <vector>
#include <memory>
#include <functional>


struct TFuncImpl {
    virtual int operator()(int a) const = 0;
    virtual ~TFuncImpl() = default;
};

struct TFunc {
    std::unique_ptr<TFuncImpl> FuncImpl;
        
    template <typename TFunctor>
    TFunc(TFunctor f) {
        struct TFuncImpl2 : TFuncImpl {
            TFunctor F;
            TFuncImpl2(TFunctor&& f): F(std::move(f)) {}
            int operator()(int a) const override {
                return F(a);
            }
        };
        FuncImpl = std::make_unique<TFuncImpl2>(std::move(f));
    }
    
    int operator()(int a) const { return (*FuncImpl)(a); }
};



void PrintMap(const std::vector<int>& v, const TFunc& f) {
    for (int i = 0; i < v.size(); ++i) {
        std::cout << f(v[i]) << std::endl;
    }
    std::cout << std::endl;
}


int main(int argc, char** argv) {  
    PrintMap({1, 2, 3}, [argc](int a) { return a + argc; });
}
```


```cpp
%%cpp main.cpp
%run clang++ -fno-rtti -Wall -Werror -fsanitize=address main.cpp -o a.exe
%run ./a.exe 

#include <iostream>
#include <vector>
#include <memory>
#include <functional>
#include <new>


struct TFuncDescriptor {
    const char* UniqueAddr;
    int (*Caller)(char*, int);
    void (*Destroyer)(char*);
};


struct TFuncDeleter {
    void operator()(char* func) {
        TFuncDescriptor* desc = (TFuncDescriptor*)(void*)func;
        desc->Destroyer(func + sizeof(TFuncDescriptor));
        delete[] func;
    }
};

struct TFunc {
    std::unique_ptr<char[], TFuncDeleter> FuncImpl;
        
    template <typename TFunctor>
    TFunc(TFunctor f) {
        static const char UniqueVar = '\0';
        
        char* func = new char[sizeof(TFuncDescriptor) + sizeof(TFunctor)];
        TFuncDescriptor* desc = (TFuncDescriptor*)(void*)func;
        desc->UniqueAddr = &UniqueVar;
        desc->Caller = [](char* f, int a) -> int {
            return ((TFunctor*)(void*)f)->operator()(a);
        };
        desc->Destroyer = [](char* f) {
            ((TFunctor*)(void*)f)->~TFunctor();
        };
        new(func + sizeof(TFuncDescriptor)) TFunctor(std::move(f));
        FuncImpl.reset(func);
    }
    
    int operator()(int a) const { 
        TFuncDescriptor* desc = (TFuncDescriptor*)FuncImpl.get();
        return desc->Caller(FuncImpl.get() + sizeof(TFuncDescriptor), a);
    }
};



void PrintMap(const std::vector<int>& v, const TFunc& f) {
    for (int i = 0; i < v.size(); ++i) {
        std::cout << f(v[i]) << std::endl;
    }
    std::cout << std::endl;
}


int main(int argc, char** argv) {  
    PrintMap({1, 2, 3}, [argc](int a) { return a + argc; });
    std::vector<int> m = {0, 10, 20, 30, 40, 50};
    PrintMap({1, 2, 3}, [m](int a) { return m[a]; });
}
```


```cpp
%%cpp main.cpp
%run clang++ -fno-rtti -Wall -Werror -fsanitize=address main.cpp -o a.exe
%run ./a.exe 

#include <iostream>
#include <vector>
#include <memory>
#include <functional>
#include <new>


struct TSquare {
    int A;
    int GetSquare() { return A * A; }
};


struct TShapeDescriptor {
    int (*SquareGetter)(char*);
    void (*Destroyer)(char*);
};


struct TShapeDeleter {
    void operator()(char* func) {
        TShapeDescriptor* desc = (TShapeDescriptor*)(void*)func;
        desc->Destroyer(func + sizeof(TShapeDescriptor));
        delete[] func;
    }
};

struct TShape {
    std::unique_ptr<char[], TShapeDeleter> ShapeImpl;
        
    template <typename TSomeShape>
    TShape(TSomeShape q) {
        char* shape = new char[sizeof(TShapeDescriptor) + sizeof(TSomeShape)];
        TShapeDescriptor* desc = (TShapeDescriptor*)(void*)shape;
        desc->SquareGetter = [](char* shape) -> int {
            return ((TSomeShape*)(void*)shape)->GetSquare();
        };
        desc->Destroyer = [](char* shape) {
            ((TSomeShape*)(void*)shape)->~TSomeShape();
        };
        new(shape + sizeof(TShapeDescriptor)) TSomeShape(std::move(q));
        ShapeImpl.reset(shape);
    }
    
    int GetSquare() const { 
        TShapeDescriptor* desc = (TShapeDescriptor*)ShapeImpl.get();
        return desc->SquareGetter(ShapeImpl.get() + sizeof(TShapeDescriptor));
    }
};


int main(int argc, char** argv) { 
    std::cout << TShape(TSquare{.A = 3}).GetSquare() << std::endl;
}
```


```cpp
%%cpp main.cpp
%run clang++ -fno-rtti -Wall -Werror -fsanitize=address main.cpp -o a.exe
%run ./a.exe 

#include <iostream>
#include <vector>
#include <memory>
#include <functional>
#include <new>


struct TFuncDescriptor {
    const char* UniqueAddr;
    int (*Caller)(const char*, int);
    void (*ConstructCopier)(char*, const char*);
    void (*Destroyer)(char*);
};


struct TFunc {
    TFuncDescriptor Descriptor;
        
    template <typename TFunctor>
    TFunc(TFunctor f) {
        static_assert(sizeof(TFunctor) <= sizeof(FunctorStorage), "too large functor");
        static const char UniqueVar = '\0';
        
        Descriptor.UniqueAddr = &UniqueVar;
        Descriptor.Caller = [](const char* f, int a) -> int {
            return ((TFunctor*)(void*)f)->operator()(a);
        };
        Descriptor.ConstructCopier = [](char* dst, const char* src) {
            new(dst) TFunctor(*(TFunctor*)(void*)src);
        };
        Descriptor.Destroyer = [](char* f) {
            ((TFunctor*)(void*)f)->~TFunctor();
        };
        new(FunctorStorage) TFunctor(std::move(f));
    }
    
    TFunc(const TFunc& other) {
        operator=(other);
        Descriptor = other.Descriptor;
        if (Descriptor.UniqueAddr) {
            Descriptor.ConstructCopier(FunctorStorage, other.FunctorStorage);
        }
    }
    
    TFunc& operator=(const TFunc& other) {
        if (Descriptor.UniqueAddr) {
            Descriptor.Destroyer(FunctorStorage);
        }
        Descriptor = other.Descriptor;
        if (Descriptor.UniqueAddr) {
            Descriptor.ConstructCopier(FunctorStorage, other.FunctorStorage);
        }
        return *this;
    }
    
    ~TFunc() {
        if (Descriptor.UniqueAddr) {
            Descriptor.Destroyer(FunctorStorage);
        }
    }
    
    int operator()(int a) const { 
        return Descriptor.Caller(FunctorStorage, a);
    }
};



void PrintMap(const std::vector<int>& v, const TFunc& f) {
    for (int i = 0; i < v.size(); ++i) {
        std::cout << f(v[i]) << std::endl;
    }
    std::cout << std::endl;
}


int main(int argc, char** argv) {  
    PrintMap({1, 2, 3}, [argc](int a) { return a + argc; });
    std::vector<int> m = {0, 10, 20, 30, 40, 50};
    PrintMap({1, 2, 3}, [m](int a) { return m[a]; });
}
```


```python

```


```python

```
