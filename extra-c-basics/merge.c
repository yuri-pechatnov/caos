// %%cpp merge.c
// %run gcc --sanitize=address merge.c -o merge.exe
// %run ./merge.exe

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef int elem_t;

void merge_sort_impl(elem_t* a, int size, elem_t* buff) {
    if (size <= 1) {
        return;
    }
    int lsize = size / 2;
    int rsize = size - lsize;
    
    merge_sort_impl(a, lsize, buff);
    merge_sort_impl(a + lsize, rsize, buff);
    
    int L = 0, R = lsize;
    int k = 0;
    while (L < lsize && R < size) {
        if (a[L] < a[R]) {
            buff[k++] = a[L++];
        } else {
            buff[k++] = a[R++];    
        }
    }
    while (L < lsize) {
        buff[k++] = a[L++];
    }
    while (R < size) {
        buff[k++] = a[R++];
    }
    memcpy(a, buff, size * sizeof(elem_t));
}


void merge_sort(elem_t* a, int size) {
//     if (size < 1000000 / sizeof(elem_t)) {
        elem_t buff[size];
        merge_sort_impl(a, size, buff);
//     } else {
//         elem_t* buff = calloc(size, sizeof(elem_t));
//         merge_sort_impl(a, size, buff);
//         free(buff);
//     }
}



int main() {
    int a[] = {1, 5, 8, 2, 4, 9, 3};
    int a_size = sizeof(a) / sizeof(a[0]);
    
    merge_sort(a, a_size);
        
    for (int i = 0; i < a_size; ++i) {
        printf("%d, ", a[i]);
    }
    printf("\n");
    return 0;
}

