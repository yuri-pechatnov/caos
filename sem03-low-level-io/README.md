


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


Run: `gcc linux_example_read.c -o linux_example_read.exe`



Run: `echo -n "Hello from file!" > linux_example_read.txt`



Run: `echo -n "Hello from stdin!" | ./linux_example_read.exe linux_example_read.txt`


    From stdin: 'Hello from stdin!'
    From file 'linux_example_read.txt': 'Hello from file!'


**Но в данном процессе чтения есть проблема, не все данные могут быть доступны для чтения сразу.**


```python
!(echo -n "A" ; sleep 1 ; echo -n "B" 2>/dev/null) | ./linux_example_read.exe linux_example_read.txt
```

    From stdin: 'A'
    From file 'linux_example_read.txt': 'Hello from file!'


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


Run: `gcc -D_USE_READ retry_example.c -o retry_example.exe`



Run: `echo -n "Hello_world_1!" | ./retry_example.exe`


    Read 'Hello_world_1!'


Run: `gcc -D_USE_READ retry_example.c -o retry_example.exe`



Run: `(echo -n "Hello_" ; sleep 0.2 ; echo -n "world_2!") | ./retry_example.exe`


    Read 'Hello_'/usr/bin/sh: 1: echo: echo: I/O error



Run: `gcc -D_USE_READ_RETRY retry_example.c -o retry_example.exe`



Run: `(echo -n "Hello_" ; sleep 0.2 ; echo -n "world_3!") | ./retry_example.exe`


    Read 'Hello_world_3!'


Run: `gcc -D_USE_SCANF retry_example.c -o retry_example.exe`



Run: `(echo -n "Hello_" ; sleep 0.2 ; echo -n "world_4!") | ./retry_example.exe`


    Read 'Hello_world_4!'

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


Run: `gcc linux_example.c -o linux_example.exe`



Run: `./linux_example.exe linux_example_input_002.txt`


    Hello to stdout!
    Hello to stderr!



Run: `cat linux_example_input_002.txt`


    Hello to file!



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


Run: `gcc linux_example.c -o linux_example.exe`



Run: `echo -n "Hello students!" > linux_example_input_001.txt`



Run: `./linux_example.exe linux_example_input_001.txt`


    Linux by write



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


Run: `gcc strange_example.c -o strange_example.exe`



Run: `echo "Hello world!" > a.txt`



Run: `./strange_example.exe 5< a.txt > strange_example.out`



Run: `cat strange_example.out`


    Hello world!


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


Run: `gcc lseek_example.c -o lseek_example.exe`



Run: `./lseek_example.exe lseek_example.txt`


    File size: 6



Run: `cat lseek_example.txt`


    _75__


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


Run: `gcc pread_example.c -o pread_example.exe`



Run: `./pread_example.exe pread_example.txt`


    File size: 6



Run: `cat pread_example.txt`


    h2llo


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


Run: `gcc writev_example.c -o writev_example.exe`



Run: `./writev_example.exe writev_example.txt`


    Hello world!



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


Run: `gcc simple_open.c -o simple_open.exe`



Run: `./simple_open.exe`


    lsof: WARNING: can't stat() tmpfs file system /run/snapd/ns
          Output information may be incomplete.
    lsof: WARNING: can't stat() nsfs file system /run/snapd/ns/snap-store.mnt
          Output information may be incomplete.
    COMMAND      PID      USER   FD   TYPE DEVICE SIZE/OFF    NODE NAME
    simple_op 231244 pechatnov  cwd    DIR    8,5     4096 4724417 /home/pechatnov/vbox/caos/sem03-low-level-io
    simple_op 231244 pechatnov  rtd    DIR    8,5     4096       2 /
    simple_op 231244 pechatnov  txt    REG    8,5    16880 4724365 /home/pechatnov/vbox/caos/sem03-low-level-io/simple_open.exe
    simple_op 231244 pechatnov  mem    REG    8,5  2029592 1837182 /usr/lib/x86_64-linux-gnu/libc-2.31.so
    simple_op 231244 pechatnov  mem    REG    8,5   191504 1836157 /usr/lib/x86_64-linux-gnu/ld-2.31.so
    simple_op 231244 pechatnov    0u   CHR 136,13      0t0      16 /dev/pts/13
    simple_op 231244 pechatnov    1u   CHR 136,13      0t0      16 /dev/pts/13
    simple_op 231244 pechatnov    2u   CHR 136,13      0t0      16 /dev/pts/13



