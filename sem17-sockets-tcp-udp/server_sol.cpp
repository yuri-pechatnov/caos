// %%cpp server_sol.cpp --ejudge-style
// %run gcc server_sol.cpp -o server_sol.exe
// %run ./server_sol.exe

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

static volatile sig_atomic_t is_signaled = 0;
static volatile sig_atomic_t has_connection = 0;
static volatile sig_atomic_t socket_fd;

const int BUFSIZE = 4096;


int send_data(int fd, char* msg, long msg_size) {
    int bytes_sent = 0;
    while (1) {
        int num = send(fd, msg + bytes_sent, msg_size - bytes_sent, MSG_NOSIGNAL);
        if (num == -1) {
            return 0;
        } else if (num == 0) {
            break;
        }
        bytes_sent += num;
    }
    return bytes_sent;
}

void write_file(int fd, int file_fd, int size) {
    char buf[size];
    int read_ret;
    while (1) {
        read_ret = read(file_fd, buf, size);
        if (read_ret == -1) {
            return;
        } else if (read_ret == 0) {
            break;
        }
        send_data(fd, buf, read_ret);
        size -= read_ret;
    }
}

static void handler(int signum) {
    if (signum == SIGINT || signum == SIGTERM) {
        is_signaled = 1;
    }
    if (!has_connection) {
        shutdown(socket_fd, SHUT_RDWR);
        close(socket_fd);
        exit(0);
    }
}

int main(int argc, char** argv) {
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        perror("socket failed");
        exit(-1);
    }

    struct sockaddr_in addr ={
            .sin_family=AF_INET,
            .sin_port=htons(atoi(argv[1])),
            .sin_addr={.s_addr = htonl(INADDR_LOOPBACK)}
    };

    int bind_ret = bind(socket_fd, (const struct sockaddr*)&addr, sizeof(addr));
    if (bind_ret < 0) {
        perror("bind failed");
        exit(-1);
    }

    int listen_ret = listen(socket_fd, SOMAXCONN);
    if (listen_ret < 0) {
        perror("listen failed");
        exit(-1);
    }
    struct sigaction action = {.sa_handler = handler, .sa_flags = SA_RESTART};
    sigaction(
            SIGTERM,
            &action,
            NULL);
    sigaction(
            SIGINT,
            &action,
            NULL);

    while (!is_signaled) {
        int fd = accept(socket_fd, NULL, NULL);
        if (fd == -1) {
            break;
        }
        has_connection = 1;
        char string[BUFSIZE];
        char filename[BUFSIZE];
        strcpy(filename, argv[2]);
        strcat(filename, "/");
        if (read(fd, string, BUFSIZE) <= 0) {
            break;
        }

        sscanf(string, "GET %s", filename + strlen(filename));

        if (access(filename, F_OK) == -1) {
            char msg1[] = "HTTP/1.1 404 Not Found\r\n";
            char msg2[] = "Content-Length: 0\r\n\r\n";
            send_data(fd,  msg1, strlen(msg1));
            send_data(fd,  msg2, strlen(msg2));
        } else if (access(filename, R_OK) == -1) {
            char msg1[] = "HTTP/1.1 403 Forbidden\r\n";
            char msg2[] = "Content-Length: 0\r\n\r\n";
            send_data(fd,  msg1, strlen(msg1));
            send_data(fd,  msg2, strlen(msg2));
        } else if (access(filename, X_OK) == -1) {
            int fd_to_read = open(filename, O_RDONLY);
            if (fd_to_read == -1) {
                perror("read failed");
                exit(-1);
            } else {
                struct stat s;
                stat(filename, &s);
                char msg1[] = "HTTP/1.1 200 OK\r\n";
                char msg2[BUFSIZE];
                sprintf(msg2, "Content-Length: %ld\r\n\r\n", s.st_size);
                send_data(fd, msg1, strlen(msg1));
                send_data(fd, msg2, strlen(msg2));
                write_file(fd, fd_to_read, s.st_size);
            }
            close(fd_to_read);
        } else {
            char msg1[] = "HTTP/1.1 200 OK\r\n";
            send_data(fd, msg1, strlen(msg1));
            pid_t pid = fork();
            if (pid == 0) {
                dup2(fd, 1);
                close(fd);
                execl(filename, filename, NULL);
                perror("exec error");
                exit(-1);
            } else if (pid < 0) {
                perror("couldn't fork");
                exit(-1);
            } else {
                waitpid(pid, 0, 0);
            }
        }

        shutdown(fd, SHUT_RDWR);
        close(fd);
        has_connection = 0;
    }

    shutdown(socket_fd, SHUT_RDWR);
    close(socket_fd);

    return 0;
}

// line without \n