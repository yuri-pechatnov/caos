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


int main() {
    { 
        struct timespec spec = {0}; 
        clock_gettime(CLOCK_REALTIME, &spec); 
        time_t seconds = spec.tv_sec;
        struct tm local_time = {0};
        localtime_r(&seconds, &local_time);
        char time_str[100]; 
        size_t time_len = strftime(time_str, sizeof(time_str), "%Y.%m.%d %H:%M:%S", &local_time);
        time_len += snprintf(time_str + time_len, sizeof(time_str) - time_len, ".%09ld", spec.tv_nsec);
        time_len += strftime(time_str + time_len, sizeof(time_str) - time_len, " %Z", &local_time);
        printf("Current time: %s\n", time_str);
    }
    
    {
        const char* utc_time = "2020.08.15 12:48:06";
        struct tm local_time = {0};
        char time_str_recovered[100]; 
        // Я не уверен, что так делать норм
        strptime(utc_time, "%Y.%m.%d %H:%M:%S", &local_time); // распарсит как локальное время
        //                              ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓ Извращение, чтобы получить нормальный таймстемп UTC
        time_t tt = mktime(&local_time) + local_time.tm_gmtoff; // mktime распарсит как локальное время, даже если tm_gmtoff в 0 сбросить
        localtime_r(&tt, &local_time);
        size_t time_len = strftime(time_str_recovered, sizeof(time_str_recovered), "%Y.%m.%d %H:%M:%S%z", &local_time);
        printf("Recovered time by strptime: %s (given utc time: %s)\n", time_str_recovered, utc_time);
    }
    
    {
        time_t timestamps[] = {1589227667, 840124800, -1};
        for (time_t* timestamp = timestamps; *timestamp != -1; ++timestamp) {
            struct tm local_time = {0};
            localtime_r(timestamp, &local_time);
            char time_str[100]; 
            size_t time_len = strftime(time_str, sizeof(time_str), "%Y.%m.%d %H:%M:%S", &local_time);
            printf("Timestamp %ld -> %s\n", *timestamp, time_str);
        }
    }

    return 0;
}

