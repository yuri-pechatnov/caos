// %%cpp socket_unix.c
// %run gcc socket_unix.c -lpthread -o socket_unix.exe
// %run ./socket_unix.exe

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

