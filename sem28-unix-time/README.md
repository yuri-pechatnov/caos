


# Опрос для всех, кто зашел на эту страницу

Он не страшный, там всего два обязательных вопроса на выбор одного варианта из трёх. Извиняюсь за размер, но к сожалению студенты склонны игнорировать опросы :| 

Пытаюсь компенсировать :)

<a href="https://docs.google.com/forms/d/e/1FAIpQLSdUnBAae8nwdSduZieZv7uatWPOMv9jujCM4meBZcHlTikeXg/viewform?usp=sf_link"><img src="poll.png" width="100%"  align="left" alt="Опрос"></a>



# Работа со временем в С/С++

Поговорим о типах времени в C/C++ и функциях для получения текущего времени, парсинга из строк, сериализации в строки.

Меня всегда дико напрягало отсутствие одного хорошего типа времени, наличие времени в разных часовых поясах и куча разных типов сериализации. Постараюсь собрать полезную информацию в одном месте, чтобы жилось проще.

<table width=100%  > <tr>
    <th width=15%> <b>Видео с семинара &rarr; </b> </th>
    <th>
    <a href="???"><img src="video.jpg" width="320" 
   height="160" align="left" alt="Видео с семинара"></a>
    </th>
    <th> </th>
 </table>



Сегодня в программе:
* <a href="types_c" style="color:#856024"> Типы времени в C </a>
* <a href="funcs_c" style="color:#856024"> Функции для работы со временем в C </a>
* <a href="types_cpp" style="color:#856024"> Типы времени в C++ </a>
* <a href="funcs_cpp" style="color:#856024"> Функции для работы со временем в C++ </a>
* <a href="types_cpp" style="color:#856024"> Конвертация типов времени C/C++ </a>
<br><br>
* <a href="clocks_and_cpu" style="color:#856024"> Разные часы и процессорное время </a>
<br><br>
* <a href="problems" style="color:#856024"> Задачки для самостоятельного решения </a>

 


## <a name="types_c"></a> Типы времени в C

Что у нас есть?

