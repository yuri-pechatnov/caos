
#include <stdio.h>
#include <assert.h>

double mul(double a);
double mul2(double a, double b);
    
__asm__ (R"(
.text
mul:
    movsd    xmm0, [esp+4]  
    lea      eax, .mconst13
    mulsd    xmm0, QWORD PTR [eax]
    movsd    [esp+4], xmm0
    fld      QWORD PTR [esp+4]
    ret
mul2:
    movsd   xmm0, [esp+12]  
    mulsd   xmm0, QWORD PTR [esp+4]
    movsd   [esp+12], xmm0
    fld     QWORD PTR [esp+12]
    ret
.mconst13:
    .long 0
    .long 1076494336
)");

int main() {
    printf("mul(1.5) = %0.9lf\n", mul(1.5));
    printf("mul2(2.1, 20) = %0.9lf\n", mul2(2.1, 20));
    return 0;
}



