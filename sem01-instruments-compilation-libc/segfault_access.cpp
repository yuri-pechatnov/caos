// %%cpp segfault_access.cpp

#include<stdio.h>

int a[] = {41, 42};

int get_element(int i) {
    return a[i]; // проезд по памяти при некорректных i
}

