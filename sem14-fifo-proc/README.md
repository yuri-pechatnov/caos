


# named FIFO

Ранее мы познакомились с пайпами (анонимными fifo (далее буду называть просто pipe'ами)). Теперь же посмотрим на именованые.
Отличие в том, что именоваванные fifo (дальше буду называть просто fifo) являются файлами в файловой системе linux. Соответственно они могут существовать, не будучи открытыми какой-либо программой. Как и файл их можно удалить.

Как создать из консоли - `man mkfifo`, как создать из кода на C - `man 3 mkfifo`. Чтение и запись в fifo происходит так же как и с обычным файлом.

**Важно:** fifo, это файл читаемый двумя процессами и важно, кто открыл процесс на запись, кто на чтение. Например, fifo не может быть открыта на запись, пока кто-нибудь не открыл ее на чтение.


```python
!rm -f my_fifo
!mkfifo my_fifo
```


```python
!echo "Hello" > my_fifo
```

    ^C
    /usr/bin/sh: 1: cannot create my_fifo: Interrupted system call



```python
a = TInteractiveLauncher(
    'echo "Hello" > my_fifo ; echo "After writing to my_fifo"'
)
```





```
L | Process started. PID = 8402
O | After writing to my_fifo
L | Process finished. Exit code 0

```





```python
!cat my_fifo
```

    Hello



```python
a.close()
```

# Теперь на С
Обратите внимание, что fifo не может открыться на запись, пока ее не начнут читать.


```cpp
%%cpp write_fifo.cpp
%run gcc write_fifo.cpp -o write_fifo.exe

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <sys/wait.h>
#include <fcntl.h>

int main() {
    fprintf(stderr, "Started\n"); fflush(stderr);
    int fd = open("my_fifo", O_WRONLY);
    assert(fd >= 0);
    fprintf(stderr, "Opened\n"); fflush(stderr);
    const char str[] = "Hello from C!\n";
    assert(write(fd, str, sizeof(str)) == sizeof(str));
    fprintf(stderr, "Written\n"); fflush(stderr);
    assert(close(fd) == 0);
    fprintf(stderr, "Closed\n"); fflush(stderr);
    return 0;
}
```


Run: `gcc write_fifo.cpp -o write_fifo.exe`



```python
a = TInteractiveLauncher('./write_fifo.exe')
```





```
L | Process started. PID = 8412
E | Started
E | Opened
E | Written
E | Closed
L | Process finished. Exit code 0

```





```python
!cat my_fifo
```

    Hello from C!
    


```python
a.close()
```

# Директория /proc/<pid\>/*

Интересная штука директория `/proc` это виртулаьная файловая система в которой можно получать сведения о процессах, читая из из файлов. (Это не обычные файлы на диске, а скорее некоторое view на сведения о процессах из ядра системы).

Что есть в proc: http://man7.org/linux/man-pages/man5/proc.5.html

Имеющая отношение к делу статья на хабре: https://habr.com/ru/post/209446/

Посмотрим, что можно узнать о запущенном процессе:


```python
# запустим процесс в фоне
a = TInteractiveLauncher('echo "Hello" > my_fifo')
```





```
L | Process started. PID = 8416
L | Process finished. Exit code 0

```





```python
!cat /proc/3599/status
```

    cat: /proc/3599/status: No such file or directory



```python
!cat my_fifo
```

    Hello



```python
!ps aux | grep write_fifo 
```

    pechatn+    8421  0.0  0.0   2608   588 pts/2    Ss+  12:25   0:00 /usr/bin/sh -c ps aux | grep write_fifo 
    pechatn+    8423  0.0  0.0  17668   716 pts/2    S+   12:25   0:00 grep write_fifo



```python
a.close()
```

# Пример применения на моей практике
Только на семинаре


```python

```


```python

```

# Пример, почему важно правильно открывать fifo


```python
import os
```


```python
fd = os.open("my_fifo", os.O_RDWR) # создаем ненужное открытие файла на запись
```


```python
a = TInteractiveLauncher('cat my_fifo')
```





```
L | Process started. PID = 8425
O | Hello 1
O | Hello 2
O | Hello 3
L | Process finished. Exit code 0

```





```python
!echo "Hello 1" > my_fifo
!echo "Hello 2" > my_fifo
!echo "Hello 3" > my_fifo
```


```python
os.close(fd) # Только после закрытия дескриптора процесс 'cat my_fifo' завершится. Так как закроется fifo
```


```python
a.close()
```

### Если же ненужного чтения не создавать:


```python
a = TInteractiveLauncher('cat my_fifo')
```





```
L | Process started. PID = 8430
O | Hello 1
L | Process finished. Exit code 0

```





```python
!echo "Hello 1" > my_fifo
```


```python
b = TInteractiveLauncher(
    'echo "Hello 2" > my_fifo # то все зависнет тут'
)
```





```
L | Process started. PID = 8433
L | Process finished. Got signal 9

```





```python
os.kill(b.get_pid(), 9)
b.close()
```


```python
a.close()
```


```python

```
