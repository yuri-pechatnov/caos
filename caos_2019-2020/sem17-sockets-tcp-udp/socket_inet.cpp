// %%cpp socket_inet.cpp
// %run gcc -DDEBUG socket_inet.cpp -o socket_inet.exe
// %run ./socket_inet.exe

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
            // last 2 arguments: struct sockaddr *src_addr, socklen_t *addrlen)
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

