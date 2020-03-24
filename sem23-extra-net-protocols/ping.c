// %%cpp ping.c
// %run gcc -Wall -Werror -fsanitize=thread ping.c -lpthread -o ping.exe
// %run echo $PASSWORD | sudo -S setcap cap_net_raw,cap_net_admin+eip ./ping.exe
// %run ./ping.exe localhost

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

#define MAXPACKET   4096    /* max packet size */
const int datalen = 64 - 8;        /* How much data */
int ident;

uint64_t timespec_to_ns(struct timespec *ts) {
    return ts->tv_sec * 1000000000L + ts->tv_nsec;
} 

uint64_t time_ns() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return timespec_to_ns(&ts);
}

char* pretty_icmp_type(int t) {
    static char *ttab[] = {
        "Echo Reply", "ICMP 1", "ICMP 2", "Dest Unreachable", "Source Quench", "Redirect", "ICMP 6", 
        "ICMP 7", "Echo Request", "ICMP 9", "ICMP 10", "Time Exceeded", "Parameter Problem",
        "Timestamp", "Timestamp Reply", "Info Request", "Info Reply"
    };
    assert(0 <= t && t <= 16);
    return ttab[t];
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

void send_ping(int socket_fd, struct sockaddr* whereto, int icmp_seq_no) {
    static u_char outpack[MAXPACKET];
    struct icmp *icp = (struct icmp *) outpack;
    icp->icmp_type = ICMP_ECHO;
    icp->icmp_code = 0;
    icp->icmp_cksum = 0;
    icp->icmp_seq = icmp_seq_no;
    icp->icmp_id = ident;     
    u_char* data = icp->icmp_data; 
    *(uint64_t*)data = time_ns(); data += sizeof(uint64_t);
    
    for(int i = 0; i < datalen; i++)
        *data++ = i;
    
    int cc = datalen;  
    icp->icmp_cksum = check_sum((u_short*)(void*)icp, cc);
    int res = sendto(socket_fd, outpack, cc, 0, whereto, sizeof(struct sockaddr));
    if(res != cc )  {
        if(res < 0)  perror("sendto");
        printf("ping: wrote %d chars, ret=%d\n", cc, res);
        fflush(stdout);
    }

}

typedef struct {
    int socket_fd;
    struct sockaddr* whereto;
} send_ping_forever_args_t;

void send_ping_forever(send_ping_forever_args_t* arg) 
{
    int icmp_seq_no = 0;
    while (1) {
        send_ping(arg->socket_fd, arg->whereto, icmp_seq_no++);
        sleep(1);
    }
}


void parse_and_print(u_char* buf, int cc, struct sockaddr_in * from) {
    struct ip *ip = (struct ip *) buf;
    int hlen = ip->ip_hl << 2;
    cc -= hlen;
    assert(cc >= ICMP_MINLEN);
    struct icmp *icp = (struct icmp *)(buf + hlen);
    
    if (icp->icmp_id != ident)
        return;
    if (icp->icmp_type == ICMP_ECHOREPLY) {
        printf("%d bytes from %s: icmp_type=%d  icmp_seq=%d time=%d ns\n", cc,
            inet_ntoa(from->sin_addr), ICMP_ECHOREPLY,
            icp->icmp_seq, (int)(time_ns() - (*(uint64_t *)icp->icmp_data)));
    } else {
        printf("%d bytes from %s: icmp_type=%d (%s) icmp_code=%d\n",
            cc, inet_ntoa(from->sin_addr),
            icp->icmp_type, pretty_icmp_type(icp->icmp_type), icp->icmp_code);

    }
}


int main(int argc, char **argv) {
    assert(argc == 2);
    
    ident = getpid() & 0xFFFF;
    
    struct hostent *hp = gethostbyname(argv[1]);
    assert(hp);
    
    struct sockaddr_in whereto = {.sin_family = hp->h_addrtype};
    memcpy(&whereto.sin_addr, hp->h_addr, hp->h_length);
    char* hostname = hp->h_name;

    struct protoent *proto = getprotobyname("icmp");
    assert(proto);
    
    int socket_fd = socket(AF_INET, SOCK_RAW, proto->p_proto);
    assert(socket_fd >= 0);
    
    if(whereto.sin_family == AF_INET) {
        printf("PING %s (%s): %d data bytes\n", hostname,
          inet_ntoa(whereto.sin_addr), datalen);    /* DFM */
    } else {
        printf("PING %s: %d data bytes\n", hostname, datalen );
    }
    
    pthread_t thread;
    send_ping_forever_args_t args = {.socket_fd = socket_fd, .whereto = (struct sockaddr *)&whereto};
    assert(pthread_create(&thread, NULL, (void* (*)(void*))send_ping_forever, (void*)&args) == 0);
      
    while (1) { 
        u_char packet[MAXPACKET];
        int len = sizeof (packet);
        struct sockaddr_in from;
        socklen_t fromlen = sizeof(from);
        int cc;
        if ((cc = recvfrom(socket_fd, packet, len, 0, (struct sockaddr*)&from, &fromlen)) < 0) {
            if( errno == EINTR )
                continue;
            perror("ping: recvfrom");
            continue;
        }
        parse_and_print(packet, cc, &from);
       
    }
    /*NOTREACHED*/
}

