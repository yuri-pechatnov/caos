


# Аттрибуты файлов и файловых дескрипторов

<p><a href="https://www.youtube.c?????" target="_blank">
    <h3>Видеозапись семинара</h3>
</a></p>


[Ридинг Яковлева](https://github.com/victor-yacovlev/mipt-diht-caos/tree/master/practice/stat_fcntl) 


Сегодня в программе:
* <a href="#stat" style="color:#856024"> Атрибуты файлов и разные способы их получения </a>  
  * <a href="#time" style="color:#856024"> Извлечем время доступа из атрибутов файла </a>  
  * <a href="#username" style="color:#856024"> Определим логин пользователя, изменившего файл </a>  
  * <a href="#access" style="color:#856024"> Проверим свои права на файл </a>  
* <a href="#link" style="color:#856024"> Ссылки жесткие и символические </a>  
* <a href="#fds" style="color:#856024"> Атрибуты файловых дескрипторов </a>  


## <a name="stat"></a> Атрибуты файлов и разные способы их получения

Сигнатуры функций, с помощью которых можно получить аттрибуты

```c
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

int stat(const char *pathname, struct stat *buf);
int fstat(int fd, struct stat *buf);
int lstat(const char *pathname, struct stat *buf);

#include <fcntl.h>           /* Definition of AT_* constants */
#include <sys/stat.h>

int fstatat(int dirfd, const char *pathname, struct stat *buf,
		   int flags);
```


Описание структуры из `man 2 stat`:

```c
struct stat {
   dev_t     st_dev;         /* ID of device containing file */
   ino_t     st_ino;         /* inode number */
   mode_t    st_mode;        /* protection */
   nlink_t   st_nlink;       /* number of hard links */
   uid_t     st_uid;         /* user ID of owner */
   gid_t     st_gid;         /* group ID of owner */
   dev_t     st_rdev;        /* device ID (if special file) */
   off_t     st_size;        /* total size, in bytes */
   blksize_t st_blksize;     /* blocksize for filesystem I/O */
   blkcnt_t  st_blocks;      /* number of 512B blocks allocated */

   /* Since Linux 2.6, the kernel supports nanosecond
      precision for the following timestamp fields.
      For the details before Linux 2.6, see NOTES. */

   struct timespec st_atim;  /* time of last access */
   struct timespec st_mtim;  /* time of last modification */
   struct timespec st_ctim;  /* time of last status change */

#define st_atime st_atim.tv_sec      /* Backward compatibility */
#define st_mtime st_mtim.tv_sec
#define st_ctime st_ctim.tv_sec
};
```

Особый интерес будет представлять поле `.st_mode`

Биты соответствующие маскам:
* `0170000` - тип файла.

  Эти биты стоит рассматривать как одно число, по значению которого можно определить тип файла. Сравнивая это число с:  
    * `S_IFSOCK   0140000   socket`
    * `S_IFLNK    0120000   symbolic link`
    * `S_IFREG    0100000   regular file`
    * `S_IFBLK    0060000   block device`
    * `S_IFDIR    0040000   directory`
    * `S_IFCHR    0020000   character device`
    * `S_IFIFO    0010000   FIFO`
* `0777` - права на файл.

  Эти биты можно рассматривать как независимые биты, каджый из которых отвечает за право (пользователя, его группы, всех остальных) (читать/писать/выполнять) файл.

**fstat** - смотрит по файловому дескриптору


```cpp
%%cpp stat.c
%run gcc stat.c -o stat.exe
%run rm -rf tmp && mkdir tmp && touch tmp/a && ln -s ./a tmp/a_link && mkdir tmp/dir
%run ./stat.exe < tmp/a
%run ./stat.exe < tmp/dir
%run ./stat.exe < tmp/a_link

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>

int main(int argc, char *argv[])
{   
    struct stat s;
    fstat(0, &s); // get stat for stdin
    printf("is regular: %s    ", ((s.st_mode & S_IFMT) == S_IFREG) ? "yes" : "no "); // can use predefined mask
    printf("is directory: %s    ", S_ISDIR(s.st_mode) ? "yes" : "no "); // or predefined macro
    printf("is symbolic link: %s\n", S_ISLNK(s.st_mode) ? "yes" : "no "); 
    return 0;
}
```


Run: `gcc stat.c -o stat.exe`



Run: `rm -rf tmp && mkdir tmp && touch tmp/a && ln -s ./a tmp/a_link && mkdir tmp/dir`



Run: `./stat.exe < tmp/a`


    is regular: yes    is directory: no     is symbolic link: no 



Run: `./stat.exe < tmp/dir`


    is regular: no     is directory: yes    is symbolic link: no 



Run: `./stat.exe < tmp/a_link`


    is regular: yes    is directory: no     is symbolic link: no 


Обратите внимание, что для симлинки результат показан как для регулярного файла, на который она ссылается.
Тут, вероятно, замешан bash, который открывает файл на месте stdin для нашей программы. Видно, что он проходит по симлинкам.

**stat** - смотри по имени файла, следует по симлинкам


```cpp
%%cpp stat.c
%run gcc stat.c -o stat.exe
%run rm -rf tmp && mkdir tmp && touch tmp/a && ln -s ./a tmp/a_link && mkdir tmp/dir
%run ./stat.exe tmp/a
%run ./stat.exe tmp/dir
%run ./stat.exe tmp/a_link

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>

int main(int argc, char *argv[])
{   
    assert(argc == 2);
    struct stat s;
    stat(argv[1], &s); // Следует по симлинкам
    printf("is regular: %s    ", ((s.st_mode & S_IFMT) == S_IFREG) ? "yes" : "no "); // can use predefined mask
    printf("is directory: %s    ", S_ISDIR(s.st_mode) ? "yes" : "no "); // or predefined macro
    printf("is symbolic link: %s\n", S_ISLNK(s.st_mode) ? "yes" : "no ");
    return 0;
}
```


Run: `gcc stat.c -o stat.exe`



Run: `rm -rf tmp && mkdir tmp && touch tmp/a && ln -s ./a tmp/a_link && mkdir tmp/dir`



Run: `./stat.exe tmp/a`


    is regular: yes    is directory: no     is symbolic link: no 



Run: `./stat.exe tmp/dir`


    is regular: no     is directory: yes    is symbolic link: no 



Run: `./stat.exe tmp/a_link`


    is regular: yes    is directory: no     is symbolic link: no 


Обратите внимание, что для симлинки результат показан как для регулярного файла, на который она ссылается.

**lstat** - смотрит по имени файла, не следует по симлинкам.


```cpp
%%cpp stat.c
%run gcc stat.c -o stat.exe
%run rm -rf tmp && mkdir tmp && touch tmp/a && ln -s ./a tmp/a_link && mkdir tmp/dir
%run ./stat.exe tmp/a
%run ./stat.exe tmp/dir
%run ./stat.exe tmp/a_link

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>

int main(int argc, char *argv[])
{   
    assert(argc == 2);
    struct stat s;
    lstat(argv[1], &s); // Не следует по симлинкам, то есть можно узнать stat самого файла симлинки
    printf("is regular: %s    ", ((s.st_mode & S_IFMT) == S_IFREG) ? "yes" : "no "); // can use predefined mask
    printf("is directory: %s    ", S_ISDIR(s.st_mode) ? "yes" : "no "); // or predefined macro
    printf("is symbolic link: %s\n", S_ISLNK(s.st_mode) ? "yes" : "no ");
    return 0;
}
```


Run: `gcc stat.c -o stat.exe`



Run: `rm -rf tmp && mkdir tmp && touch tmp/a && ln -s ./a tmp/a_link && mkdir tmp/dir`



Run: `./stat.exe tmp/a`


    is regular: yes    is directory: no     is symbolic link: no 



Run: `./stat.exe tmp/dir`


    is regular: no     is directory: yes    is symbolic link: no 



Run: `./stat.exe tmp/a_link`


    is regular: no     is directory: no     is symbolic link: yes


Сейчас результат для симлинки показан как для самой симлинки. Так как используем специальную функцию.

**open(...O_NOFOLLOW | O_PATH) + fstat** - открываем файл так, чтобы не следовать по симлинкам и далее смотрим его stat

Кстати, открываем не очень честно. С опцией O_PATH нельзя потом применять read, write и еще некоторые операции.


```cpp
%%cpp stat.c
%run gcc stat.c -o stat.exe
%run rm -rf tmp && mkdir tmp && touch tmp/a && ln -s ./a tmp/a_link && mkdir tmp/dir
%run ./stat.exe tmp/a
%run ./stat.exe tmp/dir
%run ./stat.exe tmp/a_link

#define _GNU_SOURCE // need for O_PATH

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>

int main(int argc, char *argv[])
{   
    assert(argc == 2);
    struct stat s;
    int fd = open(argv[1], O_RDONLY | O_NOFOLLOW | O_PATH);
    assert(fd >= 0);
    fstat(fd, &s); 
    printf("is regular: %s    ", ((s.st_mode & S_IFMT) == S_IFREG) ? "yes" : "no "); // can use predefined mask
    printf("is directory: %s    ", S_ISDIR(s.st_mode) ? "yes" : "no "); // or predefined macro
    printf("is symbolic link: %s\n", S_ISLNK(s.st_mode) ? "yes" : "no ");
    close(fd);
    return 0;
}
```


Run: `gcc stat.c -o stat.exe`



Run: `rm -rf tmp && mkdir tmp && touch tmp/a && ln -s ./a tmp/a_link && mkdir tmp/dir`



Run: `./stat.exe tmp/a`


    is regular: yes    is directory: no     is symbolic link: no 



Run: `./stat.exe tmp/dir`


    is regular: no     is directory: yes    is symbolic link: no 



Run: `./stat.exe tmp/a_link`


    is regular: no     is directory: no     is symbolic link: yes


Здесь тоже результат показан для самой симлинки, поскольку вызываем open со специальными опциями, чтобы не следовать по симлинкам.

**Поэтому важно понимать, какое поведение вы хотите и использовать stat, fstat или lstat**

## <a name="time"></a> Извлечем время доступа из атрибутов файла


```cpp
%%cpp stat.c
%run gcc stat.c -o stat.exe

%run rm -rf tmp && mkdir tmp 
%run touch tmp/a && sleep 2 
%run ln -s ./a tmp/a_link && sleep 2 
%run mkdir tmp/dir

%run ./stat.exe < tmp/a
%run ./stat.exe < tmp/dir 
%run ./stat.exe < tmp/a_link


#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>

int main(int argc, char *argv[])
{   
    struct stat s;
    fstat(0, &s); 
    
    printf("update time: %s", asctime(gmtime(&s.st_mtim.tv_sec))); // '\n' есть в строке генерируемой asctime
    printf("access time: %s", asctime(gmtime(&s.st_atim.tv_sec)));

    return 0;
}
```


Run: `gcc stat.c -o stat.exe`



Run: `rm -rf tmp && mkdir tmp`



Run: `touch tmp/a && sleep 2`



Run: `ln -s ./a tmp/a_link && sleep 2`



Run: `mkdir tmp/dir`



Run: `./stat.exe < tmp/a`


    update time: Sat Nov  7 17:34:56 2020
    access time: Sat Nov  7 17:34:56 2020



Run: `./stat.exe < tmp/dir`


    update time: Sat Nov  7 17:35:00 2020
    access time: Sat Nov  7 17:35:00 2020



Run: `./stat.exe < tmp/a_link`


    update time: Sat Nov  7 17:34:56 2020
    access time: Sat Nov  7 17:34:56 2020


## example from man 2 stat


```cpp
%%cpp stat.c
%run gcc stat.c -o stat.exe

%run rm -rf tmp && mkdir tmp 
%run touch tmp/a
%run ln -s ./a tmp/a_link
%run mkdir tmp/dir

%run ./stat.exe tmp/a
%run ./stat.exe tmp/dir 
%run ./stat.exe tmp/a_link
%run ./stat.exe /bin/sh

#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
   struct stat sb;

   if (argc != 2) {
       fprintf(stderr, "Usage: %s <pathname>\n", argv[0]);
       exit(EXIT_FAILURE);
   }

   if (stat(argv[1], &sb) == -1) {
       perror("stat");
       exit(EXIT_FAILURE);
   }

   printf("File type:                ");

   switch (sb.st_mode & S_IFMT) {
       case S_IFBLK:  printf("block device\n");            break;
       case S_IFCHR:  printf("character device\n");        break;
       case S_IFDIR:  printf("directory\n");               break;
       case S_IFIFO:  printf("FIFO/pipe\n");               break;
       case S_IFLNK:  printf("symlink\n");                 break;
       case S_IFREG:  printf("regular file\n");            break;
       case S_IFSOCK: printf("socket\n");                  break;
       default:       printf("unknown?\n");                break;
   }

   printf("I-node number:            %ld\n", (long) sb.st_ino);

   printf("Mode:                     %lo (octal)\n",
           (unsigned long) sb.st_mode);

   printf("Link count:               %ld\n", (long) sb.st_nlink);
   printf("Ownership:                UID=%ld   GID=%ld\n",
           (long) sb.st_uid, (long) sb.st_gid);

   printf("Preferred I/O block size: %ld bytes\n",
           (long) sb.st_blksize);
   printf("File size:                %lld bytes\n",
           (long long) sb.st_size);
   printf("Blocks allocated:         %lld\n",
           (long long) sb.st_blocks);

   printf("Last status change:       %s", ctime(&sb.st_ctime));
   printf("Last file access:         %s", ctime(&sb.st_atime));
   printf("Last file modification:   %s", ctime(&sb.st_mtime));

   exit(EXIT_SUCCESS);
}
```


Run: `gcc stat.c -o stat.exe`



Run: `rm -rf tmp && mkdir tmp`



Run: `touch tmp/a`



Run: `ln -s ./a tmp/a_link`



Run: `mkdir tmp/dir`



Run: `./stat.exe tmp/a`


    File type:                regular file
    I-node number:            4723910
    Mode:                     100664 (octal)
    Link count:               1
    Ownership:                UID=1000   GID=1000
    Preferred I/O block size: 4096 bytes
    File size:                0 bytes
    Blocks allocated:         0
    Last status change:       Sat Nov  7 20:37:14 2020
    Last file access:         Sat Nov  7 20:37:14 2020
    Last file modification:   Sat Nov  7 20:37:14 2020



Run: `./stat.exe tmp/dir`


    File type:                directory
    I-node number:            4723912
    Mode:                     40775 (octal)
    Link count:               2
    Ownership:                UID=1000   GID=1000
    Preferred I/O block size: 4096 bytes
    File size:                4096 bytes
    Blocks allocated:         8
    Last status change:       Sat Nov  7 20:37:15 2020
    Last file access:         Sat Nov  7 20:37:15 2020
    Last file modification:   Sat Nov  7 20:37:15 2020



Run: `./stat.exe tmp/a_link`


    File type:                regular file
    I-node number:            4723910
    Mode:                     100664 (octal)
    Link count:               1
    Ownership:                UID=1000   GID=1000
    Preferred I/O block size: 4096 bytes
    File size:                0 bytes
    Blocks allocated:         0
    Last status change:       Sat Nov  7 20:37:14 2020
    Last file access:         Sat Nov  7 20:37:14 2020
    Last file modification:   Sat Nov  7 20:37:14 2020



Run: `./stat.exe /bin/sh`


    File type:                regular file
    I-node number:            1835214
    Mode:                     100755 (octal)
    Link count:               1
    Ownership:                UID=0   GID=0
    Preferred I/O block size: 4096 bytes
    File size:                129816 bytes
    Blocks allocated:         256
    Last status change:       Sat May 16 15:45:50 2020
    Last file access:         Sat Nov  7 13:03:41 2020
    Last file modification:   Thu Jul 18 21:15:27 2019


## <a name="username"></a> get user string name


```cpp
%%cpp stat.c
%run gcc stat.c -o stat.exe
%run mkdir tmp2
%run touch tmp2/a && echo $PASSWORD | sudo -S touch tmp2/b # create this file with sudo
%run ./stat.exe < tmp2/a  # created by me
%run ./stat.exe < tmp2/b  # created by root (with sudo)

#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <pwd.h>
#include <stdio.h>
#include <assert.h>

int main(int argc, char *argv[])
{
    struct stat s;
    fstat(0, &s);
    struct passwd *pw = getpwuid(s.st_uid);
    assert(pw);
    printf("%s\n", pw->pw_name);
    return 0;
}
```


Run: `gcc stat.c -o stat.exe`



Run: `mkdir tmp2`


    mkdir: cannot create directory ‘tmp2’: File exists



Run: `touch tmp2/a && echo $PASSWORD | sudo -S touch tmp2/b # create this file with sudo`


    [sudo] password for pechatnov: 


Run: `./stat.exe < tmp2/a  # created by me`


    pechatnov



Run: `./stat.exe < tmp2/b  # created by root (with sudo)`


    root


## <a name="access"></a> Проверка своих прав


```cpp
%%cpp stat.c
%run gcc stat.c -o stat.exe
%run rm -rf tmp2 && mkdir tmp2
%run touch tmp2/a 
%run touch tmp2/b && chmod +x tmp2/b 
%run ./stat.exe tmp2/a  # usual
%run ./stat.exe tmp2/b  # executable

#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>

int main(int argc, char *argv[])
{
    assert(argc >= 1);
    struct stat s;
    printf("Can execute: %s\n", (access(argv[1], X_OK) == 0) ? "yes" : "no");
    return 0;
}
```


Run: `gcc stat.c -o stat.exe`



Run: `rm -rf tmp2 && mkdir tmp2`



Run: `touch tmp2/a`



Run: `touch tmp2/b && chmod +x tmp2/b`



Run: `./stat.exe tmp2/a  # usual`


    Can execute: no



Run: `./stat.exe tmp2/b  # executable`


    Can execute: yes


## <a name="link"></a> Ссылки жесткие и символические


```python
!(rm x.txt ; rm x_hard.txt ; rm x_sym.txt) 2> /dev/null
!touch x_ordinary.txt
!touch x.txt
!link x.txt x_hard.txt
!ln -s x.txt ./x_sym.txt
!ls -la x*
```

    -rw-rw-r-- 2 pechatnov pechatnov 0 ноя  7 20:47 x_hard.txt
    -rw-rw-r-- 1 pechatnov pechatnov 0 ноя  7 20:47 x_ordinary.txt
    lrwxrwxrwx 1 pechatnov pechatnov 5 ноя  7 20:47 x_sym.txt -> x.txt
    -rw-rw-r-- 2 pechatnov pechatnov 0 ноя  7 20:47 x.txt



```python

```

## <a name="fds"></a> Атрибуты файловых дескрипторов

`fcntl(fd, F_GETFD, 0)`, флаг `FD_CLOEXEC`


```cpp
%%cpp fcntl_flags.cpp
%run gcc fcntl_flags.cpp -o fcntl_flags.exe
%run ./fcntl_flags.exe

#ifndef _GNU_SOURCE
  #define _GNU_SOURCE
#endif
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>

void describe_fd(const char* prefix, int fd) {
    int ret = fcntl(fd, F_GETFD, 0); // Функция принимает 3 аргумента, поэтому обязаны записать все 3 (даже если третий не будет использоваться)
    if (ret & FD_CLOEXEC) {
        printf("%s: fd %d has CLOEXEC flag\n", prefix, fd);
    } else {
        printf("%s: fd %d doesn't have CLOEXEC flag\n", prefix, fd);
    } 
}

int main() {
    int fd[2];
    
    pipe(fd);
    describe_fd("pipe", fd[0]);

    pipe2(fd, O_CLOEXEC); 
    describe_fd("pipe2 + O_CLOEXEC", fd[0]);

    pipe(fd);
    fcntl(fd[0], F_SETFD, fcntl(fd[0], F_GETFD, 0) | FD_CLOEXEC); //руками сделали так что у pipe есть флаг O_CLOEXEC
    describe_fd("pipe + manually set flag", fd[0]);
    return 0;
}
```

`fcntl(fd, F_GETFL, 0)`, флаги `O_RDWR`, `O_RDONLY`, `O_WRONLY`, `O_APPEND`, `O_TMPFILE`, `O_ASYNC`, `O_DIRECT`

На самом деле это только ограниченное подмножество флагов из тех, что указываются при открытии файла.


```cpp
%%cpp fcntl_open_flags.cpp
%run gcc fcntl_open_flags.cpp -o fcntl_open_flags.exe
%run ./fcntl_open_flags.exe

#ifndef _GNU_SOURCE
  #define _GNU_SOURCE
#endif
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>

void describe_fd(const char* prefix, int fd) {
    int ret = fcntl(fd, F_GETFL, 0);
    
#define flag_cond_str_expanded(flag, mask, name) ((ret & (mask)) == flag ? name : "")
#define flag_cond_str_mask(flag, mask) flag_cond_str_expanded(flag, mask, #flag)
#define flag_cond_str(flag) flag_cond_str_expanded(flag, flag, #flag)
    //printf("%d\n", ret & 3);
    printf("%s: %s, %s, %s, %s, %s, %s, %s, %s, %s, %s\n", prefix
        , flag_cond_str_mask(O_RDONLY, O_RDONLY | O_WRONLY | O_RDWR)
        , flag_cond_str_mask(O_WRONLY, O_RDONLY | O_WRONLY | O_RDWR)
        , flag_cond_str_mask(O_RDWR, O_RDONLY | O_WRONLY | O_RDWR)
        , flag_cond_str(O_TRUNC)
        , flag_cond_str(O_APPEND)
        , flag_cond_str(O_CREAT)
        , flag_cond_str(O_CLOEXEC)
        , flag_cond_str(O_TMPFILE)
        , flag_cond_str(O_ASYNC)
        , flag_cond_str(O_DIRECT)
    );
}

void check_fd(int fd) {
    if (fd < 0) {
        perror("open");
        assert(fd >= 0);
    }
} 

int main() {
    describe_fd("0 (stdin)", 0);
    describe_fd("1 (stdout)", 1);
    describe_fd("2 (stderr)", 2);
    
    int f1 = open("fcntl_open_flags.1", O_CREAT|O_TRUNC|O_WRONLY, 0664); check_fd(f1);
    describe_fd("f1 O_CREAT|O_TRUNC|O_WRONLY", f1);
    
    int f2 = open("fcntl_open_flags.2", O_CREAT|O_RDWR, 0664); check_fd(f2);
    describe_fd("f2 O_CREAT|O_RDWR", f2);
    
    int f3 = open("fcntl_open_flags.2", O_WRONLY|O_APPEND); check_fd(f3);
    describe_fd("f3 O_WRONLY|O_APPEND", f3);

    int f4 = open("fcntl_open_flags.2", O_RDONLY|O_NONBLOCK|O_ASYNC|O_DIRECT); check_fd(f4);
    describe_fd("f4 O_RDONLY|O_NONBLOCK|O_ASYNC|O_DIRECT", f4);
    
    int f5 = open("./", O_TMPFILE|O_RDWR, 0664); check_fd(f5);
    describe_fd("f5 O_TMPFILE|O_RDWR", f5);
    
    int fds[2];
    pipe2(fds, O_CLOEXEC); 
    describe_fd("pipe2(fds, O_CLOEXEC)", fds[0]);
    return 0;
}
```


```python

```


```python

```
