// %%cpp ld_exec_dynlib_func.c
// %# `-lsum` - подключаем динамическую библиотеку `libsum.so`
// %# `-L.` - во время компиляции ищем библиотеку в директории `.`
// %# `-Wl,-rpath -Wl,'$ORIGIN/'.` - говорим линкеру, чтобы он собрал программу так
// %# чтобы при запуске она искала библиотеку в `'$ORIGIN/'.`. То есть около исполняемого файла программы
// %run gcc -Wall -g ld_exec_dynlib_func.c -L. -lsum -Wl,-rpath -Wl,'$ORIGIN/'. -o ld_exec_dynlib_func.exe
// %run ./ld_exec_dynlib_func.exe

#include <stdio.h>

// объявляем функции
// ~ #include "sum.h"
int sum(int a, int b);
float sum_f(float a, float b);

int main() {  
    #define p(stmt, fmt) printf(#stmt " = " fmt "\n", stmt);
    p(sum(1, 1), "%d");
    p(sum(40, 5000), "%d");
    
    p(sum_f(1, 1), "%0.2f");
    p(sum_f(4.0, 500.1), "%0.2f");
    return 0;
}

