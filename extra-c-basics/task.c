// %%cpp task.c
// %run gcc -fsanitize=address task.c -o task.exe
// %run ./task.exe

#include <stdio.h>
#include <string.h>
#include <assert.h>


typedef struct {
    int x, y;
} point_t;

// 1 пункт
int compare_struct(point_t* a, point_t* b) {
    // напишите компаратор такой же, какой нужен для qsort
    // используйте man qsort в консоли или поисковике
    if (a->x != b->x) 
        return a->x - b->x;
    return a->y - b->y;
}

// 2 пункт
void quadratic_sort(
    void* base, size_t array_size, size_t elem_size, 
    int (*comparator)(const void *, const void *)
) {
    // напишите квадратичную сортировку
    char* base_c = (char*)base;
    char tmp_buff[elem_size];
    for (int i = 0; i < array_size; ++i) {
        for (int j = i + 1; j < array_size; ++j) {
            char* a = base + i * elem_size;
            char* b = base + j * elem_size;
            if (comparator(a, b) > 0) {
                memcpy(tmp_buff, a, elem_size);
                memcpy(a, b, elem_size);
                memcpy(b, tmp_buff, elem_size);
            }
        }
    }
}

// 3 пункт
void do_test_1() {
    // напишите тесты на quadratic_sort с использованием структуры point_t
    {
        point_t arr[2] = {{3, 4}, {1, 2}};
        quadratic_sort(arr, sizeof(arr) / sizeof(point_t), sizeof(point_t), 
                       (int (*)(const void *, const void *))compare_struct);
        assert(arr[0].x == 1);
        assert(arr[0].y == 2);
        assert(arr[1].x == 3);
        assert(arr[1].y == 4);
    }
    {
        point_t arr[] = {{3, 4}, {1, 2}, {-1, -3}, {2, 10}, {-1, -5}};
        quadratic_sort(arr, sizeof(arr) / sizeof(point_t), sizeof(point_t), 
                       (int (*)(const void *, const void *))compare_struct);
        assert(arr[0].x == -1);
        assert(arr[0].y == -5);
        assert(arr[1].x == -1);
        assert(arr[1].y == -3);
        assert(arr[2].x == 1);
        assert(arr[2].y == 2);
        
        assert(arr[3].x == 2);
        assert(arr[3].y == 10);
        
        assert(arr[4].x == 3);
        assert(arr[4].y == 4);
    }
}


// 4 пункт
// напишите макрос, который будет создавать функцию сортировки для стандартных типов
// (использовать обычный < для сравнения)
// при этом делегировать сортировку функции quadratic_sort
#define DECLARE_SORT_FUNCTION_IMPL_2(name, type, tag)                                     \
    int compare_##tag(type* a, type* b) {                                                 \
        return (*a < *b) ? -1 : ((*a > *b) ? 1 : 0);                                      \
    }                                                                                     \
    void name(type* first, type* last) {                                                  \
        quadratic_sort(first, last - first, sizeof(*first),                               \
            (int (*)(const void *, const void *))compare_##tag);                          \
    }

#define DECLARE_SORT_FUNCTION_IMPL(name, type, tag) DECLARE_SORT_FUNCTION_IMPL_2(name, type, tag)
#define DECLARE_SORT_FUNCTION(name, type) DECLARE_SORT_FUNCTION_IMPL(name, type, __LINE__)


DECLARE_SORT_FUNCTION(sort_int, int);
DECLARE_SORT_FUNCTION(sort_double, double);

// 5 пункт
void do_test_2() {
    // протестируйте, что функция sort_int правильно работает
    {
        int a[] = {1, 3, 2, 4};
        sort_int(a, a + sizeof(a) / sizeof(int));
        assert(a[0] == 1);
        assert(a[1] == 2);
        assert(a[2] == 3);
        assert(a[3] == 4);
    }
    {
        int a[] = {3, 1};
        sort_int(a, a + sizeof(a) / sizeof(int));
        assert(a[0] == 1);
        assert(a[1] == 3);
    }
    
    {
        double a[] = {1, 3, 2, 4};
        sort_double(a, a + sizeof(a) / sizeof(double));
        assert(a[0] == 1);
        assert(a[1] == 2);
        assert(a[2] == 3);
        assert(a[3] == 4);
    }
    {
        double a[] = {3, 1};
        sort_double(a, a + sizeof(a) / sizeof(double));
        assert(a[0] == 1);
        assert(a[1] == 3);
    }
}


int main() {
    //do_test_1();
    do_test_2();
    fprintf(stderr, "SUCCESS\n");
    return 0;
}






