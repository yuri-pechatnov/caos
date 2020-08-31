

# FUSE

<table width=100%  > <tr>
    <th width=15%> <b>Видео с семинара &rarr; </b> </th>
    <th>
    <a href="https://www.youtube.com/watch?v=__RuADlaK0k&list=PLjzMm8llUm4CL-_HgDrmoSTZBCdUk5HQL&index=5"><img src="video.jpg" width="320" 
   height="160" align="left" alt="Видео с семинара"></a>
    </th>
    <th> </th>
</table>

Сегодня в программе:
* <a href="#fs_posix" style="color:#856024"> Работа с файловой системой POSIX </a>
  * <a href="#opendir" style="color:#856024"> Просмотр содержимого директории c фильтрацией по регулярке </a>
  * <a href="#glob" style="color:#856024"> glob или история о том, как вы пишете *.cpp в терминале </a>
  * <a href="#ftw" style="color:#856024"> Рекурсивный просмотр. Правда с помощью устаревшей функции. </a>
  * <a href="#fs_stat" style="color:#856024"> Информация о файловой системе. </a>
  
* <a href="#fusepy" style="color:#856024"> Примонтируем json как read-only файловую систему. Python + fusepy </a>
* <a href="#fuse_с" style="color:#856024"> Файловая система с одним файлом на C </a>


https://ru.wikipedia.org/wiki/FUSE_(модуль_ядра)

