// %%cpp look_at_addresses.c
// %run gcc -m32 -masm=intel -O0 look_at_addresses.c -o look_at_addresses.exe
// %run ./look_at_addresses.exe

#include <stdio.h>
#include <stdlib.h>

int func(int a) {
    return a;
}

int* func_s() {
    static int a;
    return &a;
}

int data[123] = {1, 2, 3};


int main2() {
   int local2 = 5;
   printf("Local 'local2' addr = %p\n", &local2); 
}


int main() {
    int local = 1;
    static int st = 2;
    int* all = malloc(12);
    
    printf("Func func addr = %p\n", (void*)func);
    printf("Func func_s addr = %p\n", (void*)func_s);
    printf("Global var addr = %p\n", data);
    
    printf("Static 'st' addr = %p\n", &st);
    printf("Static 'func_s.a' addr = %p\n", func_s());
    
    printf("Local 'local' addr = %p\n", &local);
    main2();
    
    printf("Heap 'all' addr = %p\n", all);
    
    return 0;
}

