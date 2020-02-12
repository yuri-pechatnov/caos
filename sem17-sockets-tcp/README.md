```python
# look at tools/set_up_magics.ipynb
get_ipython().run_cell('# one_liner_str <too much code> \n')
None
```


    <IPython.core.display.Javascript object>


# Сокеты и tcp-сокеты в частности

<br>
<div style="text-align: right"> Спасибо <a href="https://github.com/???"> [пока еще никому :( ] </a> за участие в написании текста </div>
<br>

**Модель OSI**

 [Подробнее про уровни](https://zvondozvon.ru/tehnologii/model-osi)
    
1. Физический уровень (PHYSICAL)
2. Канальный уровень (DATA LINK)
3. Сетевой уровень (NETWORK)
4. Транспортный уровень (TRANSPORT) <- сокеты это интерфейсы вот этого уровня <br>
  Реализуются часто в ядре операционной системы <br>
  Еще стоит понимать, что транспортный уровень, предоставляя один интерфейс может иметь разные реализации. Например сокеты UNIX, в этом случае под транспортным уровнем нет сетевого, так как передача данных ведется внутри одной машины.
5. Сеансовый уровень (SESSION) (IMHO не нужен)
6. Уровень представления данных (PRESENTATION) (IMHO не нужен)
7. Прикладной уровень (APPLICATION)

Сегодня в программе:
* `socketpair` - <a href="#socketpair" style="color:#856024">аналог `pipe`</a>, но полученные дескрипторы обладают сокетными свойствами: файловый дескриптор работает и на чтение и на запись (соответственно этот "pipe" двусторонний), закрывать нужно с вызовом `shutdown`
* `socket` - функция создания сокета
  * <a href="#socket_unix" style="color:#856024">AF_UNIX</a> - сокет внутри системы. Адрес в данном случае - адрес файла сокета в файловой системе.
  * <a href="#socket_inet" style="color:#856024">AF_INET</a> - сокет для стандартных ipv4 соединений.
  * <a href="#socket_inet6" style="color:#856024">AF_INET6</a> - сокет для стандартных ipv6 соединений.
  
  
[Сайт с хорошими картинками про порядок низкоуровневых вызовов в клиентском и серверном приложении](http://support.fastwel.ru/AppNotes/AN/AN-0001.html#server_tcp_init)



<a href="#hw" style="color:#856024">Комментарии к ДЗ</a>

[Ридинг Яковлева](https://github.com/victor-yacovlev/mipt-diht-caos/tree/master/practice/sockets-tcp)


```python

```


```python

```

# <a name="socketpair"></a> socketpair в качестве pipe

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
    assert(socketpair(AF_UNIX, SOCK_STREAM, 0, fds.arr_fd) == 0);
    
    pid_t pid_1, pid_2;
    if ((pid_1 = fork()) == 0) {
        close(fds.fd_2);
        write_smth(fds.fd_1);
        shutdown(fds.fd_1, SHUT_RDWR); // important, try to comment out and look at time
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


     11:49:08 : Read 1000 bytes
     11:49:08 : Writing is done



```python

```

# <a name="socket_unix"></a> socket + AF_UNIX


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
        
        struct sockaddr_un addr = {.sun_family = AF_UNIX};
        strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);
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
        int connection_fd = accept(socket_fd, (struct sockaddr*)&peer_addr, &peer_addr_size);
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


     19:45:59 : Read 1000 bytes
     19:45:59 : server finished
     19:45:59 : client finished



```python

```

# <a name="socket_inet"></a> socket + AF_INET

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
        sleep(1);
        int socket_fd = socket(AF_INET, SOCK_STREAM, 0); // == connection_fd in this case
        conditional_handle_error(socket_fd == -1, "can't initialize socket");
     
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(PORT);
        struct hostent *hosts = gethostbyname("localhost"); // simple function but it is legacy. Prefer getaddrinfo
        conditional_handle_error(!hosts, "can't get host by name");
        memcpy(&addr.sin_addr, hosts->h_addr_list[0], sizeof(addr.sin_addr));

        int connect_ret = connect(socket_fd, (struct sockaddr*)&addr, sizeof(addr));
        conditional_handle_error(connect_ret == -1, "can't connect to unix socket");
        
        write_smth(socket_fd);
        log_printf("writing is done\n");
        shutdown(socket_fd, SHUT_RDWR); 
        close(socket_fd);
        log_printf("client finished\n");
        return 0;
    }
    if ((pid_2 = fork()) == 0) {
        // server
        int socket_fd = socket(AF_INET, SOCK_STREAM, 0); 
        conditional_handle_error(socket_fd == -1, "can't initialize socket");
        #ifdef DEBUG
        int reuse_val = 1;
        setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &reuse_val, sizeof(reuse_val));
        setsockopt(socket_fd, SOL_SOCKET, SO_REUSEPORT, &reuse_val, sizeof(reuse_val));
        #endif
        
        struct sockaddr_in addr = {.sin_family = AF_INET, .sin_port = htons(PORT)};
        // addr.sin_addr == 0, so we are ready to receive connections directed to all our addresses
        int bind_ret = bind(socket_fd, (struct sockaddr*)&addr, sizeof(addr)); 
        conditional_handle_error(bind_ret == -1, "can't bind to unix socket");
        
        int listen_ret = listen(socket_fd, LISTEN_BACKLOG);
        conditional_handle_error(listen_ret == -1, "can't listen to unix socket");

        struct sockaddr_in peer_addr = {0};
        socklen_t peer_addr_size = sizeof(struct sockaddr_in);
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


Run: `gcc -DDEBUG socket_inet.cpp -o socket_inet.exe`



Run: `./socket_inet.exe`


     11:34:54 : writing is done
     11:34:54 : Read 1000 bytes
     11:34:54 : server finished
     11:34:57 : client finished



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
    58c61
    <         int socket_fd = socket(AF_UNIX, SOCK_STREAM, 0); // == connection_fd in this case
    ---
    >         int socket_fd = socket(AF_INET, SOCK_STREAM, 0); // == connection_fd in this case
    60,63c63,71
    <         
    <         struct sockaddr_un addr = {.sun_family = AF_UNIX};
    <         strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);
    <         int connect_ret = connect(socket_fd, (const struct sockaddr*)&addr, sizeof(addr.sun_path));
    ---
    >      
    >         struct sockaddr_in addr;
    >         addr.sin_family = AF_INET;
    >         addr.sin_port = htons(PORT);
    >         struct hostent *hosts = gethostbyname("localhost"); // simple function but it is legacy. Prefer getaddrinfo
    >         conditional_handle_error(!hosts, "can't get host by name");
    >         memcpy(&addr.sin_addr, hosts->h_addr_list[0], sizeof(addr.sin_addr));
    > 
    >         int connect_ret = connect(socket_fd, (struct sockaddr*)&addr, sizeof(addr));
    66a75
    >         log_printf("writing is done\n");
    68c77,78
    <         close(socket_fd);
    ---
    >         //close(socket_fd);
    >         sleep(3);
    74c84
    <         int socket_fd = socket(AF_UNIX, SOCK_STREAM, 0); 
    ---
    >         int socket_fd = socket(AF_INET, SOCK_STREAM, 0); 
    75a86,90
    >         #ifdef DEBUG
    >         int reuse_val = 1;
    >         setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &reuse_val, sizeof(reuse_val));
    >         setsockopt(socket_fd, SOL_SOCKET, SO_REUSEPORT, &reuse_val, sizeof(reuse_val));
    >         #endif
    77,80c92,94
    <         unlink(SOCKET_PATH); // remove socket if exists, because bind fail if it exists
    <         struct sockaddr_un addr = {.sun_family = AF_UNIX};
    <         strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);
    <         int bind_ret = bind(socket_fd, (struct sockaddr*)&addr, sizeof(addr.sun_path)); 
    ---
    >         struct sockaddr_in addr = {.sin_family = AF_INET, .sin_port = htons(PORT)};
    >         // addr.sin_addr == 0, so we are ready to receive connections directed to all our addresses
    >         int bind_ret = bind(socket_fd, (struct sockaddr*)&addr, sizeof(addr)); 
    86,87c100,101
    <         struct sockaddr_un peer_addr = {0};
    <         socklen_t peer_addr_size = sizeof(struct sockaddr_un);
    ---
    >         struct sockaddr_in peer_addr = {0};
    >         socklen_t peer_addr_size = sizeof(struct sockaddr_in);
    97d110
    <         unlink(SOCKET_PATH);



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

