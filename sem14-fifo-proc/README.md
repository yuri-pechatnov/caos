```python
# look at tools/set_up_magics.ipynb
get_ipython().run_cell('# one_liner_str <too much code> \n')
None
```


    <IPython.core.display.Javascript object>


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
    /bin/sh: 1: cannot create my_fifo: Interrupted system call



```python
%bash_async echo "Hello" > my_fifo ; echo "After writing to my_fifo"
```



<table width="100%">
<colgroup>
   <col span="1" style="width: 70px;">
   <col span="1">
</colgroup>    
<tbody>
  <tr> <td><b>STDOUT</b> <td> 
    
     ``` 
 
 ``` 

      
  <tr> <td><b>STDERR</b> <td> 
    
     ``` 
 After writing to my_fifo

 ``` 

      
  <tr> <td><b>RUN LOG</b> <td> 
    
     ``` 
 Process started! pid=6801
Process finished! exit_code=0

 ``` 

      
</tbody>
</table>




```python
!cat my_fifo
```

    Hello


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
%bash_async ./write_fifo.exe
```



<table width="100%">
<colgroup>
   <col span="1" style="width: 70px;">
   <col span="1">
</colgroup>    
<tbody>
  <tr> <td><b>STDOUT</b> <td> 
    
     ``` 
 
 ``` 

      
  <tr> <td><b>STDERR</b> <td> 
    
     ``` 
 Started
Opened
Written
Closed

 ``` 

      
  <tr> <td><b>RUN LOG</b> <td> 
    
     ``` 
 Process started! pid=6819
Process finished! exit_code=0

 ``` 

      
</tbody>
</table>




```python
!cat my_fifo
```

    Hello from C!
    

# Директория /proc/<pid\>/*

Интересная штука директория `/proc` это виртулаьная файловая система в которой можно получать сведения о процессах, читая из из файлов. (Это не обычные файлы на диске, а скорее некоторое view на сведения о процессах из ядра системы).

Что есть в proc: http://man7.org/linux/man-pages/man5/proc.5.html

Имеющая отношение к делу статья на хабре: https://habr.com/ru/post/209446/

Посмотрим, что можно узнать о запущенном процессе:


```python
# запустим процесс в фоне
%bash_async echo "Hello" > my_fifo 
```



<table width="100%">
<colgroup>
   <col span="1" style="width: 70px;">
   <col span="1">
</colgroup>    
<tbody>
  <tr> <td><b>STDOUT</b> <td> 
    
     ``` 
 
 ``` 

      
  <tr> <td><b>STDERR</b> <td> 
    
     ``` 
 
 ``` 

      
  <tr> <td><b>RUN LOG</b> <td> 
    
     ``` 
 Process started! pid=6852
Process finished! exit_code=0

 ``` 

      
</tbody>
</table>




```python
!cat /proc/6852/status
```

    Name:	bash
    Umask:	0002
    State:	S (sleeping)
    Tgid:	6852
    Ngid:	0
    Pid:	6852
    PPid:	6851
    TracerPid:	0
    Uid:	1000	1000	1000	1000
    Gid:	1000	1000	1000	1000
    FDSize:	64
    Groups:	4 24 27 30 46 113 128 130 999 1000 
    NStgid:	6852
    NSpid:	6852
    NSpgid:	4079
    NSsid:	4079
    VmPeak:	   19596 kB
    VmSize:	   19596 kB
    VmLck:	       0 kB
    VmPin:	       0 kB
    VmHWM:	    3132 kB
    VmRSS:	    3132 kB
    RssAnon:	     200 kB
    RssFile:	    2932 kB
    RssShmem:	       0 kB
    VmData:	     176 kB
    VmStk:	     132 kB
    VmExe:	     976 kB
    VmLib:	    2112 kB
    VmPTE:	      76 kB
    VmSwap:	       0 kB
    HugetlbPages:	       0 kB
    CoreDumping:	0
    Threads:	1
    SigQ:	4/7735
    SigPnd:	0000000000000000
    ShdPnd:	0000000000000000
    SigBlk:	0000000000000000
    SigIgn:	0000000000000006
    SigCgt:	0000000000010000
    CapInh:	0000000000000000
    CapPrm:	0000000000000000
    CapEff:	0000000000000000
    CapBnd:	0000003fffffffff
    CapAmb:	0000000000000000
    NoNewPrivs:	0
    Seccomp:	0
    Speculation_Store_Bypass:	vulnerable
    Cpus_allowed:	1
    Cpus_allowed_list:	0
    Mems_allowed:	00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000001
    Mems_allowed_list:	0
    voluntary_ctxt_switches:	1
    nonvoluntary_ctxt_switches:	1



```python
!cat my_fifo
```

    Hello



```python
!ps aux | grep write_fifo 
```

    pechatn+  6857  0.0  0.0   4504   740 pts/20   Ss+  00:19   0:00 /bin/sh -c ps aux | grep write_fifo 
    pechatn+  6859  0.0  0.0  21292  1084 pts/20   S+   00:19   0:00 grep write_fifo



```python

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
%bash_async cat my_fifo
```



<table width="100%">
<colgroup>
   <col span="1" style="width: 70px;">
   <col span="1">
</colgroup>    
<tbody>
  <tr> <td><b>STDOUT</b> <td> 
    
     ``` 
 
 ``` 

      
  <tr> <td><b>STDERR</b> <td> 
    
     ``` 
 Hello 1
Hello 2
Hello 3

 ``` 

      
  <tr> <td><b>RUN LOG</b> <td> 
    
     ``` 
 Process started! pid=6867
Process finished! exit_code=0

 ``` 

      
</tbody>
</table>




```python
!echo "Hello 1" > my_fifo
!echo "Hello 2" > my_fifo
!echo "Hello 3" > my_fifo
```


```python
os.close(fd) # Только после закрытия дескриптора процесс 'cat my_fifo' завершится. Так как закроется fifo
```

### Если же ненужного чтения не создавать:


```python
%bash_async cat my_fifo
```



<table width="100%">
<colgroup>
   <col span="1" style="width: 70px;">
   <col span="1">
</colgroup>    
<tbody>
  <tr> <td><b>STDOUT</b> <td> 
    
     ``` 
 
 ``` 

      
  <tr> <td><b>STDERR</b> <td> 
    
     ``` 
 Hello 1

 ``` 

      
  <tr> <td><b>RUN LOG</b> <td> 
    
     ``` 
 Process started! pid=6888
Process finished! exit_code=0

 ``` 

      
</tbody>
</table>




```python
!echo "Hello 1" > my_fifo
```


```python
!echo "Hello 2" > my_fifo # то все зависнет тут
```

    ^C
    /bin/sh: 1: cannot create my_fifo: Interrupted system call



```python

```
