// %%cpp main.cpp
// %run g++ main.cpp lib.cpp -o main.exe
// %run echo "10.1 20.2" | ./main.exe

#include <stdio.h>
#include <iostream>
#include <iomanip>

struct point_t {
    double x;
    double y;

    point_t operator-(const point_t& b) const {
        return point_t{.x = x - b.x, .y = y - b.y};
    }
    
    point_t operator*(double k) const {
        return {.x = x * k, .y = y * k};
    }
    
    static point_t read(FILE* file) {
        point_t p;
        fscanf(file, "%lf%lf", &p.x, &p.y); 
        return p;
    }
    void write(FILE* file) const {
        fprintf(file, "{.x = %lf, .y = %lf}", x, y);
    }
};

std::istream& operator>>(std::istream& in, point_t& p) {
    return in >> p.x >> p.y;
}

std::ostream& operator<<(std::ostream& out, const point_t& p) {
    return out << "{" << std::fixed << std::setprecision(3) << p.x << ", " << p.y << "}";
}

int main() {
    //(point_t::read(stdin) * 2).write(stdout);
    point_t p;
    std::cin >> p;
    std::cout << (p * 2);
    return 0;
}

