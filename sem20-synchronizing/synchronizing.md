```python
# look at tools/set_up_magics.ipynb
get_ipython().run_cell('# one_liner_str <too much code> \n')
None
```


    <IPython.core.display.Javascript object>


# Синхронизация потоков

<br>
<div style="text-align: right"> Спасибо ?? за участие в написании текста </div>
<br>


Сегодня в программе:
* <a href="#mutex" style="color:#856024">Мьютексы</a>
  <br> MUTEX ~ MUTual EXclusion
* <a href="#spinlock" style="color:#856024">Spinlock'и</a>
* <a href="#condvar" style="color:#856024">Condition variable (aka условные переменные)</a>
* <a href="#spinlock" style="color:#856024">Spinlock'и</a>
*
  
  


<a href="#hw" style="color:#856024">Комментарии к ДЗ</a>

[Ридинг Яковлева](https://github.com/victor-yacovlev/mipt-diht-caos/tree/master/practice/mutex-condvar-atomic)

# <a name="mutex"></a> Mutex


```cpp
%%cpp mutex.c
%run gcc -fsanitize=thread mutex.c -lpthread -o mutex.exe # вспоминаем про санитайзеры
%run ./mutex.exe

#define _GNU_SOURCE 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <pthread.h>
#include <stdatomic.h>

const char* log_prefix(const char* func, int line) {
    struct timespec spec; clock_gettime(CLOCK_REALTIME, &spec); long long current_msec = spec.tv_sec * 1000L + spec.tv_nsec / 1000000;
    static _Atomic long long start_msec_storage = -1; long long start_msec = -1; if (atomic_compare_exchange_strong(&start_msec_storage, &start_msec, current_msec)) start_msec = current_msec;
    long long delta_msec = current_msec - start_msec;
    static __thread char prefix[100]; sprintf(prefix, "%lld.%03lld %13s():%d [tid=%ld]", delta_msec / 1000, delta_msec % 1000, func, line, syscall(__NR_gettid));
    return prefix;
}
#define log_printf_impl(fmt, ...) { time_t t = time(0); dprintf(2, "%s: " fmt "%s", log_prefix(__FUNCTION__, __LINE__), __VA_ARGS__); }
#define log_printf(...) log_printf_impl(__VA_ARGS__, "")

// thread-aware assert
#define ta_assert(stmt) if (stmt) {} else { log_printf("'" #stmt "' failed"); exit(EXIT_FAILURE); }


typedef enum {
    VALID_STATE = 0,
    INVALID_STATE = 1
} state_t;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // protects state
state_t current_state = VALID_STATE;

void thread_safe_func() {
    // all function is critical section, protected by mutex
    pthread_mutex_lock(&mutex); // try comment lock&unlock out and look at result
    ta_assert(current_state == VALID_STATE);
    current_state = INVALID_STATE; // do some work with state. 
    sched_yield();
    current_state = VALID_STATE;
    pthread_mutex_unlock(&mutex);
}

// Возвращаемое значение потока (~код возврата процесса) -- любое машинное слово.
static void* thread_func(void* arg) 
{
    int i = (char*)arg - (char*)NULL;
    log_printf("  Thread %d started\n", i);
    for (int j = 0; j < 10000; ++j) {
        thread_safe_func();
    }
    log_printf("  Thread %d finished\n", i);
    return NULL;
}

int main()
{
    log_printf("Main func started\n");
    const int threads_count = 2;
    pthread_t threads[threads_count];
    for (int i = 0; i < threads_count; ++i) {
        log_printf("Creating thread %d\n", i);
        ta_assert(pthread_create(&threads[i], NULL, thread_func, (char*)NULL + i) == 0);
    }
    for (int i = 0; i < threads_count; ++i) {
        ta_assert(pthread_join(threads[i], NULL) == 0); 
        log_printf("Thread %d joined\n", i);
    }
    log_printf("Main func finished\n");
    return 0;
}
```


Run: `gcc -fsanitize=thread mutex.c -lpthread -o mutex.exe # вспоминаем про санитайзеры`



Run: `./mutex.exe`


    0.000          main():61 [tid=7110]: Main func started
    0.001          main():65 [tid=7110]: Creating thread 0
    0.032   thread_func():51 [tid=7112]:   Thread 0 started
    0.033          main():65 [tid=7110]: Creating thread 1
    0.046   thread_func():51 [tid=7113]:   Thread 1 started
    1.305   thread_func():55 [tid=7113]:   Thread 1 finished
    1.340   thread_func():55 [tid=7112]:   Thread 0 finished
    1.340          main():70 [tid=7110]: Thread 0 joined
    1.340          main():70 [tid=7110]: Thread 1 joined
    1.340          main():72 [tid=7110]: Main func finished


# <a name="spinlock"></a> Spinlock


[spinlock в стандартной библиотеке](https://linux.die.net/man/3/pthread_spin_init)


```cpp
%%cpp spinlock.c
%run gcc -fsanitize=thread -std=c11 spinlock.c -lpthread -o spinlock.exe
%run ./spinlock.exe

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/time.h>
#include <pthread.h>
#include <stdatomic.h> //!

const char* log_prefix(const char* func, int line) {
    struct timespec spec; clock_gettime(CLOCK_REALTIME, &spec); long long current_msec = spec.tv_sec * 1000L + spec.tv_nsec / 1000000;
    static _Atomic long long start_msec_storage = -1; long long start_msec = -1; if (atomic_compare_exchange_strong(&start_msec_storage, &start_msec, current_msec)) start_msec = current_msec;
    long long delta_msec = current_msec - start_msec;
    static __thread char prefix[100]; sprintf(prefix, "%lld.%03lld %13s():%d [tid=%ld]", delta_msec / 1000, delta_msec % 1000, func, line, syscall(__NR_gettid));
    return prefix;
}
#define log_printf_impl(fmt, ...) { time_t t = time(0); dprintf(2, "%s: " fmt "%s", log_prefix(__FUNCTION__, __LINE__), __VA_ARGS__); }
#define log_printf(...) log_printf_impl(__VA_ARGS__, "")

// thread-aware assert
#define ta_assert(stmt) if (stmt) {} else { log_printf("'" #stmt "' failed"); exit(EXIT_FAILURE); }


typedef enum {
    VALID_STATE = 0,
    INVALID_STATE = 1
} state_t;

_Atomic int lock = 0; // protects state
state_t current_state = VALID_STATE;

void sl_lock(_Atomic int* lock) {
    int expected = 0;
    // atomic_compare_exchange_weak can change `expected`!
    while (!atomic_compare_exchange_weak(lock, &expected, 1)) {
        expected = 0;
    }
}

void sl_unlock(_Atomic int* lock) {
    atomic_fetch_sub(lock, 1);
}

void thread_safe_func() {
    // all function is critical section, protected by mutex
    sl_lock(&lock); // try comment lock&unlock out and look at result
    ta_assert(current_state == VALID_STATE);
    current_state = INVALID_STATE; // do some work with state. 
    sched_yield(); // increase probability of fail of incorrect lock realisation
    current_state = VALID_STATE;
    sl_unlock(&lock);
}

// Возвращаемое значение потока (~код возврата процесса) -- любое машинное слово.
static void* thread_func(void* arg) 
{
    int i = (char*)arg - (char*)NULL;
    log_printf("  Thread %d started\n", i);
    for (int j = 0; j < 10000; ++j) {
        thread_safe_func();
    }
    log_printf("  Thread %d finished\n", i);
    return NULL;
}

int main()
{
    log_printf("Main func started\n");
    const int threads_count = 2;
    pthread_t threads[threads_count];
    for (int i = 0; i < threads_count; ++i) {
        log_printf("Creating thread %d\n", i);
        ta_assert(pthread_create(&threads[i], NULL, thread_func, (char*)NULL + i) == 0);
    }
    for (int i = 0; i < threads_count; ++i) {
        ta_assert(pthread_join(threads[i], NULL) == 0); 
        log_printf("Thread %d joined\n", i);
    }
    log_printf("Main func finished\n");
    return 0;
}
```


Run: `gcc -fsanitize=thread -std=c11 spinlock.c -lpthread -o spinlock.exe`



Run: `./spinlock.exe`


    0.000          main():73 [tid=7099]: Main func started
    0.005          main():77 [tid=7099]: Creating thread 0
    0.141   thread_func():63 [tid=7101]:   Thread 0 started
    0.141          main():77 [tid=7099]: Creating thread 1
    0.149   thread_func():63 [tid=7102]:   Thread 1 started
    0.589   thread_func():67 [tid=7101]:   Thread 0 finished
    0.589          main():82 [tid=7099]: Thread 0 joined
    0.829   thread_func():67 [tid=7102]:   Thread 1 finished
    0.829          main():82 [tid=7099]: Thread 1 joined
    0.829          main():84 [tid=7099]: Main func finished


# <a name="condvar"></a> Condition variable


```cpp
%%cpp condvar.c
%run gcc -fsanitize=thread condvar.c -lpthread -o condvar.exe
%run (for i in $(seq 0 100000); do echo -n "$i " ; done) | ./condvar.exe > out.txt
//%run cat out.txt

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <pthread.h>
#include <stdatomic.h>

const char* log_prefix(const char* func, int line) {
    struct timespec spec; clock_gettime(CLOCK_REALTIME, &spec); long long current_msec = spec.tv_sec * 1000L + spec.tv_nsec / 1000000;
    static _Atomic long long start_msec_storage = -1; long long start_msec = -1; if (atomic_compare_exchange_strong(&start_msec_storage, &start_msec, current_msec)) start_msec = current_msec;
    long long delta_msec = current_msec - start_msec;
    static __thread char prefix[100]; sprintf(prefix, "%lld.%03lld %13s():%d [tid=%ld]", delta_msec / 1000, delta_msec % 1000, func, line, syscall(__NR_gettid));
    return prefix;
}
#define log_printf_impl(fmt, ...) { time_t t = time(0); dprintf(2, "%s: " fmt "%s", log_prefix(__FUNCTION__, __LINE__), __VA_ARGS__); }
#define log_printf(...) log_printf_impl(__VA_ARGS__, "")

// thread-aware assert
#define ta_assert(stmt) if (stmt) {} else { log_printf("'" #stmt "' failed"); exit(EXIT_FAILURE); }

#define queue_max_size 5

struct {
    // рекомендую порядок записи переменных:
    pthread_mutex_t mutex; // мьютекс
    pthread_cond_t condvar; // переменная условия (если нужна)
    
    // все переменные защищаемые мьютексом
    int data[queue_max_size];
    int begin; // [begin, end) 
    int end;
} queue;

void queue_init() {
    pthread_mutex_init(&queue.mutex, NULL);
    pthread_cond_init(&queue.condvar, NULL);
    queue.begin = queue.end = 0;
}

void queue_push(int val) {
    pthread_mutex_lock(&queue.mutex); // try comment lock&unlock out and look at result
    while (queue.begin + queue_max_size == queue.end) {
        pthread_cond_wait(&queue.condvar, &queue.mutex); // mutex in unlocked inside this func
    }
    _Bool was_empty = (queue.begin == queue.end);
    queue.data[queue.end++ % queue_max_size] = val;
    pthread_mutex_unlock(&queue.mutex);
    
    if (was_empty) {
        pthread_cond_signal(&queue.condvar); // notify if there was nothing and now will be elements
    }
}

int queue_pop() {
    pthread_mutex_lock(&queue.mutex); // try comment lock&unlock out and look at result
    while (queue.begin == queue.end) {
        pthread_cond_wait(&queue.condvar, &queue.mutex); // mutex in unlocked inside this func
    }
    if (queue.end - queue.begin == queue_max_size) {
        pthread_cond_signal(&queue.condvar); // notify if buffer was full and now will have free space
    }
    int val = queue.data[queue.begin++ % queue_max_size];
    if (queue.begin >= queue_max_size) {
        queue.begin -= queue_max_size;
        queue.end -= queue_max_size;
    }
    pthread_mutex_unlock(&queue.mutex);
    return val;
}

static void* producer_func(void* arg) 
{
    int val;
    while (scanf("%d", &val) > 0) {
        queue_push(val);
        //nanosleep(&(struct timespec) {.tv_nsec = 1000000}, NULL); // 1ms
    }
    queue_push(-1);
    return NULL;
}

static void* consumer_func(void* arg) 
{
    int val;
    while ((val = queue_pop()) >= 0) {
        printf("'%d', ", val);
    }
    return NULL;
}

int main()
{
    queue_init();
    
    log_printf("Main func started\n");
    
    pthread_t producer_thread;
    log_printf("Creating producer thread\n");
    ta_assert(pthread_create(&producer_thread, NULL, producer_func, NULL) == 0);
    
    pthread_t consumer_thread;
    log_printf("Creating producer thread\n");
    ta_assert(pthread_create(&consumer_thread, NULL, consumer_func, NULL) == 0);
    
    ta_assert(pthread_join(producer_thread, NULL) == 0); 
    log_printf("Producer thread joined\n");
    
    ta_assert(pthread_join(consumer_thread, NULL) == 0); 
    log_printf("Consumer thread joined\n");
    
    log_printf("Main func finished\n");
    return 0;
}
```


Run: `gcc -fsanitize=thread condvar.c -lpthread -o condvar.exe`



Run: `(for i in $(seq 0 100000); do echo -n "$i " ; done) | ./condvar.exe > out.txt`


    0.000          main():102 [tid=6723]: Main func started
    0.006          main():105 [tid=6723]: Creating producer thread
    0.040          main():109 [tid=6723]: Creating producer thread
    9.308          main():113 [tid=6723]: Producer thread joined
    9.315          main():116 [tid=6723]: Consumer thread joined
    9.315          main():118 [tid=6723]: Main func finished



```python

```


```python

```

# <a name="hw"></a> Комментарии к ДЗ

* 


```python

```


```python

```
