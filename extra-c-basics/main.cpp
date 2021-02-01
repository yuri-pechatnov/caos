// %%cpp main.cpp
// %run g++ -std=c++17 -Wall -Werror -fsanitize=address -fno-exceptions -fno-rtti main.cpp -o a.exe
// %run ./a.exe 

#include <stdio.h>
#include <math.h>

struct vec_t {
    double x;
    double y;
    
    double norm() const {
        return std::sqrt(x * x + y * y);
    }
    vec_t operator+(const vec_t& b) const {
        return {this->x + b.x, this->y + b.y};
    }
    vec_t operator-() const {
        return {-x, -y};
    }
    void print() const {
        printf("{%lf, %lf}\n", x, y);
    }
};

vec_t operator*(const vec_t& a, double k) {
    return {a.x * k, a.y * k};
}

int main() {
    vec_t{1, 2}.print();
    vec_t a = {10, 20};
    vec_t b = {100, 200};
    (a + b).print();
    (-a).print();
    (a * -2).print();
    return 0; 
}

