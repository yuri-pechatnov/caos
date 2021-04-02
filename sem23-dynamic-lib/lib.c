// %%cpp lib.c
// %MD `-shared` - создаем *разделяемую* библиотеку
// %MD `-fPIC` - генерируем *позиционно-независимый код* (Positional Independant Code)
// %MD **Скопилируем разделяемую библиотеку:**
// %run gcc -Wall -shared -fPIC lib.c -o libsum.so
// %MD **Выведем символы из скомпилированной библиотеки, фильтруя их по подстроке `sum`**
// %run objdump -t libsum.so | grep sum 

int sum(int a, int b) {
    return a + b;
}

float sum_f(float a, float b) {
    return a + b;
}

