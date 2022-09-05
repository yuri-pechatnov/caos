// %%cpp ping.c
// %run gcc -Wall -Werror -fsanitize=thread ping.c -lpthread -o ping.exe
// %# Чтобы использовать SOCK_RAW нужны capabilities для исполняемого файла
// %run echo $PASSWORD | sudo -S setcap cap_net_raw,cap_net_admin+eip ./ping.exe 2>/dev/null
// %run timeout 1 ./ping.exe ya.ru

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define MAXPACKET   4096     

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

int parse_and_print(u_char* buf, int length, struct sockaddr_in* from) {
    struct ip *ip = (struct ip *) buf; // откусываем ip-заголовок
    int hlen = ip->ip_hl << 2;
    length -= hlen;
    assert(length >= ICMP_MINLEN);
    struct icmp *icp = (struct icmp *)(buf + hlen);
    
    printf("From %s get %d bytes, icmp_type=%d (%s) icmp_seq=%d icmp_code=%d\n",
        inet_ntoa(from->sin_addr), length, icp->icmp_type, pretty_icmp_type[icp->icmp_type], 
        icp->icmp_seq, icp->icmp_code);
    return icp->icmp_type;
}


int main(int argc, char **argv) {
    assert(argc == 2);
    
    struct hostent *hp = gethostbyname(argv[1]);
    assert(hp && hp->h_addrtype == AF_INET);
    
    struct sockaddr_in whereto = {.sin_family = hp->h_addrtype, .sin_port = 33342};
    memcpy(&whereto.sin_addr, hp->h_addr, hp->h_length);

    // struct protoent *proto = getprotobyname("icmp"); // use proto->p_proto // можно так вместо IPPROTO_ICMP
    // с PF_INET получается интересная комбинация, при отправке мы IP-хедер не указываем, а при получении получаем IP-хедер
    int icmp_receiving_sock = socket(PF_INET, SOCK_RAW, IPPROTO_ICMP);
    assert(icmp_receiving_sock >= 0);
    
    // Почти обычный UDP-сокет
    int udp_sending_sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    
    int ttl = 1;
    while (true) {
        // Вот тут мы говорим сокету, что в его IP заголовках надо бы в TTL писать ttl
        assert(0 == setsockopt(udp_sending_sock, 0, IP_TTL, &ttl, sizeof(ttl))); 

        printf("Send packed to %s (%s), ttl = %d\n", hp->h_name, inet_ntoa(whereto.sin_addr), ttl); 
        int val = 0;
        sendto(udp_sending_sock, &val, sizeof(val), 0,
            (struct sockaddr *)&whereto, sizeof(struct sockaddr_in));

        u_char packet[MAXPACKET];
        struct sockaddr_in from;
        socklen_t fromlen = sizeof(from);
        int recv_size = recvfrom(icmp_receiving_sock, packet, sizeof(packet), 0, (struct sockaddr*)&from, &fromlen);
        if (recv_size < 0) {
            assert(errno == EINTR);
            return 0;
        }
        int icmp_type = parse_and_print(packet, recv_size, &from);
        if (icmp_type == 11) { // time limit exceeded
            ++ttl;
        } else if (icmp_type == 3) { // Dest Unreachable
            break;
        }
        
        usleep(100000);
    }
}

