// %%cpp main.cpp
// %run clang++ -fno-rtti -std=c++17 -Wall -Werror -fsanitize=address main.cpp -o a.exe
// %run ./a.exe 

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

