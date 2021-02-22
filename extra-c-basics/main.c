// %%cpp main.c
// %run clang -std=c99 -Wall -Werror -fsanitize=address main.c -o a.exe
// %run ./a.exe 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


template <typename T>
struct TErrorOr {
    bool IsOk;
    T Value;
    std::string Err;
    
    TErrorOr(T value) {
        IsOk = true;
        Value = value;
    }
};

template <typename T>
TErrorOr<T> CreateError(std::string str) {
    TErrorOr<T> err;
    err.IsOk = false;
    err.Err = str;
}


TErrorOr<int> f(int a) {
    if (a > 40000)
        return CreateError<int>("a too big");
    return a * a; 
}

int main() {
    int a = 2000000000;
    TErrorOr<int> x = f(a);
    if (!x.IsOk) {
        printf("Error\n");
    } else {
        printf("Success, res = %d\n", x);    
    }
    return 0;
}

