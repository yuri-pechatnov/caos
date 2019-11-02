```python
get_ipython().run_cell_magic('javascript', '', '// setup cpp code highlighting\nIPython.CodeCell.options_default.highlight_modes["text/x-c++src"] = {\'reg\':[/^%%cpp/]} ;')

# creating magics
from IPython.core.magic import register_cell_magic, register_line_magic
from IPython.display import display, Markdown

@register_cell_magic
def save_file(fname, cell):
    cell = cell if cell[-1] == '\n' else cell + "\n"
    cmds = []
    with open(fname, "w") as f:
        for line in cell.split("\n"):
            if line.startswith("%"):
                run_prefix = "%run "
                assert line.startswith(run_prefix)
                cmds.append(line[len(run_prefix):].strip())
            else:
                f.write(line + "\n")
    for cmd in cmds:
        display(Markdown("Run: `%s`" % cmd))
        get_ipython().system(cmd)

@register_cell_magic
def cpp(fname, cell):
    save_file(fname, cell)

@register_cell_magic
def asm(fname, cell):
    save_file(fname, cell)
    
@register_cell_magic
def makefile(fname, cell):
    assert not fname
    save_file("makefile", cell.replace(" " * 4, "\t"))
        
@register_line_magic
def p(line):
    try:
        expr, comment = line.split(" #")
        display(Markdown("`{} = {}`  # {}".format(expr.strip(), eval(expr), comment.strip())))
    except:
        display(Markdown("{} = {}".format(line, eval(line))))
    
```


    <IPython.core.display.Javascript object>



```python
%p 1 + 1 # 1
```


`1 + 1 = 2`  # 1


# Низкоуровневый ввод-вывод

## Linux

Здесь полезно рассматривать процесс как объект в операционной системе. Помимо основного пользовательского потока выполнения у процесса-объекта есть множество атрибутов.

