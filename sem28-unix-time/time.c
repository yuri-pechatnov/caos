// %%cpp time.c
// %run gcc -fsanitize=address time.c -lpthread -o time_c.exe
// %run ./time_c.exe

#define _BSD_SOURCE
#define _GNU_SOURCE  // для strptime

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/time.h>
#include <assert.h>
#include <string.h>

// Я не уверен, что так делать норм
time_t as_utc_timestamp(struct tm timeTm) {
    time_t timestamp = mktime(&timeTm); // mktime распарсит как локальное время, даже если tm_gmtoff в 0 сбросить
    //               ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓ Извращение, чтобы получить нормальный таймстемп UTC
    return timestamp + timeTm.tm_gmtoff; // mktime выставит tm_gmtoff в соответствии с текущей таймзоной
}

int main() {
    { // (1)
        struct timespec spec = {0}; 
        clock_gettime(CLOCK_REALTIME, &spec);
        
        time_t timestamp = spec.tv_sec;
        struct tm local_tm = {0};
        localtime_r(&timestamp, &local_tm);
        
        char time_str[100]; 
        size_t time_len = strftime(time_str, sizeof(time_str), "%Y.%m.%d %H:%M:%S", &local_tm);
        time_len += snprintf(time_str + time_len, sizeof(time_str) - time_len, ".%09ld", spec.tv_nsec);
        time_len += strftime(time_str + time_len, sizeof(time_str) - time_len, " %Z", &local_tm);
        printf("(1) Current time: %s\n", time_str);
    }
    
    { // (2)
        const char* utc_time = "2020.08.15 12:48:06";
        
        struct tm local_tm = {0};
        strptime(utc_time, "%Y.%m.%d %H:%M:%S", &local_tm); // распарсит как локальное время
        
        time_t timestamp = as_utc_timestamp(local_tm); 
        localtime_r(&timestamp, &local_tm);
        
        char time_str[100]; 
        size_t time_len = strftime(time_str, sizeof(time_str), "%Y.%m.%d %H:%M:%S%z", &local_tm);
        printf("(2) Recovered time by strptime: %s (given utc time: %s)\n", time_str, utc_time);
    }
    
    { // (3)
        time_t timestamps[] = {1589227667, 840124800, -1};
        for (time_t* timestamp = timestamps; *timestamp != -1; ++timestamp) {
            struct tm local_time = {0};
            localtime_r(timestamp, &local_time);
            char time_str[100]; 
            size_t time_len = strftime(time_str, sizeof(time_str), "%Y.%m.%d %H:%M:%S", &local_time);
            printf("(3) Timestamp %ld -> %s\n", *timestamp, time_str);
        }
    }

    return 0;
}

