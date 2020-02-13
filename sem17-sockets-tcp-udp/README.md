```python
# look at tools/set_up_magics.ipynb
get_ipython().run_cell('# one_liner_str <too much code> \n')
None
```


    <IPython.core.display.Javascript object>


# Сокеты и tcp-сокеты в частности

<br>
<div style="text-align: right"> Спасибо <a href="https://github.com/SyrnikRebirth">Сове Глебу</a> и <a href="https://github.com/Disadvantaged">Голяр Димитрису</a> за участие в написании текста </div>
<br>

**Модель OSI**

 [Подробнее про уровни](https://zvondozvon.ru/tehnologii/model-osi)
    
1. Физический уровень (PHYSICAL)
2. Канальный уровень (DATA LINK) <br>
  Отвечает за передачу фреймов информации. Для этого каждому к каждому блоку добавляется метаинформация и чексумма. <br>
  Справляется с двумя важными проблемами: <br>
    1. Передача фрейма данных
    2. Коллизии данных. <br>
      Это можно сделать двумя способами: повторно передавать данные либо передавать данные через Ethernet кабели, добавив коммутаторы в качестве посредника (наличие всего двух субъектов на каждом канале упрощает совместное использование среды).
3. Сетевой уровень (NETWORK) <br>
  Появляются IP-адреса. Выполняет выбор маршрута передачи данных, учитывая длительность пути, нагруженность сети, etc. <br>
  Один IP может соответствовать нескольким устройствам. Для этого используется хак на уровне маршрутизатора(NAT)
  Одному устройству может соответствовать несколько IP. Это без хаков.
4. Транспортный уровень (TRANSPORT) <- сокеты это интерфейсы вот этого уровня <br>
  Реализуются часто в ядре операционной системы <br>
  Еще стоит понимать, что транспортный уровень, предоставляя один интерфейс может иметь разные реализации. Например сокеты UNIX, в этом случае под транспортным уровнем нет сетевого, так как передача данных ведется внутри одной машины.  
  Появляется понятие порта -- порт идентифицирует программу-получателя на хосте. <br>
  Протоколы передачи данных:
  1. TCP -- устанавливает соединение, похожее на пайп. Надежный, переотправляет данные, но медленный. Регулирует скорость отправки данных в зависимости от нагрузки сети, чтобы не дропнуть интернет.
  2. UDP -- Быстрый, ненадёжный. Отправляет данные сразу все.
  
  Важный момент. Сетевой уровень - это про пересылку сообщений между конкретными хостами. А транспортный - между конкретными программами на конкретных хостах. 
5. Сеансовый уровень (SESSION) (IMHO не нужен)
6. Уровень представления данных (PRESENTATION) (IMHO не нужен)
7. Прикладной уровень (APPLICATION)

Сегодня в программе:
* `socketpair` - <a href="#socketpair" style="color:#856024">аналог `pipe`</a>, но полученные дескрипторы обладают сокетными свойствами: файловый дескриптор работает и на чтение и на запись (соответственно этот "pipe" двусторонний), закрывать нужно с вызовом `shutdown`
* `socket` - функция создания сокета
  * TCP
      * <a href="#socket_unix" style="color:#856024">AF_UNIX</a> - сокет внутри системы. Адрес в данном случае - адрес файла сокета в файловой системе.
      * <a href="#socket_inet" style="color:#856024">AF_INET</a> - сокет для стандартных ipv4 соединений. **И это самый важный пример в этом ноутбуке**.
      * <a href="#socket_inet6" style="color:#856024">AF_INET6</a> - сокет для стандартных ipv6 соединений.
  * UDP
      * <a href="#socket_udp" style="color:#856024">AF_INET</a> - посылаем датаграммы по ipv4.
  
  
[Сайт с хорошими картинками про порядок низкоуровневых вызовов в клиентском и серверном приложении](http://support.fastwel.ru/AppNotes/AN/AN-0001.html#server_tcp_init)



<a href="#hw" style="color:#856024">Комментарии к ДЗ</a>

[Ридинг Яковлева](https://github.com/victor-yacovlev/mipt-diht-caos/tree/master/practice/sockets-tcp)


```python

```


```python

```

# <a name="socketpair"></a> socketpair в качестве pipe

Socket в качестве pipe (т.е. внутри системы) используется для написания примерно одинакового кода (для локальных соединений и соединений через интернет) и для использования возможностей сокета.

[close vs shutdown](https://stackoverflow.com/questions/48208236/tcp-close-vs-shutdown-in-linux-os)



```cpp
%%cpp socketpair.cpp
%run gcc socketpair.cpp -o socketpair.exe
%run ./socketpair.exe

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <errno.h>
#include <time.h>

char* extract_t(char* s) { s[19] = '\0'; return s + 10; }
#define log_printf_impl(fmt, ...) { time_t t = time(0); dprintf(2, "%s : " fmt "%s", extract_t(ctime(&t)), __VA_ARGS__); }
#define log_printf(...) log_printf_impl(__VA_ARGS__, "")

void write_smth(int fd) {
    for (int i = 0; i < 1000; ++i) {
        write(fd, "X", 1);
        struct timespec t = {.tv_sec = 0, .tv_nsec = 10000};
        nanosleep(&t, &t);  
    }
}

void read_all(int fd) {
    int bytes = 0;
    while (true) {
        char c;
        int r = read(fd, &c, 1);
        if (r > 0) {
            bytes += r;
        } else if (r < 0) {
            assert(errno == EAGAIN);
        } else {
            break;
        }
    }
    log_printf("Read %d bytes\n", bytes);
}

int main() {
    union {
        int arr_fd[2]; 
        struct {
            int fd_1; // ==arr_fd[0] can change order, it will work
            int fd_2; // ==arr_fd[1]
        };
    } fds;
    assert(socketpair(AF_UNIX, SOCK_STREAM, 0, fds.arr_fd) == 0); //socketpair создает пару соединенных сокетов(по сути pipe)
    
    pid_t pid_1, pid_2;
    if ((pid_1 = fork()) == 0) {
        close(fds.fd_2);
        write_smth(fds.fd_1);
        shutdown(fds.fd_1, SHUT_RDWR); // important, try to comment out and look at time. Если мы не закроем соединение, то мы будем сидеть и ждать информации, даже когда ее уже нет
        close(fds.fd_1);
        log_printf("Writing is done\n");
        sleep(3);
        return 0;
    }
    if ((pid_2 = fork()) == 0) {
        close(fds.fd_1);
        read_all(fds.fd_2);
        shutdown(fds.fd_2, SHUT_RDWR);
        close(fds.fd_2);
        return 0;
    }
    close(fds.fd_1);
    close(fds.fd_2);
    int status;
    assert(waitpid(pid_1, &status, 0) != -1);
    assert(waitpid(pid_2, &status, 0) != -1);
    return 0;
}
```


Run: `gcc socketpair.cpp -o socketpair.exe`



Run: `./socketpair.exe`


     23:24:59 : Read 1000 bytes
     23:24:59 : Writing is done



```python

```

# <a name="socket_unix"></a> socket + AF_UNIX + TCP


```cpp
%%cpp socket_unix.cpp
%run gcc socket_unix.cpp -o socket_unix.exe
%run ./socket_unix.exe

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <errno.h>
#include <time.h>
#include <sys/un.h>

char* extract_t(char* s) { s[19] = '\0'; return s + 10; }
#define log_printf_impl(fmt, ...) { time_t t = time(0); dprintf(2, "%s : " fmt "%s", extract_t(ctime(&t)), __VA_ARGS__); }
#define log_printf(...) log_printf_impl(__VA_ARGS__, "")

#define conditional_handle_error(stmt, msg) \
    do { if (stmt) { perror(msg " (" #stmt ")"); exit(EXIT_FAILURE); } } while (0)

void write_smth(int fd) {
    for (int i = 0; i < 1000; ++i) {
        write(fd, "X", 1);
        struct timespec t = {.tv_sec = 0, .tv_nsec = 10000};
        nanosleep(&t, &t);  
    }
}

void read_all(int fd) {
    int bytes = 0;
    while (true) {
        char c;
        int r = read(fd, &c, 1);
        if (r > 0) {
            bytes += r;
        } else if (r < 0) {
            assert(errno == EAGAIN);
        } else {
            break;
        }
    }
    log_printf("Read %d bytes\n", bytes);
}

// important to use "/tmp/*", otherwise you can have problems with permissions
const char* SOCKET_PATH = "/tmp/my_precious_unix_socket";
const int LISTEN_BACKLOG = 2;

int main() { 
    pid_t pid_1, pid_2;
    if ((pid_1 = fork()) == 0) {
        // client
        sleep(1);
        int socket_fd = socket(AF_UNIX, SOCK_STREAM, 0); // == connection_fd in this case
        conditional_handle_error(socket_fd == -1, "can't initialize socket");
        
        // Тип переменной адреса (sockaddr_un) отличается от того что будет в следующем примере (т.е. тип зависит от того какое соединение используется)
        struct sockaddr_un addr = {.sun_family = AF_UNIX}; 
        strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);
        // Кастуем sockaddr_un* -> sockaddr*. Знакомьтесь, сишные абстрактные структуры.
        int connect_ret = connect(socket_fd, (const struct sockaddr*)&addr, sizeof(addr.sun_path));
        conditional_handle_error(connect_ret == -1, "can't connect to unix socket");
        
        write_smth(socket_fd);
        shutdown(socket_fd, SHUT_RDWR); 
        close(socket_fd);
        log_printf("client finished\n");
        return 0;
    }
    if ((pid_2 = fork()) == 0) {
        // server
        int socket_fd = socket(AF_UNIX, SOCK_STREAM, 0); 
        conditional_handle_error(socket_fd == -1, "can't initialize socket");
        
        unlink(SOCKET_PATH); // remove socket if exists, because bind fail if it exists
        struct sockaddr_un addr = {.sun_family = AF_UNIX};
        strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);
        int bind_ret = bind(socket_fd, (struct sockaddr*)&addr, sizeof(addr.sun_path)); 
        conditional_handle_error(bind_ret == -1, "can't bind to unix socket");
        
        int listen_ret = listen(socket_fd, LISTEN_BACKLOG);
        conditional_handle_error(listen_ret == -1, "can't listen to unix socket");

        struct sockaddr_un peer_addr = {0};
        socklen_t peer_addr_size = sizeof(struct sockaddr_un);
        int connection_fd = accept(socket_fd, (struct sockaddr*)&peer_addr, &peer_addr_size); // После accept можно делать fork и обрабатывать соединение в отдельном процессе
        conditional_handle_error(connection_fd == -1, "can't accept incoming connection");
                
        read_all(connection_fd);
        
        shutdown(connection_fd, SHUT_RDWR); 
        close(connection_fd);
        shutdown(socket_fd, SHUT_RDWR); 
        close(socket_fd);
        unlink(SOCKET_PATH);
        log_printf("server finished\n");
        return 0;
    }
    int status;
    assert(waitpid(pid_1, &status, 0) != -1);
    assert(waitpid(pid_2, &status, 0) != -1);
    return 0;
}
```


Run: `gcc socket_unix.cpp -o socket_unix.exe`



Run: `./socket_unix.exe`


     23:25:05 : Read 1000 bytes
     23:25:05 : server finished
     23:25:05 : client finished



```python

```

# <a name="socket_inet"></a> socket + AF_INET + TCP

[На первый взгляд приличная статейка про программирование на сокетах в linux](https://www.rsdn.org/article/unix/sockets.xml)


```cpp
%%cpp socket_inet.cpp
%run gcc -DDEBUG socket_inet.cpp -o socket_inet.exe
%run ./socket_inet.exe
%run diff socket_unix.cpp socket_inet.cpp  | grep -v "// %" | grep -e '>' -e '<' -C 1

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <errno.h>
#include <time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>

char* extract_t(char* s) { s[19] = '\0'; return s + 10; }
#define log_printf_impl(fmt, ...) { time_t t = time(0); dprintf(2, "%s : " fmt "%s", extract_t(ctime(&t)), __VA_ARGS__); }
#define log_printf(...) log_printf_impl(__VA_ARGS__, "")

#define conditional_handle_error(stmt, msg) \
    do { if (stmt) { perror(msg " (" #stmt ")"); exit(EXIT_FAILURE); } } while (0)

void write_smth(int fd) {
    for (int i = 0; i < 1000; ++i) {
        int write_ret = write(fd, "X", 1);
        conditional_handle_error(write_ret != 1, "writing failed");
        struct timespec t = {.tv_sec = 0, .tv_nsec = 10000};
        nanosleep(&t, &t);  
    }
}

void read_all(int fd) {
    int bytes = 0;
    while (true) {
        char c;
        int r = read(fd, &c, 1);
        if (r > 0) {
            bytes += r;
        } else if (r < 0) {
            assert(errno == EAGAIN);
        } else {
            break;
        }
    }
    log_printf("Read %d bytes\n", bytes);
}

const int PORT = 31008;
const int LISTEN_BACKLOG = 2;

int main() { 
    pid_t pid_1, pid_2;
    if ((pid_1 = fork()) == 0) {
        // client
        sleep(1); // Нужен, чтобы сервер успел запуститься.
                  // В нормальном мире ошибки у пользователя решаются через retry.
        int socket_fd = socket(AF_INET, SOCK_STREAM, 0); // == connection_fd in this case
        conditional_handle_error(socket_fd == -1, "can't initialize socket"); // Проверяем на ошибку. Всегда так делаем, потому что что угодно (и где угодно) может сломаться при работе с сетью
         
        // Формирование адреса
        struct sockaddr_in addr; // Структурка адреса сервера, к которому обращаемся
        addr.sin_family = AF_INET; // Указали семейство протоколов
        addr.sin_port = htons(PORT); // Указали порт. htons преобразует локальный порядок байтов в сетевой(little endian to big).
        struct hostent *hosts = gethostbyname("localhost"); // simple function but it is legacy. Prefer getaddrinfo. Получили информацию о хосте с именем localhost
        conditional_handle_error(!hosts, "can't get host by name");
        memcpy(&addr.sin_addr, hosts->h_addr_list[0], sizeof(addr.sin_addr)); // Указали в addr первый адрес из hosts

        int connect_ret = connect(socket_fd, (struct sockaddr*)&addr, sizeof(addr)); //Тут делаем коннект
        conditional_handle_error(connect_ret == -1, "can't connect to unix socket");
        
        write_smth(socket_fd);
        log_printf("writing is done\n");
        shutdown(socket_fd, SHUT_RDWR); // Закрываем соединение
        close(socket_fd); // Закрываем файловый дескриптор уже закрытого соединения. Стоит делать оба закрытия.
        log_printf("client finished\n");
        return 0;
    }
    if ((pid_2 = fork()) == 0) {
        // server
        int socket_fd = socket(AF_INET, SOCK_STREAM, 0); 
        conditional_handle_error(socket_fd == -1, "can't initialize socket");
        #ifdef DEBUG
        // Смотри ридинг Яковлева. Вызовы, которые скажут нам, что мы готовы переиспользовать порт (потому что он может ещё не быть полностью освобожденным после прошлого использования)
        int reuse_val = 1;
        setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &reuse_val, sizeof(reuse_val));
        setsockopt(socket_fd, SOL_SOCKET, SO_REUSEPORT, &reuse_val, sizeof(reuse_val));
        #endif
        
        struct sockaddr_in addr = {.sin_family = AF_INET, .sin_port = htons(PORT)};
        // addr.sin_addr == 0, so we are ready to receive connections directed to all our addresses
        int bind_ret = bind(socket_fd, (struct sockaddr*)&addr, sizeof(addr)); // Привязали сокет к порту
        conditional_handle_error(bind_ret == -1, "can't bind to unix socket");
        
        int listen_ret = listen(socket_fd, LISTEN_BACKLOG); // Говорим что готовы принимать соединения. Не больше чем LISTEN_BACKLOG за раз
        conditional_handle_error(listen_ret == -1, "can't listen to unix socket");

        struct sockaddr_in peer_addr = {0}; // Сюда запишется адрес клиента, который к нам подключится
        socklen_t peer_addr_size = sizeof(struct sockaddr_in); // Считаем длину, чтобы accept() безопасно записал адрес и не переполнил ничего
        int connection_fd = accept(socket_fd, (struct sockaddr*)&peer_addr, &peer_addr_size); // Принимаем соединение и записываем адрес
        conditional_handle_error(connection_fd == -1, "can't accept incoming connection");
                
        read_all(connection_fd);
        
        shutdown(connection_fd, SHUT_RDWR); // }
        close(connection_fd);               // }Закрыли сокет соединение

        shutdown(socket_fd, SHUT_RDWR);     // }
        close(socket_fd);                   // } Закрыли сам сокет
        log_printf("server finished\n");
        return 0;
    }
    int status;
    assert(waitpid(pid_1, &status, 0) != -1);
    assert(waitpid(pid_2, &status, 0) != -1);
    return 0;
}
```


Run: `gcc -DDEBUG socket_inet.cpp -o socket_inet.exe`



Run: `./socket_inet.exe`


     23:25:08 : writing is done
     23:25:08 : Read 1000 bytes
     23:25:08 : server finished
     23:25:08 : client finished



Run: `diff socket_unix.cpp socket_inet.cpp  | grep -v "// %" | grep -e '>' -e '<' -C 1`


    16c17,19
    < #include <sys/un.h>
    ---
    > #include <netinet/in.h>
    > #include <netdb.h>
    > #include <string.h>
    27c30,31
    <         write(fd, "X", 1);
    ---
    >         int write_ret = write(fd, "X", 1);
    >         conditional_handle_error(write_ret != 1, "writing failed");
    49,50c53
    < // important to use "/tmp/*", otherwise you can have problems with permissions
    < const char* SOCKET_PATH = "/tmp/my_precious_unix_socket";
    ---
    > const int PORT = 31008;
    57,65c60,73
    <         sleep(1);
    <         int socket_fd = socket(AF_UNIX, SOCK_STREAM, 0); // == connection_fd in this case
    <         conditional_handle_error(socket_fd == -1, "can't initialize socket");
    <         
    <         // Тип переменной адреса (sockaddr_un) отличается от того что будет в следующем примере (т.е. тип зависит от того какое соединение используется)
    <         struct sockaddr_un addr = {.sun_family = AF_UNIX}; 
    <         strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);
    <         // Кастуем sockaddr_un* -> sockaddr*. Знакомьтесь, сишные абстрактные структуры.
    <         int connect_ret = connect(socket_fd, (const struct sockaddr*)&addr, sizeof(addr.sun_path));
    ---
    >         sleep(1); // Нужен, чтобы сервер успел запуститься.
    >                   // В нормальном мире ошибки у пользователя решаются через retry.
    >         int socket_fd = socket(AF_INET, SOCK_STREAM, 0); // == connection_fd in this case
    >         conditional_handle_error(socket_fd == -1, "can't initialize socket"); // Проверяем на ошибку. Всегда так делаем, потому что что угодно (и где угодно) может сломаться при работе с сетью
    >          
    >         // Формирование адреса
    >         struct sockaddr_in addr; // Структурка адреса сервера, к которому обращаемся
    >         addr.sin_family = AF_INET; // Указали семейство протоколов
    >         addr.sin_port = htons(PORT); // Указали порт. htons преобразует локальный порядок байтов в сетевой(little endian to big).
    >         struct hostent *hosts = gethostbyname("localhost"); // simple function but it is legacy. Prefer getaddrinfo. Получили информацию о хосте с именем localhost
    >         conditional_handle_error(!hosts, "can't get host by name");
    >         memcpy(&addr.sin_addr, hosts->h_addr_list[0], sizeof(addr.sin_addr)); // Указали в addr первый адрес из hosts
    > 
    >         int connect_ret = connect(socket_fd, (struct sockaddr*)&addr, sizeof(addr)); //Тут делаем коннект
    69,70c77,79
    <         shutdown(socket_fd, SHUT_RDWR); 
    <         close(socket_fd);
    ---
    >         log_printf("writing is done\n");
    >         shutdown(socket_fd, SHUT_RDWR); // Закрываем соединение
    >         close(socket_fd); // Закрываем файловый дескриптор уже закрытого соединения. Стоит делать оба закрытия.
    76c85
    <         int socket_fd = socket(AF_UNIX, SOCK_STREAM, 0); 
    ---
    >         int socket_fd = socket(AF_INET, SOCK_STREAM, 0); 
    77a87,92
    >         #ifdef DEBUG
    >         // Смотри ридинг Яковлева. Вызовы, которые скажут нам, что мы готовы переиспользовать порт (потому что он может ещё не быть полностью освобожденным после прошлого использования)
    >         int reuse_val = 1;
    >         setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &reuse_val, sizeof(reuse_val));
    >         setsockopt(socket_fd, SOL_SOCKET, SO_REUSEPORT, &reuse_val, sizeof(reuse_val));
    >         #endif
    79,82c94,96
    <         unlink(SOCKET_PATH); // remove socket if exists, because bind fail if it exists
    <         struct sockaddr_un addr = {.sun_family = AF_UNIX};
    <         strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);
    <         int bind_ret = bind(socket_fd, (struct sockaddr*)&addr, sizeof(addr.sun_path)); 
    ---
    >         struct sockaddr_in addr = {.sin_family = AF_INET, .sin_port = htons(PORT)};
    >         // addr.sin_addr == 0, so we are ready to receive connections directed to all our addresses
    >         int bind_ret = bind(socket_fd, (struct sockaddr*)&addr, sizeof(addr)); // Привязали сокет к порту
    85c99
    <         int listen_ret = listen(socket_fd, LISTEN_BACKLOG);
    ---
    >         int listen_ret = listen(socket_fd, LISTEN_BACKLOG); // Говорим что готовы принимать соединения. Не больше чем LISTEN_BACKLOG за раз
    88,90c102,104
    <         struct sockaddr_un peer_addr = {0};
    <         socklen_t peer_addr_size = sizeof(struct sockaddr_un);
    <         int connection_fd = accept(socket_fd, (struct sockaddr*)&peer_addr, &peer_addr_size); // После accept можно делать fork и обрабатывать соединение в отдельном процессе
    ---
    >         struct sockaddr_in peer_addr = {0}; // Сюда запишется адрес клиента, который к нам подключится
    >         socklen_t peer_addr_size = sizeof(struct sockaddr_in); // Считаем длину, чтобы accept() безопасно записал адрес и не переполнил ничего
    >         int connection_fd = accept(socket_fd, (struct sockaddr*)&peer_addr, &peer_addr_size); // Принимаем соединение и записываем адрес
    95,99c109,113
    <         shutdown(connection_fd, SHUT_RDWR); 
    <         close(connection_fd);
    <         shutdown(socket_fd, SHUT_RDWR); 
    <         close(socket_fd);
    <         unlink(SOCKET_PATH);
    ---
    >         shutdown(connection_fd, SHUT_RDWR); // }
    >         close(connection_fd);               // }Закрыли сокет соединение
    > 
    >         shutdown(socket_fd, SHUT_RDWR);     // }
    >         close(socket_fd);                   // } Закрыли сам сокет



```python

```


```python

```

# getaddrinfo

Резолвим адрес по имени.

[Документация](https://linux.die.net/man/3/getaddrinfo)

Из документации взята реализация. Но она не работала, пришлось ее подправить :)


```cpp
%%cpp getaddrinfo.cpp
%run gcc -DDEBUG getaddrinfo.cpp -o getaddrinfo.exe
%run ./getaddrinfo.exe

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>

int try_connect_by_name(const char* name, int port, int ai_family) {
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int sfd, s, j;
    size_t len;
    ssize_t nread;
   
    /* Obtain address(es) matching host/port */
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = ai_family;    
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = 0;
    hints.ai_protocol = 0;          /* Any protocol */
    
    char port_s[20];
    sprintf(port_s, "%d", port);
    s = getaddrinfo(name, port_s, &hints, &result);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }

    /* getaddrinfo() returns a list of address structures.
       Try each address until we successfully connect(2).
       If socket(2) (or connect(2)) fails, we (close the socket
       and) try the next address. */

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
        if (getnameinfo(rp->ai_addr, rp->ai_addrlen, hbuf, sizeof(hbuf), sbuf, sizeof(sbuf), NI_NUMERICHOST | NI_NUMERICSERV) == 0)
            fprintf(stderr, "Try ai_family=%d host=%s, serv=%s\n", rp->ai_family, hbuf, sbuf);
        sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sfd == -1)
            continue;
        if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1)
            break;                  /* Success */
        close(sfd);
    }

    freeaddrinfo(result);
    
    if (rp == NULL) {               /* No address succeeded */
        fprintf(stderr, "Could not connect\n");
        return -1;
    }
    return sfd;
}


int main() { 
    try_connect_by_name("localhost", 22, AF_UNSPEC);
    try_connect_by_name("localhost", 22, AF_INET6);
    try_connect_by_name("ya.ru", 80, AF_UNSPEC);
    try_connect_by_name("ya.ru", 80, AF_INET6);
    return 0;
}

```


Run: `gcc -DDEBUG getaddrinfo.cpp -o getaddrinfo.exe`



Run: `./getaddrinfo.exe`


    Try ai_family=2 host=127.0.0.1, serv=22
    Try ai_family=10 host=::1, serv=22
    Try ai_family=2 host=87.250.250.242, serv=80
    Try ai_family=10 host=2a02:6b8::2:242, serv=80
    Could not connect



```python

```

# <a name="socket_inet6"></a> socket + AF_INET6 + getaddrinfo + TCP

Вынужден использовать getaddrinfo из-за ipv6. При этом пришлось его немного поломать, так как при реализации из мануала rp->ai_socktype и rp->ai_protocol давали неподходящие значения для установки соединения.



```cpp
%%cpp socket_inet6.cpp
%run gcc -DDEBUG socket_inet6.cpp -o socket_inet6.exe
%run ./socket_inet6.exe
%run diff socket_inet.cpp socket_inet6.cpp  | grep -v "// %" | grep -e '>' -e '<' -C 1

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <errno.h>
#include <time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>

char* extract_t(char* s) { s[19] = '\0'; return s + 10; }
#define log_printf_impl(fmt, ...) { time_t t = time(0); dprintf(2, "%s : " fmt "%s", extract_t(ctime(&t)), __VA_ARGS__); }
#define log_printf(...) log_printf_impl(__VA_ARGS__, "")

#define conditional_handle_error(stmt, msg) \
    do { if (stmt) { perror(msg " (" #stmt ")"); exit(EXIT_FAILURE); } } while (0)

void write_smth(int fd) {
    for (int i = 0; i < 1000; ++i) {
        int write_ret = write(fd, "X", 1);
        conditional_handle_error(write_ret != 1, "writing failed");
        struct timespec t = {.tv_sec = 0, .tv_nsec = 10000};
        nanosleep(&t, &t);  
    }
}

void read_all(int fd) {
    int bytes = 0;
    while (true) {
        char c;
        int r = read(fd, &c, 1);
        if (r > 0) {
            bytes += r;
        } else if (r < 0) {
            assert(errno == EAGAIN);
        } else {
            break;
        }
    }
    log_printf("Read %d bytes\n", bytes);
}

int try_connect_by_name(const char* name, int port, int ai_family) {
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int sfd, s, j;
    size_t len;
    ssize_t nread;
   
    /* Obtain address(es) matching host/port */
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = ai_family;    
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = 0;
    hints.ai_protocol = 0;          /* Any protocol */
    
    char port_s[20];
    sprintf(port_s, "%d", port);
    s = getaddrinfo(name, port_s, &hints, &result);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }

    /* getaddrinfo() returns a list of address structures.
       Try each address until we successfully connect(2).
       If socket(2) (or connect(2)) fails, we (close the socket
       and) try the next address. */

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
        if (getnameinfo(rp->ai_addr, rp->ai_addrlen, hbuf, sizeof(hbuf), sbuf, sizeof(sbuf), NI_NUMERICHOST | NI_NUMERICSERV) == 0)
            fprintf(stderr, "Try ai_family=%d host=%s, serv=%s\n", rp->ai_family, hbuf, sbuf);
        sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sfd == -1)
            continue;
        if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1)
            break;                  /* Success */
        close(sfd);
    }

    freeaddrinfo(result);
    
    if (rp == NULL) {               /* No address succeeded */
        fprintf(stderr, "Could not connect\n");
        return -1;
    }
    return sfd;
}


const int PORT = 31008;
const int LISTEN_BACKLOG = 2;

int main() { 
    pid_t pid_1, pid_2;
    if ((pid_1 = fork()) == 0) {
        // client
        sleep(1);
        int socket_fd = try_connect_by_name("localhost", PORT, AF_INET6);
        write_smth(socket_fd);
        shutdown(socket_fd, SHUT_RDWR); 
        close(socket_fd);
        log_printf("client finished\n");
        return 0;
    }
    if ((pid_2 = fork()) == 0) {
        // server
        int socket_fd = socket(AF_INET6, SOCK_STREAM, 0); 
        conditional_handle_error(socket_fd == -1, "can't initialize socket");
        #ifdef DEBUG
        int reuse_val = 1;
        setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &reuse_val, sizeof(reuse_val));
        setsockopt(socket_fd, SOL_SOCKET, SO_REUSEPORT, &reuse_val, sizeof(reuse_val));
        #endif
        
        struct sockaddr_in6 addr = {.sin6_family = AF_INET6, .sin6_port = htons(PORT)};
        // addr.sin6_addr == 0, so we are ready to receive connections directed to all our addresses
        int bind_ret = bind(socket_fd, (struct sockaddr*)&addr, sizeof(addr)); 
        conditional_handle_error(bind_ret == -1, "can't bind to unix socket");
        
        int listen_ret = listen(socket_fd, LISTEN_BACKLOG);
        conditional_handle_error(listen_ret == -1, "can't listen to unix socket");

        struct sockaddr_in6 peer_addr = {0};
        socklen_t peer_addr_size = sizeof(struct sockaddr_in6);
        int connection_fd = accept(socket_fd, (struct sockaddr*)&peer_addr, &peer_addr_size);
        conditional_handle_error(connection_fd == -1, "can't accept incoming connection");
                
        read_all(connection_fd);
        
        shutdown(connection_fd, SHUT_RDWR); 
        close(connection_fd);
        shutdown(socket_fd, SHUT_RDWR); 
        close(socket_fd);
        log_printf("server finished\n");
        return 0;
    }
    int status;
    assert(waitpid(pid_1, &status, 0) != -1);
    assert(waitpid(pid_2, &status, 0) != -1);
    return 0;
}
```


Run: `gcc -DDEBUG socket_inet6.cpp -o socket_inet6.exe`



Run: `./socket_inet6.exe`


    Try ai_family=10 host=::1, serv=31008
     23:25:12 : Read 1000 bytes
     23:25:12 : server finished
     23:25:12 : client finished



Run: `diff socket_inet.cpp socket_inet6.cpp  | grep -v "// %" | grep -e '>' -e '<' -C 1`


    52a53,101
    > int try_connect_by_name(const char* name, int port, int ai_family) {
    >     struct addrinfo hints;
    >     struct addrinfo *result, *rp;
    >     int sfd, s, j;
    >     size_t len;
    >     ssize_t nread;
    >    
    >     /* Obtain address(es) matching host/port */
    >     memset(&hints, 0, sizeof(struct addrinfo));
    >     hints.ai_family = ai_family;    
    >     hints.ai_socktype = SOCK_STREAM;
    >     hints.ai_flags = 0;
    >     hints.ai_protocol = 0;          /* Any protocol */
    >     
    >     char port_s[20];
    >     sprintf(port_s, "%d", port);
    >     s = getaddrinfo(name, port_s, &hints, &result);
    >     if (s != 0) {
    >         fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
    >         exit(EXIT_FAILURE);
    >     }
    > 
    >     /* getaddrinfo() returns a list of address structures.
    >        Try each address until we successfully connect(2).
    >        If socket(2) (or connect(2)) fails, we (close the socket
    >        and) try the next address. */
    > 
    >     for (rp = result; rp != NULL; rp = rp->ai_next) {
    >         char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
    >         if (getnameinfo(rp->ai_addr, rp->ai_addrlen, hbuf, sizeof(hbuf), sbuf, sizeof(sbuf), NI_NUMERICHOST | NI_NUMERICSERV) == 0)
    >             fprintf(stderr, "Try ai_family=%d host=%s, serv=%s\n", rp->ai_family, hbuf, sbuf);
    >         sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    >         if (sfd == -1)
    >             continue;
    >         if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1)
    >             break;                  /* Success */
    >         close(sfd);
    >     }
    > 
    >     freeaddrinfo(result);
    >     
    >     if (rp == NULL) {               /* No address succeeded */
    >         fprintf(stderr, "Could not connect\n");
    >         return -1;
    >     }
    >     return sfd;
    > }
    > 
    > 
    60,75c109,110
    <         sleep(1); // Нужен, чтобы сервер успел запуститься.
    <                   // В нормальном мире ошибки у пользователя решаются через retry.
    <         int socket_fd = socket(AF_INET, SOCK_STREAM, 0); // == connection_fd in this case
    <         conditional_handle_error(socket_fd == -1, "can't initialize socket"); // Проверяем на ошибку. Всегда так делаем, потому что что угодно (и где угодно) может сломаться при работе с сетью
    <          
    <         // Формирование адреса
    <         struct sockaddr_in addr; // Структурка адреса сервера, к которому обращаемся
    <         addr.sin_family = AF_INET; // Указали семейство протоколов
    <         addr.sin_port = htons(PORT); // Указали порт. htons преобразует локальный порядок байтов в сетевой(little endian to big).
    <         struct hostent *hosts = gethostbyname("localhost"); // simple function but it is legacy. Prefer getaddrinfo. Получили информацию о хосте с именем localhost
    <         conditional_handle_error(!hosts, "can't get host by name");
    <         memcpy(&addr.sin_addr, hosts->h_addr_list[0], sizeof(addr.sin_addr)); // Указали в addr первый адрес из hosts
    < 
    <         int connect_ret = connect(socket_fd, (struct sockaddr*)&addr, sizeof(addr)); //Тут делаем коннект
    <         conditional_handle_error(connect_ret == -1, "can't connect to unix socket");
    <         
    ---
    >         sleep(1);
    >         int socket_fd = try_connect_by_name("localhost", PORT, AF_INET6);
    77,79c112,113
    <         log_printf("writing is done\n");
    <         shutdown(socket_fd, SHUT_RDWR); // Закрываем соединение
    <         close(socket_fd); // Закрываем файловый дескриптор уже закрытого соединения. Стоит делать оба закрытия.
    ---
    >         shutdown(socket_fd, SHUT_RDWR); 
    >         close(socket_fd);
    85c119
    <         int socket_fd = socket(AF_INET, SOCK_STREAM, 0); 
    ---
    >         int socket_fd = socket(AF_INET6, SOCK_STREAM, 0); 
    88d121
    <         // Смотри ридинг Яковлева. Вызовы, которые скажут нам, что мы готовы переиспользовать порт (потому что он может ещё не быть полностью освобожденным после прошлого использования)
    94,96c127,129
    <         struct sockaddr_in addr = {.sin_family = AF_INET, .sin_port = htons(PORT)};
    <         // addr.sin_addr == 0, so we are ready to receive connections directed to all our addresses
    <         int bind_ret = bind(socket_fd, (struct sockaddr*)&addr, sizeof(addr)); // Привязали сокет к порту
    ---
    >         struct sockaddr_in6 addr = {.sin6_family = AF_INET6, .sin6_port = htons(PORT)};
    >         // addr.sin6_addr == 0, so we are ready to receive connections directed to all our addresses
    >         int bind_ret = bind(socket_fd, (struct sockaddr*)&addr, sizeof(addr)); 
    99c132
    <         int listen_ret = listen(socket_fd, LISTEN_BACKLOG); // Говорим что готовы принимать соединения. Не больше чем LISTEN_BACKLOG за раз
    ---
    >         int listen_ret = listen(socket_fd, LISTEN_BACKLOG);
    102,104c135,137
    <         struct sockaddr_in peer_addr = {0}; // Сюда запишется адрес клиента, который к нам подключится
    <         socklen_t peer_addr_size = sizeof(struct sockaddr_in); // Считаем длину, чтобы accept() безопасно записал адрес и не переполнил ничего
    <         int connection_fd = accept(socket_fd, (struct sockaddr*)&peer_addr, &peer_addr_size); // Принимаем соединение и записываем адрес
    ---
    >         struct sockaddr_in6 peer_addr = {0};
    >         socklen_t peer_addr_size = sizeof(struct sockaddr_in6);
    >         int connection_fd = accept(socket_fd, (struct sockaddr*)&peer_addr, &peer_addr_size);
    109,113c142,145
    <         shutdown(connection_fd, SHUT_RDWR); // }
    <         close(connection_fd);               // }Закрыли сокет соединение
    < 
    <         shutdown(socket_fd, SHUT_RDWR);     // }
    <         close(socket_fd);                   // } Закрыли сам сокет
    ---
    >         shutdown(connection_fd, SHUT_RDWR); 
    >         close(connection_fd);
    >         shutdown(socket_fd, SHUT_RDWR); 
    >         close(socket_fd);



```python

```

# <a name="socket_udp"></a> socket + AF_INET + UDP


```cpp
%%cpp socket_inet.cpp
%run gcc -DDEBUG socket_inet.cpp -o socket_inet.exe
%run ./socket_inet.exe

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <errno.h>
#include <time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>

char* extract_t(char* s) { s[19] = '\0'; return s + 10; }
#define log_printf_impl(fmt, ...) { time_t t = time(0); dprintf(2, "%s : " fmt "%s", extract_t(ctime(&t)), __VA_ARGS__); }
#define log_printf(...) log_printf_impl(__VA_ARGS__, "")

#define conditional_handle_error(stmt, msg) \
    do { if (stmt) { perror(msg " (" #stmt ")"); exit(EXIT_FAILURE); } } while (0)

void write_smth(int fd) {
    for (int i = 0; i < 1000; ++i) {
        int write_ret = write(fd, "X", 1);
        conditional_handle_error(write_ret != 1, "writing failed");
        struct timespec t = {.tv_sec = 0, .tv_nsec = 10000};
        nanosleep(&t, &t);  
    }
}

void read_all(int fd) {
    int bytes = 0;
    while (true) {
        char c;
        int r = read(fd, &c, 1);
        if (r > 0) {
            bytes += r;
        } else if (r < 0) {
            assert(errno == EAGAIN);
        } else {
            break;
        }
    }
    log_printf("Read %d bytes\n", bytes);
}

const int PORT = 31008;

int main() { 
    pid_t pid_1, pid_2;
    if ((pid_1 = fork()) == 0) {
        // client
        sleep(1); 
       
        int socket_fd = socket(AF_INET, SOCK_DGRAM, 0); // создаем UDP сокет
        conditional_handle_error(socket_fd == -1, "can't initialize socket");
 
        struct sockaddr_in addr = {
            .sin_family = AF_INET,
            .sin_port = htons(PORT),
            .sin_addr = {.s_addr = htonl(INADDR_LOOPBACK)}, // более эффективный способ присвоить адрес localhost
        };
        
        int written_bytes;
        // посылаем первую датаграмму, явно указываем, кому (функция sendto)
        const char msg1[] = "Hello 1";
        written_bytes = sendto(socket_fd, msg1, sizeof(msg1), 0,
               (struct sockaddr *)&addr, sizeof(addr));
        conditional_handle_error(written_bytes == -1, "can't sendto");
        
        // здесь вызываем connect. В данном случае он просто сохраняет адрес, никаких данных по сети не передается
        // посылаем вторую датаграмму, по сохраненному адресу. Используем функцию send
        const char msg2[] = "Hello 2";
        int connect_ret = connect(socket_fd, (struct sockaddr *)&addr, sizeof(addr));
        conditional_handle_error(connect_ret == -1, "can't connect OoOo");
        written_bytes = send(socket_fd, msg2, sizeof(msg2), 0);
        conditional_handle_error(written_bytes == -1, "can't send");
        
        // посылаем третью датаграмму (write - эквивалент send с последним аргументом = 0)
        const char msg3[] = "LastHello";
        written_bytes = write(socket_fd, msg3, sizeof(msg3));
        conditional_handle_error(written_bytes == -1, "can't write");

        log_printf("client finished\n");
        shutdown(socket_fd, SHUT_RDWR);     
        close(socket_fd);       
        return 0;
    }
    if ((pid_2 = fork()) == 0) {
        // server
        int socket_fd = socket(AF_INET, SOCK_DGRAM, 0); 
        conditional_handle_error(socket_fd == -1, "can't initialize socket");
        
        #ifdef DEBUG
        int reuse_val = 1;
        setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &reuse_val, sizeof(reuse_val));
        setsockopt(socket_fd, SOL_SOCKET, SO_REUSEPORT, &reuse_val, sizeof(reuse_val));
        #endif
        
        struct sockaddr_in addr = {
            .sin_family = AF_INET,
            .sin_port = htons(PORT),
            .sin_addr = {.s_addr = htonl(INADDR_ANY)}, // более надежный способ сказать, что мы готовы принимать на любой входящий адрес (раньше просто 0 неявно записывали)
        };
        
        int bind_ret = bind(socket_fd, (struct sockaddr *)&addr, sizeof(addr));
        conditional_handle_error(bind_ret < 0, "can't bind socket");

        char buf[1024];
        int bytes_read;
        while (true) {
            bytes_read = recvfrom(socket_fd, buf, 1024, 0, NULL, NULL);
            buf[bytes_read] = '\0';
            log_printf("%s\n", buf);
            if (strcmp("LastHello", buf) == 0) {
                break;
            }
        }
        log_printf("server finished\n");
        return 0;
    }
    int status;
    assert(waitpid(pid_1, &status, 0) != -1);
    assert(waitpid(pid_2, &status, 0) != -1);
    return 0;
}
```


Run: `gcc -DDEBUG socket_inet.cpp -o socket_inet.exe`



Run: `./socket_inet.exe`


     23:31:32 : client finished
     23:31:32 : Hello 1
     23:31:32 : Hello 2
     23:31:32 : LastHello
     23:31:32 : server finished



```python

```

# <a name="hw"></a> Комментарии к ДЗ

* inf14-0: posix/sockets/tcp-client -- требуется решить несколько задач:
  1. Сформировать адрес. Здесь можно использовать функции, которые делают из доменного имени адрес (им неважно, преобразовывать "192.168.1.2" или "ya.ru")
  2. Установить соединение. Так же как и раньше делали
  3. Написать логику про чтение/запись чисел. Так как порядок байт LittleEndian - тут вообще никаких сетевых особенностей нет. 
* inf14-1: posix/sockets/http-server-1 -- задача больше на работу с файлами, чем на сетевую часть. Единственный момент -- реагирование на сигналы. Тут можно просто хранить в атомиках файловые дескрипторы и в хендлере закрывать их с последующим exit. Или можно заморочиться с мультиплексированием ввода-вывода (будет на следующем семинаре)
* inf14-2: posix/sockets/udp-client -- на следующем семинаре разберём udp. Или можете сами почитать, там просто в сравнении с UDP. (Пример уже в этом ноутбуке есть)
* inf14-3: posix/sockets/http-server-2 -- усложнение inf14-1, но не по сетевой части. Просто вспомнить проверку файлов на исполняемость и позапускать, правильно прокинув файловые дескрипторы.


```python

```


```python

```


```python

```


```python

```