Советую прочитать [статью на хабре](https://habr.com/ru/post/423049/#definition), вроде там все очень неплохо написано.

Сегодня нас будут интересовать файловые дескрипторы. Каждому открытому файлу и соединению соответствует число (int). Это число используется как идентификатор в функциях, работающих с файлами/соединениями.


* 0 - stdin - стандартный поток ввода
* 1 - stdout - стандартный поток вывода
* 2 - stderr - стандартный поток ошибок

Примеры использования в bash:

* `grep String < file.txt` <-> `grep String 0< file.txt`
* `mkdir a_dir 2> /dev/null`
* `./some_program < in.txt 1> out.txt` <-> `./some_program < in.txt > out.txt` 




```python
%%cpp linux_example.c
%run gcc linux_example.c -o linux_example.exe
%run echo "Hello students!" > linux_example_input_001.txt
%run ./linux_example.exe linux_example_input_001.txt

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
    // printf("Linux by printf"); // where it will be printed?
    char linux_str[] = "Linux by write\n";
    write(1, linux_str, sizeof(linux_str)); // 1 - изначально открытый файловый дескриптор соответствующий stdout
                                            // linux_str - указатель на начало данных, 
                                            // sizeof(linux_str) - размер данных, которые хотим записать
                                            // ВАЖНО, что write может записать не все данные 
                                            //        и тогда его надо перезапустить
                                            //        но в данном примере этого нет
                                            // Подробнее в `man 2 write`
    if (argc < 2) {
        printf("Need at least 2 arguments\n");
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
    int written_bytes = snprintf(buffer2, sizeof(buffer2), "Bytes read: %d\n'''%s'''\n", bytes_read, buffer);
    write(1, buffer2, written_bytes);
    close(fd);
    return 0;
}
```


Run: `gcc linux_example.c -o linux_example.exe`



Run: `echo "Hello students!" > linux_example_input_001.txt`



Run: `./linux_example.exe linux_example_input_001.txt`


    Linux by write
    Bytes read: 16
    '''Hello students!
    '''


### Экзотический пример-игрушка


```python
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
    int bytes_read = read(5, buffer, sizeof(buffer)); 
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


Run: `gcc strange_example.c -o strange_example.exe`



Run: `echo "Hello world!" > a.txt`



Run: `./strange_example.exe 5< a.txt > strange_example.out`



Run: `cat strange_example.out`


    Hello world!


### Retry of read


```python
%%cpp retry_example.c
%run gcc retry_example.c -o retry_example.exe
%run echo "Hello world!" > a.txt
%run ./retry_example.exe < a.txt 

#include <unistd.h>
#include <stdio.h>
#include <errno.h>


int read_retry(int fd, char* data, int size) {
    char* cdata = data;
    while (1) {
        int read_bytes = read(fd, cdata, size);
        if (read_bytes == 0) {
            return cdata - data;
        }
        if (read_bytes < 0) {
            if (errno == EAGAIN || errno == EINTR) {
                continue;
            } else {
                return -1;
            }
        }
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
    int bytes_read = read_retry(0, buffer, sizeof(buffer)); 
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


Run: `gcc retry_example.c -o retry_example.exe`



Run: `echo "Hello world!" > a.txt`



Run: `./retry_example.exe < a.txt`


    Hello world!



```python

```


```python

```

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


`os.stat("a.txt") = os.stat_result(st_mode=33204, st_ino=1344254, st_dev=2049, st_nlink=1, st_uid=1000, st_gid=1000, st_size=15, st_atime=1572422090, st_mtime=1572422171, st_ctime=1572422171)`  # Атрибуты файла `a.txt`



`oct(os.stat("a.txt").st_mode) = 0o100664`  # Интересны последние три восьмеричные цифры. 664 - это обычные атрибуты прав



`oct(os.stat("./linux_example.exe").st_mode) = 0o100775`  # Аттрибуты прав исполняемого файла



`oct(os.stat("b_dir").st_mode) = 0o40775`  # Забавный факт, но все могут "исполнять директорию". [Более подробно на stackoverflow](https://unix.stackexchange.com/questions/21251/execute-vs-read-bit-how-do-directory-permissions-in-linux-work)



```python
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


Run: `gcc linux_file_hello_world.c -o linux_file_hello_world.exe`



Run: `./linux_file_hello_world.exe`


    Bytes written: 13 (expected 13)



Run: `cat linux_file_hello_world.out`


    Hello world!


```python
!rm -f linux_file_hello_world.out
```


```python
!ls -la linux_file_hello_world.out
```

    -rw-rw-r-- 1 pechatnov pechatnov 13 Oct 30 11:14 linux_file_hello_world.out



```python
oct(os.stat("linux_file_hello_world.out").st_mode)
```




    '0o100775'




```python

```

## lseek - чтение с произвольной позиции в файле

Смотрит на второй символ в файле, читает его, интерпретирует как цифру и увеличивает эту цифру на 1.


```python
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


Run: `gcc lseek_example.c -o lseek_example.exe`



Run: `./lseek_example.exe b.txt`


    File size: 5



Run: `cat b.txt`


    H5llo


```python
!echo -n "Hello" > b.txt
```


```python
!cat b.txt
```

    10

# Windows

* Вместо файловых дескрипторов - HANDLE (вроде это просто void*)
* Много алиасов для типов вроде HANDLE, DWORD, BOOL, LPTSTR, LPWSTR
* Очень много аргументов у всех функций
* Плохая документация, гуглится все плохо
* Надо установить `wine` и `mingw-w64`


```python
%%cpp winapi_example.c
%run i686-w64-mingw32-gcc winapi_example.c -o winapi_example.exe
%run echo "Hello students!" > winapi_example_input_001.txt
%run wine winapi_example.exe winapi_example_input_001.txt

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
    memset(buffer, 0, sizeof(buffer));
    DWORD bytes_read;
    BOOL success;
    success = ReadFile(fileHandle, buffer, sizeof(buffer),
                       &bytes_read, NULL);
    if (!success) {
        perror("Error reading file"); // Это ошибка, perror смотрит в errno, а не в GetLastError()
        CloseHandle(fileHandle);
        return -1;
    }
    printf("Bytes read: %d\n'''%s'''\n", bytes_read, buffer);
    CloseHandle(fileHandle);
    return 0;
}
```


Run: `i686-w64-mingw32-gcc winapi_example.c -o winapi_example.exe`



Run: `echo "Hello students!" > winapi_example_input_001.txt`



Run: `wine winapi_example.exe winapi_example_input_001.txt`


    [?1h=Defined WIN32
    Bytes read: 16
    '''Hello students!
    '''



```python

```

## Микротест:
1. вариант
  1. Определение файлового дескриптора. Стандартные дескрипторы открытые при старте программы.
  1. Каких гарантий не дают функции read и write? Кто виноват и что с этим приходится делать?
  1. Аргументы и возвращаемое значение функции lseek
1. вариант
  1. Аргументы и возвращаемое значение функции read. Обработка ошибок функции
  1. У вас есть файловый дескриптор открытого файла. Как узнать размер этого файла?
  1. Аргументы и возвращаемое значение вызова open. Особенность передачи аргументов в функцию



```python

```
