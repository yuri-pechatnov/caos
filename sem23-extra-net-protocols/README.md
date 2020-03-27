


# Протоколы в интернете


<br>
<div style="text-align: right"> Спасибо ??? за участие в написании текста </div>
<br>

### [Видео с семинара](https://www.youtube.com/watch?v=cwoQaLeFlvs&feature=youtu.be)
(в приличном качестве появится не сразу, youtube долго процессит)


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

^ кажется, на этой картинке есть некоторый обман https://habr.com/ru/post/227729/

**Сегодня в практической части программы**:
* <a href="#dns" style="color:#856024">Делаем запросы к dns: вручную составляем udp-датаграммы</a>
* <a href="#ping" style="color:#856024">Raw socket'ы + icmp -> реализуем простенькую утилиту ping</a>
* <a href="#ioctl" style="color:#856024">Получаем список сетевых интерфейсов и соответствующие адреса канального уровня (mac-адреса)  с помощью ioctl</a>
* <a href="#raw" style="color:#856024">Посылаем ethernet-пакет через SOCK_RAW'ище</a>
 
<a href="#hw" style="color:#856024">Комментарии к ДЗ</a>

[Ридинг Яковлева](https://github.com/victor-yacovlev/mipt-diht-caos/tree/master/practice/sockets-udp)


```

```

# <a name="dns"></a> Делаем запросы к dns: вручную составляем udp-датаграммы


```
from socket import socket, AF_INET, SOCK_DGRAM
import sys
import subprocess


sock = socket(AF_INET, SOCK_DGRAM)

def domen2q(s):
    ans = b""
    for part in s.split('.'):
        ans += len(part).to_bytes(length=1, byteorder="little") + bytes(part, encoding="ascii")
    return ans

for i, x in enumerate(["ya.ru", "ejudge.ru", "vk.com"]):
    ip_from_util = subprocess.check_output(["dig", "+short", x]).decode().split('\n')[0]
    x = x.strip()
    query = (
        i.to_bytes(length=2, byteorder="little") + b"\x01\x00\x00\x01\x00\x00\x00\x00\x00\x00" +
        domen2q(x) + b"\x00" +
        b"\x00\x01\x00\x01"
    )

    sock.sendto(query, ("8.8.8.8", 53))
    data, addr = sock.recvfrom(1024)
    ip_from_udp_request = ".".join(str(b) for b in data[-4:])
 
    print("From our request:", ip_from_udp_request, " from linux util: ", ip_from_util)

```

    From our request: 87.250.250.242  from linux util:  87.250.250.242
    From our request: 89.108.121.5  from linux util:  89.108.121.5
    From our request: 87.240.139.194  from linux util:  87.240.190.78


Один адрес расходится. В этом нет ничего страшного, так как у домена может быть несколько ip-адресов. А мы выбирали адреса практически случайно, так что могли легко вытянуть разные.

# <a name="ping"></a> Ping  

Я пытался сделать пример максимально минималистичным, поэтому в нем не реализовано правильно завершение при получении сигнала, а так же очень упрощена обработка ошибок. Ни в коем случае не делайте так же :)


```
%%cpp ping.c
%run gcc -Wall -Werror -fsanitize=thread ping.c -lpthread -o ping.exe
%# Чтобы использовать SOCK_RAW нужны capabilities для исполняемого файла
%run echo $PASSWORD | sudo -S setcap cap_net_raw,cap_net_admin+eip ./ping.exe 2>/dev/null
%run timeout 5 ./ping.exe ya.ru

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
```


Run: `gcc -Wall -Werror -fsanitize=thread ping.c -lpthread -o ping.exe`



Run: `echo $PASSWORD | sudo -S setcap cap_net_raw,cap_net_admin+eip ./ping.exe 2>/dev/null`



