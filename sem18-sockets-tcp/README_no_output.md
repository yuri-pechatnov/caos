

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
      Это можно сделать двумя способами: либо повторно передавать данные, либо передавать данные через Ethernet кабели, добавив коммутаторы в качестве посредника (наличие всего двух субъектов на каждом канале упрощает совместное использование среды).
3. Сетевой уровень (NETWORK) <br>
  Появляются IP-адреса. Выполняет выбор маршрута передачи данных, учитывая длительность пути, нагруженность сети, etc. <br>
  Один IP может соответствовать нескольким устройствам. Для этого используется хак на уровне маршрутизатора(NAT)
  Одному устройству может соответствовать несколько IP. Это без хаков.
4. Транспортный уровень (TRANSPORT) `<-` **сокеты** это интерфейсы вот этого уровня <br>
  Важный момент. Сетевой уровень - это про пересылку сообщений между конкретными хостами. А транспортный - между конкретными программами на конкретных хостах. <br>
  Реализуются часто в ядре операционной системы <br>
  Еще стоит понимать, что транспортный уровень, предоставляя один интерфейс может иметь разные реализации. Например сокеты UNIX, в этом случае под транспортным уровнем нет сетевого, так как передача данных ведется внутри одной машины. <br>
  Появляется понятие порта - порт идентифицирует программу-получателя на хосте. <br>
  Протоколы передачи данных:
    1. TCP - устанавливает соединение, похожее на пайп. Надежный, переотправляет данные, но медленный. Регулирует скорость отправки данных в зависимости от нагрузки сети, чтобы не дропнуть интернет.
    2. UDP - Быстрый, ненадёжный. Отправляет данные сразу все. 
5. Сеансовый уровень (SESSION)
6. Уровень представления данных (PRESENTATION) (IMHO не нужен)
7. Прикладной уровень (APPLICATION)

Сегодня в программе:
* `socketpair` - <a href="#socketpair" style="color:#856024">аналог `pipe`</a>, но полученные дескрипторы обладают сокетными свойствами: файловый дескриптор работает и на чтение и на запись (соответственно этот "pipe" двусторонний), закрывать нужно с вызовом `shutdown`
* `socket` - функция создания сокета
  * TCP
      * <a href="#socket_unix" style="color:#856024">AF_UNIX</a> - сокет внутри системы. Адрес в данном случае - адрес файла сокета в файловой системе.
      * <a href="#socket_inet" style="color:#856024">AF_INET</a> - сокет для стандартных ipv4 соединений. **И это самый важный пример в этом ноутбуке**.
      * <a href="#socket_inet6" style="color:#856024">AF_INET6</a> - сокет для стандартных ipv6 соединений.
  
