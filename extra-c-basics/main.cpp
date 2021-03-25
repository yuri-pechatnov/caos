// %%cpp main.cpp
// %run clang++ -Wall -Werror -fsanitize=address main.cpp -o a.exe
// %run ./a.exe 

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

