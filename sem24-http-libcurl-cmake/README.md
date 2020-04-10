


# HTTP, libcurl, cmake

<table width=100%  > <tr>
    <th width=15%> <b>Видео с семинара &rarr; </b> </th>
    <th>
    <a href="https://www.youtube.com/???"><img src="video.png" width="320" 
   height="160" align="left" alt="Видео с семинара"></a>
    </th>
    <th> </th>
 </table>


## HTTP

[HTTP (HyperText Transfer Protocol)](https://ru.wikipedia.org/wiki/HTTP) — протокол прикладного/транспортного уровня передачи данных. 
Изначально был создан как протокол прикладного уровня для передачи документов в html формате (теги и все вот это). Но позже был распробован и сейчас может используется для передачи произвольных данных, что характерно для транспортного уровня.

Отправка HTTP запроса:
* <a href="#get_term" style="color:#856024"> Из терминала </a>
  * <a href="#netcat" style="color:#856024"> С помощью netcat, telnet </a> на уровне TCP, самостоятельно формируя HTTP запрос.
  * <a href="#curl" style="color:#856024"> С помощью curl </a> на уровне HTTP
* <a href="#get_python" style="color:#856024"> Из python </a> на уровне HTTP
* <a href="#get_c" style="color:#856024"> Из программы на C </a> на уровне HTTP

HTTP 1.1 и HTTP 2
 
[Ридинг Яковлева](https://github.com/victor-yacovlev/mipt-diht-caos/tree/master/practice/http-curl)

## libcurl

Библиотека умеющая все то же, что и утилита curl.

[Ридинг Яковлева](https://github.com/victor-yacovlev/mipt-diht-caos/tree/master/practice/http-curl)

## cmake

* Фронтенд для систем непосредственно занимающихся сборкой
* cmake хорошо интегрирован с многими IDE 
* CMakeLists.txt в корне дерева исходников - главный конфигурационный файл и главный индикатор того, что проект собирается с помощью cmake

[Введение в CMake / Хабр](https://habr.com/ru/post/155467/)


[Ридинг Яковлева](https://github.com/victor-yacovlev/mipt-diht-caos/blob/master/practice/linux_basics/cmake.md)

  
  
<a href="#hw" style="color:#856024">Комментарии к ДЗ</a>



## <a name="get_term"></a> HTTP из терминала

#### <a name="netcat"></a> На уровне TCP


```bash
%%bash
# make request string
VAR=$(cat <<HEREDOC_END
GET / HTTP/1.1
Host: ejudge.atp-fivt.org
HEREDOC_END
)

# Если работаем в терминале, то просто пишем "nc ejudge.atp-fivt.org 80" и вводим запрос
# ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓ - имитация ввода в stdin. "-q1" - чтобы netcat не закрылся сразу после закрытия stdin 
echo -e "$VAR\n" | nc -q1 ejudge.atp-fivt.org 80 | head -n 14
#                                                ↑↑↑↑↑↑↑↑↑↑↑↑ - обрезаем только начало вывода, чтобы не затопило выводом

# Можно еще исползовать telnet: "telnet ejudge.atp-fivt.org 80"
```

    HTTP/1.1 200 OK
    Server: nginx/1.14.0 (Ubuntu)
    Date: Wed, 08 Apr 2020 21:29:29 GMT
    Content-Type: text/html; charset=UTF-8
    Content-Length: 4502
    Connection: keep-alive
    Last-Modified: Wed, 15 May 2019 07:01:47 GMT
    ETag: "1196-588e7b90e0fc5"
    Accept-Ranges: bytes
    
    <html>
      <head>
        <meta charset="utf-8"/>
        <title>АКОС ФИВТ МФТИ</title>


#### <a name="curl"></a> Сразу на уровне HTTP


```bash
%%bash
curl ejudge.atp-fivt.org | head -n 10
```

    <html>
      <head>
        <meta charset="utf-8"/>
        <title>АКОС ФИВТ МФТИ</title>
      </head>
      <body>
        <h1>Ejudge для АКОС на ФИВТ МФТИ</h1>
        <h2>Весенний семестр</h2>
        <h3>Группы ПМФ</h3>
        <p><b>!!!!!!!!!!</b> <a href="/client?contest_id=19">Контрольная 15 мая 2019</a><b>!!!!!!!!!</b></p>


      % Total    % Received % Xferd  Average Speed   Time    Time     Time  Current
                                     Dload  Upload   Total   Spent    Left  Speed
    100  4502  100  4502    0     0  12158      0 --:--:-- --:--:-- --:--:-- 12167
    (23) Failed writing body


## <a name="get_python"></a> HTTP из python


```python
import requests
data = requests.get("http://ejudge.atp-fivt.org").content.decode()
print(data[:200])
```

    <html>
      <head>
        <meta charset="utf-8"/>
        <title>АКОС ФИВТ МФТИ</title>
      </head>
      <body>
        <h1>Ejudge для АКОС на ФИВТ МФТИ</h1>
        <h2>Весенний семестр</h2>
        <h3>Группы ПМФ</h3>
        <p>


## <a name="get_c"></a> HTTP из C

Пример от Яковлева


```cpp
%%cpp curl_easy.c
%run gcc -Wall curl_easy.c -lcurl -o curl_easy.exe
%run ./curl_easy.exe | head -n 5

#include <curl/curl.h>
#include <assert.h>

int main() {
    CURL *curl = curl_easy_init();
    assert(curl);
    
    CURLcode res;
    curl_easy_setopt(curl, CURLOPT_URL, "http://ejudge.atp-fivt.org");
    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    assert(res == 0);
    return 0;
}
```


Run: `gcc -Wall curl_easy.c -lcurl -o curl_easy.exe`



Run: `./curl_easy.exe | head -n 5`


    <html>
      <head>
        <meta charset="utf-8"/>
        <title>АКОС ФИВТ МФТИ</title>
      </head>


## libcurl

Установка: `sudo apt-get install libcurl4-openssl-dev` (Но это не точно! Воспоминания годичной давности. Напишите мне пожалуйста получится или не получится)

Модифицирпованный пример от Яковлева


```cpp
%%cpp curl_medium.c
%run gcc -Wall curl_medium.c -lcurl -o curl_medium.exe
%run ./curl_medium.exe "http://ejudge.atp-fivt.org" | head -n 5

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <curl/curl.h>

typedef struct {
    char *data;
    size_t length;
    size_t capacity;
} buffer_t;

static size_t callback_function(
    char *ptr, // буфер с прочитанными данными
    size_t chunk_size, // размер фрагмента данных
    size_t nmemb, // количество фрагментов данных
    void *user_data // произвольные данные пользователя
) {
    buffer_t *buffer = user_data;
    size_t total_size = chunk_size * nmemb;
    size_t required_capacity = buffer->length + total_size;
    if (required_capacity > buffer->capacity) {
        required_capacity *= 2;
        buffer->data = realloc(buffer->data, required_capacity);
        assert(buffer->data);
        buffer->capacity = required_capacity;
    }
    memcpy(buffer->data + buffer->length, ptr, total_size);
    buffer->length += total_size;
    return total_size;
}            

int main(int argc, char *argv[]) {
    assert(argc == 2);
    const char* url = argv[1];
    CURL *curl = curl_easy_init();
    assert(curl);
    
    CURLcode res;

    // регистрация callback-функции записи
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callback_function);

    // указатель &buffer будет передан в callback-функцию
    // параметром void *user_data
    buffer_t buffer = {.data = NULL, .length = 0, .capacity = 0};
    
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);

    curl_easy_setopt(curl, CURLOPT_URL, url);
    res = curl_easy_perform(curl);
    assert(res == 0);
    
    write(STDOUT_FILENO, buffer.data, buffer.length);
    
    free(buffer.data);
    curl_easy_cleanup(curl);
}
```


Run: `gcc -Wall curl_medium.c -lcurl -o curl_medium.exe`



Run: `./curl_medium.exe "http://ejudge.atp-fivt.org" | head -n 5`


    <html>
      <head>
        <meta charset="utf-8"/>
        <title>АКОС ФИВТ МФТИ</title>
      </head>



```python


```


```python

```

## cmake

#### Простой пример

Источник: [Введение в CMake / Хабр](https://habr.com/ru/post/155467/). Там же можно найти множество более интересных примеров.


```python
!mkdir simple_cmake_example || true
```

    mkdir: cannot create directory ‘simple_cmake_example’: File exists
    mkdir: cannot create directory ‘simple_cmake_example/build’: File exists



```python
%%cmake simple_cmake_example/CMakeLists.txt
cmake_minimum_required(VERSION 2.8) # Проверка версии CMake.
                                    # Если версия установленой программы
                                    # старее указаной, произайдёт аварийный выход.

add_executable(main main.cpp)       # Создает исполняемый файл с именем main
                                    # из исходника main.cpp
```


```cpp
%%cpp simple_cmake_example/main.cpp
%run mkdir simple_cmake_example/build #// cоздаем директорию для файлов сборки
%# // переходим в нее, вызываем cmake, чтобы он создал правильный Makefile
%# // а затем make, который по Makefile правильно все соберет
%run cd simple_cmake_example/build && cmake .. && make  
%run simple_cmake_example/build/main #// запускаем собранный бинарь
%run ls -la simple_cmake_example #// смотрим, а что же теперь есть в основной директории 
%run ls -la simple_cmake_example/build #// ... и в директории сборки
%run rm -r simple_cmake_example/build #// удаляем директорию с файлами сборки

#include <iostream>
int main(int argc, char** argv)
{
    std::cout << "Hello, World!" << std::endl;
    return 0;
}
```


Run: `mkdir simple_cmake_example/build #// cоздаем директорию для файлов сборки`



Run: `cd simple_cmake_example/build && cmake .. && make`


    -- The C compiler identification is GNU 5.5.0
    -- The CXX compiler identification is GNU 5.5.0
    -- Check for working C compiler: /usr/bin/cc
    -- Check for working C compiler: /usr/bin/cc -- works
    -- Detecting C compiler ABI info
    -- Detecting C compiler ABI info - done
    -- Detecting C compile features
    -- Detecting C compile features - done
    -- Check for working CXX compiler: /usr/bin/c++
    -- Check for working CXX compiler: /usr/bin/c++ -- works
    -- Detecting CXX compiler ABI info
    -- Detecting CXX compiler ABI info - done
    -- Detecting CXX compile features
    -- Detecting CXX compile features - done
    -- Configuring done
    -- Generating done
    -- Build files have been written to: /home/pechatnov/vbox/caos_2019-2020/sem24-http-libcurl-cmake/simple_cmake_example/build
    [35m[1mScanning dependencies of target main[0m
    [ 50%] [32mBuilding CXX object CMakeFiles/main.dir/main.cpp.o[0m
    [100%] [32m[1mLinking CXX executable main[0m
    [100%] Built target main



Run: `simple_cmake_example/build/main #// запускаем собранный бинарь`


    Hello, World!



Run: `ls -la simple_cmake_example #// смотрим, а что же теперь есть в основной директории`


    total 20
    drwxrwxr-x 3 pechatnov pechatnov 4096 апр 10 13:16 .
    drwxrwxr-x 5 pechatnov pechatnov 4096 апр 10 13:14 ..
    drwxrwxr-x 3 pechatnov pechatnov 4096 апр 10 13:16 build
    -rw-rw-r-- 1 pechatnov pechatnov  523 апр 10 13:09 CMakeLists.txt
    -rw-rw-r-- 1 pechatnov pechatnov  981 апр 10 13:16 main.cpp



Run: `ls -la simple_cmake_example/build #// ... и директории сборки`


    total 48
    drwxrwxr-x 3 pechatnov pechatnov  4096 апр 10 13:16 .
    drwxrwxr-x 3 pechatnov pechatnov  4096 апр 10 13:16 ..
    -rw-rw-r-- 1 pechatnov pechatnov 11809 апр 10 13:16 CMakeCache.txt
    drwxrwxr-x 5 pechatnov pechatnov  4096 апр 10 13:16 CMakeFiles
    -rw-rw-r-- 1 pechatnov pechatnov  1479 апр 10 13:16 cmake_install.cmake
    -rwxrwxr-x 1 pechatnov pechatnov  9216 апр 10 13:16 main
    -rw-rw-r-- 1 pechatnov pechatnov  4986 апр 10 13:16 Makefile



Run: `rm -r simple_cmake_example/build #// удаляем директорию с файлами сборки`


#### Пример с libcurl


```python
!mkdir curl_cmake_example || true
!cp curl_medium.c curl_cmake_example/main.c
```


```python
%%cmake curl_cmake_example/CMakeLists.txt
%run mkdir curl_cmake_example/build 
%run cd curl_cmake_example/build && cmake .. && make  
%run curl_cmake_example/build/main "http://ejudge.atp-fivt.org" | head -n 5 #// запускаем собранный бинарь
%run rm -r curl_cmake_example/build


cmake_minimum_required(VERSION 2.8) 

set(CMAKE_C_FLAGS "-std=gnu11") # дополнительные опции компилятора Си

# найти библиотеку CURL; опция REQUIRED означает,
# что библиотека является обязательной для сборки проекта,
# и если необходимые файлы не будут найдены, cmake
# завершит работу с ошибкой
find_package(CURL REQUIRED)

# это библиотека в проекте не нужна, просто пример, как написать обработку случаев, когда библиотека не найдена
find_package(SDL)
if(NOT SDL_FOUND)
    message(">>>>> Failed to find SDL (not a problem)")
else()
    message(">>>>> Managed to find SDL, can add include directories, add target libraries")
endif()

# это библиотека в проекте не нужна, просто пример, как подключить модуль интеграции с pkg-config
find_package(PkgConfig REQUIRED)
# и ненужный в этом проекте FUSE через pkg-config
pkg_check_modules(
  FUSE         # имя префикса для названий выходных переменных
  # REQUIRED # опционально можно писать, чтобы было required
  fuse3        # имя библиотеки, должен существовать файл fuse3.pc
)
if(NOT FUSE_FOUND)
    message(">>>>> Failed to find SDL (not a problem)")
else()
    message(">>>>> Managed to find FUSE, can add include directories, add target libraries")
endif()

# добавляем цель собрать исполняемый файл из перечисленных исходнико
add_executable(main main.c)
            
# добавляет в список каталогов для цели main, 
# которые превратятся в опции -I компилятора для всех 
# каталогов, которые перечислены в переменной CURL_INCLUDE_DIRECTORIES
target_include_directories(main PUBLIC ${CURL_INCLUDE_DIRECTORIES}) 
# include_directories(${CURL_INCLUDE_DIRECTORIES}) # можно вот так

# для цели my_cool_program указываем библиотеки, с которыми
# программа будет слинкована (в результате станет опциями -l и -L)
target_link_libraries(main ${CURL_LIBRARIES})
            
```


Run: `mkdir curl_cmake_example/build`



Run: `cd curl_cmake_example/build && cmake .. && make`


    -- The C compiler identification is GNU 5.5.0
    -- The CXX compiler identification is GNU 5.5.0
    -- Check for working C compiler: /usr/bin/cc
    -- Check for working C compiler: /usr/bin/cc -- works
    -- Detecting C compiler ABI info
    -- Detecting C compiler ABI info - done
    -- Detecting C compile features
    -- Detecting C compile features - done
    -- Check for working CXX compiler: /usr/bin/c++
    -- Check for working CXX compiler: /usr/bin/c++ -- works
    -- Detecting CXX compiler ABI info
    -- Detecting CXX compiler ABI info - done
    -- Detecting CXX compile features
    -- Detecting CXX compile features - done
    -- Found CURL: /usr/lib/x86_64-linux-gnu/libcurl.so (found version "7.47.0") 
    -- Looking for pthread.h
    -- Looking for pthread.h - found
    -- Looking for pthread_create
    -- Looking for pthread_create - not found
    -- Looking for pthread_create in pthreads
    -- Looking for pthread_create in pthreads - not found
    -- Looking for pthread_create in pthread
    -- Looking for pthread_create in pthread - found
    -- Found Threads: TRUE  
    -- Could NOT find SDL (missing:  SDL_LIBRARY SDL_INCLUDE_DIR) 
    >>>>> Failed to find SDL
    -- Found PkgConfig: /usr/bin/pkg-config (found version "0.29.1") 
    -- Checking for module 'fuse3'
    --   No package 'fuse3' found
    >>>>> Failed to find FUSE
    -- Configuring done
    -- Generating done
    -- Build files have been written to: /home/pechatnov/vbox/caos_2019-2020/sem24-http-libcurl-cmake/curl_cmake_example/build
    [35m[1mScanning dependencies of target main[0m
    [ 50%] [32mBuilding C object CMakeFiles/main.dir/main.c.o[0m
    [100%] [32m[1mLinking C executable main[0m
    [100%] Built target main



Run: `curl_cmake_example/build/main "http://ejudge.atp-fivt.org" | head -n 5 #// запускаем собранный бинарь`


    <html>
      <head>
        <meta charset="utf-8"/>
        <title>АКОС ФИВТ МФТИ</title>
      </head>



Run: `rm -r curl_cmake_example/build`



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
