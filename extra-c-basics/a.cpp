// %%cpp a.cpp

typedef struct {
    int x, y;
} point_t;

// 1 пункт
int compare_struct(point_t* a, point_t* b) {
    // напишите компаратор такой же, какой нужен для qsort
    // используйте man qsort в консоли или поисковике
}

// 2 пунтк
void quadratic_sort(
    void* base, size_t array_size, size_t elem_size, 
    int (*comparator)(const void *, const void *)
) {
    // напишите квадратичную сортировку
}

// 3 пункт
void do_test_1() {
    // напишите тесты на quadratic_sort с использованием структуры point_t
}


// 4 пункт
#define DECLARE_SORT_FUNCTION(name, type) //...???....

DECLARE_SORT_FUNCTION(sort_int, int);

void do_test_2() {
    
}


int main() {
    do_test_1();
    do_test_2();
    fprintf(stderr, "SUCCESS\n");
    return 0;
}