# <a name="socket_inet6"></a> socket + AF_INET6 + getaddrinfo

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
     11:54:55 : Read 1000 bytes
     11:54:55 : server finished
     11:54:55 : client finished



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
    61,73c110
    <         int socket_fd = socket(AF_INET, SOCK_STREAM, 0); // == connection_fd in this case
    <         conditional_handle_error(socket_fd == -1, "can't initialize socket");
    <      
    <         struct sockaddr_in addr;
    <         addr.sin_family = AF_INET;
    <         addr.sin_port = htons(PORT);
    <         struct hostent *hosts = gethostbyname("localhost"); // simple function but it is legacy. Prefer getaddrinfo
    <         conditional_handle_error(!hosts, "can't get host by name");
    <         memcpy(&addr.sin_addr, hosts->h_addr_list[0], sizeof(addr.sin_addr));
    < 
    <         int connect_ret = connect(socket_fd, (struct sockaddr*)&addr, sizeof(addr));
    <         conditional_handle_error(connect_ret == -1, "can't connect to unix socket");
    <         
    ---
    >         int socket_fd = try_connect_by_name("localhost", PORT, AF_INET6);
    75d111
    <         log_printf("writing is done\n");
    77,78c113
    <         //close(socket_fd);
    <         sleep(3);
    ---
    >         close(socket_fd);
    84c119
    <         int socket_fd = socket(AF_INET, SOCK_STREAM, 0); 
    ---
    >         int socket_fd = socket(AF_INET6, SOCK_STREAM, 0); 
    92c127
    <         struct sockaddr_in addr = {.sin_family = AF_INET, .sin_port = htons(PORT)};
    ---
    >         struct sockaddr_in6 addr = {.sin6_family = AF_INET6, .sin6_port = htons(PORT)};
    100,101c135,136
    <         struct sockaddr_in peer_addr = {0};
    <         socklen_t peer_addr_size = sizeof(struct sockaddr_in);
    ---
    >         struct sockaddr_in6 peer_addr = {0};
    >         socklen_t peer_addr_size = sizeof(struct sockaddr_in6);



```python

```


```python

```


```python

```


```python

```

# <a name="hw"></a> Комментарии к ДЗ

* Пока нет


```python

```


```python

```


```python

```


```python

```
