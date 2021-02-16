// %%cpp main.cpp
// %run g++ -std=c++17 main.cpp lib.cpp -o main.exe
// %run ./main.exe

#include <stdio.h>
#include <iostream>
#include <memory>
#include <vector>
#include <unordered_map>


template <typename T>
struct array2d_t {
    T* arr;
    int n;
    int m;
    
    array2d_t(int n_, int m_) {
        n = n_;
        m = m_;
        arr = new T[n * m];
//         for (int i = 0; i < n * m; ++i) {
//             arr[i] = {};
//         }
    }
    
    T* operator[](int i) {
        return arr + i * m;
    }
    
    ~array2d_t() {
        delete[] arr;
    }
    
};


int main() {
    array2d_t<int> arr(5, 5);
    arr[3][3] = 142;
    
    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 5; ++j) {
            printf("%d ", arr[i][j]);
        }
        printf("\n");
    }
   
    return 0;
}

