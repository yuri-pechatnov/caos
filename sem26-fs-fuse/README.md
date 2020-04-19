


# FUSE

<table width=100%  > <tr>
    <th width=15%> <b>Видео с семинара &rarr; </b> </th>
    <th>
    <a href="https://youtu.be/???"><img src="video.jpg" width="320" 
   height="160" align="left" alt="Видео с семинара"></a>
    </th>
    <th> </th>
 </table>

Сегодня в программе:
* <a href="#fusepy" style="color:#856024"> Примонтируем json как read-only файловую систему. Python + fusepy </a>

https://ru.wikipedia.org/wiki/FUSE_(модуль_ядра)

https://habr.com/ru/post/315654/ - на питоне

https://engineering.facile.it/blog/eng/write-filesystem-fuse/




[Ридинг Яковлева](https://github.com/victor-yacovlev/mipt-diht-caos/tree/master/practice/openssl)
  
  
<a href="#hw" style="color:#856024">Комментарии к ДЗ</a>




```python

```


```python

```

## <a name="fusepy"></a> Python + fusepy

Установк: `pip install --user fusepy`


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

    Overwriting fuse_json.py



```python
!mkdir fuse_json 2>&1 | grep -v "File exists" || true
a = TInteractiveLauncher("python2 fuse_json.py example.txt fuse_json 2>&1")
```





```
L | Process started. PID = 24132
L | Process finished. Got signal 9

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

    + tree fuse_json --noreport
    fuse_json
    ├── a
    ├── c
    │   ├── c1
    │   └── __json__
    └── __json__
    + cat fuse_json/__json__ new_line
    {"a": "b", "c": {"c1": "234"}}
    + cat fuse_json/a new_line
    b
    + cat fuse_json/c/__json__ new_line
    {"c1": "234"}



```python
os.kill(a.get_pid(), signal.SIGKILL)
a.close()
```


```python
!fusermount -u fuse_json
```


```bash
%%bash
tree fuse_json --noreport
```

    fuse_json



```python

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

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_FILE_OFFSET_BITS=64")

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake" ${CMAKE_MODULE_PATH})

find_package(FUSE REQUIRED)

include_directories(${FUSE_INCLUDE_DIR})
add_executable(fuse-example main.c)
target_link_libraries(fuse-example ${FUSE_LIBRARIES})
```


```python

```


```cpp
%%cpp fuse_c_example/main.c
%run mkdir fuse_c_example/build 2>&1 | grep -v "File exists"
%run cd fuse_c_example/build && cmake .. > /dev/null && make
#include <string.h>
#include <errno.h>
#include <stddef.h>

#define FUSE_USE_VERSION 26
#include <fuse.h>

typedef struct { 
    char* filename;
    char* filecontent;
} my_options_t;

struct fuse_opt opt_specs[] = {
    { "--file-name %s", offsetof(my_options_t, filename), 0 },
    { "--file-content %s", offsetof(my_options_t, filecontent), 0 },
    { NULL, 0, 0},
};

my_options_t my_options;

int getattr_callback(const char* path, struct stat* stbuf) {
    if (strcmp(path, "/") == 0) {
        *stbuf = (struct stat) {.st_mode = S_IFDIR | 0755, .st_nlink = 2};
        return 0;
    }
    if (path[0] == '/' && strcmp(path + 1, my_options.filename) == 0) {
        *stbuf = (struct stat) {.st_mode = S_IFREG | 0777, .st_nlink = 1, .st_size = strlen(my_options.filecontent)};
        return 0;
    }
    return -ENOENT;
}

int readdir_callback(const char* path, void* buf, fuse_fill_dir_t filler, 
                     off_t offset, struct fuse_file_info* fi) {
    (void) offset; (void) fi;
    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);
    filler(buf, my_options.filename, NULL, 0);
    return 0;
}

int open_callback(const char *path, struct fuse_file_info *fi) {
    return 0;
}

int read_callback(const char* path, char* buf, size_t size, off_t offset, struct fuse_file_info* fi) {
    if (path[0] == '/' && strcmp(path + 1, my_options.filename) == 0) {
        size_t len = strlen(my_options.filecontent);
        if (offset >= len) {
            return 0;
        }
        size = (offset + size <= len) ? size : (len - offset);
        memcpy(buf, my_options.filecontent + offset, size);
        return size;
    }
    return -ENOENT;
}

struct fuse_operations fuse_example_operations = {
    .getattr = getattr_callback,
    .open = open_callback,
    .read = read_callback,
    .readdir = readdir_callback,
};

int main(int argc, char** argv) {
    struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
    fuse_opt_parse(&args, &my_options, opt_specs, NULL);
    int ret = fuse_main(args.argc, args.argv, &fuse_example_operations, NULL);
    return ret;
}
```


Run: `mkdir fuse_c_example/build 2>&1 | grep -v "File exists"`



Run: `cd fuse_c_example/build && cmake .. > /dev/null && make`


    [35m[1mScanning dependencies of target fuse-example[0m
    [ 50%] [32mBuilding C object CMakeFiles/fuse-example.dir/main.c.o[0m
    [100%] [32m[1mLinking C executable fuse-example[0m
    [100%] Built target fuse-example



```python
!mkdir fuse_c || true
a = TInteractiveLauncher("fuse_c_example/build/fuse-example fuse_c -f "
                         "--file-name my_file --file-content 'My file content\n'")
```

    mkdir: cannot create directory ‘fuse_c’: File exists






```
L | Process started. PID = 23946
L | Process finished. Exit code 0

```





```bash
%%bash
exec 2>&1 ; set -o xtrace

tree fuse_c --noreport 

cat fuse_c/my_file
```

    + tree fuse_c --noreport
    fuse_c
    └── my_file
    + cat fuse_c/my_file
    My file content



```python
!fusermount -u fuse_c
a.close()
```


```bash
%%bash
tree fuse_c --noreport
```

    fuse_c



```python

```


```python

```


```python

```


```python

```


```python

```


```python

```


```python

```


```python

```


```python

```

# <a name="hw"></a> Комментарии к ДЗ

* 
*


```python

```


```python

```