Run: `./simple_open.exe < a.txt`


    lsof: WARNING: can't stat() tmpfs file system /run/snapd/ns
          Output information may be incomplete.
    lsof: WARNING: can't stat() nsfs file system /run/snapd/ns/snap-store.mnt
          Output information may be incomplete.
    COMMAND      PID      USER   FD   TYPE DEVICE SIZE/OFF    NODE NAME
    simple_op 231249 pechatnov  cwd    DIR    8,5     4096 4724417 /home/pechatnov/vbox/caos/sem03-low-level-io
    simple_op 231249 pechatnov  rtd    DIR    8,5     4096       2 /
    simple_op 231249 pechatnov  txt    REG    8,5    16880 4724365 /home/pechatnov/vbox/caos/sem03-low-level-io/simple_open.exe
    simple_op 231249 pechatnov  mem    REG    8,5  2029592 1837182 /usr/lib/x86_64-linux-gnu/libc-2.31.so
    simple_op 231249 pechatnov  mem    REG    8,5   191504 1836157 /usr/lib/x86_64-linux-gnu/ld-2.31.so
    simple_op 231249 pechatnov    0r   REG    8,5       13 4724472 /home/pechatnov/vbox/caos/sem03-low-level-io/a.txt
    simple_op 231249 pechatnov    1u   CHR 136,13      0t0      16 /dev/pts/13
    simple_op 231249 pechatnov    2u   CHR 136,13      0t0      16 /dev/pts/13



Run: `./simple_open.exe < a.txt 2> b.txt`


    COMMAND      PID      USER   FD   TYPE DEVICE SIZE/OFF    NODE NAME
    simple_op 231254 pechatnov  cwd    DIR    8,5     4096 4724417 /home/pechatnov/vbox/caos/sem03-low-level-io
    simple_op 231254 pechatnov  rtd    DIR    8,5     4096       2 /
    simple_op 231254 pechatnov  txt    REG    8,5    16880 4724365 /home/pechatnov/vbox/caos/sem03-low-level-io/simple_open.exe
    simple_op 231254 pechatnov  mem    REG    8,5  2029592 1837182 /usr/lib/x86_64-linux-gnu/libc-2.31.so
    simple_op 231254 pechatnov  mem    REG    8,5   191504 1836157 /usr/lib/x86_64-linux-gnu/ld-2.31.so
    simple_op 231254 pechatnov    0r   REG    8,5       13 4724472 /home/pechatnov/vbox/caos/sem03-low-level-io/a.txt
    simple_op 231254 pechatnov    1u   CHR 136,13      0t0      16 /dev/pts/13
    simple_op 231254 pechatnov    2w   REG    8,5      222 4724464 /home/pechatnov/vbox/caos/sem03-low-level-io/b.txt



```python

```

## <a name="access"></a> access - проверка доступа


