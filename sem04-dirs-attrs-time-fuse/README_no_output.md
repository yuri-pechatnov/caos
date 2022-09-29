

# Аттрибуты файлов и файловых дескрипторов

<p><a href="https://www.youtube.com/watch?v=bMmE7PPA1LQ&list=PLjzMm8llUm4AmU6i_hPU0NobgA4VsBowc&index=12" target="_blank">
    <h3>Видеозапись семинара</h3>
</a></p>


[Ридинг Яковлева про stat](https://github.com/victor-yacovlev/mipt-diht-caos/tree/master/practice/stat_fcntl) 
<br>[Ридинг Яковлева про работу с директориями, временем и еще несколькими вещами](https://github.com/victor-yacovlev/mipt-diht-caos/tree/master/practice/posix_dirent_time)
<br>[Ридинг Яковлева про FUSE](https://github.com/victor-yacovlev/mipt-diht-caos/tree/master/practice/fuse)


Сегодня в программе:
* <a href="#fs_posix" style="color:#856024"> Работа с файловой системой POSIX </a>
  * <a href="#opendir" style="color:#856024"> Просмотр содержимого директории c фильтрацией по регулярке </a>
  * <a href="#glob" style="color:#856024"> glob или история о том, как вы пишете *.cpp в терминале </a>
  * <a href="#fs_stat" style="color:#856024"> Информация о файловой системе. </a>
* <a href="#stat" style="color:#856024"> Атрибуты файлов и разные способы их получения </a>  
  * <a href="#time" style="color:#856024"> Извлечем время доступа из атрибутов файла </a>  
* <a href="#link" style="color:#856024"> Ссылки жесткие и символические </a>  
* <a href="#fds" style="color:#856024"> Атрибуты файловых дескрипторов </a> 
* <a href="#fusepy" style="color:#856024"> FUSE: Python + fusepy.  Примонтируем json как read-only файловую систему. </a>


## <a name="fs_posix"></a> Работа с файловой системой в POSIX



## <a name="opendir"></a> Просмотр содержимого директории с фильтрацией по регулярке



```cpp
%%cpp traverse_dir.c
%run gcc -Wall -Werror -fsanitize=address traverse_dir.c -lpthread -o traverse_dir.exe
%run ./traverse_dir.exe ..

#include <stdio.h>
#include <dirent.h>
#include <assert.h>
#include <fnmatch.h>

int main(int argc, char** argv) {
    assert(argc == 2);
    const char* dir_path = argv[1];
    DIR *pDir = opendir(dir_path);
    if (pDir == NULL) {
        fprintf(stderr, "Cannot open directory '%s'\n", dir_path);
        return 1;
    }
    int limit = 4;
    for (struct dirent *pDirent; (pDirent = readdir(pDir)) != NULL && limit > 0;) {
        // + Регулярочки
        if (fnmatch("sem0[12]*", pDirent->d_name, 0) == 0) {
            printf("%s\n", pDirent->d_name);
            --limit;
        }
    }
    closedir(pDir);
    return 0;
}
```

## <a name="glob"></a> glob или история о том, как вы пишете *.cpp в терминале

Это не совсем про файловую систему, но тем не менее интересно

glob хорошо сочетается с exec, пример тут http://man7.org/linux/man-pages/man3/glob.3.html


```cpp
%%cpp traverse_dir.c
%run gcc -Wall -Werror -fsanitize=address traverse_dir.c -lpthread -o traverse_dir.exe
%run ./traverse_dir.exe .. | head -n 5

#include <stdio.h>
#include <assert.h>
#include <glob.h>

int main() {
    glob_t globbuf = {0};
    glob("*.c", 0, NULL, &globbuf);
    glob("../*/*.c", GLOB_APPEND, NULL, &globbuf);
    for (char** path = globbuf.gl_pathv; *path; ++path) {
        printf("%s\n", *path);;
    }
    globfree(&globbuf);
    return 0;
}
```


```python
import glob
glob.glob("../*/*.c")[:4]
```

## <a name="fs_stat"></a> Информация о файловой системе


```cpp
%%cpp fs_stat.c
%run gcc -Wall -Werror -fsanitize=address fs_stat.c -lpthread -o fs_stat.exe
%run ./fs_stat.exe /
%run ./fs_stat.exe /dev/shm
%run ./fs_stat.exe /dev

#include <stdio.h>
#include <sys/statvfs.h>
#include <assert.h>

    
int main(int argc, char** argv) {
    assert(argc == 2);
    const char* dir_path = argv[1];
    struct statvfs stat;
    statvfs(dir_path, &stat);
    
    printf("Free 1K-blocks %lu/%lu", stat.f_bavail * stat.f_bsize / 1024, stat.f_blocks * stat.f_bsize / 1024);
    return 0;
}
```


```python
!df | grep -E "Filesys|/dev$|/dev/shm$|/$"
```


```python

```


```python

```

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
%%cpp stat.h

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

void describe_stat_st_mode(const struct stat* s) {   
    printf("is regular: %s    ", ((s->st_mode & S_IFMT) == S_IFREG) ? "yes" : "no "); // can use predefined mask
    printf("is directory: %s    ", S_ISDIR(s->st_mode) ? "yes" : "no "); // or predefined macro
    printf("is symbolic link: %s\n", S_ISLNK(s->st_mode) ? "yes" : "no "); 
}
```


```cpp
%%cpp stat.c
%run gcc stat.c -o stat.exe
%run rm -rf tmp && mkdir tmp && touch tmp/a && ln -s ./a tmp/a_link && mkdir tmp/dir
%run for i in tmp/a tmp/dir tmp/a_link ; do (printf "%10s    " $i ; ./stat.exe < $i); done

#include "stat.h"
#include <assert.h>

int main(int argc, char *argv[])
{   
    struct stat s;
    fstat(0, &s); // get stat for stdin
    describe_stat_st_mode(&s); 
    return 0;
}
```

Обратите внимание, что для симлинки результат показан как для регулярного файла, на который она ссылается.
Тут, вероятно, замешан bash, который открывает файл на месте stdin для нашей программы. Видно, что он проходит по симлинкам.

**stat** - смотри по имени файла, следует по симлинкам


```cpp
%%cpp stat.c
%run gcc stat.c -o stat.exe
%run rm -rf tmp && mkdir tmp && touch tmp/a && ln -s ./a tmp/a_link && mkdir tmp/dir
%run for i in tmp/a tmp/dir tmp/a_link ; do (printf "%10s    " $i ; ./stat.exe $i); done

#include "stat.h"
#include <assert.h>

int main(int argc, char *argv[])
{   
    assert(argc == 2);
    struct stat s;
    stat(argv[1], &s); // Следует по симлинкам
    describe_stat_st_mode(&s); 
    return 0;
}
```

Обратите внимание, что для симлинки результат показан как для регулярного файла, на который она ссылается.

**lstat** - смотрит по имени файла, не следует по симлинкам.


```cpp
%%cpp stat.c
%run gcc stat.c -o stat.exe
%run rm -rf tmp && mkdir tmp && touch tmp/a && ln -s ./a tmp/a_link && mkdir tmp/dir
%run for i in tmp/a tmp/dir tmp/a_link ; do (printf "%10s    " $i ; ./stat.exe $i); done

#include "stat.h"
#include <assert.h>

int main(int argc, char *argv[])
{   
    assert(argc == 2);
    struct stat s;
    lstat(argv[1], &s); // Не следует по симлинкам, то есть можно узнать stat самого файла симлинки
    describe_stat_st_mode(&s); 
    return 0;
}
```

Сейчас результат для симлинки показан как для самой симлинки. Так как используем специальную функцию.

**open(...O_NOFOLLOW | O_PATH) + fstat** - открываем файл так, чтобы не следовать по симлинкам и далее смотрим его stat

Кстати, открываем не очень честно. С опцией O_PATH нельзя потом применять read, write и еще некоторые операции.


```cpp
%%cpp stat.c
%run gcc stat.c -o stat.exe
%run rm -rf tmp && mkdir tmp && touch tmp/a && ln -s ./a tmp/a_link && mkdir tmp/dir
%run for i in tmp/a tmp/dir tmp/a_link ; do (printf "%10s    " $i ; ./stat.exe $i); done

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

Здесь тоже результат показан для самой симлинки, поскольку вызываем open со специальными опциями, чтобы не следовать по симлинкам.

**Поэтому важно понимать, какое поведение вы хотите и использовать stat, fstat или lstat**

## <a name="time"></a> Извлечем время доступа из атрибутов файла

Работа с временем это на самом деле отдельная большая тема. Три года назад на нее был [целый семинар](https://github.com/yuri-pechatnov/caos/tree/master/caos_2019-2020/sem28-unix-time).


```cpp
%%cpp stat.c
%run gcc stat.c -o stat.exe
%run rm -rf tmp && mkdir tmp &&  touch tmp/a && sleep 1.1 &&  ln -s ./a tmp/a_link && sleep 1.1  && mkdir tmp/dir
%run for i in tmp/a tmp/dir tmp/a_link ; do (printf "%10s    " $i ; ./stat.exe < $i); done

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>


char* pretty_time(const struct timespec* timespec_time) {
    // Whole function is not thread safe because of buf.
    struct tm tm_time;
    assert(localtime_r(&timespec_time->tv_sec, &tm_time)); // Thread-safe
    static char buf[100];
    assert(strftime(buf, sizeof(buf), "%H:%M:%S", &tm_time) > 0); // Thread-safe
    return buf; 
}

int main(int argc, char *argv[])
{   
    struct stat s;
    fstat(0, &s); 
    printf("modification time: %s   ", pretty_time(&s.st_mtim));
    printf("access time: %s\n", pretty_time(&s.st_atim));
    return 0;
}
```

## <a name="link"></a> Ссылки жесткие и символические


```python
!(rm lexmpl_*) 2> /dev/null
!touch lexmpl_ordinary.txt
!touch lexmpl_x.txt
!link lexmpl_x.txt lexmpl_x_hard.txt
!ln -s lexmpl_x.txt ./lexmpl_x_sym.txt
!ls -la lexmpl_x*
```


```python
!echo "Hello" > lexmpl_x.txt
!ls -la lexmpl_*
```

## <a name="fds"></a> Атрибуты файловых дескрипторов

`fcntl(fd, F_GETFL, 0)`, флаги `O_RDWR`, `O_RDONLY`, `O_WRONLY`, ...


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
#include <fcntl.h>

void describe_fd_impl(const char* prefix, int fd) {
    if (fd < 0) { // Check that fd is valid.
        perror("open"); 
        abort();
    }
    int ret = fcntl(fd, F_GETFL, 0); // Get flags here!
    int mask = O_RDONLY | O_WRONLY | O_RDWR;
#define flag_cond_str_expanded(flag, name) ((ret & mask) == flag ? name : "")
#define flag_cond_str(flag) flag_cond_str_expanded(flag, #flag)
    printf("%55s:   %s%s%s\n", prefix, flag_cond_str(O_RDONLY), flag_cond_str(O_WRONLY), flag_cond_str(O_RDWR));
}

int main() {
#define describe(op) describe_fd_impl(#op, op)
    describe(open("fcntl_open_flags.1", O_CREAT | O_WRONLY, 0664));
    describe(open("fcntl_open_flags.2", O_CREAT | O_RDWR, 0664));
    describe(open("fcntl_open_flags.2", O_RDONLY));
    return 0;
}
```


```python

```


```cpp
%%cpp istty.c
%run gcc istty.c -o istty.exe
%run ./istty.exe > a.txt
%run ./istty.exe 

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
    if (isatty(STDOUT_FILENO)) {
        fprintf(stderr, "\033[0;31mIt's terminal\033[0m\n");
    } else {
        fprintf(stderr, "It's NOT terminal\n");
    }
    return 0;
}
```


```python

```


```python

```

## <a name="fusepy"></a> FUSE: Python + fusepy.  Примонтируем json как read-only файловую систему. </a>



[FUSE на wiki](https://ru.wikipedia.org/wiki/FUSE_(модуль_ядра))

![FUSE](https://upload.wikimedia.org/wikipedia/commons/thumb/0/08/FUSE_structure.svg/490px-FUSE_structure.svg.png)


https://habr.com/ru/post/315654/ - на питоне

https://engineering.facile.it/blog/eng/write-filesystem-fuse/
  
  

Установка: `pip2 install --user fusepy`


```python
%%writefile fuse_json.py
from __future__ import print_function

import logging
import os
import json
from errno import EIO, ENOENT, EROFS
from stat import S_IFDIR, S_IFREG
from sys import argv, exit
from time import time

from fuse import FUSE, FuseOSError, LoggingMixIn, Operations

NOW = time()

DIR_ATTRS = dict(st_mode=(S_IFDIR | 0o555), st_nlink=2)
FILE_ATTRS = dict(st_mode=(S_IFREG | 0o444), st_nlink=1)

def find_json_path(j, path):
    for part in path.split('/'):
        if len(part) > 0:
            if part == '__json__':
                return json.dumps(j)
            if part not in j:
                return None
            j = j[part]
    return j
    

class FuseOperations(LoggingMixIn, Operations):

    def __init__(self, j):
        self.j = j
        self.fd = 0

    def open(self, path, flags):
        self.fd += 1
        return self.fd

    def read(self, path, size, offset, fh):
        logging.debug("Read %r %r %r", path, size, offset)
        node = find_json_path(self.j, path)
        if not isinstance(node, str):
            raise FuseOSError(EIO)
        return node[offset:offset + size]

    def readdir(self, path, fh):
        logging.debug("Readdir %r %r", path, fh)
        node = find_json_path(self.j, path)
        if node is None:
            raise FuseOSError(EROFS)
        return ['.', '..', '__json__'] + list(node.keys())

    def getattr(self, path, fh=None):
        node = find_json_path(self.j, path)
        if isinstance(node, dict):
            return DIR_ATTRS
        elif isinstance(node, str):
            attrs = dict(FILE_ATTRS)
            attrs["st_size"] = len(node)
            return attrs
        else:
            raise FuseOSError(ENOENT)

if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)
    j = {
        'a': 'b',
        'c': {
            'c1': '234'
        }
    }
    FUSE(FuseOperations(j), "./fuse_json", foreground=True)
```


```python
!mkdir fuse_json 2>&1 | grep -v "File exists" || true
a = TInteractiveLauncher("python2 fuse_json.py example.txt fuse_json 2>&1")
```


```bash
%%bash
exec 2>&1 ; set -v ; set -o pipefail

ls -la fuse_json                      # List dir. 

tree fuse_json --noreport             # Recursively list dirs (install: sudo apt install tree). 

cat fuse_json/__json__    && echo     #

cat fuse_json/a           && echo     #

cat fuse_json/c/__json__  && echo     #
```


```python
!fusermount -u fuse_json
a.close()
```


```python

```
