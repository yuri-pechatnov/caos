// %%cpp segfault_2.cpp

#include<stdio.h>

int get_element(int i);

int main() {
    printf("%d\n", get_element(100500));// проезд по памяти
}

