// %%cpp io.c
// %// .exe не имеет никакого практического смысла с точки зрения запуска программы
// %// однако позволяет выбирать все бинари регуляркой *.exe, автор кода этим пользуется
// %run gcc io.c -o io.exe -fsanitize=address  
// %run echo 42 | ./io.exe 7
// %run cat out.txt

#undef NDEBUG // Ensure assert works.
#include <assert.h>
#include <stdio.h>

int main(int argc, char** argv) {
    assert(argc == 2);
    int cmd_arg_value = 0, stdin_value = 0;
    assert(sscanf(argv[1], "%d", &cmd_arg_value) == 1); // Чтение из строки
    int printf_ret = printf("stdout: cmd_arg_value = %d\n", cmd_arg_value); // Запись в stdout. 
    assert(printf_ret > 0); // Проверять, что нет ошибки полезно.
    assert(printf_ret == 26); // Если нет ошибки, то в printf_ret количество записанных символов. Такое проверять не надо, разумеется.
    assert(fprintf(stderr, "stderr: cmd_arg_value = %d\n", cmd_arg_value) > 0); // Запись в stderr.
    
    assert(scanf("%d", &stdin_value) > 0);
    char buf[100];
    int snprintf_ret = snprintf(buf, sizeof(buf), "stdin_value = %d", stdin_value); // Печать в буфер.
    assert(snprintf_ret > 0 && snprintf_ret + 1 < sizeof(buf)); // Нет ошибки и влезли в буффер.
    
    FILE* f = fopen("out.txt", "w");
    fprintf(f, "file: %s\n", buf); // Печать в файл.
    fclose(f);
    return 0;
}

