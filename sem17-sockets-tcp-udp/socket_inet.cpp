// %%cpp socket_inet.cpp
// %run gcc -DDEBUG socket_inet.cpp -o socket_inet.exe
// %run ./socket_inet.exe
// %run diff socket_unix.cpp socket_inet.cpp  | grep -v "// %" | grep -e '>' -e '<' -C 1

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
        //close(socket_fd);
        sleep(3);
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

