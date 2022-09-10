// %%cpp spec.c
// %// .exe не имеет никакого практического смысла с точки зрения запуска программы
// %// однако позволяет выбирать все бинари регуляркой *.exe, автор кода этим пользуется
// %run gcc spec.c -o spec.exe -fsanitize=address  
// %run ./spec.exe

#include <stdio.h>
#include <inttypes.h>

int main() {
    printf("s = %.*s\n", 4, "01234567" + 2); // Распечатать 4 символа, начиная с 2го
    uint32_t i_32 = 42;
    printf("i_32 = %" PRIu32 ", sizeof(i_32) = %d\n", i_32, (int)sizeof(i_32)); // Совместимый макрос PRId32
    
    // Не очень просто безопасно прочитать строчку)
    char buffer[10];
    int max_len = (int)sizeof(buffer) - 1, actual_len = 0;
    char format[32]; // Гарантированно достаточный буффер для генерируемой форматной строки.
    snprintf(format, sizeof(format), "%%%ds%%n", (int)max_len);
    if (sscanf("string_input_input_input", format, buffer, &actual_len) == 1 && actual_len != max_len) {
        printf("complete read: %s\n", buffer);
    } else {
        printf("incomplete read: %s\n", buffer);
    }
    return 0;
}

