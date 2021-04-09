// %%cpp tmp.c
// %run gcc -Wall -Werror tmp.c -lpthread -o tmp.exe
// %run ./tmp.exe

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


int main()
{
    printf("%d", (int)(unsigned char)'\300');
    return 0;
}

