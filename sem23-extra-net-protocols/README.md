


# Протоколы в интернете


<br>
<div style="text-align: right"> Спасибо ??? за участие в написании текста </div>
<br>

### [Видео с семинара ???]()


**Немного теории о часто упоминающихся протоколах**:
* [MAC](https://ru.wikipedia.org/wiki/Управление_доступом_к_среде) (media access control) - протокол канального уровня модели OSI, отвечающий за взаимодействие устройств общающихся через одну среду. На этом же уровне существуют mac-адреса - это уникальные идентификаторы устройств.
* [ARP](https://ru.wikipedia.org/wiki/ARP) - протокол сетевого уровня модели OSI. Протокол предназначен для перевода ip-адресов в mac-адреса.
* [ICMP](https://ru.wikipedia.org/wiki/ICMP), [ICMPv6](https://ru.wikipedia.org/wiki/ICMPv6) (Internet Control Message Protocol) - протокол передачи сообщений об ошибках и других исключительных ситуациях. Реализует высокоуровневую часть сетевого уровня модели OSI.
* [IPv4](https://ru.wikipedia.org/wiki/IPv4), [IPv6](https://ru.wikipedia.org/wiki/IPv6) - протоколы сетевого уровня модели OSI. Позволяют отправить пакет данных на другой конец земного шара, зная только ip-адрес получателя.
* [TCP](https://ru.wikipedia.org/wiki/Transmission_Control_Protocol), [UDP](https://ru.wikipedia.org/wiki/UDP) - потоковая и датаграммная передача данных. Работают поверх IP
* [DNS](https://ru.wikipedia.org/wiki/DNS) (Domain Name System) - распределенная система получения информации о доменах. В простейшем случае это получение ip-адреса по доменному имени. Работает поверх UDP (маленькие запросы-ответы) и TCP (большие ответы).
* [DHCP](https://ru.wikipedia.org/wiki/DHCP) (Dynamic Host Configuration Protocol) - протокол позволяющий хосту динамически получить себе  ip-адрес. Протокол клиент-серверный, поэтому в сети должен быть DHCP-сервер, который собственно будет выделять адрес. Работает поверх UDP: клиент посылает широковещательную датаграмму (указывая 0 в качестве своего ip-адреса, свой mac-адрес в качестве адреса устройства), в сервер в ответ отправляет предложение ip-адреса клиенту (отправка может быть направленной на mac-адрес, может быть широковещательной). Дальше следует еще подтверждающее сообщение.
* [HTTP](https://ru.wikipedia.org/wiki/HTTP) - (HyperText Transfer Protocol) - протокол прикладного уровня модели OSI. (Хотя тут все неоднозначно, так как он часто исползуется в качестве транспортного уровня, немного размазынне границы получаются.)
* POP3/IMAP/SMTP - протоколы прикладного уровня модели OSI. Про работу с электронной почтой.
* Telnet (teletype network) / SSH (Secure Shell) - протоколы прикладного уровня модели OSI. Реализуют терминальные интерфейсы. ssh еще шифрует трафик (+ имеет более широкий функционал, может использоваться в качестве транспортного уровня (например, если вы пользуетесь пробросом портов))
* FTP (File Transfer Protocol) - протокол прикладного уровня модели OSI. Используется для передачи файлов.

Когда речь заходит о передаче сообщения из точки A в точку B. То собственно пользовательское сообщение по пути сквозь пространство и уровни протоколов приобретает и теряет заголовки (headers) и окончания (trailers).

При передаче более низкому уровню сообщение заворачивается в сообщение более низкого уровня. Например ip-сообщение нужно передать другому узлу через ethernet сеть. На уровне ethernet заголовки ip-пакета никакого смысла не имеют. Поэтому ip-сообщение вместе с заголовками интерпретируется как полезная нагрузка. И эта нагрузка передается в составе ethernet-кадра. А в ethernet-кадрe уже свой заголовок и окончание.


![Протоколы](pic/protocols.png)

**Сегодня в практической части программы**:
* <a href="#ping" style="color:#856024">Raw socket'ы + icmp -> реализуем простенькую утилиту ping</a>


???
  * <a href="#???" style="color:#856024">???</a>
  

  
<a href="#hw" style="color:#856024">Комментарии к ДЗ</a>

[Ридинг Яковлева ???](https://github.com/victor-yacovlev/mipt-diht-caos/tree/master/practice/???)

# <a name="ping"></a> Ping  


```cpp
%%cpp ping.c
%run gcc -Wall -Werror -fsanitize=thread ping.c -lpthread -o ping.exe
%# Чтобы использовать SOCK_RAW нужны capabilities для исполняемого файла
%run echo $PASSWORD | sudo -S setcap cap_net_raw,cap_net_admin+eip ./ping.exe
%run ./ping.exe localhost

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
const int datalen = 64 - 8;     
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
        // обратите внимание, что собственные отправленные запросы мы тоже получаем на вход
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
    
    if (whereto.sin_family == AF_INET) {
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
            if (errno == EINTR)
                continue;
            perror("ping: recvfrom");
            continue;
        }
        parse_and_print(packet, cc, &from);
       
    }
    /*NOTREACHED*/
}
```


Run: `gcc -Wall -Werror -fsanitize=thread ping.c -lpthread -o ping.exe`



Run: `echo $PASSWORD | sudo -S setcap cap_net_raw,cap_net_admin+eip ./ping.exe`


    [sudo] password for pechatnov: 


Run: `./ping.exe localhost`


    PING localhost (127.0.0.1): 56 data bytes
    56 bytes from 127.0.0.1: icmp_type=8 (Echo Request) icmp_code=0
    56 bytes from 127.0.0.1: icmp_type=0  icmp_seq=0 time=376630 ns
    56 bytes from 127.0.0.1: icmp_type=8 (Echo Request) icmp_code=0
    56 bytes from 127.0.0.1: icmp_type=0  icmp_seq=1 time=139792 ns
    56 bytes from 127.0.0.1: icmp_type=8 (Echo Request) icmp_code=0
    56 bytes from 127.0.0.1: icmp_type=0  icmp_seq=2 time=356033 ns
    56 bytes from 127.0.0.1: icmp_type=8 (Echo Request) icmp_code=0
    56 bytes from 127.0.0.1: icmp_type=0  icmp_seq=3 time=365133 ns
    56 bytes from 127.0.0.1: icmp_type=8 (Echo Request) icmp_code=0
    56 bytes from 127.0.0.1: icmp_type=0  icmp_seq=4 time=233780 ns
    56 bytes from 127.0.0.1: icmp_type=8 (Echo Request) icmp_code=0
    56 bytes from 127.0.0.1: icmp_type=0  icmp_seq=5 time=198266 ns
    56 bytes from 127.0.0.1: icmp_type=8 (Echo Request) icmp_code=0
    56 bytes from 127.0.0.1: icmp_type=0  icmp_seq=6 time=270847 ns
    56 bytes from 127.0.0.1: icmp_type=8 (Echo Request) icmp_code=0
    56 bytes from 127.0.0.1: icmp_type=0  icmp_seq=7 time=236937 ns
    56 bytes from 127.0.0.1: icmp_type=8 (Echo Request) icmp_code=0
    56 bytes from 127.0.0.1: icmp_type=0  icmp_seq=8 time=96229 ns
    56 bytes from 127.0.0.1: icmp_type=8 (Echo Request) icmp_code=0
    56 bytes from 127.0.0.1: icmp_type=0  icmp_seq=9 time=89005 ns
    56 bytes from 127.0.0.1: icmp_type=8 (Echo Request) icmp_code=0
    56 bytes from 127.0.0.1: icmp_type=0  icmp_seq=10 time=99018 ns
    56 bytes from 127.0.0.1: icmp_type=8 (Echo Request) icmp_code=0
    56 bytes from 127.0.0.1: icmp_type=0  icmp_seq=11 time=88776 ns
    ^C



```python

```

# <a name="hw"></a> Комментарии к ДЗ

* 


```python

```


```python

```