![FUSE](https://upload.wikimedia.org/wikipedia/commons/thumb/0/08/FUSE_structure.svg/490px-FUSE_structure.svg.png)


https://habr.com/ru/post/315654/ - на питоне

https://engineering.facile.it/blog/eng/write-filesystem-fuse/




[Ридинг Яковлева](https://github.com/victor-yacovlev/mipt-diht-caos/tree/master/practice/fuse)
  
  
<a href="#hw" style="color:#856024">Комментарии к ДЗ</a>



## <a name="fs_posix"></a> Работа с файловой системой в POSIX




Заголовочные файлы, в которых есть функции для работы с файловой системой ([wiki-источник](https://en.wikipedia.org/wiki/C_POSIX_library)):

| Header file | Description |
|-------------|-------------|
| `<fcntl.h>` |	File opening, locking and other operations |
| `<fnmatch.h>` |	Filename matching |
| `<ftw.h>` |	File tree traversal |
| `<sys/stat.h>` |	File information (stat et al.) |
| `<sys/statvfs.h>` |	File System information |
| `<dirent.h>` | Directories opening, traversing |


read, write, stat, fstat - это все было раньше


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
        if (fnmatch("sem2*", pDirent->d_name, 0) == 0) {
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
    glob("*.c", GLOB_DOOFFS, NULL, &globbuf);
    glob("../*/*.c", GLOB_DOOFFS | GLOB_APPEND, NULL, &globbuf);
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

## <a name="ftw"></a> Рекурсивный просмотр. Правда с помощью устаревшей функции.


```cpp
%%cpp traverse_dir_2.c
%run gcc -Wall -Werror -fsanitize=address traverse_dir_2.c -lpthread -o traverse_dir_2.exe
%run ./traverse_dir_2.exe ..

#include <stdio.h>
#include <ftw.h>
#include <assert.h>

int limit = 4;
    
int callback(const char* fpath, const struct stat* sb, int typeflag) {
    printf("%s %ld\n", fpath, sb->st_size);
    return (--limit == 0);
}
    
int main(int argc, char** argv) {
    assert(argc == 2);
    const char* dir_path = argv[1];
    ftw(dir_path, callback, 0);
    return 0;
}
```

## <a name="fs_stat"></a> Информация о файловой системе


```cpp
%%cpp fs_stat.c
%run gcc -Wall -Werror -fsanitize=address fs_stat.c -lpthread -o fs_stat.exe
%run ./fs_stat.exe ..
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
!df
```


```python

```

# FUSE

Важные опции
* `-f` - запуск в синхронном режиме (без этой опции будет создан демон, а сама программа почти сразу завершится)
* `-s` - запуск в однопоточном режиме.

В этом месте что-нибудь про демонизацию стоит расскзать, наверное.

## <a name="fusepy"></a> Python + fusepy

Установк: `pip2 install --user fusepy`


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


```python
!ls fuse_json
!cat fuse_json/c/__json__
```


```bash
%%bash
echo -n -e "\n" > new_line
exec 2>&1 ; set -o xtrace

tree fuse_json --noreport 

cat fuse_json/__json__    new_line
cat fuse_json/a           new_line
cat fuse_json/c/__json__  new_line
```


```python
!fusermount -u fuse_json
a.close()
```

`sudo apt install tree`


```bash
%%bash
tree fuse_json --noreport
```


```python

```

## <a name="fuse_c"></a> fuse + с

Надо поставить `libfuse-dev`. Возможно, для этого нужно подаунгрейдить `libfuse2`.

Да, обращаю внимание, что у Яковлева в ридинге используется fuse3. Но что-то его пока не очень тривиально поставить в Ubuntu 16.04 (за час не справился) и мне не хочется ненароком себе что-нибудь сломать в системе :)

fuse3 немного отличается по API. В примере я поддержал компилируемость и с fuse2, и с fuse3.

Для установки на Ubuntu может оказаться полезным [Официальный репозиторий Fuse](https://github.com/libfuse/libfuse).  
В нём указаны шаги установки. Правда, может понадобиться поставить ещё [*Ninja*](https://ninja-build.org/) и [*Meson*](https://mesonbuild.com/).


```cmake
%%cmake with_fuse_1.cmake
cmake_minimum_required(VERSION 3.15)
project(hw23 CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fsanitize=leak -g")
set(FUSE_PATH "downloads/fuse")

add_executable(hw23 task.cpp)

target_include_directories(hw23 PUBLIC ${FUSE_PATH}/include) # -I/usr/include/fuse3
target_link_libraries(hw23 ${FUSE_PATH}/build/lib/libfuse3.so) # -lfuse3 -lpthread

```

Либо, если следовать скрипту ниже, то может помочь такой CMake


```cmake
%%cmake with_fuse_2.cmake
cmake_minimum_required(VERSION 2.7)

find_package(PkgConfig REQUIRED)
pkg_check_modules(FUSE REQUIRED fuse3)

include_directories(${FUSE_INCLUDE_DIRS})
add_executable(main main.c)
target_link_libraries(main ${FUSE_LIBRARIES})
```


```python

```

Код во многом взят отсюда: https://github.com/fntlnz/fuse-example


```python
!mkdir fuse_c_example 2>&1 | grep -v "File exists" || true
!mkdir fuse_c_example/CMake 2>&1 | grep -v "File exists" || true
```


```cmake
%%cmake fuse_c_example/CMake/FindFUSE.cmake
# copied from https://github.com/fntlnz/fuse-example/blob/master/CMake/FindFUSE.cmake
# Кстати, вот пример модуля CMake который умеет искать библиотеку

IF (FUSE_INCLUDE_DIR)
    SET (FUSE_FIND_QUIETLY TRUE)
ENDIF (FUSE_INCLUDE_DIR)

FIND_PATH (FUSE_INCLUDE_DIR fuse.h /usr/local/include/osxfuse /usr/local/include /usr/include)

if (APPLE)
    SET(FUSE_NAMES libosxfuse.dylib fuse)
else (APPLE)
    SET(FUSE_NAMES fuse)
endif (APPLE)
FIND_LIBRARY(FUSE_LIBRARIES NAMES ${FUSE_NAMES} PATHS /lib64 /lib /usr/lib64 /usr/lib /usr/local/lib64 /usr/local/lib /usr/lib/x86_64-linux-gnu)

include ("FindPackageHandleStandardArgs")
find_package_handle_standard_args ("FUSE" DEFAULT_MSG FUSE_INCLUDE_DIR FUSE_LIBRARIES)

mark_as_advanced (FUSE_INCLUDE_DIR FUSE_LIBRARIES)
```


```cmake
%%cmake fuse_c_example/CMakeLists.txt
# copied from https://github.com/fntlnz/fuse-example/blob/master/CMakeLists.txt

cmake_minimum_required(VERSION 3.0 FATAL_ERROR)

project(fuse_c_example)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_FILE_OFFSET_BITS=64 -DFUSE2 -g -fsanitize=address")

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake" ${CMAKE_MODULE_PATH}) # Говорим, где еще можно искать модули

find_package(FUSE REQUIRED)

include_directories(${FUSE_INCLUDE_DIR})
add_executable(fuse-example main.c)
target_link_libraries(fuse-example ${FUSE_LIBRARIES})
```

---

Чтобы пользователь мог пользоваться вашим модулем Fuse, нужно добавить основные операции для взаимодействия. Они реализуются в виде колбэков, которые Fuse будет вызывать при выполнении определённого действия пользователем.  
В C/C++ это реализуется путём заполнения структурки [fuse_operations](http://libfuse.github.io/doxygen/structfuse__operations.html).  

---


```cpp
%%cpp fuse_c_example/main.c
%run mkdir fuse_c_example/build 2>&1 | grep -v "File exists"
%run cd fuse_c_example/build && cmake .. > /dev/null && make
#include <string.h>
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#ifdef FUSE2
    #define FUSE_USE_VERSION 26
#else
    #define FUSE_USE_VERSION 30
#endif
#include <fuse.h>

typedef struct { 
    char* filename;
    char* filecontent;
    char* log;
} my_options_t;
my_options_t my_options;


void print_cwd() {
    if (my_options.log) {
        FILE* f = fopen(my_options.log, "at");
        char buffer[1000];
        getcwd(buffer, sizeof(buffer));
        fprintf(f, "Current working dir: %s\n", buffer);
        fclose(f);
    }
}

// Самый важный колбэк. Вызывается первым при любом другом колбэке. 
// Заполняет структуру stbuf.
int getattr_callback(const char* path, struct stat* stbuf
#ifndef FUSE2
    , struct fuse_file_info *fi
#endif
) {
#ifndef FUSE2
    (void) fi;
#endif   
    if (strcmp(path, "/") == 0) {
        // st_mode(тип файла, а также права доступа)
        // st_nlink(количество ссылок на файл)
        // Интересный факт, что количество ссылок у папки = 2 + n, где n -- количество подпапок.
        *stbuf = (struct stat) {.st_nlink = 2, .st_mode = S_IFDIR | 0755};
        return 0;
    }
    if (path[0] == '/' && strcmp(path + 1, my_options.filename) == 0) {
        *stbuf = (struct stat) {.st_nlink = 2, .st_mode = S_IFREG | 0777, .st_size = (__off_t)strlen(my_options.filecontent)};
        return 0;
    }
    return -ENOENT; // При ошибке, вместо errno возвращаем (-errno).
}

// filler(buf, filename, stat, flags) -- заполняет информацию о файле и вставляет её в buf.
int readdir_callback(const char* path, void* buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info* fi
#ifndef FUSE2
    , enum fuse_readdir_flags flags
#endif
) {
#ifdef FUSE2
    (void) offset; (void) fi;
    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);
    filler(buf, my_options.filename, NULL, 0);
#else
    (void) offset; (void) fi; (void)flags;
    filler(buf, ".", NULL, 0, (enum fuse_fill_dir_flags)0);
    filler(buf, "..", NULL, 0, (enum fuse_fill_dir_flags)0);
    filler(buf, my_options.filename, NULL, 0, (enum fuse_fill_dir_flags)0);
#endif   
    return 0;
}

// Вызывается после успешной обработки open.
int read_callback(const char* path, char* buf, size_t size, off_t offset, struct fuse_file_info* fi) {
    // "/"
    if (strcmp(path, "/") == 0) {
        return -EISDIR;
    }
    print_cwd();
    // "/my_file"
    if (path[0] == '/' && strcmp(path + 1, my_options.filename) == 0) {
        size_t len = strlen(my_options.filecontent);
        if (offset >= len) {
            return 0;
        }
        size = (offset + size <= len) ? size : (len - offset);
        memcpy(buf, my_options.filecontent + offset, size);
        return size;
    }
    return -EIO;
}

// Структура с колбэками. 
struct fuse_operations fuse_example_operations = {
    .getattr = getattr_callback,
    .read = read_callback,
    .readdir = readdir_callback,
};

struct fuse_opt opt_specs[] = {
    { "--file-name %s", offsetof(my_options_t, filename), 0 },
    { "--file-content %s", offsetof(my_options_t, filecontent), 0 },
    { "--log %s", offsetof(my_options_t, log), 0 },
    FUSE_OPT_END // Структурка заполненная нулями. В общем такой типичный zero-terminated массив
};

int main(int argc, char** argv) {
    struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
    
    /*
    * Если не хотите создавать структурку с данными, а нужно только распарсить одну строку,
    * То можно вторым аргументом передать char*.
    * Тогда в opt_specs это можно указать как {"--src %s", 0, 0}
    *
    * ВАЖНО: заполняемые поля должны быть инициализированы нулями. 
    * (В противном случае fuse3 может делать что-то очень плохое. TODO)
    */
    my_options.filename = "asdfrgt";
    fuse_opt_parse(&args, &my_options, opt_specs, NULL);
    print_cwd();
    
    int ret = fuse_main(args.argc, args.argv, &fuse_example_operations, NULL);
    fuse_opt_free_args(&args);
    return ret;
}
```

Запустим в синхронном режиме (программа работает, пока `fusermount -u` не будет сделан)


```python
!mkdir fuse_c 2>&1 | grep -v "File exists" || true
!fusermount -u fuse_c
!truncate --size=0 err.txt || true
a = TInteractiveLauncher("fuse_c_example/build/fuse-example fuse_c -f "
                         "--file-name my_file --file-content 'My file content\n' --log `pwd`/err.txt")
```


```bash
%%bash
exec 2>&1 ; set -o xtrace

tree fuse_c --noreport 

cat fuse_c/my_file
```


```python
!fusermount -u fuse_c
a.close()
```


```bash
%%bash
tree fuse_c --noreport
cat err.txt
```

А теперь в асинхронном (в режиме демона, в параметрах запуска нет `-f`):


```python
!mkdir fuse_c 2>&1 | grep -v "File exists" || true
!fusermount -u fuse_c
!truncate --size=0 err.txt || true
a = TInteractiveLauncher("fuse_c_example/build/fuse-example fuse_c "
                         "--file-name my_file --file-content 'My file content\n' --log `pwd`/err.txt")
```


```bash
%%bash
exec 2>&1 ; set -o xtrace

tree fuse_c --noreport 

cat fuse_c/my_file

fusermount -u fuse_c
```


```python
a.close()
```


```bash
%%bash
tree fuse_c --noreport
cat err.txt
```

Парам-пам-пам, изменилась текущая директория! Учиытвайте это в ДЗ


```python

```


```python

```

# <a name="hw"></a> Комментарии к ДЗ

* Пример входных данных в первой задаче: 

```
2
a.txt 3
b.txt 5

AaAbBbBb
```

* В ejudge fuse запускается без опции `-f` поэтому текущая директория будет меняться и относительные пути могут становиться невалидными. Рекомендую: `man 3 realpath`

1) В задачах на fuse основная цель -- реализовать 3 метода(read, readdir, getattr).  
Для этого может понадобиться сохранить свои данные в какую-то глобальную переменную и доставать их оттуда в вызовах колбэка.  

2) В 23-1 Чтобы не усложнять себе жизнь, можно ходить по папкам при каждом вызове.  
Тогда задача сводится к поиску конкретного файла в каждой папке из условия и выборе из этих файлов последнего.  
Либо, в случае readdir, можно вызвать opendir/readdir/closedir к каждому пути и сформировать словарик из уникальных файлов в папках.


```python

```
