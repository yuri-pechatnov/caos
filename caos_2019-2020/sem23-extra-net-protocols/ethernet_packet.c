// %%cpp ethernet_packet.c
// %run gcc -Wall -Werror ethernet_packet.c -lpthread -o ethernet_packet.exe
// %run ./ethernet_packet.exe

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


struct some_header {
    uint16_t field1;
    uint8_t field2;
    uint8_t data[];
} __attribute__((__packed__));

struct some_tailer {
    uint8_t checksum;
} __attribute__((__packed__));

int main() {
    unsigned char buffer[100];
    
    struct some_header *header = (void*)buffer;
    header->field1 = 0xA0A0;
    header->field2 = 0xFF;
    
    const int data_len = 10;
    memset(header->data, 0x11, data_len);
    
    struct some_tailer *tailer = (void*)(header->data + data_len);
    tailer->checksum = 0x42;
    
    const int total_length = sizeof(struct some_header) + data_len + sizeof(struct some_tailer);
    
    printf("sizeof(some_header) = %d\n", (int)sizeof(struct some_header));
    for (int i = 0; i < total_length; ++i) {
        printf("%02X ", buffer[i]); 
    }
    return 0;
}

