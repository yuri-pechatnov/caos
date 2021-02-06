// %%cpp main.cpp
// %run g++ -std=c++17 -Wall -Werror -fsanitize=address -fno-exceptions -fno-rtti main.cpp -o a.exe
// %run ./a.exe 

#include "common.h"

int main() {
    obj20* o20 = new obj20;
    delete o20;
    return 0; 
}