Собственно типы времени
* `time_t` - целочисленный тип, в котором хранится количество секунд с начала эпохи. [man](https://www.opennet.ru/man.shtml?topic=time&category=2)
* `struct tm` - структурка в которой хранится год, месяц, ..., секунда [man](https://www.opennet.ru/cgi-bin/opennet/man.cgi?topic=ctime&category=3)
* `struct timeval` пара (секунды, миллисекунды) (с начала эпохи, если используется как момент времени) [man](https://www.opennet.ru/cgi-bin/opennet/man.cgi?topic=gettimeofday&category=2)
* `struct timespec` пара (секунды, наносекунды) [man](https://www.opennet.ru/man.shtml?topic=select&category=2&russian=)
* `struct timeb` - секунды, миллисекунды, таймзона+информация о летнем времени [man](https://www.opennet.ru/cgi-bin/opennet/man.cgi?topic=ftime&category=3) (Я ни разу не сталкивался, но и такая есть)

Часовой пояс
* `struct timezone` - [man](https://www.opennet.ru/cgi-bin/opennet/man.cgi?topic=gettimeofday&category=2)


## <a name="funcs_c"></a> Функции для работы с временем в C

До всего последующего хочется напомнить, что многие функции в C не потокобезопасны (если не заканчиваются на `_r`, что означает reentrant, ну и потокобезопасность). Поэтому, перед использованием, стоит посмотреть документацию.

Конвертация:
<table>
<tr>
  <th>Из чего\Во что</th>
  <th>time_t</th>
  <th>struct tm</th>
  <th>struct timeval</th>
  <th>struct timespec</th>
 
<tr> <td>time_t
  <td>=
  <td><a href="https://www.opennet.ru/cgi-bin/opennet/man.cgi?topic=ctime&category=3"><code>gmtime_r</code></a>/<a href="https://www.opennet.ru/cgi-bin/opennet/man.cgi?topic=ctime&category=3"><code>localtime_r</code></a>
  <td>{.tv_sec = x}
  <td>{.tv_sec = x}

<tr> <td>struct tm
  <td><a href="https://www.opennet.ru/cgi-bin/opennet/man.cgi?topic=ctime&category=3"><code>mktime</code></a> [1]
  <td>=
  <td>через time_t
  <td>через time_t

<tr> <td>struct timeval
  <td>x.tv_sec
  <td>через time_t
  <td>=
  <td>{.tv_sec = x.tv_sec, .tv_nsec = x.tv_usec * 1000}

<tr> <td>struct timespec
  <td>x.tv_sec
  <td>через time_t
  <td>{.tv_sec = x.tv_sec, .tv_usec = x.tv_nsec / 1000}
  <td>=

</table>

[1] - `mktime` неадекватно работает, когда у вас не локальное время. Подробности и как с этим жить - в примерах.

Получение:
* `time` - получить время как `time_t` [man](https://www.opennet.ru/man.shtml?topic=time&category=2)
* `clock_gettime` - получить время как `struct timespec` [man](https://www.opennet.ru/man.shtml?topic=clock_gettime&category=3&russian=2)
* `gettimeofday` - получить время как `struct timeval` [man](https://www.opennet.ru/cgi-bin/opennet/man.cgi?topic=settimeofday&category=2)

Парсинг:
* Если таймстемп - то просто читаем как число.
* `strptime` [man](https://www.opennet.ru/man.shtml?topic=strptime&category=3&russian=0) Не умеет во временные зоны, всегда локальную выставляет
* `getdate` [man](https://opennet.ru/man.shtml?topic=getdate&category=3) Не рекомендую, не очень умная функция.

Сериализация:
* Всегда можно просто записать таймстемп в секундах/миллисекундах.
* `strftime` - позволяет превратить struct tm в строку, используя printf-подобную форматную строку [man](https://www.opennet.ru/man.shtml?topic=strftime&category=3)

Арифметические операции:
* Их нет, все вручную?

Работа с часовыми поясами:
* Сериализация таймстемпа как локального или UTC времени - `localtime_t`/`gmtime_r`.
* Парсинг локального времени - `strptime`.
* Другие часовые пояса и парсинг human-readable строк c заданным часовым поясом только через установку локалей, переменных окружения. В общем избегайте этого


```

```


```
%%cpp time.c
%run gcc -fsanitize=address time.c -lpthread -o time_c.exe
%run ./time_c.exe

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
```


Run: `gcc -fsanitize=address time.c -lpthread -o time_c.exe`



Run: `./time_c.exe`


    Current time: 2020.05.13 22:30:53.083377747 MSK
    Recovered time by strptime: 2020.08.15 15:48:06+0300 (given utc time: 2020.08.15 12:48:06)
    Timestamp 1589227667 -> 2020.05.11 23:07:47
    Timestamp 840124800 -> 1996.08.15 20:00:00



```

```

## <a name="types_cpp"></a> Типы времени в C++

Для начала нам доступно все то же, что было в С.

Новые типы времени
* `std::tm = struct tm`, `std::time_t = struct tm` - типы старые, но способ написания новый :)
* `std::chrono::time_point` [doc](https://en.cppreference.com/w/cpp/chrono/time_point)
* `std::chrono::duration` [doc](https://en.cppreference.com/w/cpp/chrono/duration)


Скажу откровенно, добавились не самые удобные типы. Единственное, что сделано удобно - арифметика времени.

## <a name="funcs_cpp"></a> Функции для работы с временем в C++


Конвертация:
* `std::chrono::system_clock::to_time_t`, `std::chrono::system_clock::from_time_t`

Сериализация и парсинг:
* `std::get_time` / `std::put_time` - примерно то же самое, что `strftime` и `strptime` в C. Работают с `std::tm`. [doc](https://en.cppreference.com/w/cpp/io/manip/get_time)

Арифметические операции:
* Из коробки, обычными +/*



```
%%cpp time.cpp
%run clang++ -std=c++14 -fsanitize=address time.cpp -lpthread -o time_cpp.exe
%run ./time_cpp.exe

#include <iostream>
#include <sstream>
#include <locale>
#include <iomanip>
#include <chrono>
#include <time.h> // localtime_r

int main() {
    { // (0)
        using namespace std::literals;
        auto now = std::chrono::system_clock::now();
        std::time_t timestamp = std::chrono::system_clock::to_time_t(now);
        std::tm tmTime = {};
        timestamp = 1589401219;
        localtime_r(&timestamp, &tmTime); 
        uint64_t nowMs = (now.time_since_epoch() % 1s) / 1ms;
        std::cout << "(0) Current time: " 
                  << std::put_time(&tmTime, "%Y.%m.%d %H:%M:%S") 
                  << "." << std::setfill('0') << std::setw(3) << nowMs << " "
                  << std::put_time(&tmTime, "%z") << " "
                  << ", timestamp = " << timestamp << "'\n";
    }

    { // (1)
        std::string timeStr = "2011-Jan-18 23:12:34";
        
        std::tm t = {};
        
        std::istringstream ss{timeStr};
        ss.imbue(std::locale("en_US.utf-8"));
        ss >> std::get_time(&t, "%Y-%b-%d %H:%M:%S");
        
        if (ss.fail()) {
            std::cout << "(1) Parse failed\n";
        } else {
            std::cout << "(1) Parsed time '" << std::put_time(&t, "%Y.%m.%d %H:%M:%S %z") << "'"
                      << " from '" << timeStr << "''\n";
        }
    }
    
    { // (2)
        using namespace std::literals;
        auto chronoNow = std::chrono::system_clock::now();
        for (int i = 0; i < 2; ++i, chronoNow += 23h + 55min) {
            std::time_t now = std::chrono::system_clock::to_time_t(chronoNow);
            std::tm localTime = {};
            localtime_r(&now, &localTime); // кажись в C++ нет потокобезопасной функции
            std::cout << "(2) Composed time: " << std::put_time(&localTime, "%Y.%m.%d %H:%M:%S %z") << "\n";
        }
    }
    
    { // (3)
        using namespace std::literals;
        
        //std::string timeStr = "1977.01.11 22:35:22";
        std::string timeStr = "2020.05.13 23:02:38";
        
        std::tm t = {};
        std::istringstream ss(timeStr);
        ss.imbue(std::locale("en_US.utf-8"));
        ss >> std::get_time(&t, "%Y.%m.%d %H:%M:%S"); // read as UTC/GMT time
        
        std::cout << "(3) Original time: " << std::put_time(&t, "%Y.%m.%d %H:%M:%S %z") << "\n";
        if (ss.fail()) {
            std::cout << "(3) Parse failed\n";
        } else {
            std::time_t timestamp = std::mktime(&t); // превратит в таймстемп, как будто в &t локальное время
            std::cout << "(3) Original time after mktime: " << std::put_time(&t, "%Y.%m.%d %H:%M:%S %z") 
                      << ", time = " << timestamp << "\n";
            // В t после mktime подменился часовой пояс
            timestamp += t.tm_gmtoff; // Делаем time нормальным utc таймстемпом
            
            auto chronoInstant = std::chrono::system_clock::from_time_t(timestamp);
            chronoInstant += 23h + 55min;
            std::time_t anotherTimestamp = std::chrono::system_clock::to_time_t(chronoInstant);
            std::tm localTime = {};
            gmtime_r(&timestamp, &localTime); // вот эта фигня проинтерпретировала время как локальное
            std::tm anotherLocalTime = {};
            gmtime_r(&anotherTimestamp, &anotherLocalTime); 
            
            std::cout << "(3) Take '" 
                      << std::put_time(&localTime, "%Y.%m.%d %H:%M:%S %z") << "', add 23:55, and get '"
                      << std::put_time(&anotherLocalTime, "%Y.%m.%d %H:%M:%S %z") << "'\n";
        }
    }

    return 0;
}
```


Run: `clang++ -std=c++14 -fsanitize=address time.cpp -lpthread -o time_cpp.exe`



Run: `./time_cpp.exe`


    (0) Current time: 2020.05.13 23:20:19.014 +0300 , timestamp = 1589401219'
    (1) Parsed time '2011.01.18 23:12:34 +0000' from '2011-Jan-18 23:12:34''
    (2) Composed time: 2020.05.13 23:37:04 +0300
    (2) Composed time: 2020.05.14 23:32:04 +0300
    (3) Original time: 2020.05.13 23:02:38 +0000
    (3) Original time after mktime: 2020.05.13 23:02:38 +0300, time = 1589400158
    (3) Take '2020.05.13 23:02:38 +0000', add 23:55, and get '2020.05.14 22:57:38 +0000'


Стоит обратить внимание, что в С++ не навязывается местный часовой пояс при парсинге времени. Хорошо это или плохо - не знаю.





## <a name="clocks_and_cpu"></a> Разные часы и процессорное время

Процессорное время
* `clock_t` - целочисленный тип, в котором хранится время затраченное процессором на исполнение потока/программы. Измеряется в непонятных единицах, связанных с секундами через CLOCKS_PER_SEC. [man](https://www.opennet.ru/cgi-bin/opennet/man.cgi?topic=clock&category=3)


Тип часов
* `clockid_t` - тип часов [man](https://www.opennet.ru/cgi-bin/opennet/man.cgi?topic=clock_gettime&category=3)


```

```

# Черно-черный черновик доп семинара Яковлева про время


Ну и тут только то, что меня заинтересовало. Что возможно буду рассказывать в следующем году, если будет такая тема




Проблема 2039 года

man time
time_t




```
import time
time.time()
```




    1588590245.7464755




```
time.mktime((2020, 5, 4, 14, 5, 0, 0, 0, 0)) # год месяц день, ...
```




    1588590300.0




```
(
    time.mktime((2020, 5, 4, 14, 5, 0, 0, 0, 0)) - 
    time.mktime((2016, 5, 4, 14, 5, 0, 0, 0, 0))
) / 60 / 60 / 24
```




    1461.0




```
# вот заметны переводы времени летнее/зимнее...
(
    time.mktime((2012, 5, 4, 14, 5, 0, 0, 0, 0)) - 
    time.mktime((2008, 5, 4, 14, 5, 0, 0, 0, 0))
) / 60 / 60 / 24
```




    1460.9583333333333



iana - база данных временных зон


```
time.gmtime(1588590300.0)
```




    time.struct_time(tm_year=2020, tm_mon=5, tm_mday=4, tm_hour=11, tm_min=5, tm_sec=0, tm_wday=0, tm_yday=125, tm_isdst=0)




```
# tm_hour другой
time.localtime(1588590300.0)
```




    time.struct_time(tm_year=2020, tm_mon=5, tm_mday=4, tm_hour=14, tm_min=5, tm_sec=0, tm_wday=0, tm_yday=125, tm_isdst=0)



UTC - общее время. Как GMT но без переводов времени
linux хранит хардверное время в UTC
А windows в localtime

Поэтому при перезагрузках время может прыгать на 3 часа (если вы в Москве)

man 4 rtc - команды для работы с хардверными часами
  например, можно настроить пробуждение системы из режима сна в конкретное время
  взаимодействовать с часами может только root
  
Хардверное и системное время
```
  -> sudo hwclock
Пт 24 апр 2020 00:28:52  .356966 seconds
  -> date
Пн май  4 14:28:24 MSK 2020
```

--------

man 2 stat

struct timespec


```
time.clock_gettime(time.CLOCK_REALTIME)
```




    1588592356.5319774




```
time.clock_gettime(time.CLOCK_MONOTONIC)
```




    1153059.033746184




```
time.clock_gettime(time.CLOCK_PROCESS_CPUTIME_ID)
```




    1.731587002



sleep, nanosleep

timerfd


```

```
