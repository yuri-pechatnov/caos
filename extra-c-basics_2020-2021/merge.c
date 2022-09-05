// %%cpp merge.c
// %run gcc -Wall -Werror --sanitize=address merge.c -o merge.exe
// %run ./merge.exe

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MODULE 10

typedef struct {
    int len;
    int* digits;
} big_number_t;

big_number_t to_big_number(char* s) {
    // "1234" aka {'1', '2', '3', '4'}
    // ->
    // {4, 3, 2, 1}
    // real value = 4 * 10**0 + 3 * 10**1 + 2 * 10**2 + 1 * 10**3
    big_number_t result;
    result.len = strlen(s);
    result.digits = calloc(result.len, sizeof(int));
    for (int i = 0; i < result.len; ++i) {
        result.digits[i] = s[result.len - 1 - i] - '0'; // '0'..'9' - '0' ~ 0..9
    }
    return result;
}

void destroy_big_number(big_number_t n) {
    free(n.digits);
    n.digits = NULL;
}

big_number_t add_big_number(big_number_t a, big_number_t b) {
    if (a.len < b.len) {
        return add_big_number(b, a);
    }
    big_number_t c;
    c.len = a.len + 1;
    c.digits = calloc(c.len, sizeof(int));
    for (int i = 0; i < b.len; ++i) {
        c.digits[i] += a.digits[i] + b.digits[i];
        c.digits[i + 1] += c.digits[i] / MODULE;
        c.digits[i] %= MODULE;
    }
    for (int i = b.len; i < a.len; ++i) {
        c.digits[i] += a.digits[i];
        c.digits[i + 1] += c.digits[i] / MODULE;
        c.digits[i] %= MODULE;
    }
    while (c.len && c.digits[c.len - 1] == 0) {
        --c.len;
    }
    return c;
}

big_number_t mul_big_number(big_number_t a, big_number_t b) {
    big_number_t c;
    c.len = a.len + b.len;
    c.digits = calloc(c.len, sizeof(int));
    for (int i = 0; i < a.len; ++i) {      
        for (int j = 0; j < b.len; ++j) {
            c.digits[i + j] += a.digits[i] * b.digits[j];
        }
    }
    for (int i = 0; i + 1 < c.len; ++i) {
        c.digits[i + 1] += c.digits[i] / MODULE;
        c.digits[i] %= MODULE;
    }
    while (c.len && c.digits[c.len - 1] == 0) {
        --c.len;
    }
    return c;
}

big_number_t div_small_big_number(big_number_t a, int b, int* remainder) {
    big_number_t c;
    c.len = a.len;
    c.digits = calloc(c.len, sizeof(int));
    int r = 0;
    for (int i = a.len - 1; i >= 0; --i) {
        int cur = r * MODULE + a.digits[i];
        c.digits[i] = cur / b;
        r = cur % b;
    }
    if (remainder) {
        *remainder = r;
    }
    while (c.len && c.digits[c.len - 1] == 0) {
        --c.len;
    }
    return c;
}

void print_big_number(big_number_t n) {
    for (int i = n.len - 1; i >= 0; --i) {
        printf("%c", '0' + n.digits[i]);
    }
}


int main() {
    char* a = "999999999999999999";
    char* b = "999999999999999999";
    big_number_t a_n = to_big_number(a);
    big_number_t b_n = to_big_number(b);
    big_number_t c_n = add_big_number(a_n, b_n);
    big_number_t d_n = div_small_big_number(c_n, 11, NULL);
    print_big_number(a_n); printf("\n");
    print_big_number(b_n); printf("\n");
    print_big_number(c_n); printf("\n");
    print_big_number(d_n); printf("\n");
    
    destroy_big_number(a_n); 
    destroy_big_number(c_n); 
    destroy_big_number(b_n); 
    destroy_big_number(d_n); 
    
    return 0;
}

