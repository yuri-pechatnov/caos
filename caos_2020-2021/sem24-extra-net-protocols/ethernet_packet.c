// %%cpp ethernet_packet.c
// %run gcc -Wall -Werror ethernet_packet.c -lpthread -o ethernet_packet.exe
// %# Чтобы использовать SOCK_RAW нужны capabilities для исполняемого файла
// %run echo $PASSWORD | sudo -S setcap cap_net_raw,cap_net_admin+eip ./ethernet_packet.exe 2>/dev/null
// %run ./ethernet_packet.exe

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <linux/if_ether.h> // вот тут объявлен ethernet_header, там же есть struct ether_arp с ARP хедером
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
        // Можно использовать ETH_P_ARP, чтобы получать только ARP-пакеты
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
    } __attribute__((__packed__))
      request = {.request_id = 17171819, .value = 42424242}, 
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