Run: `timeout 5 ./ping.exe ya.ru`


    PING ya.ru (87.250.250.242): 64 data bytes
    64 bytes from 87.250.250.242: icmp_type=0 (Echo Reply) icmp_seq=0 icmp_code=0 time=12463064 ns
    64 bytes from 87.250.250.242: icmp_type=0 (Echo Reply) icmp_seq=1 icmp_code=0 time=47367167 ns
    64 bytes from 87.250.250.242: icmp_type=0 (Echo Reply) icmp_seq=2 icmp_code=0 time=13276648 ns
    64 bytes from 87.250.250.242: icmp_type=0 (Echo Reply) icmp_seq=3 icmp_code=0 time=700816130 ns



```

```

# <a name="ioctl"></a> Получаем mac-адреса с помощью ioctl


```
%%cpp get_mac.c
%run gcc -Wall -Werror get_mac.c -lpthread -o get_mac.exe
%run ./get_mac.exe

#include <stdio.h>
#include <sys/ioctl.h>
#include <net/if.h> 
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <assert.h>
#include <sys/socket.h>
#include <arpa/inet.h>

void print_eth_interface(struct ifreq* ifr_ip, struct ifreq* ifr_hw, _Bool is_loopback) {
    printf("%10s mac=", ifr_ip->ifr_name);
    for (int i = 0; i < 6; ++i) {
        printf("%02x%s", (int)(unsigned char)ifr_hw->ifr_hwaddr.sa_data[i], i + 1 < 6 ? ":" : "");
    }
    printf(" ip=%s", inet_ntoa(((struct sockaddr_in*)&ifr_ip->ifr_addr)->sin_addr));
    printf(is_loopback ? " <- it is loopback\n" : "\n");
}

int main()
{
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    assert(sock != -1);

    char buf[1024];
    struct ifconf ifc = {
        .ifc_len = sizeof(buf),
        .ifc_buf = buf
    };
    assert(ioctl(sock, SIOCGIFCONF, &ifc) != -1);  // получаем список интерфейсов

    for (struct ifreq* it = ifc.ifc_req; it != ifc.ifc_req + (ifc.ifc_len / sizeof(struct ifreq)); ++it) {
        struct ifreq ifr = *it; // Получаем структуру с заполненным .ifr_name
        assert(ioctl(sock, SIOCGIFFLAGS, &ifr) == 0); // получаем флаги интерфейса по имени (.ifr_name)
        // поля ответов в ifrec лежат в union, поэтому читать ответ нужно после каждого применения ioctl https://www.opennet.ru/man.shtml?topic=netdevice&category=7&russian=
        _Bool is_loopback = (ifr.ifr_flags & IFF_LOOPBACK); // смотрим, является ли интерфейс loopback'ом (типа 127.0.0.1 только для ethernet)
        assert(ioctl(sock, SIOCGIFHWADDR, &ifr) == 0); // получаем аппаратный адрес устройства (mac)
        print_eth_interface(it, &ifr, is_loopback);   
    }    
}
```


Run: `gcc -Wall -Werror get_mac.c -lpthread -o get_mac.exe`



Run: `./get_mac.exe`


            lo mac=00:00:00:00:00:00 ip=127.0.0.1 <- it is loopback
        enp0s3 mac=08:00:27:67:e5:f7 ip=10.0.2.15
        enp0s8 mac=08:00:27:34:68:f0 ip=10.0.3.15



```
# Сравниваем с выводом системной утилиты
!ifconfig | grep encap -A 1
```

    enp0s3    Link encap:Ethernet  HWaddr 08:00:27:67:e5:f7  
              inet addr:10.0.2.15  Bcast:10.0.2.255  Mask:255.255.255.0
    --
    enp0s8    Link encap:Ethernet  HWaddr 08:00:27:34:68:f0  
              inet addr:10.0.3.15  Bcast:10.0.3.255  Mask:255.255.255.0
    --
    lo        Link encap:Local Loopback  
              inet addr:127.0.0.1  Mask:255.0.0.0


# <a name="raw"></a> Посылаем ethernet-пакет через SOCK_RAW'ище

Вдохновился названием статьи: https://habr.com/ru/company/smart_soft/blog/184430/

