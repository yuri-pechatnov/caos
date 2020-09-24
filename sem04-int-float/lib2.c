// %%cpp lib2.c
// %run gcc -O3 -shared -fPIC lib2.c -o lib2.so

typedef unsigned int uint;

int sum(int x, int y) { return x + y; }
uint usum(uint x, uint y) { return x + y; }

int mul(int x, int y) { return x * y; }
uint umul(uint x, uint y) { return x * y; }

int cmp(int x, int y) { return x < y; }
int ucmp(uint x, uint y) { return x < y; }

int div(int x, int y) { return x / y; }
int udiv(uint x, uint y) { return x / y; }

