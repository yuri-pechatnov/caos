// %%cpp time.cpp
// %run clang++ -std=c++14 -fsanitize=address time.cpp -lpthread -o time_cpp.exe
// %run ./time_cpp.exe

#include <iostream>
#include <sstream>
#include <locale>
#include <iomanip>
#include <chrono>
#include <time.h> // localtime_r

time_t as_utc_timestamp(struct tm t) {
    time_t timestamp = mktime(&t); // mktime распарсит как локальное время, даже если tm_gmtoff в 0 сбросить
    //               ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓ Извращение, чтобы получить нормальный таймстемп UTC
    return timestamp + t.tm_gmtoff; // mktime выставит tm_gmtoff в соответствии с текущей таймзоной
}

int main() {
    { // (0)
        using namespace std::literals;
        auto nowChrono = std::chrono::system_clock::now();
        std::time_t timestamp = std::chrono::system_clock::to_time_t(nowChrono);
        std::tm timeTm = {};
        timestamp = 1589401219;
        localtime_r(&timestamp, &timeTm); 
        uint64_t nowMs = (nowChrono.time_since_epoch() % 1s) / 1ms;
        std::cout << "(0) Current time: " 
                  << std::put_time(&timeTm, "%Y.%m.%d %H:%M:%S") 
                  << "." << std::setfill('0') << std::setw(3) << nowMs << " "
                  << std::put_time(&timeTm, "%z") << " "
                  << ", timestamp = " << timestamp << "'\n";
    }

    { // (1)
        std::string timeStr = "2011-Jan-18 23:12:34";
        
        std::tm timeTm = {};
        
        std::istringstream timeStrStream{timeStr};
        timeStrStream.imbue(std::locale("en_US.utf-8"));
        timeStrStream >> std::get_time(&timeTm, "%Y-%b-%d %H:%M:%S");
        
        if (timeStrStream.fail()) {
            std::cout << "(1) Parse failed\n";
        } else {
            std::cout << "(1) Parsed time '" << std::put_time(&timeTm, "%Y.%m.%d %H:%M:%S %z") << "'"
                      << " from '" << timeStr << "''\n";
        }
    }
    
    { // (2)
        using namespace std::literals;
        auto nowChrono = std::chrono::system_clock::now();
        for (int i = 0; i < 2; ++i, nowChrono += 23h + 55min) {
            std::time_t nowTimestamp = std::chrono::system_clock::to_time_t(nowChrono);
            std::tm localTm = {};
            localtime_r(&nowTimestamp, &localTm); // кажись в C++ нет потокобезопасной функции
            std::cout << "(2) Composed time: " << std::put_time(&localTm, "%Y.%m.%d %H:%M:%S %z") << "\n";
        }
    }
    
    { // (3)
        using namespace std::literals;
        
        std::string timeStr = "1977.01.11 22:35:22";
        
        std::tm timeTm = {};
        std::istringstream timeStrStream{timeStr};
        timeStrStream >> std::get_time(&timeTm, "%Y.%m.%d %H:%M:%S"); // read as UTC/GMT time
        
        std::cout << "(3) Original time: " << std::put_time(&timeTm, "%Y.%m.%d %H:%M:%S %z") << "\n";
        if (timeStrStream.fail()) {
            std::cout << "(3) Parse failed\n";
        } else {
            std::time_t timestamp = as_utc_timestamp(timeTm);
            auto instantChrono = std::chrono::system_clock::from_time_t(timestamp);
            instantChrono += 23h + 55min;
            std::time_t anotherTimestamp = std::chrono::system_clock::to_time_t(instantChrono);
            std::tm localTm = {};
            gmtime_r(&timestamp, &localTm); // вот эта фигня проинтерпретировала время как локальное
            std::tm anotherLocalTm = {};
            gmtime_r(&anotherTimestamp, &anotherLocalTm); 
            
            std::cout << "(3) Take '" 
                      << std::put_time(&localTm, "%Y.%m.%d %H:%M:%S %z") << "', add 23:55, and get '"
                      << std::put_time(&anotherLocalTm, "%Y.%m.%d %H:%M:%S %z") << "'\n";
        }
    }

    return 0;
}

