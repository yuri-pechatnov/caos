

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
<br><br>
* <a href="clocks_and_cpu" style="color:#856024"> Разные часы и процессорное время </a>
* <a href="benchmarking" style="color:#856024"> Время для бенчмарков </a>
<br><br>
* <a href="sleep" style="color:#856024"> Как поспать? </a>
<br><br>
* <a href="problems" style="color:#856024"> Задачки для самостоятельного решения </a>

 


## <a name="types_c"></a> Типы времени в C

Что у нас есть?

Собственно типы времени
* `time_t` - целочисленный тип, в котором хранится количество секунд с начала эпохи. В общем таймстемп в секундах. [man](https://www.opennet.ru/man.shtml?topic=time&category=2)
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

[1] - `mktime` неадекватно работает, когда у вас не локальное время. Подробности и как с этим жить - в примерах. https://stackoverflow.com/questions/530519/stdmktime-and-timezone-info

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
  Прежде всего замечание: в рамках этого семинара считаем, что время в GMT = время в UTC.

* Сериализация таймстемпа как локального или UTC времени - `localtime_t`/`gmtime_r`.
* Парсинг локального времени - `strptime`.
* Другие часовые пояса и парсинг human-readable строк c заданным часовым поясом только через установку локалей, переменных окружения. В общем избегайте этого


```python
# В питоне примерно то же самое, что и в С
import time
print("* Таймстемп (time_t): ", time.time())
print("* Дата (struct tm): ", time.localtime(time.time()))
print("* Дата (struct tm): ", time.gmtime(time.time()), "(обращаем внимание на разницу в часовых поясах)")
print("* tm_gmtoff для local:", time.localtime(time.time()).tm_gmtoff, 
      "и для gm: ", time.gmtime(time.time()).tm_gmtoff, "(скрытое поле, но оно используется :) )")
print("* Дата human-readable (local): ", time.strftime("%Y.%m.%d %H:%M:%S %z", time.localtime(time.time())))
print("* Дата human-readable (gmt): ", time.strftime("%Y.%m.%d %H:%M:%S %z", time.gmtime(time.time())))
```


```cpp
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
```


```python

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



```cpp
%%cpp time.cpp
%run clang++ -std=c++14 -fsanitize=address time.cpp -lpthread -o time_cpp.exe
%run ./time_cpp.exe

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
```

Стоит обратить внимание, что в С++ не навязывается местный часовой пояс при парсинге времени. Хорошо это или плохо - не знаю.





## <a name="clocks_and_cpu"></a> Разные часы и процессорное время

[Проблема 2038 года](https://ru.wikipedia.org/wiki/Проблема_2038_года), связанная с переполнением 32-битного time_t. Просто обозначаю, что она есть.

[iana](https://www.iana.org/time-zones) - база данных временных зон.

Хардверные часы. Обычные кварцевые часы, для которых на материнской плате есть отдельная батарейка. Они не очень точные. А еще разные системы могут хранить там время по-разному. Поэтому при перезагрузках между ubuntu и windows время может прыгать на 3 часа (если выбрано Московское время).
```
  -> sudo hwclock
Пт 24 апр 2020 00:28:52  .356966 seconds
  -> date
Пн май  4 14:28:24 MSK 2020
```

Процессорное время:
* [C/C++: как измерять процессорное время / Хабр](https://habr.com/ru/post/282301/)
* `clock_t clock(void);` - время затраченное процессором на исполнение потока/программы. Измеряется в непонятных единицах, связанных с секундами через CLOCKS_PER_SEC. [man](https://www.opennet.ru/cgi-bin/opennet/man.cgi?topic=clock&category=3)
* `clock_gettime` c параметрами `CLOCK_PROCESS_CPUTIME_ID`, `CLOCK_THREAD_CPUTIME_ID` - процессорное время программы и потока.
* 


Тип часов
* `clockid_t` - тип часов [man](https://www.opennet.ru/cgi-bin/opennet/man.cgi?topic=clock_gettime&category=3)
* `CLOCK_MONOTONIC` - тип часов, который стоит отдельно выделить. Это монотонные часы, то есть время, которое они показывают всегда возрастает несмотря ни на какие переводы времени. Их правильно использовать для замеров интервалов времени.


```python
for time_type in (time.CLOCK_REALTIME, time.CLOCK_MONOTONIC, time.CLOCK_PROCESS_CPUTIME_ID):
    print(time.clock_gettime(time_type))
```


```python

```

## <a name="benchmarking"></a> Время для бенчмарков

#### Что измерять?
Стоит измерять процессорное время. В зависимости от того, делаете ли вы в измеряемой части программы системные вызовы или нет, имеет смысл измерять только пользовательское время или пользовательское и системное вместе.

#### Как измерять?

Чтобы замеры были максимально точными, стоит минимизировать влияние среды и максимизировать стабильность измерений. 

Какие есть способы повысить стабильность?

0. Повторить замер столько раз, сколько можете себе позволить по времени, и усреднить.
1. Увеличить минимальное время, которое шедулер гарантирует процессу, если он сам не отдает управления. Его можно увеличить до 1с.
2. Запускать бенчмарк на выделенном ядре. 
То есть запретить шедулеру запускать что-то еще на ядре, 
где будет работать бенчмарк, и его парном гипертрединговом.

А теперь подбробнее
1. `sudo sysctl -w kernel.sched_min_granularity_ns='999999999'` - выкручиваем квант времени шедулера.
2. В конфиге grub (`/etc/default/grub`) добавляем `isolcpu=2,3` (у меня это второе физическое ядро) в строку параметров запуска.
  <br> Обновляем grub. `sudo grub-mkconfig`, `sudo grub-mkconfig -o /boot/grub/grub.cfg`. Перезапускаем систему.
  <br> Теперь запускаем бенчмарк как `taskset 0x4 ./my_benchmark`. (4 == 1 << 2, 2 - номер виртуального ядра, на котором запускаем процесс)


#### Чем измерять?
* perf stat

perf вообще очень мощная штука, помимо бенчмаркинга позволяет профилировать программу, смотреть, какие функции сколько работают.

Устанавливается так:

```bash
$ sudo apt install linux-tools-$(uname -r) linux-tools-generic
$ echo -1 > /proc/sys/kernel/perf_event_paranoid # under `sudo -i`
```

* time



```bash
%%bash
exec 2>&1 ; set -o xtrace

perf stat sleep 1
time sleep 1
```

## <a name="sleep"></a> Как поспать?

`sleep`, `nanosleep` - просто поспать. <s>На практике</s> В хороших продовых проектах такие функции нужны редко, из-за того, что такие ожидания нельзя корректно прервать внешним событием. На деле, конечно, постоянно используется.

`timerfd` - позволяет создавать таймеры, которые при срабатывании будут приходить записями, которые можно прочесть из файлового дескриптора.

`select`, `epoll_wait` - одновременное ожидание по таймауту и по файловым дескрипторам.

`pthread_cond_timedwait` - одновременное ожидание по таймауту и условной переменной.

`sigtimedwait` - одновременное ожидание по таймауту и сигнала. (Лучше все-таки свести прием сигнала к чтению из файлового дескриптора и не использовать это.)



```python

```


```python

```


```python

```
