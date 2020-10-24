// %%cpp look_at_addresses.c --under-spoiler-threshold 30
// %run gcc -m64 -masm=intel -O0 look_at_addresses.c -o look_at_addresses.exe
// %run ./look_at_addresses.exe
// %run gcc -S -m64 -masm=intel -Os look_at_addresses.c -o /dev/stdout

#include <stdio.h>
#include <stdlib.h>

int func(int a) {
    return a;
}


int* func_static_initialized() {
    static int a = 4;
    return &a;
}

const int* func_static_const_initialized() {
    static const int a = 4;
    return &a;
}

int* func_static_not_initialized() {
    static int a;
    return &a;
}


int global_initialized[123] = {1, 2, 3};
const int global_const_initialized[123] = {1, 2, 3};
int global_not_initialized[123];

int main2() {
   int local2 = 5;
   printf("Local 'local2' addr = %p\n", &local2); 
}


int main() {
    int local = 1;
    static int st = 2;
    int* all = malloc(12);
    
    printf("Func func addr = %p\n", (void*)func);
    printf("Func func_s addr = %p\n", (void*)func_static_initialized);
    
    printf("Global var (initialized) addr = %p\n", global_initialized);
    printf("Global var (const initialized) addr = %p\n", global_const_initialized);
    printf("Global var (not initialized) addr = %p\n", global_not_initialized);
    
    printf("Static 'st' addr = %p\n", &st);
    printf("Static 'func_static_initialized.a' addr = %p\n", func_static_initialized());
    printf("Static 'func_static_const_initialized.a' addr = %p\n", func_static_const_initialized());
    printf("Static 'func_static_not_initialized.a' addr = %p\n", func_static_not_initialized());
    
    printf("Local 'local' addr = %p\n", &local);
    main2();
    
    printf("Heap 'all' addr = %p\n", all);
    free(all);
    return 0;
}

