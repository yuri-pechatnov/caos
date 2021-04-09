// %%cpp get_mac.c
// %run gcc -Wall -Werror get_mac.c -lpthread -o get_mac.exe
// %run ./get_mac.exe

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

