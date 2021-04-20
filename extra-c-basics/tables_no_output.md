


```python

```


```cpp
%%cpp main.cpp
%run clang++ -fno-rtti -std=c++17 -Wall -Werror -fsanitize=address main.cpp -o a.exe
%run ./a.exe 

#include <iostream>
#include <vector>
#include <memory>
#include <functional>
#include <new>


struct IBaseObject {
    template <typename TObject>
    static void DestructObject(char* obj) {
        ((TObject*)(void*)obj)->~TObject();
    }
    struct TVirtualTable {
        TVirtualTable* ParentClassTable;
        void (*Destructor)(char*);
    };
    inline static TVirtualTable OwnClassTable = {
        nullptr,
        &DestructObject<IBaseObject>,
    };
public:
    IBaseObject(TVirtualTable* table = &IBaseObject::OwnClassTable): VirtualTable(table) {}
protected:
    ~IBaseObject() = default;
public:
    TVirtualTable* VirtualTable;
};

template <typename TObject, typename ...TArgs>
TObject* NewObject(TArgs... args) {
    auto* obj = reinterpret_cast<TObject*>(std::aligned_alloc(alignof(TObject), sizeof(TObject)));
    new(obj) TObject(std::forward<TArgs>(args)...);
    return obj;
}

void DeleteObject(IBaseObject* object) {
    object->VirtualTable->Destructor(reinterpret_cast<char*>(object));
    std::free(object);
}

template <typename TObject>
TObject* DynamicCast(IBaseObject* object) {
    IBaseObject::TVirtualTable* table = object->VirtualTable; 
    for (; table; table = table->ParentClassTable) {
        if (&TObject::OwnClassTable == table) {
            return static_cast<TObject*>(object);
        }
    }
    return nullptr;
}

struct IAnimal : IBaseObject {
    template <typename TObject>
    static int GetAgeOfObject(char* obj) {
        return ((TObject*)(void*)obj)->GetAgeImpl();
    }
    struct TVirtualTable : IBaseObject::TVirtualTable {
        int (*AgeGetter)(char*);
    };
    inline static TVirtualTable OwnClassTable = {
        {&IBaseObject::OwnClassTable, &DestructObject<IAnimal>},
        &GetAgeOfObject<IAnimal>,
    };
public:
    IAnimal(TVirtualTable* table = &IAnimal::OwnClassTable): IBaseObject(table) {}
    int GetAge() { return static_cast<IAnimal::TVirtualTable*>(VirtualTable)->AgeGetter(reinterpret_cast<char*>(this)); }
    int GetAgeImpl() { return 0; }
};

struct THuman : IAnimal {
    using TVirtualTable = IAnimal::TVirtualTable;
    inline static TVirtualTable OwnClassTable = {
        {&IAnimal::OwnClassTable, &DestructObject<THuman>},
        &GetAgeOfObject<THuman>,
    };
public:
    THuman(int age, TVirtualTable* table = &THuman::OwnClassTable): IAnimal(table), Age(age) {}
    int GetAgeImpl() { return Age; }
private:
    int Age;
};

struct THamster : IAnimal {
    using TVirtualTable = IAnimal::TVirtualTable;
    inline static TVirtualTable OwnClassTable = {
        {&IAnimal::OwnClassTable, &DestructObject<THamster>},
        &GetAgeOfObject<THamster>,
    };
public:
    THamster(int age, TVirtualTable* table = &THamster::OwnClassTable): IAnimal(table), Age2(age) {}
    int GetAgeImpl() { return Age2; }
private:
    int Age2;
};



int main(int argc, char** argv) {  
    auto* obj = NewObject<IBaseObject>();
    DeleteObject(obj);
    
    IAnimal* ani = NewObject<IAnimal>();
    std::cout << ani->GetAge() << std::endl;
    DeleteObject(ani);
    
    IAnimal* hum = NewObject<THuman>(45);
    std::cout << hum->GetAge() << std::endl;
    
    
    IAnimal* ham = NewObject<THamster>(2);
    std::cout << ham->GetAge() << std::endl;
    
    
    std::cout << "cast Human as Human: " << DynamicCast<THuman>(hum) << std::endl;
    std::cout << "cast Hamster as Human: " << DynamicCast<THuman>(ham) << std::endl;
    std::cout << "cast Hamster as Animal: " << DynamicCast<IAnimal>(ham) << std::endl;
    
    DeleteObject(hum);
    DeleteObject(ham);
}
```


```cpp
%%cpp main.cpp
%run clang++ -fno-rtti -std=c++17 -Wall -Werror -fsanitize=address main.cpp -o a.exe
%run ./a.exe 

#include <iostream>
#include <vector>
#include <memory>
#include <functional>
#include <new>

template <typename T>
struct TArrayRef {
    T* Begin;
    T* End; 
    
    struct TIterator {
        T* Ptr;
        
        void operator++() { ++Ptr; }
        bool operator!=(const TIterator& other) const { return Ptr != other.Ptr; }
        T& operator*() { return *Ptr; }
    };
    
    TIterator begin() { return TIterator{Begin}; }
    TIterator end() { return TIterator{End}; }
};

int main(int argc, char** argv) {  
    int a[] = {1, 2, 3, 4, 5, 6};
    
    for (int x : TArrayRef<int>{a + 2, a + 5}) {
        std::cout << x << ", ";
    }
    std::cout << "\n";
    {
        auto&& c = TArrayRef<int>{a + 2, a + 5};
        auto first = std::begin(c);
        auto last = std::end(c);
        for (auto it = first; it != last; ++it) {
            int x = *it;
            std::cout << x << ", ";
        }
    }

}
```


```python

```


```python

```


```python

```