[Сайт с хорошими картинками про порядок низкоуровневых вызовов в клиентском и серверном приложении](http://support.fastwel.ru/AppNotes/AN/AN-0001.html#pic3)



<a href="#hw" style="color:#856024">Комментарии к ДЗ</a>

[Ридинг Яковлева](https://github.com/victor-yacovlev/mipt-diht-caos/tree/master/practice/sockets-tcp)

# netcat

Для отладки может быть полезным:
* `netcat -lv localhost 30000` - слушать указанный порт по TCP. Выводит все, что пишет клиент. Данные из своего stdin отправляет подключенному клиенту.
* `netcat localhost 30000` - подключиться к серверу по TCP. Ввод вывод работает. Потренироваться можно, например, на www.opennet.ru
* `netcat -lvu localhost 30000` - слушать по UDP. Но что-то мне кажется, эта команда умеет только одну датаграмму принимать, потом что-то ломается.
* `echo "asfrtvf" | netcat -u -q1 localhost 30000` - отправить датаграмму. Опция -v в этом случае ведет себя странно почему-то.


```python

```

# <a name="socketpair"></a> socketpair в качестве pipe

Socket в качестве pipe (т.е. внутри системы) используется для написания примерно одинакового кода (для локальных соединений и соединений через интернет) и для использования возможностей сокета.

[close vs shutdown](https://stackoverflow.com/questions/48208236/tcp-close-vs-shutdown-in-linux-os)



```cpp
%%cpp socketpair.c
%run gcc socketpair.c -o socketpair.exe
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
#include <sys/syscall.h>
#include <errno.h>
#include <time.h>
#include <stdatomic.h>
#include <stdbool.h>

// log_printf - макрос для отладочного вывода, добавляющий время с первого использования, имя функции и номер строки
const char* log_prefix(const char* func, int line) {
    struct timespec spec; clock_gettime(CLOCK_REALTIME, &spec); long long current_msec = spec.tv_sec * 1000L + spec.tv_nsec / 1000000;
    static _Atomic long long start_msec_storage = -1; long long start_msec = -1; if (atomic_compare_exchange_strong(&start_msec_storage, &start_msec, current_msec)) start_msec = current_msec;
    long long delta_msec = current_msec - start_msec; const int max_func_len = 10;
    static __thread char prefix[100]; sprintf(prefix, "%lld.%03lld %*s():%d    ", delta_msec / 1000, delta_msec % 1000, max_func_len, func, line); sprintf(prefix + max_func_len + 13, "[tid=%ld]", syscall(__NR_gettid));
    return prefix;
}
#define log_printf_impl(fmt, ...) { time_t t = time(0); dprintf(2, "%s: " fmt "%s", log_prefix(__FUNCTION__, __LINE__), __VA_ARGS__); }
// Format: <time_since_start> <func_name>:<line> : <custom_message>
#define log_printf(...) log_printf_impl(__VA_ARGS__, "")


#define SWAP(a, b) { __typeof__(a) c = (a); (a) = (b); (b) = (c); }


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
    log_printf("Start\n");
    int fd[2];
    assert(socketpair(AF_UNIX, SOCK_STREAM, 0, fd) == 0); //socketpair создает пару соединенных сокетов(по сути pipe)
    //SWAP(fd[0], fd[1]); // can change order, it will work
    
    pid_t pid_1, pid_2;
    if ((pid_1 = fork()) == 0) {
        close(fd[1]);
        write_smth(fd[0]);
        shutdown(fd[0], SHUT_RDWR); // Важное дополнение к close в случае потоковых сокетов
        close(fd[0]);
        log_printf("Writing is done\n");
        sleep(1);
        return 0;
    }
    if ((pid_2 = fork()) == 0) {
        close(fd[0]);
        read_all(fd[1]);
        shutdown(fd[1], SHUT_RDWR);
        close(fd[1]);
        return 0;
    }
    close(fd[0]);
    close(fd[1]);
    int status;
    assert(waitpid(pid_1, &status, 0) != -1);
    assert(waitpid(pid_2, &status, 0) != -1);
    return 0;
}
```


```python

```

# <a name="socket_unix"></a> socket + AF_UNIX + TCP


```cpp
%%cpp socket_unix.c
%run gcc socket_unix.c -lpthread -o socket_unix.exe
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
#include <sys/syscall.h>
#include <errno.h>
#include <time.h>
#include <sys/un.h>
#include <stdatomic.h>
#include <stdbool.h>

// log_printf - макрос для отладочного вывода, добавляющий время с первого использования, имя функции и номер строки
const char* log_prefix(const char* func, int line) {
    struct timespec spec; clock_gettime(CLOCK_REALTIME, &spec); long long current_msec = spec.tv_sec * 1000L + spec.tv_nsec / 1000000;
    static _Atomic long long start_msec_storage = -1; long long start_msec = -1; if (atomic_compare_exchange_strong(&start_msec_storage, &start_msec, current_msec)) start_msec = current_msec;
    long long delta_msec = current_msec - start_msec; const int max_func_len = 13;
    static __thread char prefix[100]; sprintf(prefix, "%lld.%03lld %*s():%d    ", delta_msec / 1000, delta_msec % 1000, max_func_len, func, line); sprintf(prefix + max_func_len + 13, "[tid=%ld]", syscall(__NR_gettid));
    return prefix;
}
#define log_printf_impl(fmt, ...) { time_t t = time(0); dprintf(2, "%s: " fmt "%s", log_prefix(__FUNCTION__, __LINE__), __VA_ARGS__); }
// Format: <time_since_start> <func_name>:<line> : <custom_message>
#define log_printf(...) log_printf_impl(__VA_ARGS__, "")


#define conditional_handle_error(stmt, msg) \
    do { if (stmt) { perror(msg " (" #stmt ")"); exit(EXIT_FAILURE); } } while (0)

void write_smth(int fd) {
    int N = 1000;
    for (int i = 0; i < N; ++i) {
        write(fd, "X", 1);
        struct timespec t = {.tv_sec = 0, .tv_nsec = 10000};
        nanosleep(&t, &t);  
    }
    log_printf("Written %d bytes\n", N);
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

void client_func() {
    log_printf("Client started\n");
    int socket_fd = socket(AF_UNIX, SOCK_STREAM, 0); // == connection_fd in this case
    conditional_handle_error(socket_fd == -1, "can't initialize socket");

    // Тип переменной адреса (sockaddr_un) отличается от того что будет в следующем примере (т.е. тип зависит от того какое соединение используется)
    struct sockaddr_un addr = {.sun_family = AF_UNIX}; 
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);
    
    while (true) { //
        // Кастуем sockaddr_un* -> sockaddr*. Знакомьтесь, сишные абстрактные структуры.
        int connect_ret = connect(socket_fd, (const struct sockaddr*)&addr, sizeof(addr));
        if (connect_ret != -1) {
            break;
        }
        log_printf("Can't connect to unix socket. Retry after second\n");
        sleep(1);
    }
    log_printf("Client connected and start writing\n");
    write_smth(socket_fd);
    shutdown(socket_fd, SHUT_RDWR); 
    close(socket_fd);
    log_printf("Client finished\n");
}


void server_func() {
    log_printf("Server started\n");
    int socket_fd = socket(AF_UNIX, SOCK_STREAM, 0); 
    conditional_handle_error(socket_fd == -1, "can't initialize socket");

    unlink(SOCKET_PATH); // remove socket if exists, because bind fail if it exists
    struct sockaddr_un addr = {.sun_family = AF_UNIX};
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);
    int bind_ret = bind(socket_fd, (struct sockaddr*)&addr, sizeof(addr)); 
    conditional_handle_error(bind_ret == -1, "can't bind to unix socket");
    log_printf("Socket is bound\n");
    
    int listen_ret = listen(socket_fd, LISTEN_BACKLOG);
    conditional_handle_error(listen_ret == -1, "can't listen to unix socket");
    log_printf("Listening started\n");

    struct sockaddr_un peer_addr = {0};
    socklen_t peer_addr_size = sizeof(struct sockaddr_un);
    int connection_fd = accept(socket_fd, (struct sockaddr*)&peer_addr, &peer_addr_size); // После accept можно делать fork и обрабатывать соединение в отдельном процессе
    conditional_handle_error(connection_fd == -1, "can't accept incoming connection");
    log_printf("Server accepted conntection and start reading\n");
    
    read_all(connection_fd);

    shutdown(connection_fd, SHUT_RDWR); 
    close(connection_fd);
    shutdown(socket_fd, SHUT_RDWR); 
    close(socket_fd);
    unlink(SOCKET_PATH);
    log_printf("Server finished\n");
}

int main() { 
    log_printf("Program started\n");
    pid_t pid_1, pid_2;
    if ((pid_1 = fork()) == 0) {
        client_func();
        return 0;
    }
    if ((pid_2 = fork()) == 0) {
        server_func();
        return 0;
    }
    int status;
    assert(waitpid(pid_1, &status, 0) != -1);
    assert(waitpid(pid_2, &status, 0) != -1);
    log_printf("Program finished\n");
    return 0;
}
```


```python

```

# <a name="socket_inet"></a> socket + AF_INET + TCP

[На первый взгляд приличная статейка про программирование на сокетах в linux](https://www.rsdn.org/article/unix/sockets.xml)

[Ответ на stackoverflow про то, что делает shutdown](https://stackoverflow.com/a/23483487)


```cpp
%%cpp socket_inet.c
%run gcc -DDEBUG socket_inet.c -o socket_inet.exe
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
#include <sys/syscall.h>
#include <errno.h>
#include <time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <stdatomic.h>
#include <stdbool.h>
    

// log_printf - макрос для отладочного вывода, добавляющий время с первого использования, имя функции и номер строки
const char* log_prefix(const char* func, int line) {
    struct timespec spec; clock_gettime(CLOCK_REALTIME, &spec); long long current_msec = spec.tv_sec * 1000L + spec.tv_nsec / 1000000;
    static _Atomic long long start_msec_storage = -1; long long start_msec = -1; if (atomic_compare_exchange_strong(&start_msec_storage, &start_msec, current_msec)) start_msec = current_msec;
    long long delta_msec = current_msec - start_msec; const int max_func_len = 13;
    static __thread char prefix[100]; sprintf(prefix, "%lld.%03lld %*s():%d    ", delta_msec / 1000, delta_msec % 1000, max_func_len, func, line); sprintf(prefix + max_func_len + 13, "[tid=%ld]", syscall(__NR_gettid));
    return prefix;
}
#define log_printf_impl(fmt, ...) { time_t t = time(0); dprintf(2, "%s: " fmt "%s", log_prefix(__FUNCTION__, __LINE__), __VA_ARGS__); }
// Format: <time_since_start> <func_name>:<line> : <custom_message>
#define log_printf(...) log_printf_impl(__VA_ARGS__, "")


#define conditional_handle_error(stmt, msg) \
    do { if (stmt) { perror(msg " (" #stmt ")"); exit(EXIT_FAILURE); } } while (0)

void write_smth(int fd) {
    int N = 1000;
    for (int i = 0; i < N; ++i) {
        write(fd, "X", 1);
        struct timespec t = {.tv_sec = 0, .tv_nsec = 10000};
        nanosleep(&t, &t);  
    }
    log_printf("Written %d bytes\n", N);
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

void client_func() {
    log_printf("Client started\n");
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0); // == connection_fd in this case
    conditional_handle_error(socket_fd == -1, "can't initialize socket"); // Проверяем на ошибку. Всегда так делаем, потому что что угодно (и где угодно) может сломаться при работе с сетью

    // Формирование адреса
    struct sockaddr_in addr;     // Структурка адреса сервера, к которому обращаемся
    addr.sin_family = AF_INET;   // Указали семейство протоколов
    addr.sin_port = htons(PORT); // Указали порт. htons преобразует локальный порядок байтов в сетевой(little endian to big).
    struct hostent *hosts = gethostbyname("localhost"); // Простая функция, но устаревшая. Лучше использовать getaddrinfo. Получили информацию о хосте с именем localhost
    conditional_handle_error(!hosts, "can't get host by name");
    memcpy(&addr.sin_addr, hosts->h_addr_list[0], sizeof(addr.sin_addr)); // Указали в addr первый адрес из hosts

    while (true) { // Пытаемся подключиться
        int connect_ret = connect(socket_fd, (struct sockaddr*)&addr, sizeof(addr)); //Тут делаем коннект
        if (connect_ret != -1) {
            break;
        }
        log_printf("Can't connect to tcp socket. Retry after second\n");
        sleep(1);
    }
    log_printf("Client connected and start writing\n");
    write_smth(socket_fd);
    log_printf("Writing is done\n");
    shutdown(socket_fd, SHUT_RDWR); 
    close(socket_fd);
    log_printf("Client finished\n");
}


void server_func() {    
    log_printf("Server started\n");
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0); 
    conditional_handle_error(socket_fd == -1, "can't initialize socket");
    #ifdef DEBUG
    // Смотри ридинг Яковлева. Вызовы, которые скажут нам, что мы готовы переиспользовать порт (потому что он может ещё не быть полностью освобожденным после прошлого использования)
    int reuse_val = 1;
    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &reuse_val, sizeof(reuse_val));
    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEPORT, &reuse_val, sizeof(reuse_val));
    #endif

    struct sockaddr_in addr = {.sin_family = AF_INET, .sin_port = htons(PORT), .sin_addr = 0};
    // addr.sin_addr == 0, so we are ready to receive connections directed to all our addresses
    int bind_ret = bind(socket_fd, (struct sockaddr*)&addr, sizeof(addr)); // Привязали сокет к порту
    conditional_handle_error(bind_ret == -1, "can't bind to unix socket");
    log_printf("Socket is bound\n");

    int listen_ret = listen(socket_fd, LISTEN_BACKLOG); // Говорим что готовы принимать соединения. Не больше чем LISTEN_BACKLOG за раз
    conditional_handle_error(listen_ret == -1, "can't listen to unix socket");
    log_printf("Listening started\n");
    
    struct sockaddr_in peer_addr = {0}; // Сюда запишется адрес клиента, который к нам подключится
    socklen_t peer_addr_size = sizeof(struct sockaddr_in); // Считаем длину, чтобы accept() безопасно записал адрес и не переполнил ничего
    int connection_fd = accept(socket_fd, (struct sockaddr*)&peer_addr, &peer_addr_size); // Принимаем соединение и записываем адрес
    conditional_handle_error(connection_fd == -1, "can't accept incoming connection");
    log_printf("Server accepted conntection and start reading\n");
    
    read_all(connection_fd);

    shutdown(connection_fd, SHUT_RDWR); // }
    close(connection_fd);               // }Закрыли сокет соединение

    shutdown(socket_fd, SHUT_RDWR);     // }
    close(socket_fd);                   // } Закрыли сам сокет
    log_printf("Server finished\n");
}

int main() { 
    log_printf("Program started\n");
    pid_t pid_1, pid_2;
    if ((pid_1 = fork()) == 0) {
        client_func();
        return 0;
    }
    if ((pid_2 = fork()) == 0) {
        server_func();
        return 0;
    }
    int status;
    assert(waitpid(pid_1, &status, 0) != -1);
    assert(waitpid(pid_2, &status, 0) != -1);
    log_printf("Program finished\n");
    return 0;
}
```


```python

```


```python

```

# getaddrinfo

Резолвим адрес по имени.

[Документация](https://linux.die.net/man/3/getaddrinfo)

Из документации взята реализация. Но она не работала, пришлось ее подправить :)


```cpp
%%cpp getaddrinfo.c
%run gcc -DDEBUG getaddrinfo.c -o getaddrinfo.exe
%run ./getaddrinfo.exe

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>

#include <sys/syscall.h>
#include <time.h>
#include <stdatomic.h>
#include <stdbool.h>


// log_printf - макрос для отладочного вывода, добавляющий время с первого использования, имя функции и номер строки
const char* log_prefix(const char* func, int line) {
    struct timespec spec; clock_gettime(CLOCK_REALTIME, &spec); long long current_msec = spec.tv_sec * 1000L + spec.tv_nsec / 1000000;
    static _Atomic long long start_msec_storage = -1; long long start_msec = -1; if (atomic_compare_exchange_strong(&start_msec_storage, &start_msec, current_msec)) start_msec = current_msec;
    long long delta_msec = current_msec - start_msec; const int max_func_len = 19;
    static __thread char prefix[100]; sprintf(prefix, "%lld.%03lld %*s():%d    ", delta_msec / 1000, delta_msec % 1000, max_func_len, func, line); sprintf(prefix + max_func_len + 13, "[tid=%ld]", syscall(__NR_gettid));
    return prefix;
}
#define log_printf_impl(fmt, ...) { time_t t = time(0); dprintf(2, "%s: " fmt "%s", log_prefix(__FUNCTION__, __LINE__), __VA_ARGS__); }
// Format: <time_since_start> <func_name>:<line> : <custom_message>
#define log_printf(...) log_printf_impl(__VA_ARGS__, "")


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
        log_printf("getaddrinfo: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }

    /* getaddrinfo() returns a list of address structures.
       Try each address until we successfully connect(2).
       If socket(2) (or connect(2)) fails, we (close the socket
       and) try the next address. */

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
        if (getnameinfo(rp->ai_addr, rp->ai_addrlen, hbuf, sizeof(hbuf), sbuf, sizeof(sbuf), NI_NUMERICHOST | NI_NUMERICSERV) == 0)
            log_printf("Try ai_family=%d host=%s, serv=%s\n", rp->ai_family, hbuf, sbuf);
        sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sfd == -1)
            continue;
        if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1) {
            log_printf("Success with ai_family=%d host=%s, serv=%s\n", rp->ai_family, hbuf, sbuf);
            break;                  /* Success */
        }
        close(sfd);
    }

    freeaddrinfo(result);
    
    if (rp == NULL) {               /* No address succeeded */
        log_printf("Could not connect to %s:%d\n", name, port);
        return -1;
    }
    return sfd;
}


int main() { 
    try_connect_by_name("localhost", 22, AF_UNSPEC);
    try_connect_by_name("localhost", 22, AF_INET);
    try_connect_by_name("localhost", 22, AF_INET6);
    try_connect_by_name("ya.ru", 80, AF_UNSPEC);
    try_connect_by_name("ya.ru", 80, AF_INET6);
    return 0;
}

```


```python

```

# <a name="socket_inet6"></a> socket + AF_INET6 + getaddrinfo + TCP

Вынужден использовать getaddrinfo из-за ipv6. При этом пришлось его немного поломать, так как при реализации из мануала rp->ai_socktype и rp->ai_protocol давали неподходящие значения для установки соединения.



```cpp
%%cpp socket_inet6.c
%run gcc -DDEBUG socket_inet6.c -o socket_inet6.exe
%run ./socket_inet6.exe

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/syscall.h>
#include <errno.h>
#include <time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <stdatomic.h>
#include <stdbool.h>

// log_printf - макрос для отладочного вывода, добавляющий время с первого использования, имя функции и номер строки
const char* log_prefix(const char* func, int line) {
    struct timespec spec; clock_gettime(CLOCK_REALTIME, &spec); long long current_msec = spec.tv_sec * 1000L + spec.tv_nsec / 1000000;
    static _Atomic long long start_msec_storage = -1; long long start_msec = -1; if (atomic_compare_exchange_strong(&start_msec_storage, &start_msec, current_msec)) start_msec = current_msec;
    long long delta_msec = current_msec - start_msec; const int max_func_len = 19;
    static __thread char prefix[100]; sprintf(prefix, "%lld.%03lld %*s():%d    ", delta_msec / 1000, delta_msec % 1000, max_func_len, func, line); sprintf(prefix + max_func_len + 13, "[tid=%ld]", syscall(__NR_gettid));
    return prefix;
}
#define log_printf_impl(fmt, ...) { time_t t = time(0); dprintf(2, "%s: " fmt "%s", log_prefix(__FUNCTION__, __LINE__), __VA_ARGS__); }
// Format: <time_since_start> <func_name>:<line> : <custom_message>
#define log_printf(...) log_printf_impl(__VA_ARGS__, "")


#define conditional_handle_error(stmt, msg) \
    do { if (stmt) { perror(msg " (" #stmt ")"); exit(EXIT_FAILURE); } } while (0)

void write_smth(int fd) {
    int N = 1000;
    for (int i = 0; i < N; ++i) {
        write(fd, "X", 1);
        struct timespec t = {.tv_sec = 0, .tv_nsec = 10000};
        nanosleep(&t, &t);  
    }
    log_printf("Written %d bytes\n", N);
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
        log_printf("getaddrinfo: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }

    /* getaddrinfo() returns a list of address structures.
       Try each address until we successfully connect(2).
       If socket(2) (or connect(2)) fails, we (close the socket
       and) try the next address. */

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
        if (getnameinfo(rp->ai_addr, rp->ai_addrlen, hbuf, sizeof(hbuf), sbuf, sizeof(sbuf), NI_NUMERICHOST | NI_NUMERICSERV) == 0)
            log_printf("Try ai_family=%d host=%s, serv=%s\n", rp->ai_family, hbuf, sbuf);
        sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sfd == -1)
            continue;
        if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1) {
            log_printf("Success with ai_family=%d host=%s, serv=%s\n", rp->ai_family, hbuf, sbuf);
            break;                  /* Success */
        }
        close(sfd);
    }

    freeaddrinfo(result);
    
    if (rp == NULL) {               /* No address succeeded */
        log_printf("Could not connect to %s:%d\n", name, port);
        return -1;
    }
    return sfd;
}

const int PORT = 31008;
const int LISTEN_BACKLOG = 2;


void client_func() {
    log_printf("Client started\n");
    int socket_fd = -1;
    while (true) { // Пытаемся подключиться
        socket_fd = try_connect_by_name("localhost", PORT, AF_INET6);
        if (socket_fd != -1) {
            break;
        }
        log_printf("Can't connect to tcp socket. Retry after second\n");
        sleep(1);
    }
    log_printf("Client connected and start writing\n");
    write_smth(socket_fd);
    log_printf("Writing is done\n");
    shutdown(socket_fd, SHUT_RDWR); 
    close(socket_fd);
    log_printf("Client finished\n");
}


void server_func() {    
    log_printf("Server started\n");
    int socket_fd = socket(AF_INET6, SOCK_STREAM, 0); 
    conditional_handle_error(socket_fd == -1, "can't initialize socket");
    #ifdef DEBUG
    // Смотри ридинг Яковлева. Вызовы, которые скажут нам, что мы готовы переиспользовать порт (потому что он может ещё не быть полностью освобожденным после прошлого использования)
    int reuse_val = 1;
    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &reuse_val, sizeof(reuse_val));
    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEPORT, &reuse_val, sizeof(reuse_val));
    #endif

    struct sockaddr_in6 addr = {.sin6_family = AF_INET6, .sin6_port = htons(PORT)};
    // addr.sin6_addr == 0, so we are ready to receive connections directed to all our addresses
    int bind_ret = bind(socket_fd, (struct sockaddr*)&addr, sizeof(addr)); 
    conditional_handle_error(bind_ret == -1, "can't bind to unix socket");
    log_printf("Socket is bound\n");

    int listen_ret = listen(socket_fd, LISTEN_BACKLOG); // Говорим что готовы принимать соединения. Не больше чем LISTEN_BACKLOG за раз
    conditional_handle_error(listen_ret == -1, "can't listen to unix socket");
    log_printf("Listening started\n");
    
    struct sockaddr_in6 peer_addr = {0};
    socklen_t peer_addr_size = sizeof(struct sockaddr_in6);
    int connection_fd = accept(socket_fd, (struct sockaddr*)&peer_addr, &peer_addr_size);
    conditional_handle_error(connection_fd == -1, "can't accept incoming connection");

    log_printf("Server accepted conntection and start reading\n");
    
    read_all(connection_fd);

    shutdown(connection_fd, SHUT_RDWR); // }
    close(connection_fd);               // }Закрыли сокет соединение

    shutdown(socket_fd, SHUT_RDWR);     // }
    close(socket_fd);                   // } Закрыли сам сокет
    log_printf("Server finished\n");
}

int main() { 
    log_printf("Program started\n");
    pid_t pid_1, pid_2;
    if ((pid_1 = fork()) == 0) {
        client_func();
        return 0;
    }
    if ((pid_2 = fork()) == 0) {
        server_func();
        return 0;
    }
    int status;
    assert(waitpid(pid_1, &status, 0) != -1);
    assert(waitpid(pid_2, &status, 0) != -1);
    log_printf("Program finished\n");
    return 0;
}
```


```python

```


```python

```

# QA
```
Димитрис Голяр, [23 февр. 2020 г., 18:11:26 (23.02.2020, 18:09:14)]:
Привет! У меня возник вопрос по работе сервера. В задаче  14-1 написано, что программа должна прослушивать соединения на сервере localhost. А что вообще произойдёт если я пропишу не localhost, а что-то другое?) Я буду прослушивать соединения другого какого-то сервера?

Yuri Pechatnov, [23 февр. 2020 г., 18:36:07]:
Я это понимаю так: у хоста может быть несколько IP адресов. Например, глобальный в интернете и 127.0.0.1 (=localhost)

Если ты укзазываешь адрес 0 при создании сервера, то ты принимаешь пакеты адресованные на любой IP этого хоста

А если указываешь конкретный адрес, то только пакеты адресованнные на этот конкретный адрес

И если ты указываешь localhost, то обрабатываешь только те пакеты у которых целевой адрес 127.0.0.1
а эти пакеты могли быть отправлены только с твоего хоста (иначе бы они остались на хосте-отправителе и не дошли до тебя)

Кстати, эта особенность стреляет при запуске jupyter notebook. Если ты не укажешь «—ip=0.0.0.0» то не сможешь подключиться к нему с другой машины, так как он сядет слушать только пакеты адресованные в localhost

```


```python

```

# <a name="hw"></a> Комментарии к ДЗ

* inf14-0: posix/sockets/tcp-client -- требуется решить несколько задач:
  1. Сформировать адрес. Здесь можно использовать функции, которые делают из доменного имени адрес (им неважно, преобразовывать "192.168.1.2" или "ya.ru"). А можно специальную функцию `inet_aton` или `inet_pton`.
  2. Установить соединение. Так же как и раньше делали
  3. Написать логику про чтение/запись чисел. Так как порядок байт LittleEndian - тут вообще никаких сетевых особенностей нет. 
* inf14-1: posix/sockets/http-server-1 -- задача больше на работу с файлами, чем на сетевую часть. Единственный момент -- реагирование на сигналы.
* inf14-3: posix/sockets/http-server-2 -- усложнение inf14-1, но не по сетевой части. Просто вспомнить проверку файлов на исполняемость и позапускать, правильно прокинув файловые дескрипторы.


Комментарий про задачи-серверы:

`man sendfile` - эта функция вам пригодится.

Работать с сигналами в задачах-серверах нужно очень аккуратно. Очень легко получить гонку хендлеров с accept. Легко не выйти из ожидания при получении сигнала, легко сделать максимально асинхронно-небезопасное нечто при его обработке.
Поэтому предлагаю (в очередной раз) избегать классических обработчиков сигналов и получать их иным способом. Например, как написано ниже.


```python

```

# <a name="hw_server"></a> Безопасный шаблон для домашки про сервер

По опыту прошлых лет очень много прям откровенно плохой обработки сигналов (сходу придумываются кейсы, когда решения ломаются). Поэтому предлагаю такой шаблон остановки при получении сигналов.

Суть в том, чтобы избежать асинхронной обработки сигналов и связанных с этим проблем. Превратить пришедший сигнал в данные в декскрипторе и следить за ним с помощью epoll (будет подбробнее разобран на следущем семинаре).

Если заметите, что в шаблоне есть баги - пишите (я его недавно переписывал-упрощал и не проверял).


```cpp
%%cpp server_sol.c --ejudge-style
//%run gcc server_sol.c -o server_sol.exe
//%run ./server_sol.exe 30045

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <wait.h>
#include <sys/epoll.h>
#include <assert.h>


#define conditional_handle_error(stmt, msg) \
    do { if (stmt) { perror(msg " (" #stmt ")"); exit(EXIT_FAILURE); } } while (0)

//...

// должен работать до тех пор, пока в stop_fd не появится что-нибудь доступное для чтения
int server_main(int argc, char** argv, int stop_fd) {
    assert(argc >= 2);
    
    //...
    // int socket_fd = ....
    
    // создаем специальную штуку, чтобы ждать события из двух файловых дескрипторов разом: из сокета и из останавливающего дескриптора
    int epoll_fd = epoll_create1(0);
    {
        int fds[] = {stop_fd, socket_fd, -1};
        for (int* fd = fds; *fd != -1; ++fd) {
            struct epoll_event event = {
                .events = EPOLLIN | EPOLLERR | EPOLLHUP, 
                .data = {.fd = *fd}
            };
            epoll_ctl(epoll_fd, EPOLL_CTL_ADD, *fd, &event);
        }
    }

    while (true) {
        // Ждем события в epoll_fd (произойдет при появлении данных в stop_fd или socket_fd)
        struct epoll_event event;
        int epoll_ret = epoll_wait(epoll_fd, &event, 1, 1000); // Читаем события из epoll-объект (то есть из множества файловых дескриптотров, по которым есть события)
        if (epoll_ret <= 0) {
            continue;
        }
        // Если пришло событие из stop_fd - пора останавливаться
        if (event.data.fd == stop_fd) {
            break;
        }
        // Иначе пришло событие из socket_fd и accept
        // отработает мгновенно, так как уже подождали в epoll
        int fd = accept(socket_fd, NULL, NULL);
        // ... а тут обрабатываем соединение
        shutdown(fd, SHUT_RDWR);
        close(fd);
    }
    
    close(epoll_fd);

    shutdown(socket_fd, SHUT_RDWR);
    close(socket_fd);

    return 0;
}


static int stop_pipe_fds[2] = {-1, -1};

static void stop_signal_handler(int signum) {
    write(stop_pipe_fds[1], "X", 1); // Самая первая запись одного символа в пайп пройдет всегда успешно, так как буффер пуст. 
}

int main(int argc, char** argv) {
    // Идея такая: в момент прихода терминирующего сигнала запишем что-то в пайп
    pipe(stop_pipe_fds);
    fcntl(stop_pipe_fds[1], F_SETFL, fcntl(stop_pipe_fds[1], F_GETFL, 0) | O_NONBLOCK); // Делаем запись неблокирующей, чтобы никогда не зависнуть в хендлере (даже если придет 100500 сигналов)
    
    // Пусть при получении указанных сигналов, что-нибудь запишется в пайп
    int signals[] = {SIGINT, SIGTERM, 0};
    for (int* signal = signals; *signal; ++signal) {
        sigaction(*signal, &(struct sigaction){.sa_handler = stop_signal_handler, .sa_flags = SA_RESTART}, NULL);
    }
    
    int ret = server_main(argc, argv, stop_pipe_fds[0]);
    
    close(stop_pipe_fds[0]);
    close(stop_pipe_fds[1]]);
    return ret;
}
```


```python

```


```python

```


```python

```
