// %%cpp socket_inet6.cpp
// %run gcc -DDEBUG socket_inet6.cpp -o socket_inet6.exe
// %run ./socket_inet6.exe
// %run diff socket_inet.cpp socket_inet6.cpp  | grep -v "// %" | grep -e '>' -e '<' -C 1

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
#define log_printf(fmt, ...) \
    { time_t t = time(0); dprintf(2, "%s : " fmt, extract_t(ctime(&t)), __VA_ARGS__); }

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
    hints.ai_family = ai_family;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
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
            fprintf(stderr, "try_connect_by_name: Try ai_family=%d ai_socktype=%d ai_protocol=%d host=%s, serv=%s\n", 
                    rp->ai_family, rp->ai_socktype, rp->ai_protocol, hbuf, sbuf);
        sfd = socket(rp->ai_family, /*rp->ai_socktype*/ SOCK_STREAM, /*rp->ai_protocol*/ 0);
        if (sfd == -1)
            continue;
        if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1)
            break;                  /* Success */
        close(sfd);
    }

    if (rp == NULL) {               /* No address succeeded */
        fprintf(stderr, "Could not connect\n");
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(result);  
    fprintf(stderr, "try_connect_by_name: return fd = %d\n", sfd);
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
        log_printf("client finished\n%s", "");
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
        log_printf("server finished\n%s", "");
        return 0;
    }
    int status;
    assert(waitpid(pid_1, &status, 0) != -1);
    assert(waitpid(pid_2, &status, 0) != -1);
    return 0;
}

