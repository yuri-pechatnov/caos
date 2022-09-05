

# Низкоуровневый ввод-вывод

<p><a href="https://www.youtube.com/watch?v=c3rUDPA9Ocs&list=PLjzMm8llUm4AmU6i_hPU0NobgA4VsBowc&index=11" target="_blank">
    <h3>Видеозапись семинара</h3>
</a></p>


[Ридинг Яковлева](https://github.com/victor-yacovlev/mipt-diht-caos/tree/master/practice/file_io) 


Сегодня в программе:
* <a href="#linux" style="color:#856024"> Linux </a>  
  * <a href="#read" style="color:#856024"> Чтение из stdin и файлов </a>  
  * <a href="#write" style="color:#856024"> Запись в stdout/stderr и файл </a>
  * <a href="#attrs" style="color:#856024"> Атрибуты файлов </a>  
  * <a href="#lseek" style="color:#856024"> Произвольный доступ к файлам (lseek) </a>
  * <a href="#lsof" style="color:#856024"> Список открытых файлов (lsof) </a>
* <a href="#win" style="color:#856024"> Windows </a>
* <a href="#hw" style="color:#856024"> Комментарии к ДЗ </a>



## <a name="linux"></a> Linux

Здесь полезно рассматривать процесс как объект в операционной системе. Помимо основного пользовательского потока выполнения у процесса-объекта есть множество атрибутов.

Советую прочитать [статью на хабре](https://habr.com/ru/post/423049/#definition), вроде там все очень неплохо написано.

Сегодня нас будут интересовать файловые дескрипторы. Каждому открытому файлу и соединению соответствует число (int). Это число используется как идентификатор в функциях, работающих с файлами/соединениями.


* 0 - stdin - стандартный поток ввода (STDIN_FILENO - стандартный макрос в C)
* 1 - stdout - стандартный поток вывода (STDOUT_FILENO)
* 2 - stderr - стандартный поток ошибок (STDERR_FILENO)

Примеры использования в bash:

* `grep String < file.txt` <-> `grep String 0< file.txt`
* `mkdir a_dir 2> /dev/null`
* `./some_program < in.txt 1> out.txt` <-> `./some_program < in.txt > out.txt` 




```python

```

## <a name="read"></a> Read 


```cpp
%%cpp linux_example_read.c
%run gcc linux_example_read.c -o linux_example_read.exe
%run echo -n "Hello from file!" > linux_example_read.txt
%run echo -n "Hello from stdin!" | ./linux_example_read.exe linux_example_read.txt

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>

int main(int argc, char *argv[])
{
    assert(argc >= 2);
    const char* file_name = argv[1];
    
    char buffer[4096];
    int bytes_read = read(STDIN_FILENO, buffer, sizeof(buffer));
    printf("From stdin: '%.*s'\n", bytes_read, buffer); // buffer not zero-terminated string after `read`!
    
    int fd = open(file_name, O_RDONLY); // O_RDWR also works 
    bytes_read = read(fd, buffer, sizeof(buffer));
    printf("From file '%s': '%.*s'\n", file_name, bytes_read, buffer); 
    close(fd);
    
    return 0;
}
```

**Но в данном процессе чтения есть проблема, не все данные могут быть доступны для чтения сразу.**


```python
!(echo -n "A" ; sleep 1 ; echo -n "B") ; echo "" 

!(echo -n "A" ; sleep 1 ; echo -n "B" 2>/dev/null) | ./linux_example_read.exe linux_example_read.txt
```

Поэтому надо делать ретраи.


```cpp
%%cpp retry_example.c
%run gcc -D_USE_READ retry_example.c -o retry_example.exe
%run echo -n "Hello_world_1!" | ./retry_example.exe 
%run gcc -D_USE_READ retry_example.c -o retry_example.exe
%run (echo -n "Hello_" ; sleep 0.2 ; echo -n "world_2!") | ./retry_example.exe  
%run gcc -D_USE_READ_RETRY retry_example.c -o retry_example.exe
%run (echo -n "Hello_" ; sleep 0.2 ; echo -n "world_3!") | ./retry_example.exe  
%run gcc -D_USE_SCANF retry_example.c -o retry_example.exe
%run (echo -n "Hello_" ; sleep 0.2 ; echo -n "world_4!") | ./retry_example.exe  

#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>


int read_retry(int fd, char* data, int size) {
    char* cdata = data;
    while (1) {
        int read_bytes = read(fd, cdata, size);
        if (read_bytes == 0) { // если read вернул 0, значит файловый дескриптор закрыт с другого конца 
                               // или конец файла
            return cdata - data;
        }
        if (read_bytes < 0) { // если возвращено значение < 0, то это ошибка
            if (errno == EAGAIN || errno == EINTR) { // она может быть retryable
                continue;
            } else { // а может быть критичной, и нет смысла пытаться повторить попытку чтения
                return -1;
            }
        }
        // если возвращенное значение > 0, значит успешно прочитано столько байт
        cdata += read_bytes;
        size -= read_bytes;
        if (size == 0) {
            return cdata - data;
        }
    }
}


int main(int argc, char *argv[])
{ 
    char buffer[4096];
#if defined(_USE_READ) || defined(_USE_READ_RETRY)
  #if defined(_USE_READ)
    int bytes_read = read(0, buffer, sizeof(buffer)); 
  #endif
  #if defined(_USE_READ_RETRY)
    int bytes_read = read_retry(0, buffer, sizeof(buffer)); 
  #endif    
    if (bytes_read < 0) {
        perror("Error reading file");
        return -1;
    }
#endif
#if defined(_USE_SCANF)
    scanf("%s", buffer);
    int bytes_read = strlen(buffer);
#endif  
    printf("Read '%.*s'", bytes_read, buffer);
    
    return 0;
}
```

Как можно заметить, scanf молодец. А read надо ретраить.


```python

```

## <a name="write"></a> Write

С write есть та же проблема, что и с read. Ретраится примерно точно так же.


```cpp
%%cpp linux_example.c
%run gcc linux_example.c -o linux_example.exe
%run ./linux_example.exe linux_example_input_002.txt 
%run cat linux_example_input_002.txt 

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

int main(int argc, char *argv[])
{
    const char to_stdout[] = "Hello to stdout!\n";
    const char to_stderr[] = "Hello to stderr!\n";
    const char to_file[] = "Hello to file!\n";
    
    assert(argc >= 2);
    const char* file_name = argv[1];
    
    int bytes_written = write(STDOUT_FILENO, to_stdout, strlen(to_stdout));
    assert(bytes_written == strlen(to_stdout));
    
    bytes_written = write(STDERR_FILENO, to_stderr, strlen(to_stderr));
    assert(bytes_written == strlen(to_stderr));
    
    int fd = open(file_name, O_WRONLY | O_CREAT, 0644); // 0644 это важно! Но об этом позже 
    // open(file_name, O_WRONLY | O_CREAT) ~ printf("%s")
    bytes_written = write(fd, to_file, strlen(to_file));
    close(fd);
    
    return 0;
}
```


```python

```

### Общий пример


```cpp
%%cpp linux_example.c
%run gcc linux_example.c -o linux_example.exe
%run echo -n "Hello students!" > linux_example_input_001.txt
%run ./linux_example.exe linux_example_input_001.txt

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
```


```python

```

### Экзотический пример-игрушка


```cpp
%%cpp strange_example.c
%run gcc strange_example.c -o strange_example.exe
%run echo "Hello world!" > a.txt
%run ./strange_example.exe 5< a.txt > strange_example.out
%run cat strange_example.out

#include <unistd.h>
#include <stdio.h>

int main(int argc, char *argv[])
{ 
    char buffer[4096];
    int bytes_read = read(5, buffer, sizeof(buffer)); // not standart opened fd
    if (bytes_read < 0) {
        perror("Error reading file");
        return -1;
    }
    int written_bytes = write(1, buffer, bytes_read);
    if (written_bytes < 0) {
        perror("Error writing file");
        return -1;
    }
    return 0;
}
```


```python

```

## <a name="attrs"></a> Атрибуты файла

При открытии файла с флагом создания (O_WRONLY | O_CREAT) важно адекватно проставлять маску прав доступа. Давайте с ней разберемся.

Заметка о правописании: **Attribute, но атрибут**


```python
!echo "Hello jupyter!" > a.txt  # создаем файлик с обычными "настройками"
!mkdir b_dir 2> /dev/null

import os  # В модуле os есть почти в чистом виде почти все системные вызовы: write, read, open...
from IPython.display import display

%p os.stat("a.txt") # Атрибуты файла `a.txt`
%p oct(os.stat("a.txt").st_mode)  # Интересны последние три восьмеричные цифры. 664 - это обычные атрибуты прав

%p oct(os.stat("./linux_example.exe").st_mode)  # Аттрибуты прав исполняемого файла

%p oct(os.stat("b_dir").st_mode)  # Забавный факт, но все могут "исполнять директорию". [Более подробно на stackoverflow](https://unix.stackexchange.com/questions/21251/execute-vs-read-bit-how-do-directory-permissions-in-linux-work)

```


```python
!stat ./linux_example.exe

```


```cpp
%%cpp linux_file_hello_world.c
%run gcc linux_file_hello_world.c -o linux_file_hello_world.exe
%run ./linux_file_hello_world.exe
%run cat linux_file_hello_world.out

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

int main(int argc, char *argv[])
{   
    int fd = open("linux_file_hello_world.out", O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH); 
    // S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH == 0664
    // попробуйте не указывать 0664   
    // (ошибка такая же как в printf("%d");)
    // для справки `man 2 open`
     
    if (fd < 0) {
        perror("Can't open file");
        return -1;
    }
    char buffer[] = "Hello world!";
    int bytes_written = write(fd, buffer, sizeof(buffer));
    if (bytes_written < 0) {
        perror("Error writing file");
        close(fd);
        return -1;
    }
    printf("Bytes written: %d (expected %d)\n", bytes_written, (int)sizeof(buffer));
    close(fd);
    return 0;
}
```


```python
oct(os.stat("linux_file_hello_world.out").st_mode)
```


```python

```

## <a name="lseek"></a> lseek - чтение с произвольной позиции в файле

Смотрит на второй символ в файле, читает его, интерпретирует как цифру и увеличивает эту цифру на 1.


```cpp
%%cpp lseek_example.c
%run gcc lseek_example.c -o lseek_example.exe
%run ./lseek_example.exe b.txt
%run cat b.txt

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>

int main(int argc, char *argv[])
{   
    assert(argc >= 2);
    // O_RDWR - открытие файла на чтение и запись одновременно
    int fd = open(argv[1], O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH); 
    
    // Перемещаемся на конец файла, получаем позицию конца файла - это размер файла
    int size = lseek(fd, 0, SEEK_END);
    
    printf("File size: %d\n", size);
    
    // если размер меньше 2, то дописываем цифры
    if (size < 2) {
        const char s[] = "10";
        lseek(fd, 0, SEEK_SET);
        write(fd, s, sizeof(s) - 1);
        printf("Written bytes: %d\n", (int)sizeof(s) - 1);    
        size = lseek(fd, 0, SEEK_END);
        printf("File size: %d\n", size);
    }
    
    // читаем символ со 2й позиции
    lseek(fd, 1, SEEK_SET);
    char c;
    read(fd, &c, 1);
    c = (c < '0' || c > '9') ? '0' : ((c - '0') + 1) % 10 + '0';
    
    // записываем символ в 2ю позицию
    lseek(fd, 1, SEEK_SET);
    write(fd, &c, 1);
    
    close(fd);
    return 0;
}
```


```python
!echo "hello" > b.txt
```


```python

```

## <a name="lsof"></a> Список открытых файлов


```cpp
%%cpp simple_open.c
%run gcc simple_open.c -o simple_open.exe
%run ./simple_open.exe
%run ./simple_open.exe < a.txt
%run ./simple_open.exe < a.txt 2> b.txt

#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


void apply_lsof() {
    char cmd[100];
    snprintf(cmd, sizeof(cmd), "lsof -p %d", getpid());
    system(cmd);
}


int main()
{  
    apply_lsof();
    return 0;
}
```


```python

```

# <a name="win"></a> Windows

* Вместо файловых дескрипторов - HANDLE (вроде это просто void*)
* Много алиасов для типов вроде HANDLE, DWORD, BOOL, LPTSTR, LPWSTR
* Очень много аргументов у всех функций
* Плохая документация, гуглится все плохо
* Надо установить `wine` и `mingw-w64`
  * `sudo apt install gcc-mingw-w64`
  * `sudo apt install wine64`


```cpp
%%cpp winapi_example.c
%run i686-w64-mingw32-gcc winapi_example.c -o winapi_example.exe
%run echo "Hello students!" > winapi_example_input_001.txt
%run WINEDEBUG=-all wine winapi_example.exe winapi_example_input_001.txt

#include <windows.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
#ifdef WIN32
    printf("Defined WIN32\n");
#else
    printf("Not WIN32\n");
#endif
    if (argc < 2) {
        printf("Need at least 2 arguments\n");
        return 1;
    }
    HANDLE fileHandle = CreateFileA(
        argv[1], GENERIC_READ, FILE_SHARE_READ, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (fileHandle == INVALID_HANDLE_VALUE) {
        char errorBuffer[1024];
        if (!FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                           NULL, GetLastError(),
                           MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                           errorBuffer, sizeof(errorBuffer), NULL))
        {
            printf("Format message failed with 0x%x\n", GetLastError());
            return -1;
        }
        printf("Can't open file: %s\n", errorBuffer);
        return -1;
    }
    
    char buffer[4096];
    DWORD bytes_read;
    BOOL success;
    success = ReadFile(fileHandle, buffer, sizeof(buffer),
                       &bytes_read, NULL);
    if (!success) {
        perror("Error reading file"); // Это ошибка, perror смотрит в errno, а не в GetLastError()
        CloseHandle(fileHandle);
        return -1;
    }
    printf("Bytes read: %d\n'''%.*s'''\n", bytes_read, bytes_read, buffer);
    CloseHandle(fileHandle);
    return 0;
}
```


```python

```

## <a name="hw"></a> Комментарии к ДЗ
* 0777 - реджект, исполняемый файл создавать не надо
* open -> close. Нужно закрывать все файлы
* Рассматривать ошибки и случаи неполного чтения в случае работы НЕ с обычными файлами с файловой системы. (Например, если это может быть ввод с терминала или пайпа).
* files-io/read-filter-write - запрещено читать по одному символу. И читать все сразу в память тоже не надо. Делайте буферизацию.
* files-io/sort-file-contents - запрещено читать и выводить по одному инту - ужасно неэффективно. Так же решения не за $O(N ~ log ~ N)$ получат максимум 100 баллов.


```python

```


```python

```