```python
!ls -la
```

    total 732
    drwxrwxr-x  4 pechatnov pechatnov   4096 сен 24 20:04 .
    drwxrwxr-x 10 pechatnov pechatnov   4096 сен 20 23:39 ..
    -rw-rw-r--  1 pechatnov pechatnov    807 сен 21 23:51 access.c
    -rwxrwxr-x  1 pechatnov pechatnov  16840 сен 21 23:51 access.exe
    -rw-rw-r--  1 pechatnov pechatnov     13 сен 24 19:39 a.txt
    drwxrwxr-x  2 pechatnov pechatnov   4096 сен 20 23:37 b_dir
    -rw-rw-r--  1 pechatnov pechatnov    222 сен 24 20:05 b.txt
    drwxrwxr-x  2 pechatnov pechatnov   4096 сен 20 23:06 .ipynb_checkpoints
    -rw-rw-r--  1 pechatnov pechatnov   3539 сен 24 19:36 linux_example.c
    -rwxrwxr-x  1 pechatnov pechatnov  16792 сен 24 19:36 linux_example.exe
    -rw-rw-r--  1 pechatnov pechatnov     15 сен 24 19:36 linux_example_input_001.txt
    -rw-r--r--  1 pechatnov pechatnov     15 сен 24 19:28 linux_example_input_002.txt
    -rw-rw-r--  1 pechatnov pechatnov    882 сен 24 19:02 linux_example_read.c
    -rwxrwxr-x  1 pechatnov pechatnov  16992 сен 24 19:02 linux_example_read.exe
    -rw-rw-r--  1 pechatnov pechatnov     16 сен 24 19:02 linux_example_read.txt
    -rw-rw-r--  1 pechatnov pechatnov   1073 сен 20 23:38 linux_file_hello_world.c
    -rwxrwxr-x  1 pechatnov pechatnov  16936 сен 20 23:38 linux_file_hello_world.exe
    -rw-rw-r--  1 pechatnov pechatnov     13 сен 20 23:38 linux_file_hello_world.out
    -rw-rw-r--  1 pechatnov pechatnov  57395 сен 24 20:01 low-level-io.ipynb
    -rw-rw-r--  1 pechatnov pechatnov   1467 сен 24 19:51 lseek_example.c
    -rwxrwxr-x  1 pechatnov pechatnov  17072 сен 24 19:51 lseek_example.exe
    -rw-rw-r--  1 pechatnov pechatnov      6 сен 24 19:51 lseek_example.txt
    -rw-rw-r--  1 pechatnov pechatnov   1445 сен 24 19:54 pread_example.c
    -rwxrwxr-x  1 pechatnov pechatnov  17072 сен 24 19:54 pread_example.exe
    -rw-rw-r--  1 pechatnov pechatnov      6 сен 24 19:54 pread_example.txt
    -rw-rw-r--  1 pechatnov pechatnov  27843 сен 22 12:15 README.md
    -rw-rw-r--  1 pechatnov pechatnov  20499 сен 22 12:15 README_no_output.md
    -rw-rw-r--  1 pechatnov pechatnov   2335 сен 24 19:12 retry_example.c
    -rwxrwxr-x  1 pechatnov pechatnov  16984 сен 24 19:12 retry_example.exe
    -rw-rw-r--  1 pechatnov pechatnov    461 сен 24 20:04 simple_open.c
    -rwxrwxr-x  1 pechatnov pechatnov  16880 сен 24 20:04 simple_open.exe
    -rw-rw-r--  1 pechatnov pechatnov    641 сен 24 19:39 strange_example.c
    -rwxrwxr-x  1 pechatnov pechatnov  16848 сен 24 19:39 strange_example.exe
    -rw-rw-r--  1 pechatnov pechatnov     13 сен 24 19:39 strange_example.out
    -rw-rw-r--  1 pechatnov pechatnov   1665 сен 20 23:06 winapi_example.c
    -rwxrwxr-x  1 pechatnov pechatnov 289581 сен 20 23:06 winapi_example.exe
    -rw-rw-r--  1 pechatnov pechatnov    188 сен 24 18:43 write_for_gdb_2.c
    -rw-rw-r--  1 pechatnov pechatnov   6144 сен 24 18:43 write_for_gdb_2.o
    -rw-rw-r--  1 pechatnov pechatnov    335 сен 24 18:44 write_for_gdb.c
    -rwxrwxr-x  1 pechatnov pechatnov  19616 сен 24 18:44 write_for_gdb.exe
    -rw-rw-r--  1 pechatnov pechatnov     57 сен 24 18:39 write_for_gdb_helper.gdb
    -rw-rw-r--  1 pechatnov pechatnov    457 сен 24 19:58 writev_example.c
    -rwxrwxr-x  1 pechatnov pechatnov  16760 сен 24 19:58 writev_example.exe


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


