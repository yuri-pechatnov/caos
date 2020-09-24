// %%cpp stand.cpp
// %run gcc stand.cpp -o stand.exe
// %run ./stand.exe

#include <math.h>
#include "stand.h"

int main() {
    double dd[] = {1.5, NAN, -NAN, 0.0 / 0.0, INFINITY, -INFINITY, 0};
    print_doubles(dd);
}