В данном примере мы будем почти полностью генерировать ethernet пакет: с хедером, но без окончания с чексуммой (предполагаю, что окончание добавляется на аппаратном уровне).


```
%%cpp ethernet_packet.c
%run gcc -Wall -Werror ethernet_packet.c -lpthread -o ethernet_packet.exe
%# Чтобы использовать SOCK_RAW нужны capabilities для исполняемого файла
%run echo $PASSWORD | sudo -S setcap cap_net_raw,cap_net_admin+eip ./ethernet_packet.exe 2>/dev/null
%run ./ethernet_packet.exe

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <linux/if_ether.h> // вот тут объявлен ethernet_header, там же есть struct ether_arp
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

typedef struct {
    char s[3 * 6];
} mac_str;

mac_str mac_to_str(u_char* mac) {
    mac_str ms;
    sprintf(ms.s, "%02x:%02x:%02x:%02x:%02x:%02x", (int)mac[0], (int)mac[1], (int)mac[2], (int)mac[3], (int)mac[4], (int)mac[5]);
    return ms;
}

int main(int argc, char *argv[])
{
    // http://man7.org/linux/man-pages/man7/packet.7.htm
    int sock = socket(
        AF_PACKET, // используем низкоуровневые адреса sockaddr_ll
        SOCK_RAW,  // сырые пакеты
        htons(ETH_P_ALL) // мы хотим получать сообщения всех протоколов (система может фильтровать и доставлять только некоторые)
    );
    assert(sock != -1);

    struct ifreq ifr;
    strcpy((char*)&ifr.ifr_name, "lo");           
    ioctl(sock, SIOCGIFINDEX, &ifr); // получаем индекс интерфейса
    struct sockaddr_ll device = {
        .sll_ifindex = ifr.ifr_ifindex,
        .sll_halen = ETH_ALEN, // длина адреса (=6 в ethernet) 
        .sll_addr = {0, 0, 0, 0, 0, 0} // loopback (но, кажется, тут может быть любой мусор)
    };
   
    struct {
        struct ether_header ethernet_header; // по дефолту будет наполнен нулями, а это loopback адрес, чего и хочется в этом примере
        // Вот тут может начинаться хедер протокола более высокого уровня
        uint64_t request_id; // идентификатор, чтобы узнать наш пакет, среди всех проходящих пакетов
        uint64_t value; // имитация полезной нагрузки
    } request = {.request_id = 17171819, .value = 42424242}, 
      response;

    int sendto_res = sendto(sock, &request, sizeof(request), 0,
                            (struct sockaddr*)&device, sizeof(device));
    assert(sendto_res != -1);
    
    while (true) {
        int recv_result = recv(sock, &response, sizeof(response), 0);
        assert(recv_result != -1);
        if (response.request_id == request.request_id) { //[1]
            printf("Hey, I got it! response.value = %" PRIu64 ", eth_type = %#06x, src_mac = %s, dst_mac = %s\n", 
                   response.value, response.ethernet_header.ether_type,
                   mac_to_str(response.ethernet_header.ether_shost).s, mac_to_str(response.ethernet_header.ether_dhost).s);
            break; //[1]
        } //[1]
    }
    close(sock);
    return 0;
}

```


Run: `gcc -Wall -Werror ethernet_packet.c -lpthread -o ethernet_packet.exe`



Run: `echo $PASSWORD | sudo -S setcap cap_net_raw,cap_net_admin+eip ./ethernet_packet.exe 2>/dev/null`



Run: `./ethernet_packet.exe`


    Hey, I got it! response.value = 42424242, eth_type = 000000, src_mac = 00:00:00:00:00:00, dst_mac = 00:00:00:00:00:00


Если закомментировать [1], то можно понаблюдать за всеми пакетами

# <a name="hw"></a> Комментарии к ДЗ

* dns: перепишите код с семинара на С :)
* Пошлите и получите правильный ethernet пакет.


```

```


```

```
