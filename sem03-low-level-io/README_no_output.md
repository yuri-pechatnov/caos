

# Низкоуровневый ввод-вывод

<p><a href="https://www.youtube.com/watch?v=DK-IzHeJFPA&list=PLjzMm8llUm4DuIDzX8pmWKYbBy6Enly4i&index=1" target="_blank">
    <h3>Видеозапись семинара</h3>
</a></p>


[Ридинг Яковлева про низкоуровневый ввод-вывод](https://github.com/victor-yacovlev/mipt-diht-caos/tree/master/practice/file_io) 


Сегодня в программе:
* <a href="#linux" style="color:#856024"> Linux </a>  
  * <a href="#read" style="color:#856024"> Чтение из stdin и файлов </a>  
  * <a href="#write" style="color:#856024"> Запись в stdout/stderr и файл </a>
  * <a href="#lseek" style="color:#856024"> Произвольный доступ к файлам (lseek) </a>
  * <a href="#pread_pwrite" style="color:#856024"> pread/pwrite - чтение/запись с произвольной позиции в файле </a>
  * <a href="#offset64" style="color:#856024"> Большие оффсеты </a>
  * <a href="#readv_writev" style="color:#856024"> readv/writev - чтение/запись сразу в несолько буфферов </a>
  * <a href="#lsof" style="color:#856024"> Список открытых файлов (lsof) </a>
  * <a href="#access" style="color:#856024"> access - проверка доступа </a>


Интересные вопросы: fread - исходники (FILE - ?), модел доступов в linux


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

# Самостоятельная работа 5 минут

Найти в man-е по read/write особенности про многопоточность.


```python

```

## <a name="lseek"></a> lseek - чтение с произвольной позиции в файле

Смотрит на второй символ в файле, читает его, интерпретирует как цифру и увеличивает эту цифру на 1.


```python
!echo _4___ > lseek_example.txt
```


```cpp
%%cpp lseek_example.c
%run gcc lseek_example.c -o lseek_example.exe
%run ./lseek_example.exe lseek_example.txt
%run cat lseek_example.txt

#define _LARGEFILE64_SOURCE // Enable xxx64 functions.

#include <assert.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

int main(int argc, char *argv[])
{   
    assert(argc >= 2);
    // O_RDWR - открытие файла на чтение и запись одновременно
    int fd = open(argv[1], O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH); 
    
    // Перемещаемся на конец файла, получаем позицию конца файла - это размер файла
    uint64_t size = lseek64(fd, 0, SEEK_END);
    
    printf("File size: %" PRIu64 "\n", size);
    
    // если размер меньше 2, то дописываем цифры
    if (size < 2) {
        const char s[] = "10";
        lseek64(fd, 0, SEEK_SET);
        write(fd, s, sizeof(s) - 1);
        printf("Written bytes: %d\n", (int)sizeof(s) - 1);    
        size = lseek64(fd, 0, SEEK_END);
        printf("File size: %" PRIu64 "\n", size);
    }
    
    // читаем символ со 2й позиции
    lseek64(fd, 1, SEEK_SET);
    char c;
    read(fd, &c, 1);
    c = (c < '0' || c > '9') ? '0' : ((c - '0') + 1) % 10 + '0';
    
    // записываем символ в 2ю позицию
    lseek64(fd, 1, SEEK_SET);
    write(fd, &c, 1);
    
    close(fd);
    return 0;
}
```

## <a name="pread_pwrite"></a> pread/pwrite - чтение/запись с произвольной позиции в файле

Как и предыдущий пример, но без части вызовов lseek.


```python
!echo hello > pread_example.txt
```


```cpp
%%cpp pread_example.c
%run gcc pread_example.c -o pread_example.exe
%run ./pread_example.exe pread_example.txt
%run cat pread_example.txt

#define _LARGEFILE64_SOURCE // Enable xxx64 functions.

#include <assert.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

int main(int argc, char *argv[])
{   
    assert(argc >= 2);
    // O_RDWR - открытие файла на чтение и запись одновременно
    int fd = open(argv[1], O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH); 
    
    // Перемещаемся на конец файла, получаем позицию конца файла - это размер файла
    uint64_t size = lseek64(fd, 0, SEEK_END);
    
    printf("File size: %" PRIu64 "\n", size);
    
    // если размер меньше 2, то дописываем цифры
    if (size < 2) {
        const char s[] = "10";
        pwrite64(fd, s, sizeof(s) - 1, 0); // DIFF
        printf("Written bytes: %d\n", (int)sizeof(s) - 1);    
        size = lseek64(fd, 0, SEEK_END);
        printf("File size: %" PRIu64 "\n", size);
    }
    
    // читаем символ со 2й позиции
    char c;
    pread64(fd, &c, 1, /* offset = */ 1); // DIFF
    c = (c < '0' || c > '9') ? '0' : ((c - '0') + 1) % 10 + '0';
    
    // записываем символ в 2ю позицию
    pwrite64(fd, &c, 1, /* offset = */ 1); // DIFF
    
    close(fd);
    return 0;
}
```

## <a name="offset64"></a> Большие оффсеты

Для файлов больше 2GB обычные фунции могут не подходить, так как в них оффсет передается как int. Вместо них можно использовать lseek64/pread64/etc. Собственно такие и использованы в примерах выше.


```python

```

## <a name="readv_writev"></a> readv/writev - чтение/запись сразу в несолько буфферов.

Если у вас не один большой буффер, а много маленьких (например что-то вроде дека), то вы можете использовать readv/writev.


```cpp
%%cpp writev_example.c
%run gcc writev_example.c -o writev_example.exe
%run ./writev_example.exe writev_example.txt

#include <sys/uio.h>
#include <unistd.h>

int main(int argc, char *argv[])
{   
    char str1[] = "Hello ", str2[] = "world!\n";
    struct iovec writings[] = {
        {str1, sizeof(str1) - 1},
        {str2, sizeof(str2) - 1},
    };
    writev(STDOUT_FILENO, writings, sizeof(writings) / sizeof(struct iovec));
    return 0;
}
```


```python

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

## <a name="access"></a> access - проверка доступа


```python
!ls -la
```

`chmod +x somefile`

`chmod g+w,o-r somefile`


```python

```


```cpp
%%cpp access.c
%run gcc access.c -o access.exe
%run ./access.exe

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define PRINT_WITH_ERRNO(call) do {                                 \
    int __a_res = (call);                                           \
    char* err = ((__a_res == 0) ? "Success" : strerror(errno));     \
    printf("%s = %d, err: %s\n", #call, __a_res, err);              \
} while (0)

int main() {
    PRINT_WITH_ERRNO(access("./access.exe",    X_OK));
    PRINT_WITH_ERRNO(access("./access.c",      X_OK));
    PRINT_WITH_ERRNO(access("./access.exe",    F_OK));
    PRINT_WITH_ERRNO(access("./access788.exe", F_OK));
    PRINT_WITH_ERRNO(access("./access.exe",    W_OK));
    PRINT_WITH_ERRNO(access("/bin/bash",       W_OK));
    return 0;
}
```


```python

```


```python
%%file executable_file
#!/bin/bash
echo hello
```


```python
!chmod u+x executable_file
!strace ./executable_file
```


```python

```


```python

```
