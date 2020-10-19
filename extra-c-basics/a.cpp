// %%cpp a.cpp

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
    }
}


// 4 пункт
// напишите макрос, который будет создавать функцию сортировки для стандартных типов
// (использовать обычный < для сравнения)
// при этом делегировать сортировку функции quadratic_sort
// #define DECLARE_SORT_FUNCTION(name, type) // ...???.... quadratic_sort(....) ....

// DECLARE_SORT_FUNCTION(sort_int, int);

// // 5 пункт
// void do_test_2() {
//     // протестируйте, что функция sort_int правильно работает
// }


int main() {
    do_test_1();
//     do_test_2();
    fprintf(stderr, "SUCCESS\n");
    return 0;
}






