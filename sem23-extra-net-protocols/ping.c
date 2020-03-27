// %%cpp ping.c
// %run gcc -Wall -Werror -fsanitize=thread ping.c -lpthread -o ping.exe
// %# Чтобы использовать SOCK_RAW нужны capabilities для исполняемого файла
// %run echo $PASSWORD | sudo -S setcap cap_net_raw,cap_net_admin+eip ./ping.exe 2>/dev/null
// %run timeout 5 ./ping.exe ya.ru

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define MAXPACKET   4096    
#define DATALEN 64     

const char* const pretty_icmp_type[] = {
    "Echo Reply", "ICMP 1", "ICMP 2", "Dest Unreachable", "Source Quench", "Redirect", "ICMP 6", 
    "ICMP 7", "Echo Request", "ICMP 9", "ICMP 10", "Time Exceeded", "Parameter Problem",
    "Timestamp", "Timestamp Reply", "Info Request", "Info Reply"
};

uint64_t time_ns() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000000000L + ts.tv_nsec;
}

int check_sum(u_short * addr, int len) {
    u_short *w = addr;
    int sum = 0;
    while (len > 1) {
        sum += *w++;
        len -= 2;
    }
    sum += (len == 1) ? *(u_char *)w : 0;    
    sum = (sum >> 16) + (sum & 0xffff); 
    return ~((sum >> 16) + sum);
}

typedef struct {
    int socket_fd;
    struct sockaddr* whereto;
    int ident;
} send_ping_forever_args_t;

void send_ping(send_ping_forever_args_t* arg, int icmp_seq_no) {
    static u_char outpack[MAXPACKET];
    memset(outpack, 0, sizeof(outpack));
    struct icmp *icp = (struct icmp *) outpack;
    icp->icmp_type = ICMP_ECHO;
    icp->icmp_code = 0;
    icp->icmp_cksum = 0;
    icp->icmp_seq = icmp_seq_no;
    icp->icmp_id = arg->ident;     
    *(uint64_t*)icp->icmp_data = time_ns(); 
    
    icp->icmp_cksum = check_sum((u_short*)(void*)icp, DATALEN);
    int res = sendto(arg->socket_fd, outpack, DATALEN, 0, arg->whereto, sizeof(struct sockaddr_in));
    assert(res == DATALEN);
}

void send_ping_forever(send_ping_forever_args_t* arg) {
    for (int icmp_seq_no = 0; 1; ++icmp_seq_no) {
        send_ping(arg, icmp_seq_no);
        sleep(1);
    }
}

void parse_and_print(u_char* buf, int length, struct sockaddr_in* from, int ident) {
    struct ip *ip = (struct ip *) buf; // откусываем ip-заголовок
    int hlen = ip->ip_hl << 2;
    length -= hlen;
    assert(length >= ICMP_MINLEN);
    struct icmp *icp = (struct icmp *)(buf + hlen);
    
    if (icp->icmp_id != ident)
        return; // эти запросы отправили точно не мы

    // обратите внимание, что собственные отправленные запросы мы тоже получаем на вход
    printf("%d bytes from %s: icmp_type=%d (%s) icmp_seq=%d icmp_code=%d time=%d ns\n",
        length, inet_ntoa(from->sin_addr), icp->icmp_type, pretty_icmp_type[icp->icmp_type], 
           icp->icmp_seq, icp->icmp_code, (int)(time_ns() - (*(uint64_t *)icp->icmp_data)));
}


int main(int argc, char **argv) {
    assert(argc == 2);
    
    int ident = getpid() & 0xFFFF;
    
    struct hostent *hp = gethostbyname(argv[1]);
    assert(hp && hp->h_addrtype == AF_INET);
    
    struct sockaddr_in whereto = {.sin_family = hp->h_addrtype};
    memcpy(&whereto.sin_addr, hp->h_addr, hp->h_length);

    // struct protoent *proto = getprotobyname("icmp"); // use proto->p_proto // можно так вместо IPPROTO_ICMP
    // с PF_INET получается интересная комбинация, при отправке мы IP-хедер не указываем, а при получении получаем IP-хедер
    int socket_fd = socket(PF_INET, SOCK_RAW, IPPROTO_ICMP);
    assert(socket_fd >= 0);
    
    printf("PING %s (%s): %d data bytes\n", hp->h_name, inet_ntoa(whereto.sin_addr), DATALEN);  
    pthread_t thread;
    send_ping_forever_args_t args = {.socket_fd = socket_fd, .whereto = (struct sockaddr *)&whereto, .ident = ident};
    assert(pthread_create(&thread, NULL, (void* (*)(void*))send_ping_forever, (void*)&args) == 0);
      
    // Вечно получаем ответы
    while (1) { 
        u_char packet[MAXPACKET];
        struct sockaddr_in from;
        socklen_t fromlen = sizeof(from);
        int recv_size = recvfrom(socket_fd, packet, sizeof(packet), 0, (struct sockaddr*)&from, &fromlen);
        if (recv_size < 0) {
            assert(errno == EINTR);
            continue;
        }
        parse_and_print(packet, recv_size, &from, ident);
    }
}

