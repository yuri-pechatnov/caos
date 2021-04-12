// %%cpp main.cpp
// %run clang++ -fno-rtti -Wall -Werror -fsanitize=address main.cpp -o a.exe
// %run ./a.exe 

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

