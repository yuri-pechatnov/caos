


# HTTP, libcurl, cmake

<table width=100%  > <tr>
    <th width=15%> <b>Видео с семинара &rarr; </b> </th>
    <th>
    <a href="https://www.youtube.com/watch?v=THn5AmDlwu4&list=PLjzMm8llUm4CL-_HgDrmoSTZBCdUk5HQL&index=3"><img src="video.jpg" width="320" 
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

* <a href="#touch_http" style="color:#856024"> Более разнообразное использование HTTP </a> 

#### HTTP 1.1 и HTTP/2

На семинаре будем рассматривать HTTP 1.1, но стоит знать, что текущая версия протокола существенно более эффективна.

[Как HTTP/2 сделает веб быстрее / Хабр](https://habr.com/ru/company/nix/blog/304518/)

| HTTP 1.1 | HTTP/2 |
|----------|--------|
| одно соединение - один запрос, <br> как следствие вынужденная конкатенация, встраивание и спрайтинг (spriting) данных, | несколько запросов на соединение |
| все нужные заголовки каждый раз отправляются полностью | сжатие заголовков, позволяет не отправлять каждый раз одни и те же заголовки |
| | возможность отправки данных по инициативе сервера |
| текстовый протокол | двоичный протокол |
| | приоритезация потоков - клиент может сообщать, что ему более важно| 
 
[Ридинг Яковлева](https://github.com/victor-yacovlev/mipt-diht-caos/tree/master/practice/http-curl)

## libcurl

Библиотека умеющая все то же, что и утилита curl.


## cmake

Решает задачу кроссплатформенной сборки

* Фронтенд для систем непосредственно занимающихся сборкой
* cmake хорошо интегрирован с многими IDE 
* CMakeLists.txt в корне дерева исходников - главный конфигурационный файл и главный индикатор того, что проект собирается с помощью cmake

Примеры:
* <a href="#сmake_simple" style="color:#856024"> Простой пример </a>
* <a href="#сmake_curl" style="color:#856024"> Пример с libcurl </a>



[Введение в CMake / Хабр](https://habr.com/ru/post/155467/)


[Ридинг Яковлева](https://github.com/victor-yacovlev/mipt-diht-caos/blob/master/practice/linux_basics/cmake.md)

[Документация для libCURL](https://curl.haxx.se/libcurl/c/)  
  
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
```


```python
# Можно еще исползовать telnet: "telnet ejudge.atp-fivt.org 80"
import time
a = TInteractiveLauncher("telnet ejudge.atp-fivt.org 80 | head -n 10")
a.write("""\
GET / HTTP/1.1
Host: ejudge.atp-fivt.org

""")
time.sleep(1)
a.close()
```





```
L | Process started. PID = 8836
I | GET / HTTP/1.1
I | Host: ejudge.atp-fivt.org
I | 
O | Trying 87.251.82.74...
O | Connected to atp-fivt.org.
O | Escape character is '^]'.
O | HTTP/1.1 200 OK
O | Server: nginx/1.14.0 (Ubuntu)
O | Date: Mon, 18 May 2020 09:27:49 GMT
O | Content-Type: text/html; charset=UTF-8
O | Content-Length: 4502
O | Connection: keep-alive
O | Last-Modified: Wed, 15 May 2019 07:01:47 GMT
L | Process finished. Exit code 0

```





```bash
%%bash
VAR=$(cat <<HEREDOC_END
USER pechatnov@yandex.ru
HEREDOC_END
)

# попытка загрузить почту по POP3 протоколу (не получится, там надо с шифрованием заморочиться)
echo -e "$VAR\n" | nc -q1 pop.yandex.ru 110 
```

    +OK POP Ya! na@8-b74d0a35481b xRKan3s0qW21
    -ERR [AUTH] Working without SSL/TLS encryption is not allowed. Please visit https://yandex.ru/support/mail-new/mail-clients/ssl.html  sc=xRKan3s0qW21_180928_8-b74d0a35481b


#### <a name="curl"></a> Сразу на уровне HTTP

curl - возволяет делать произвольные HTTP запросы

wget - в первую очередь предназначен для скачивания файлов. Например, умеет выкачивать страницу рекурсивно


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
    100  4502  100  4502    0     0  48934      0 --:--:-- --:--:-- --:--:-- 48934



```bash
%%bash
wget ejudge.atp-fivt.org -O - | head -n 10
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


    --2020-05-18 12:28:56--  http://ejudge.atp-fivt.org/
    Resolving ejudge.atp-fivt.org (ejudge.atp-fivt.org)... 87.251.82.74
    Connecting to ejudge.atp-fivt.org (ejudge.atp-fivt.org)|87.251.82.74|:80... connected.
    HTTP request sent, awaiting response... 200 OK
    Length: 4502 (4,4K) [text/html]
    Saving to: ‘STDOUT’
    
         0K ....                                                  100%  120M=0s
    
    2020-05-18 12:28:57 (120 MB/s) - written to stdout [4502/4502]
    


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

`sudo apt install libcurl4-openssl-dev`

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


#### <a name="touch_http"></a> Потрогаем HTTP  более разнообразно



Установка: 
<br>https://install.advancedrestclient.com/ - программка для удобной отправки разнообразных http запросов
<br>`pip3 install --user wsgidav cheroot` - webdav сервер


```python
!mkdir webdav_dir 2>&1 | grep -v "File exists" || true
!echo "Hello!" > webdav_dir/file.txt

a = TInteractiveLauncher("wsgidav --port=9024 --root=./webdav_dir --auth=anonymous --host=0.0.0.0")

```





```
L | Process started. PID = 10565
O | Running without configuration file.
O | 2020-05-18 12:32:43.387 - <140386425976640> wsgidav.wsgidav_app         INFO    :  WsgiDAV/3.0.3 Python/3.8.2 Linux-5.4.0-29-generic-x86_64-with-glibc2.29
O | 2020-05-18 12:32:43.387 - <140386425976640> wsgidav.wsgidav_app         INFO    :  Lock manager:      LockManager(LockStorageDict)
O | 2020-05-18 12:32:43.387 - <140386425976640> wsgidav.wsgidav_app         INFO    :  Property manager:  None
O | 2020-05-18 12:32:43.387 - <140386425976640> wsgidav.wsgidav_app         INFO    :  Domain controller: SimpleDomainController()
O | 2020-05-18 12:32:43.387 - <140386425976640> wsgidav.wsgidav_app         INFO    :  Registered DAV providers by route:
O | 2020-05-18 12:32:43.387 - <140386425976640> wsgidav.wsgidav_app         INFO    :    - '/:dir_browser': FilesystemProvider for path '/home/pechatnov/.local/lib/python3.8/site-packages/wsgidav/dir_browser/htdocs' (Read-Only) (anonymous)
O | 2020-05-18 12:32:43.387 - <140386425976640> wsgidav.wsgidav_app         INFO    :    - '/': FilesystemProvider for path '/home/pechatnov/vbox/caos_2019-2020/sem24-http-libcurl-cmake/webdav_dir' (Read-Write) (anonymous)
O | 2020-05-18 12:32:43.387 - <140386425976640> wsgidav.wsgidav_app         WARNING :  Basic authentication is enabled: It is highly recommended to enable SSL.
O | 2020-05-18 12:32:43.387 - <140386425976640> wsgidav.wsgidav_app         WARNING :  Share '/' will allow anonymous write access.
O | 2020-05-18 12:32:43.387 - <140386425976640> wsgidav.wsgidav_app         WARNING :  Share '/:dir_browser' will allow anonymous read access.
O | 2020-05-18 12:32:43.387 - <140386425976640> wsgidav                     WARNING :  Could not import lxml: using xml instead (up to 10% slower). Consider `pip install lxml`(see https://pypi.python.org/pypi/lxml).
O | 2020-05-18 12:32:43.540 - <140386425976640> wsgidav                     INFO    :  Running WsgiDAV/3.0.3 Cheroot/8.3.0 Python/3.8.2
O | 2020-05-18 12:32:43.540 - <140386425976640> wsgidav                     INFO    :  Serving on http://0.0.0.0:9024 ...
O | 2020-05-18 12:32:45.044 - <140386393306880> wsgidav.wsgidav_app         INFO    :  127.0.0.1 - (anonymous) - [2020-05-18 09:32:45] "GET /" elap=0.001sec -> 200 OK
O | 2020-05-18 12:32:46.289 - <140386307667712> wsgidav.wsgidav_app         INFO    :  127.0.0.1 - (anonymous) - [2020-05-18 09:32:46] "PUT /curl_added_file.txt" length=419, elap=0.001sec -> 201 Created
O | 2020-05-18 12:32:48.854 - <140386290882304> wsgidav.wsgidav_app         INFO    :  127.0.0.1 - (anonymous) - [2020-05-18 09:32:48] "DELETE /curl_added_file.txt" depth=0, elap=0.001sec -> 204 No Content
O | 2020-05-18 12:32:50.373 - <140386425976640> wsgidav                     WARNING :  Caught Ctrl-C, shutting down...
L | Process finished. Exit code 0

```





```python
!curl localhost:9024 | head -n 4
```

      % Total    % Received % Xferd  Average Speed   Time    Time     Time  Current
                                     Dload  Upload   Total   Spent    Left  Speed
    100  1580  100  1580    0     0   771k      0 --:--:-- --:--:-- --:--:--  771k
    <!DOCTYPE html>
    <html>
    <head>
      <meta http-equiv="Content-Type" content="text/html; charset=UTF-8">



```python
!curl -X "PUT" localhost:9024/curl_added_file.txt --data-binary @curl_easy.c
```

    <!DOCTYPE HTML PUBLIC '-//W3C//DTD HTML 4.01//EN' 'http://www.w3.org/TR/html4/strict.dtd'>
    <html><head>
      <meta http-equiv='Content-Type' content='text/html; charset=UTF-8'>
      <title>201 Created</title>
    </head><body>
      <h1>201 Created</h1>
      <p>201 Created</p>
    <hr/>
    <a href='https://github.com/mar10/wsgidav/'>WsgiDAV/3.0.3</a> - 2020-05-18 12:32:46.289066
    </body></html>


```python
!ls webdav_dir
!cat webdav_dir/curl_added_file.txt | grep main -C 2
```

    curl_added_file.txt  file.txt
    #include <assert.h>
    
    int main() {
        CURL *curl = curl_easy_init();
        assert(curl);



```python
!curl -X "DELETE" localhost:9024/curl_added_file.txt 
```


```python
!ls webdav_dir
```

    file.txt



```python
os.kill(a.get_pid(), signal.SIGINT)
a.close()
```


```python

```

## libcurl

Установка: `sudo apt-get install libcurl4-openssl-dev` (Но это не точно! Воспоминания годичной давности. Напишите мне пожалуйста получится или не получится)

Документация: https://curl.haxx.se/libcurl/c/CURLOPT_WRITEFUNCTION.html  
Интересный факт: размер chunk'a всегда равен 1.

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
    size_t chunk_size, // размер фрагмента данных; всегда равен 1 
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

## cmake
Установка: `apt-get install cmake cmake-extras`

#### <a name="cmake_simple"></a> Простой пример

Источник: [Введение в CMake / Хабр](https://habr.com/ru/post/155467/). Там же можно найти множество более интересных примеров.


```python
!mkdir simple_cmake_example 2>&1 | grep -v "File exists" || true
```


```cmake
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


    CMake Warning (dev) in CMakeLists.txt:
      No project() command is present.  The top-level CMakeLists.txt file must
      contain a literal, direct call to the project() command.  Add a line of
      code such as
    
        project(ProjectName)
    
      near the top of the file, but after cmake_minimum_required().
    
      CMake is pretending there is a "project(Project)" command on the first
      line.
    This warning is for project developers.  Use -Wno-dev to suppress it.
    
    -- The C compiler identification is GNU 9.3.0
    -- The CXX compiler identification is GNU 9.3.0
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
    drwxrwxr-x 3 pechatnov pechatnov 4096 мая 18 12:33 .
    drwxrwxr-x 6 pechatnov pechatnov 4096 мая 18 12:32 ..
    drwxrwxr-x 3 pechatnov pechatnov 4096 мая 18 12:33 build
    -rw-rw-r-- 1 pechatnov pechatnov  523 мая 18 12:33 CMakeLists.txt
    -rw-rw-r-- 1 pechatnov pechatnov  984 мая 18 12:33 main.cpp



Run: `ls -la simple_cmake_example/build #// ... и в директории сборки`


    total 60
    drwxrwxr-x 3 pechatnov pechatnov  4096 мая 18 12:33 .
    drwxrwxr-x 3 pechatnov pechatnov  4096 мая 18 12:33 ..
    -rw-rw-r-- 1 pechatnov pechatnov 13984 мая 18 12:33 CMakeCache.txt
    drwxrwxr-x 5 pechatnov pechatnov  4096 мая 18 12:33 CMakeFiles
    -rw-rw-r-- 1 pechatnov pechatnov  1614 мая 18 12:33 cmake_install.cmake
    -rwxrwxr-x 1 pechatnov pechatnov 17312 мая 18 12:33 main
    -rw-rw-r-- 1 pechatnov pechatnov  4987 мая 18 12:33 Makefile



Run: `rm -r simple_cmake_example/build #// удаляем директорию с файлами сборки`


#### <a name="cmake_curl"></a> Пример с libcurl


```python
!mkdir curl_cmake_example || true
!cp curl_medium.c curl_cmake_example/main.c
```


```cmake
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
    message(">>>>> Failed to find FUSE (not a problem)")
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


    CMake Warning (dev) in CMakeLists.txt:
      No project() command is present.  The top-level CMakeLists.txt file must
      contain a literal, direct call to the project() command.  Add a line of
      code such as
    
        project(ProjectName)
    
      near the top of the file, but after cmake_minimum_required().
    
      CMake is pretending there is a "project(Project)" command on the first
      line.
    This warning is for project developers.  Use -Wno-dev to suppress it.
    
    -- The C compiler identification is GNU 9.3.0
    -- The CXX compiler identification is GNU 9.3.0
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
    -- Found CURL: /usr/lib/x86_64-linux-gnu/libcurl.so (found version "7.68.0")  
    -- Looking for pthread.h
    -- Looking for pthread.h - found
    -- Performing Test CMAKE_HAVE_LIBC_PTHREAD
    -- Performing Test CMAKE_HAVE_LIBC_PTHREAD - Failed
    -- Looking for pthread_create in pthreads
    -- Looking for pthread_create in pthreads - not found
    -- Looking for pthread_create in pthread
    -- Looking for pthread_create in pthread - found
    -- Found Threads: TRUE  
    -- Could NOT find SDL (missing: SDL_LIBRARY SDL_INCLUDE_DIR) 
    >>>>> Failed to find SDL (not a problem)
    -- Found PkgConfig: /usr/bin/pkg-config (found version "0.29.1") 
    -- Checking for module 'fuse3'
    --   No package 'fuse3' found
    >>>>> Failed to find FUSE (not a problem)
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

* `Connection: close`
* Комментарий от [Михаила Циона](https://github.com/MVCionOld):
<br> От себя хочу добавить про использование `сURL`'a. Одним из хэдеров в  `http`-запросе есть `User-Agent`, которые сигнализирует сайту про, что "вы" это то браузер, поисковый бот/скраппер, мобильный телефоны или холодильник. Некоторые сайты нормально открываются в браузере, но при попытке получить исходный `HTML` код с помощью `cURL` эти запросы могут отклоняться. Могут возвращаться коды ответов, например, 403, то есть доступ запрещён.
<br> Зачастую боты не несут никакой пользы, но в то же время создают нагрузку на сервис и/или ведут другую вредоносную активность. Насколько мне известно, есть два способа бороться с такими негодяями: проверять `User-Agent` и использование `JavaScript`. Во втором случае это инъекции на куки, асинхронная генерация страницы и тд. Что касается агента - банально денаить конкретные паттерны. У `сURL`'a есть своя строка для агента, в основном меняется только версия, например `curl/7.37.0`.
<br> Возможно, кто-то сталкивался с тем, что при написании скраппера основанного на `сURL`'e вы получали `BadRequest` (например, при тестировании задачи **inf21-2**), хотя сайт прекрасно открывался. Это как раз первый случай.
<br> Однако, можно менять агента, например, из терминала: 
<br> `curl -H "User-Agent: Mozilla/5.0" url`
<br> при использовании `libcurl`:
<br> `curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0");`


```python

```


```python

```
