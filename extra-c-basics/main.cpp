// %%cpp main.cpp
// %run clang++ -std=c++17 -Wall -Werror -fsanitize=address main.cpp -o a.exe
// %run ./a.exe 

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <exception>
#include <stdexcept>


int f(int a) {
    if (a > 40000)
        throw std::runtime_error("a too big");
    return a * a; 
}

int f2(int a, int b) {
    int64_t res = f(a) + f(b);
    if (res > 2000000000) {
        throw std::runtime_error("a + b too big");
    }
    return res;
}

int main() {
    try {
        int a = 1000000;
        int x = f(a);
        printf("Success, res = %d\n", x);  
    } catch (const std::exception& e) {
        printf("Error: %s\n", e.what());  
    }
    return 0;
}

