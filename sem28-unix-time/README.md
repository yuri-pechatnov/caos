


# Черно-черный черновик доп семинара Яковлева про время


Ну и тут только то, что меня заинтересовало. Что возможно буду рассказывать в следующем году, если будет такая тема




Проблема 2039 года

man time
time_t




```python
import time
time.time()
```




    1588590245.7464755




```python
time.mktime((2020, 5, 4, 14, 5, 0, 0, 0, 0)) # год месяц день, ...
```




    1588590300.0




```python
(
    time.mktime((2020, 5, 4, 14, 5, 0, 0, 0, 0)) - 
    time.mktime((2016, 5, 4, 14, 5, 0, 0, 0, 0))
) / 60 / 60 / 24
```




    1461.0




```python
# вот заметны переводы времени летнее/зимнее...
(
    time.mktime((2012, 5, 4, 14, 5, 0, 0, 0, 0)) - 
    time.mktime((2008, 5, 4, 14, 5, 0, 0, 0, 0))
) / 60 / 60 / 24
```




    1460.9583333333333



iana - база данных временных зон


```python
time.gmtime(1588590300.0)
```




    time.struct_time(tm_year=2020, tm_mon=5, tm_mday=4, tm_hour=11, tm_min=5, tm_sec=0, tm_wday=0, tm_yday=125, tm_isdst=0)




```python
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


```python
time.clock_gettime(time.CLOCK_REALTIME)
```




    1588592356.5319774




```python
time.clock_gettime(time.CLOCK_MONOTONIC)
```




    1153059.033746184




```python
time.clock_gettime(time.CLOCK_PROCESS_CPUTIME_ID)
```




    1.731587002



sleep, nanosleep

timerfd


```python

```