Run: `gcc access.c -o access.exe`



Run: `./access.exe`


    access("./access.exe", X_OK) = 0, err: Success
    access("./access.c", X_OK) = -1, err: Permission denied
    access("./access.exe", F_OK) = 0, err: Success
    access("./access788.exe", F_OK) = -1, err: No such file or directory
    access("./access.exe", W_OK) = 0, err: Success
    access("/bin/bash", W_OK) = -1, err: Permission denied



```python

```


```python
%%file executable_file
#!/bin/bash
echo hello
```

    Overwriting executable_file



```python
!chmod u+x executable_file
!strace ./executable_file
```

    execve("./executable_file", ["./executable_file"], 0x7ffda1ec3d40 /* 68 vars */) = 0
    brk(NULL)                               = 0x55719fc3c000
    arch_prctl(0x3001 /* ARCH_??? */, 0x7fffb702edc0) = -1 EINVAL (Invalid argument)
    access("/etc/ld.so.preload", R_OK)      = -1 ENOENT (No such file or directory)
    openat(AT_FDCWD, "/etc/ld.so.cache", O_RDONLY|O_CLOEXEC) = 3
    fstat(3, {st_mode=S_IFREG|0644, st_size=106091, ...}) = 0
    mmap(NULL, 106091, PROT_READ, MAP_PRIVATE, 3, 0) = 0x7f7d9f7f5000
    close(3)                                = 0
    openat(AT_FDCWD, "/lib/x86_64-linux-gnu/libtinfo.so.6", O_RDONLY|O_CLOEXEC) = 3
    read(3, "\177ELF\2\1\1\0\0\0\0\0\0\0\0\0\3\0>\0\1\0\0\0\240\346\0\0\0\0\0\0"..., 832) = 832
    fstat(3, {st_mode=S_IFREG|0644, st_size=192032, ...}) = 0
    mmap(NULL, 8192, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = 0x7f7d9f7f3000
    mmap(NULL, 194944, PROT_READ, MAP_PRIVATE|MAP_DENYWRITE, 3, 0) = 0x7f7d9f7c3000
    mmap(0x7f7d9f7d1000, 61440, PROT_READ|PROT_EXEC, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0xe000) = 0x7f7d9f7d1000
    mmap(0x7f7d9f7e0000, 57344, PROT_READ, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x1d000) = 0x7f7d9f7e0000
    mmap(0x7f7d9f7ee000, 20480, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x2a000) = 0x7f7d9f7ee000
    close(3)                                = 0
    openat(AT_FDCWD, "/lib/x86_64-linux-gnu/libdl.so.2", O_RDONLY|O_CLOEXEC) = 3
    read(3, "\177ELF\2\1\1\0\0\0\0\0\0\0\0\0\3\0>\0\1\0\0\0 \22\0\0\0\0\0\0"..., 832) = 832
    fstat(3, {st_mode=S_IFREG|0644, st_size=18848, ...}) = 0
    mmap(NULL, 20752, PROT_READ, MAP_PRIVATE|MAP_DENYWRITE, 3, 0) = 0x7f7d9f7bd000
    mmap(0x7f7d9f7be000, 8192, PROT_READ|PROT_EXEC, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x1000) = 0x7f7d9f7be000
    mmap(0x7f7d9f7c0000, 4096, PROT_READ, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x3000) = 0x7f7d9f7c0000
    mmap(0x7f7d9f7c1000, 8192, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x3000) = 0x7f7d9f7c1000
    close(3)                                = 0
    openat(AT_FDCWD, "/lib/x86_64-linux-gnu/libc.so.6", O_RDONLY|O_CLOEXEC) = 3
    read(3, "\177ELF\2\1\1\3\0\0\0\0\0\0\0\0\3\0>\0\1\0\0\0\300A\2\0\0\0\0\0"..., 832) = 832
    pread64(3, "\6\0\0\0\4\0\0\0@\0\0\0\0\0\0\0@\0\0\0\0\0\0\0@\0\0\0\0\0\0\0"..., 784, 64) = 784
    pread64(3, "\4\0\0\0\20\0\0\0\5\0\0\0GNU\0\2\0\0\300\4\0\0\0\3\0\0\0\0\0\0\0", 32, 848) = 32
    pread64(3, "\4\0\0\0\24\0\0\0\3\0\0\0GNU\0\30x\346\264ur\f|Q\226\236i\253-'o"..., 68, 880) = 68
    fstat(3, {st_mode=S_IFREG|0755, st_size=2029592, ...}) = 0
    pread64(3, "\6\0\0\0\4\0\0\0@\0\0\0\0\0\0\0@\0\0\0\0\0\0\0@\0\0\0\0\0\0\0"..., 784, 64) = 784
    pread64(3, "\4\0\0\0\20\0\0\0\5\0\0\0GNU\0\2\0\0\300\4\0\0\0\3\0\0\0\0\0\0\0", 32, 848) = 32
    pread64(3, "\4\0\0\0\24\0\0\0\3\0\0\0GNU\0\30x\346\264ur\f|Q\226\236i\253-'o"..., 68, 880) = 68
    mmap(NULL, 2037344, PROT_READ, MAP_PRIVATE|MAP_DENYWRITE, 3, 0) = 0x7f7d9f5cb000
    mmap(0x7f7d9f5ed000, 1540096, PROT_READ|PROT_EXEC, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x22000) = 0x7f7d9f5ed000
    mmap(0x7f7d9f765000, 319488, PROT_READ, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x19a000) = 0x7f7d9f765000
    mmap(0x7f7d9f7b3000, 24576, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x1e7000) = 0x7f7d9f7b3000
    mmap(0x7f7d9f7b9000, 13920, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_ANONYMOUS, -1, 0) = 0x7f7d9f7b9000
    close(3)                                = 0
    mmap(NULL, 12288, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = 0x7f7d9f5c8000
    arch_prctl(ARCH_SET_FS, 0x7f7d9f5c8740) = 0
    mprotect(0x7f7d9f7b3000, 16384, PROT_READ) = 0
    mprotect(0x7f7d9f7c1000, 4096, PROT_READ) = 0
    mprotect(0x7f7d9f7ee000, 16384, PROT_READ) = 0
    mprotect(0x55719efac000, 16384, PROT_READ) = 0
    mprotect(0x7f7d9f83c000, 4096, PROT_READ) = 0
    munmap(0x7f7d9f7f5000, 106091)          = 0
    openat(AT_FDCWD, "/dev/tty", O_RDWR|O_NONBLOCK) = 3
    close(3)                                = 0
    brk(NULL)                               = 0x55719fc3c000
    brk(0x55719fc5d000)                     = 0x55719fc5d000
    openat(AT_FDCWD, "/usr/lib/locale/locale-archive", O_RDONLY|O_CLOEXEC) = 3
    fstat(3, {st_mode=S_IFREG|0644, st_size=14537584, ...}) = 0
    mmap(NULL, 14537584, PROT_READ, MAP_PRIVATE, 3, 0) = 0x7f7d9e7ea000
    close(3)                                = 0
    openat(AT_FDCWD, "/usr/lib/x86_64-linux-gnu/gconv/gconv-modules.cache", O_RDONLY) = 3
    fstat(3, {st_mode=S_IFREG|0644, st_size=27002, ...}) = 0
    mmap(NULL, 27002, PROT_READ, MAP_SHARED, 3, 0) = 0x7f7d9f808000
    close(3)                                = 0
    getuid()                                = 1000
    getgid()                                = 1000
    geteuid()                               = 1000
    getegid()                               = 1000
    rt_sigprocmask(SIG_BLOCK, NULL, [], 8)  = 0
    ioctl(-1, TIOCGPGRP, 0x7fffb702ec14)    = -1 EBADF (Bad file descriptor)
    sysinfo({uptime=3880984, loads=[1440, 3200, 448], totalram=2083688448, freeram=95158272, sharedram=2068480, bufferram=123326464, totalswap=2147479552, freeswap=1796993024, procs=569, totalhigh=0, freehigh=0, mem_unit=1}) = 0
    rt_sigaction(SIGCHLD, {sa_handler=SIG_DFL, sa_mask=[], sa_flags=SA_RESTORER|SA_RESTART, sa_restorer=0x7f7d9f60e090}, {sa_handler=SIG_DFL, sa_mask=[], sa_flags=0}, 8) = 0
    rt_sigaction(SIGCHLD, {sa_handler=SIG_DFL, sa_mask=[], sa_flags=SA_RESTORER|SA_RESTART, sa_restorer=0x7f7d9f60e090}, {sa_handler=SIG_DFL, sa_mask=[], sa_flags=SA_RESTORER|SA_RESTART, sa_restorer=0x7f7d9f60e090}, 8) = 0
    rt_sigaction(SIGINT, {sa_handler=SIG_DFL, sa_mask=[], sa_flags=SA_RESTORER, sa_restorer=0x7f7d9f60e090}, {sa_handler=SIG_DFL, sa_mask=[], sa_flags=0}, 8) = 0
    rt_sigaction(SIGINT, {sa_handler=SIG_DFL, sa_mask=[], sa_flags=SA_RESTORER, sa_restorer=0x7f7d9f60e090}, {sa_handler=SIG_DFL, sa_mask=[], sa_flags=SA_RESTORER, sa_restorer=0x7f7d9f60e090}, 8) = 0
    rt_sigaction(SIGQUIT, {sa_handler=SIG_DFL, sa_mask=[], sa_flags=SA_RESTORER, sa_restorer=0x7f7d9f60e090}, {sa_handler=SIG_DFL, sa_mask=[], sa_flags=0}, 8) = 0
    rt_sigaction(SIGQUIT, {sa_handler=SIG_DFL, sa_mask=[], sa_flags=SA_RESTORER, sa_restorer=0x7f7d9f60e090}, {sa_handler=SIG_DFL, sa_mask=[], sa_flags=SA_RESTORER, sa_restorer=0x7f7d9f60e090}, 8) = 0
    rt_sigaction(SIGTSTP, {sa_handler=SIG_DFL, sa_mask=[], sa_flags=SA_RESTORER, sa_restorer=0x7f7d9f60e090}, {sa_handler=SIG_DFL, sa_mask=[], sa_flags=0}, 8) = 0
    rt_sigaction(SIGTSTP, {sa_handler=SIG_DFL, sa_mask=[], sa_flags=SA_RESTORER, sa_restorer=0x7f7d9f60e090}, {sa_handler=SIG_DFL, sa_mask=[], sa_flags=SA_RESTORER, sa_restorer=0x7f7d9f60e090}, 8) = 0
    rt_sigaction(SIGTTIN, {sa_handler=SIG_DFL, sa_mask=[], sa_flags=SA_RESTORER, sa_restorer=0x7f7d9f60e090}, {sa_handler=SIG_DFL, sa_mask=[], sa_flags=0}, 8) = 0
    rt_sigaction(SIGTTIN, {sa_handler=SIG_DFL, sa_mask=[], sa_flags=SA_RESTORER, sa_restorer=0x7f7d9f60e090}, {sa_handler=SIG_DFL, sa_mask=[], sa_flags=SA_RESTORER, sa_restorer=0x7f7d9f60e090}, 8) = 0
    rt_sigaction(SIGTTOU, {sa_handler=SIG_DFL, sa_mask=[], sa_flags=SA_RESTORER, sa_restorer=0x7f7d9f60e090}, {sa_handler=SIG_DFL, sa_mask=[], sa_flags=0}, 8) = 0
    rt_sigaction(SIGTTOU, {sa_handler=SIG_DFL, sa_mask=[], sa_flags=SA_RESTORER, sa_restorer=0x7f7d9f60e090}, {sa_handler=SIG_DFL, sa_mask=[], sa_flags=SA_RESTORER, sa_restorer=0x7f7d9f60e090}, 8) = 0
    rt_sigprocmask(SIG_BLOCK, NULL, [], 8)  = 0
    rt_sigaction(SIGQUIT, {sa_handler=SIG_IGN, sa_mask=[], sa_flags=SA_RESTORER, sa_restorer=0x7f7d9f60e090}, {sa_handler=SIG_DFL, sa_mask=[], sa_flags=SA_RESTORER, sa_restorer=0x7f7d9f60e090}, 8) = 0
    uname({sysname="Linux", nodename="pechatnov-vbox", ...}) = 0
    stat("/home/pechatnov/vbox/caos/sem03-low-level-io", {st_mode=S_IFDIR|0775, st_size=4096, ...}) = 0
    stat(".", {st_mode=S_IFDIR|0775, st_size=4096, ...}) = 0
    stat("/home", {st_mode=S_IFDIR|0755, st_size=4096, ...}) = 0
    stat("/home/pechatnov", {st_mode=S_IFDIR|0755, st_size=4096, ...}) = 0
    stat("/home/pechatnov/vbox", {st_mode=S_IFDIR|0775, st_size=4096, ...}) = 0
    stat("/home/pechatnov/vbox/caos", {st_mode=S_IFDIR|0775, st_size=4096, ...}) = 0
    stat("/home/pechatnov/vbox/caos/sem03-low-level-io", {st_mode=S_IFDIR|0775, st_size=4096, ...}) = 0
    stat("/home/pechatnov", {st_mode=S_IFDIR|0755, st_size=4096, ...}) = 0
    getpid()                                = 231437
    getppid()                               = 231434
    getpid()                                = 231437
    getpgrp()                               = 231433
    ioctl(2, TIOCGPGRP, [231433])           = 0
    rt_sigaction(SIGCHLD, {sa_handler=0x55719eef2ac0, sa_mask=[], sa_flags=SA_RESTORER|SA_RESTART, sa_restorer=0x7f7d9f60e090}, {sa_handler=SIG_DFL, sa_mask=[], sa_flags=SA_RESTORER|SA_RESTART, sa_restorer=0x7f7d9f60e090}, 8) = 0
    prlimit64(0, RLIMIT_NPROC, NULL, {rlim_cur=7728, rlim_max=7728}) = 0
    rt_sigprocmask(SIG_BLOCK, NULL, [], 8)  = 0
    openat(AT_FDCWD, "./executable_file", O_RDONLY) = 3
    stat("./executable_file", {st_mode=S_IFREG|0764, st_size=23, ...}) = 0
    ioctl(3, TCGETS, 0x7fffb702eba0)        = -1 ENOTTY (Inappropriate ioctl for device)
    lseek(3, 0, SEEK_CUR)                   = 0
    read(3, "#!/bin/bash\necho hello\n", 80) = 23
    lseek(3, 0, SEEK_SET)                   = 0
    prlimit64(0, RLIMIT_NOFILE, NULL, {rlim_cur=4*1024, rlim_max=1024*1024}) = 0
    fcntl(255, F_GETFD)                     = -1 EBADF (Bad file descriptor)
    dup2(3, 255)                            = 255
    close(3)                                = 0
    fcntl(255, F_SETFD, FD_CLOEXEC)         = 0
    fcntl(255, F_GETFL)                     = 0x8000 (flags O_RDONLY|O_LARGEFILE)
    fstat(255, {st_mode=S_IFREG|0764, st_size=23, ...}) = 0
    lseek(255, 0, SEEK_CUR)                 = 0
    read(255, "#!/bin/bash\necho hello\n", 23) = 23
    fstat(1, {st_mode=S_IFCHR|0620, st_rdev=makedev(0x88, 0xd), ...}) = 0
    write(1, "hello\n", 6hello
    )                  = 6
    read(255, "", 23)                       = 0
    rt_sigprocmask(SIG_BLOCK, [CHLD], [], 8) = 0
    rt_sigprocmask(SIG_SETMASK, [], NULL, 8) = 0
    exit_group(0)                           = ?
    +++ exited with 0 +++



```python

```


```python

```
