// %%cpp linux_example.c
// %run gcc linux_example.c -o linux_example.exe
// %run echo -n "Hello students!" > linux_example_input_001.txt
// %run ./linux_example.exe linux_example_input_001.txt

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    printf("Linux by printf"); // where it will be printed?
    
    char linux_str[] = "Linux by write\n";
    
    // write
    // 1 = STDOUT_FILENO - изначально открытый файловый дескриптор соответствующий stdout
    // linux_str - указатель на начало данных, 
    // sizeof(linux_str) - размер данных, которые хотим записать
    // ВАЖНО, что write может записать не все данные 
    //        и тогда его надо перезапустить
    //        но в данном примере этого нет
    // Подробнее в `man 2 write`
    write(1, linux_str, sizeof(linux_str) - 1); 
    // exit(0); // 1. Что выведется если раскомментировать?
    // _exit(0); // 2. Что выведется если раскомментировать это?
        
    if (argc < 2) {
        fprintf(stderr, "Need at least 2 arguments\n");
        return 1;
    }
    int fd = open(argv[1], O_RDONLY); // открываем файл и получаем связанный файловый дескриптор
                                      // O_RDONLY - флаг о том, что открываем в read-only режиме
                                      // подробнее в `man 2 open`
    if (fd < 0) {
        perror("Can't open file"); // Выводит указанную строку в stderr 
                                   // + добавляет сообщение и последней произошедшей ошибке 
                                   // ошибка хранится в errno
        return -1;
    }
    
    char buffer[4096];
    int bytes_read = read(fd, buffer, sizeof(buffer)); // fd - файловый дескриптор выше открытого файла
                                                       // 2 и 3 аргументы как во write
                                                       // Так же как и write может прочитать МЕНЬШЕ
                                                       //   чем запрошено в 3м аргументе
                                                       //   это может быть связано как с концом файла
                                                       //   так и с каким-то более приоритетным событием
    if (bytes_read < 0) {
        perror("Error reading file");
        close(fd); // закрываем файл связанный с файловым дескриптором. Ну или не файл. 
                   // Стандартные дескрипторы 0, 1, 2 тоже можно так закрывать
        return -1;
    }
    char buffer2[4096];
    // формирование строки с текстом
    int written_bytes = snprintf(buffer2, sizeof(buffer2), 
        "%d bytes read: '''%.*s'''\n", bytes_read, bytes_read, buffer);
    write(1, buffer2, written_bytes);
    close(fd);
    return 0;
}

