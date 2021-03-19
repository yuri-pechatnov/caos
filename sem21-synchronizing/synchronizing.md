


# Межпоточная и межпроцессная синхронизация

<br>
<div style="text-align: right"> Спасибо <a href="https://github.com/SyrnikRebirth">Сове Глебу</a> и <a href="https://github.com/Disadvantaged">Голяр Димитрису</a> за участие в написании текста </div>
<br>

<p><a href="https://www.youtube.com/watch?" target="_blank">
    <h3>Видеозапись семинара</h3> 
</a></p>


[Ридинг Яковлева](https://github.com/victor-yacovlev/mipt-diht-caos/tree/master/practice/posix_ipc)


В [соседнем ноутбуке](./quiz.md) лежит quiz с задачками на поиск ошибок в асинхронности с сигналами и примитивами межпоточной синхронизации. Рекомендую порешать, так как значительное количество задачек составлено на основе ошибок студентов. (А еще там есть ответы. Но не подглядывайте в них сразу, сначала хорошо подумайте.)


Сегодня в программе:
* Межпоточная синхронизация
  * <a href="#mutex" style="color:#856024">Мьютексы</a>
  <br> MUTEX ~ MUTual EXclusion (если вдруг кто забыл)
  * <a href="#spinlock" style="color:#856024">Spinlock'и и атомики</a>
  <br> [Атомики в С на cppreference](https://ru.cppreference.com/w/c/atomic)
  <br> <a href="#c_atomic_life" style="color:#856024">Atomic в C и как с этим жить </a> (раздел от <a href="https://github.com/nikvas2000">Николая Васильева</a>)
  <br> <details> <summary>Про compare_exchange_weak vs compare_exchange_strong</summary> <p>
https://stackoverflow.com/questions/4944771/stdatomic-compare-exchange-weak-vs-compare-exchange-strong
<br>The weak compare-and-exchange operations may fail spuriously, that is, return false while leaving the contents of memory pointed to by expected before the operation is the same that same as that of the object and the same as that of expected after the operation. [ Note: This spurious failure enables implementation of compare-and-exchange on a broader class of machines, e.g., loadlocked store-conditional machines. A consequence of spurious failure is that nearly all uses of weak compare-and-exchange will be in a loop. 
</p>
</details>
  
  * <a href="#condvar" style="color:#856024">Condition variable (aka условные переменные)</a>

* Межпроцессная синхронизация (Inter Process Communications / IPC)
  * Именованные каналы, сокеты, ...
  * <a href="#mmap" style="color:#856024">`mmap` для IPC</a>
  <br> Используем примитивы межпоточной синхронизации для межпроцессной. Через разделяемую память создаем правильные мьютексы.
  <br> [Ссылка про правильные мьютексы](https://linux.die.net/man/3/pthread_mutexattr_init)  
  * <a href="#shm" style="color:#856024">Объекты разделяемой памяти POSIX</a>
  <br>Это почти то же самое, что и обычные файлы, но у них ортогональное пространство имен и они не сохраняются на диск.
  <br>Вызовы `shm_open` (открывает/создает объект разделяемой памяти, аналогично `open`) и `shm_unlink` (удаляет ссылку на объект, аналогично `unlink`)  
  <br>[Документашка](https://www.opennet.ru/man.shtml?topic=shm_open&category=3&russian=0). [Отличия от `open`](https://stackoverflow.com/questions/24875257/why-use-shm-open)
  <br><br>
  * Семафоры
  <br><a href="#sem_anon" style="color:#856024">Неименованные</a>
  <br><a href="#sem_named" style="color:#856024">Именованные</a>
  * <a href="#sem_signal" style="color:#856024">Сочетаемость семафоров и сигналов</a> 

  
  
<a href="#hw" style="color:#856024">Комментарии к ДЗ</a>


# Межпоточная синхронизация

Кратко основное

## <a name="mutex"></a> Мьютексы


```cpp
%%cpp mutex.c
%# Санитайзер отслеживает небезопасный доступ 
%# к одному и тому же участку в памяти из разных потоков
%# (а так же другие небезопасные вещи). 
%# В таких задачах советую всегда использовать
%run gcc -fsanitize=thread mutex.c -lpthread -o mutex.exe
%run ./mutex.exe

#define _GNU_SOURCE 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <stdint.h>
#include <stdatomic.h>

// log_printf - макрос для отладочного вывода, добавляющий время со старта программы, имя функции и номер строки
uint64_t start_time_msec; void  __attribute__ ((constructor)) start_time_setter() { struct timespec spec; clock_gettime(CLOCK_MONOTONIC, &spec); start_time_msec = spec.tv_sec * 1000L + spec.tv_nsec / 1000000; }
const char* log_prefix(const char* func, int line) {
    struct timespec spec; clock_gettime(CLOCK_MONOTONIC, &spec); int delta_msec = spec.tv_sec * 1000L + spec.tv_nsec / 1000000 - start_time_msec;
    const int max_func_len = 13; static __thread char prefix[100]; 
    sprintf(prefix, "%d.%03d %*s():%-3d [tid=%ld]", delta_msec / 1000, delta_msec % 1000, max_func_len, func, line, syscall(__NR_gettid));
    return prefix;
}
#define log_printf_impl(fmt, ...) { time_t t = time(0); dprintf(2, "%s: " fmt "%s", log_prefix(__FUNCTION__, __LINE__), __VA_ARGS__); }
// Format: <time_since_start> <func_name>:<line> : <custom_message>
#define log_printf(...) log_printf_impl(__VA_ARGS__, "")


#define fail_with_strerror(code, msg) do { char err_buf[1024]; strerror_r(code, err_buf, sizeof(err_buf));\
    log_printf(msg " (From err code: %s)\n", err_buf);  exit(EXIT_FAILURE);} while (0)

// thread-aware assert
#define ta_verify(stmt) do { if (stmt) break; fail_with_strerror(errno, "'" #stmt "' failed."); } while (0)

// verify pthread call
#define pt_verify(pthread_call) do { int code = (pthread_call); if (code == 0) break; \
    fail_with_strerror(code, "'" #pthread_call "' failed."); } while (0)

//=============== Начало примера ======================

typedef enum {
    VALID_STATE = 0,
    INVALID_STATE = 1
} state_t;

// Инициализируем мьютекс
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // protects: state 
state_t current_state = VALID_STATE;

void thread_safe_func() {
    // all function is critical section, protected by mutex
    pthread_mutex_lock(&mutex); // try comment lock&unlock out and look at result
    ta_verify(current_state == VALID_STATE);
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
        pt_verify(pthread_create(&threads[i], NULL, thread_func, (char*)NULL + i));
    }
    for (int i = 0; i < threads_count; ++i) {
        pt_verify(pthread_join(threads[i], NULL)); 
        log_printf("Thread %d joined\n", i);
    }
    log_printf("Main func finished\n");
    return 0;
}
```


Run: `gcc -fsanitize=thread mutex.c -lpthread -o mutex.exe`



Run: `./mutex.exe`


    0.000          main():80  [tid=80692]: Main func started
    0.000          main():84  [tid=80692]: Creating thread 0
    0.001          main():84  [tid=80692]: Creating thread 1
    0.001   thread_func():70  [tid=80694]:   Thread 0 started
    0.003   thread_func():70  [tid=80695]:   Thread 1 started
    0.047   thread_func():74  [tid=80694]:   Thread 0 finished
    0.048          main():89  [tid=80692]: Thread 0 joined
    0.053   thread_func():74  [tid=80695]:   Thread 1 finished
    0.054          main():89  [tid=80692]: Thread 1 joined
    0.054          main():91  [tid=80692]: Main func finished


# <a name="spinlock"></a> Spinlock

В С++ есть `std::atomic<T>`, в Си есть и у нас нет шаблонов, а для атомарных типов есть спецификатор `_Atomic`. Синтаксически работает он так же как и `const`, то есть `_Atomic int* a` - это указатель на атомарный int. (Если хотите атомарный указатель можно писать `_Atomic (int*) a`). 

[spinlock в стандартной библиотеке](https://linux.die.net/man/3/pthread_spin_init)


```cpp
%%cpp spinlock.c
%run gcc -fsanitize=thread -std=c11 spinlock.c -lpthread -o spinlock.exe
%run ./spinlock.exe

#define _GNU_SOURCE 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <stdint.h>
#include <stdatomic.h>

// log_printf - макрос для отладочного вывода, добавляющий время со старта программы, имя функции и номер строки
uint64_t start_time_msec; void  __attribute__ ((constructor)) start_time_setter() { struct timespec spec; clock_gettime(CLOCK_MONOTONIC, &spec); start_time_msec = spec.tv_sec * 1000L + spec.tv_nsec / 1000000; }
const char* log_prefix(const char* func, int line) {
    struct timespec spec; clock_gettime(CLOCK_MONOTONIC, &spec); int delta_msec = spec.tv_sec * 1000L + spec.tv_nsec / 1000000 - start_time_msec;
    const int max_func_len = 13; static __thread char prefix[100]; 
    sprintf(prefix, "%d.%03d %*s():%-3d [tid=%ld]", delta_msec / 1000, delta_msec % 1000, max_func_len, func, line, syscall(__NR_gettid));
    return prefix;
}
#define log_printf_impl(fmt, ...) { time_t t = time(0); dprintf(2, "%s: " fmt "%s", log_prefix(__FUNCTION__, __LINE__), __VA_ARGS__); }
// Format: <time_since_start> <func_name>:<line> : <custom_message>
#define log_printf(...) log_printf_impl(__VA_ARGS__, "")


#define fail_with_strerror(code, msg) do { char err_buf[1024]; strerror_r(code, err_buf, sizeof(err_buf));\
    log_printf(msg " (From err code: %s)\n", err_buf);  exit(EXIT_FAILURE);} while (0)

// thread-aware assert
#define ta_verify(stmt) do { if (stmt) break; fail_with_strerror(errno, "'" #stmt "' failed."); } while (0)

// verify pthread call
#define pt_verify(pthread_call) do { int code = (pthread_call); if (code == 0) break; \
    fail_with_strerror(code, "'" #pthread_call "' failed."); } while (0)

//=============== Начало примера ======================

typedef enum {
    VALID_STATE = 0,
    INVALID_STATE = 1
} state_t;

_Atomic(int) lock = 0; // protects state
state_t current_state = VALID_STATE;

void sl_lock(_Atomic int* lock) { 
    int expected = 0;
    // weak отличается от strong тем, что может выдавать иногда ложный false. Но он быстрее работает.
    // atomic_compare_exchange_weak can change `expected`!
    while (!atomic_compare_exchange_weak(lock, &expected, 1)) {
        expected = 0;
    }
}

void sl_unlock(_Atomic int* lock) {
    atomic_fetch_sub(lock, 1);
}

// По сути та же функция, что и в предыдущем примере, но ипользуется spinlock вместо mutex
void thread_safe_func() { 
    // all function is critical section, protected by mutex
    sl_lock(&lock); // try comment lock&unlock out and look at result
    ta_verify(current_state == VALID_STATE);
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
        pt_verify(pthread_create(&threads[i], NULL, thread_func, (char*)NULL + i));
    }
    for (int i = 0; i < threads_count; ++i) {
        pt_verify(pthread_join(threads[i], NULL)); 
        log_printf("Thread %d joined\n", i);
    }
    log_printf("Main func finished\n");
    return 0;
}
```


Run: `gcc -fsanitize=thread -std=c11 spinlock.c -lpthread -o spinlock.exe`



Run: `./spinlock.exe`


    0.000          main():89  [tid=78309]: Main func started
    0.000          main():93  [tid=78309]: Creating thread 0
    0.003          main():93  [tid=78309]: Creating thread 1
    0.003   thread_func():79  [tid=78311]:   Thread 0 started
    0.006   thread_func():79  [tid=78312]:   Thread 1 started
    0.064   thread_func():83  [tid=78312]:   Thread 1 finished
    0.064   thread_func():83  [tid=78311]:   Thread 0 finished
    0.065          main():98  [tid=78309]: Thread 0 joined
    0.065          main():98  [tid=78309]: Thread 1 joined
    0.065          main():100 [tid=78309]: Main func finished



```cpp
%%cpp condvar.c
%run gcc -fsanitize=thread condvar.c -lpthread -o condvar.exe
%run ./condvar.exe

#define _GNU_SOURCE 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <stdint.h>
#include <stdatomic.h>

// log_printf - макрос для отладочного вывода, добавляющий время со старта программы, имя функции и номер строки
uint64_t start_time_msec; void  __attribute__ ((constructor)) start_time_setter() { struct timespec spec; clock_gettime(CLOCK_MONOTONIC, &spec); start_time_msec = spec.tv_sec * 1000L + spec.tv_nsec / 1000000; }
const char* log_prefix(const char* func, int line) {
    struct timespec spec; clock_gettime(CLOCK_MONOTONIC, &spec); int delta_msec = spec.tv_sec * 1000L + spec.tv_nsec / 1000000 - start_time_msec;
    const int max_func_len = 13; static __thread char prefix[100]; 
    sprintf(prefix, "%d.%03d %*s():%-3d [tid=%ld]", delta_msec / 1000, delta_msec % 1000, max_func_len, func, line, syscall(__NR_gettid));
    return prefix;
}
#define log_printf_impl(fmt, ...) { time_t t = time(0); dprintf(2, "%s: " fmt "%s", log_prefix(__FUNCTION__, __LINE__), __VA_ARGS__); }
// Format: <time_since_start> <func_name>:<line> : <custom_message>
#define log_printf(...) log_printf_impl(__VA_ARGS__, "")


#define fail_with_strerror(code, msg) do { char err_buf[1024]; strerror_r(code, err_buf, sizeof(err_buf));\
    log_printf(msg " (From err code: %s)\n", err_buf);  exit(EXIT_FAILURE);} while (0)

// thread-aware assert
#define ta_verify(stmt) do { if (stmt) break; fail_with_strerror(errno, "'" #stmt "' failed."); } while (0)

// verify pthread call
#define pt_verify(pthread_call) do { int code = (pthread_call); if (code == 0) break; \
    fail_with_strerror(code, "'" #pthread_call "' failed."); } while (0)

//=============== Начало примера ======================

typedef struct {
    // рекомендую порядок записи переменных:
    pthread_mutex_t mutex; // мьютекс
    pthread_cond_t condvar; // переменная условия (если нужна)
    
    int value;
} promise_t;

void promise_init(promise_t* promise) {
    pthread_mutex_init(&promise->mutex, NULL);
    pthread_cond_init(&promise->condvar, NULL);
    promise->value = -1;
}

void promise_set(promise_t* promise, int value) {
    pthread_mutex_lock(&promise->mutex); // try comment lock&unlock out and look at result
    promise->value = value; // криитическую секцию стоит делать как можно меньше
    pthread_mutex_unlock(&promise->mutex);
    // Важно понимать, когда вы уведомляете ожидающий поток о наступившем событии
    // скорее всего вам подойдетт вариант сделать это сразу после unlock
    // и скорее всего вам не нужно, 
    // чтобы между сохранением полезного состояния и cond_signal находилось ожидание чего-либо
    pthread_cond_signal(&promise->condvar); // notify if there was nothing and now will be elements
}

int promise_get(promise_t* promise) {
    pthread_mutex_lock(&promise->mutex); // try comment lock&unlock out and look at result
    while (promise->value == -1) {
        // Ждем какие-либо данные, если их нет, то спим.
        // идейно convar внутри себя разблокирует mutex, чтобы другой поток мог положить в стейт то, что мы ждем
        pthread_cond_wait(&promise->condvar, &promise->mutex);
        // после завершения wait мьютекс снова заблокирован
    }
    int value = promise->value;
    pthread_mutex_unlock(&promise->mutex);
    return value;
}

promise_t promise_1, promise_2;


static void* thread_A_func(void* arg) {
    log_printf("Func A started\n");
    promise_set(&promise_1, 42);
    log_printf("Func A set promise_1 with 42\n");
    int value_2 = promise_get(&promise_2);
    log_printf("Func A get promise_2 value = %d\n", value_2);
    return NULL;
}

static void* thread_B_func(void* arg) {
    log_printf("Func B started\n");
    int value_1 = promise_get(&promise_1);
    log_printf("Func B get promise_1 value = %d\n", value_1);
    promise_set(&promise_2, value_1 * 100);
    log_printf("Func B set promise_2 with %d\n", value_1 * 100)
    return NULL;
}

int main()
{
    promise_init(&promise_1);
    promise_init(&promise_2);
    
    log_printf("Main func started\n");
    
    pthread_t thread_A_id;
    log_printf("Creating thread A\n");
    pt_verify(pthread_create(&thread_A_id, NULL, thread_A_func, NULL));
    
    pthread_t thread_B_id;
    log_printf("Creating thread B\n");
    pt_verify(pthread_create(&thread_B_id, NULL, thread_B_func, NULL));
    
    pt_verify(pthread_join(thread_A_id, NULL)); 
    log_printf("Thread A joined\n");
    
    pt_verify(pthread_join(thread_B_id, NULL)); 
    log_printf("Thread B joined\n");
    
    log_printf("Main func finished\n");
    return 0;
}
```


Run: `gcc -fsanitize=thread condvar.c -lpthread -o condvar.exe`



Run: `./condvar.exe`


    0.000          main():107 [tid=78320]: Main func started
    0.001          main():110 [tid=78320]: Creating thread A
    0.002          main():114 [tid=78320]: Creating thread B
    0.002 thread_A_func():85  [tid=78322]: Func A started
    0.003 thread_A_func():87  [tid=78322]: Func A set promise_1 with 42
    0.004 thread_B_func():94  [tid=78323]: Func B started
    0.005 thread_B_func():96  [tid=78323]: Func B get promise_1 value = 42
    0.005 thread_A_func():89  [tid=78322]: Func A get promise_2 value = 4200
    0.005 thread_B_func():98  [tid=78323]: Func B set promise_2 with 4200
    0.005          main():118 [tid=78320]: Thread A joined
    0.005          main():121 [tid=78320]: Thread B joined
    0.005          main():123 [tid=78320]: Main func finished



```python

```


```python

```


```python

```

# Межпроцессная синхронизация


# <a name="mmap"></a> `mmap`


Разделяемая память - это когда два региона виртуальной памяти (один в одном процессе, другой в другом) 
ссылаются на одну и ту же физическую память. То есть могут обмениваться информацией через нее.

Межпроцессное взаимодействие через разделяемую память нужно, 
когда у нас есть две различные программы (могут быть написаны на разных языках программирования)
и когда нам не подходит взаимодействие через сокеты (такое взаимодействие не очень эффективно).




```cpp
%%cpp mmap.c
%run gcc -Wall -fsanitize=thread mmap.c -lpthread -o mmap.exe
%run ./mmap.exe

#define _GNU_SOURCE 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <stdint.h>
#include <stdatomic.h>

// log_printf - макрос для отладочного вывода, добавляющий время со старта программы, имя функции и номер строки
uint64_t start_time_msec; void  __attribute__ ((constructor)) start_time_setter() { struct timespec spec; clock_gettime(CLOCK_MONOTONIC, &spec); start_time_msec = spec.tv_sec * 1000L + spec.tv_nsec / 1000000; }
const char* log_prefix(const char* func, int line) {
    struct timespec spec; clock_gettime(CLOCK_MONOTONIC, &spec); int delta_msec = spec.tv_sec * 1000L + spec.tv_nsec / 1000000 - start_time_msec;
    const int max_func_len = 13; static __thread char prefix[100]; 
    sprintf(prefix, "%d.%03d %*s():%-3d [tid=%ld]", delta_msec / 1000, delta_msec % 1000, max_func_len, func, line, syscall(__NR_gettid));
    return prefix;
}
#define log_printf_impl(fmt, ...) { dprintf(2, "%s: " fmt "%s", log_prefix(__FUNCTION__, __LINE__), __VA_ARGS__); }
// Format: <time_since_start> <func_name>:<line> : <custom_message>
#define log_printf(...) log_printf_impl(__VA_ARGS__, "")


#define fail_with_strerror(code, msg) do { char err_buf[1024]; strerror_r(code, err_buf, sizeof(err_buf));\
    log_printf(msg " (From err code: %s)\n", err_buf);  exit(EXIT_FAILURE);} while (0)

// thread-aware assert
#define ta_verify(stmt) do { if (stmt) break; fail_with_strerror(errno, "'" #stmt "' failed."); } while (0)

// verify pthread call
#define pt_verify(pthread_call) do { int code = (pthread_call); if (code == 0) break; \
    fail_with_strerror(code, "'" #pthread_call "' failed."); } while (0)

//=============== Начало примера ======================

typedef enum {
    VALID_STATE = 0,
    INVALID_STATE = 1
} state_t;


typedef struct {
    pthread_mutex_t mutex; 
    state_t current_state; // protected by mutex
} shared_state_t;

shared_state_t* state; // interprocess state

// process_safe_func и process_func - функции-примеры с прошлого семинара (с точностью до замены thread/process)
void process_safe_func() {
    // all function is critical section, protected by mutex
    pthread_mutex_lock(&state->mutex); // try comment lock&unlock out and look at result
    ta_verify(state->current_state == VALID_STATE);
    state->current_state = INVALID_STATE; // do some work with state. 
    sched_yield();
    state->current_state = VALID_STATE;
    pthread_mutex_unlock(&state->mutex);
}

void process_func(int process_num) 
{
    log_printf("  Process %d started\n", process_num);
    for (int j = 0; j < 10000; ++j) {
        process_safe_func();
    }
    log_printf("  Process %d finished\n", process_num);
}

 
shared_state_t* create_state() {
    // Создаем кусок разделяемой памяти. Он будет общим для данного процесса и его дочерних
    shared_state_t* state = mmap(
        /* desired addr, addr = */ NULL, 
        /* length = */ sizeof(shared_state_t), // Размер разделяемого фрагмента памяти
        /* access attributes, prot = */ PROT_READ | PROT_WRITE, 
        /* flags = */ MAP_SHARED | MAP_ANONYMOUS,
        /* fd = */ -1,
        /* offset in file, offset = */ 0
    );
    ta_verify(state != MAP_FAILED);
    
    // create and initialize interprocess mutex
    pthread_mutexattr_t mutex_attrs; 
    pt_verify(pthread_mutexattr_init(&mutex_attrs));
    // Важно! Без этого атрибута один из процессов навсегда зависнет в lock мьютекса
    // Вероятно этот атрибут влияет на отсутствие флага FUTEX_PRIVATE_FLAG в операциях с futex
    // Если он стоит, то ядро может делать некоторые оптимизации в предположении, что futex используется одним процессом
    pt_verify(pthread_mutexattr_setpshared(&mutex_attrs, PTHREAD_PROCESS_SHARED));
    pt_verify(pthread_mutex_init(&state->mutex, &mutex_attrs));
    pt_verify(pthread_mutexattr_destroy(&mutex_attrs));
    
    state->current_state = VALID_STATE; // Инициализирем защищаемое состояние
    return state;
}

void delete_state(shared_state_t* state) {
    pt_verify(pthread_mutex_destroy(&state->mutex));
    ta_verify(munmap(state, sizeof(shared_state_t)) == 0);
}

int main()
{
    log_printf("Main process started\n");
    state = create_state(); // Создаем разделяемое состояние
    const int process_count = 2;
    pid_t processes[process_count];
    // Создаем дочерние процессы
    for (int i = 0; i < process_count; ++i) {
        log_printf("Creating process %d\n", i);
        // дочерние процессы унаследуют разделяемое состояние (оно не скопируется, а будет общим)
        ta_verify((processes[i] = fork()) >= 0); 
        if (processes[i] == 0) {
            process_func(i); // Имитируем работу из разных процессов
            exit(0);
        }
    }
    for (int i = 0; i < process_count; ++i) {
        int status;
        ta_verify(waitpid(processes[i], &status, 0) != -1);
        ta_verify(WIFEXITED(status) && WEXITSTATUS(status) == 0);
        log_printf("Process %d 'joined'\n", i);
    }
    delete_state(state);
    log_printf("Main process finished\n");
    return 0;
}
```


Run: `gcc -Wall -fsanitize=thread mmap.c -lpthread -o mmap.exe`



Run: `./mmap.exe`


    0.000          main():112 [tid=78343]: Main process started
    0.000          main():118 [tid=78343]: Creating process 0
    0.001          main():118 [tid=78343]: Creating process 1
    0.001  process_func():71  [tid=78344]:   Process 0 started
    0.002  process_func():71  [tid=78346]:   Process 1 started
    0.030  process_func():75  [tid=78346]:   Process 1 finished
    0.031  process_func():75  [tid=78344]:   Process 0 finished
    0.032          main():130 [tid=78343]: Process 0 'joined'
    0.032          main():130 [tid=78343]: Process 1 'joined'
    0.032          main():133 [tid=78343]: Main process finished


# Ну и spinlock давайте. А почему бы и нет?

Отличие только в замене инициализации и в взятии/снятии локов.


```cpp
%%cpp mmap.c
%run gcc -Wall -fsanitize=thread mmap.c -lpthread -o mmap.exe
%run ./mmap.exe

#define _GNU_SOURCE 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <stdint.h>
#include <stdatomic.h>

// log_printf - макрос для отладочного вывода, добавляющий время со старта программы, имя функции и номер строки
uint64_t start_time_msec; void  __attribute__ ((constructor)) start_time_setter() { struct timespec spec; clock_gettime(CLOCK_MONOTONIC, &spec); start_time_msec = spec.tv_sec * 1000L + spec.tv_nsec / 1000000; }
const char* log_prefix(const char* func, int line) {
    struct timespec spec; clock_gettime(CLOCK_MONOTONIC, &spec); int delta_msec = spec.tv_sec * 1000L + spec.tv_nsec / 1000000 - start_time_msec;
    const int max_func_len = 13; static __thread char prefix[100]; 
    sprintf(prefix, "%d.%03d %*s():%-3d [tid=%ld]", delta_msec / 1000, delta_msec % 1000, max_func_len, func, line, syscall(__NR_gettid));
    return prefix;
}
#define log_printf_impl(fmt, ...) { dprintf(2, "%s: " fmt "%s", log_prefix(__FUNCTION__, __LINE__), __VA_ARGS__); }
// Format: <time_since_start> <func_name>:<line> : <custom_message>
#define log_printf(...) log_printf_impl(__VA_ARGS__, "")


#define fail_with_strerror(code, msg) do { char err_buf[1024]; strerror_r(code, err_buf, sizeof(err_buf));\
    log_printf(msg " (From err code: %s)\n", err_buf);  exit(EXIT_FAILURE);} while (0)

// thread-aware assert
#define ta_verify(stmt) do { if (stmt) break; fail_with_strerror(errno, "'" #stmt "' failed."); } while (0)

// verify pthread call
#define pt_verify(pthread_call) do { int code = (pthread_call); if (code == 0) break; \
    fail_with_strerror(code, "'" #pthread_call "' failed."); } while (0)

//=============== Начало примера ======================

typedef enum {
    VALID_STATE = 0,
    INVALID_STATE = 1
} state_t;


typedef struct {
    _Atomic int lock; 
    state_t current_state; // protected by mutex
} shared_state_t;

shared_state_t* state; // interprocess state

void sl_lock(_Atomic int* lock) { 
    int expected = 0;
    while (!atomic_compare_exchange_weak(lock, &expected, 1)) {
        expected = 0;
    }
}

void sl_unlock(_Atomic int* lock) {
    atomic_fetch_sub(lock, 1);
}

void process_safe_func() {
    // all function is critical section, protected by spinlock
    sl_lock(&state->lock);
    ta_verify(state->current_state == VALID_STATE);
    state->current_state = INVALID_STATE; // do some work with state. 
    sched_yield();
    state->current_state = VALID_STATE;
    sl_unlock(&state->lock);
}

void process_func(int process_num) 
{
    log_printf("  Process %d started\n", process_num);
    for (int j = 0; j < 10000; ++j) {
        process_safe_func();
    }
    log_printf("  Process %d finished\n", process_num);
}

 
shared_state_t* create_state() {
    shared_state_t* state = mmap(
        /* desired addr, addr = */ NULL, 
        /* length = */ sizeof(shared_state_t), 
        /* access attributes, prot = */ PROT_READ | PROT_WRITE, 
        /* flags = */ MAP_SHARED | MAP_ANONYMOUS,
        /* fd = */ -1,
        /* offset in file, offset = */ 0
    );
    ta_verify(state != MAP_FAILED);
    
    state->lock = 0;
    state->current_state = VALID_STATE;
    return state;
}

void delete_state(shared_state_t* state) {
    ta_verify(munmap(state, sizeof(shared_state_t)) == 0);
}

int main()
{
    log_printf("Main process started\n");
    state = create_state();
    const int process_count = 2;
    pid_t processes[process_count];
    for (int i = 0; i < process_count; ++i) {
        log_printf("Creating process %d\n", i);
        ta_verify((processes[i] = fork()) >= 0);
        if (processes[i] == 0) {
            process_func(i);
            exit(0);
        }
    }
    for (int i = 0; i < process_count; ++i) {
        int status;
        ta_verify(waitpid(processes[i], &status, 0) != -1);
        ta_verify(WIFEXITED(status) && WEXITSTATUS(status) == 0);
        log_printf("Process %d 'joined'\n", i);
    }
    delete_state(state);
    log_printf("Main process finished\n");
    return 0;
}
```


Run: `gcc -Wall -fsanitize=thread mmap.c -lpthread -o mmap.exe`



Run: `./mmap.exe`


    0.000          main():111 [tid=78384]: Main process started
    0.001          main():116 [tid=78384]: Creating process 0
    0.002          main():116 [tid=78384]: Creating process 1
    0.003  process_func():81  [tid=78385]:   Process 0 started
    0.004  process_func():81  [tid=78387]:   Process 1 started
    0.025  process_func():85  [tid=78385]:   Process 0 finished
    0.026  process_func():85  [tid=78387]:   Process 1 finished
    0.026          main():127 [tid=78384]: Process 0 'joined'
    0.027          main():127 [tid=78384]: Process 1 'joined'
    0.027          main():130 [tid=78384]: Main process finished


# <a name="shm"></a> `shm_open`

Сделаем то же самое, что и в предыдущем примере, но на этот раз не из родственных процессов. Воспользуемся именноваными объектами разделяемой памяти.


```cpp
%%cpp shm.c
%# Обратите внимание: -lrt. Здесь нужна новая разделяемая библиотека
%run gcc -Wall -fsanitize=thread shm.c -lrt -lpthread -o s.exe
%run ./s.exe remove_shm /my_shm  # на случай, если прошлый запуск закончился плохо и shm не удалилась
%run ./s.exe create_shm /my_shm
%run ./s.exe work 1 /my_shm & PID=$! ; ./s.exe work 2 /my_shm ; wait $PID
%run ./s.exe remove_shm /my_shm


#define _GNU_SOURCE 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdatomic.h>
#include <fcntl.h>

// log_printf - макрос для отладочного вывода, добавляющий время со старта программы, имя функции и номер строки
uint64_t start_time_msec; void  __attribute__ ((constructor)) start_time_setter() { struct timespec spec; clock_gettime(CLOCK_MONOTONIC, &spec); start_time_msec = spec.tv_sec * 1000L + spec.tv_nsec / 1000000; }
const char* log_prefix(const char* func, int line) {
    struct timespec spec; clock_gettime(CLOCK_MONOTONIC, &spec); int delta_msec = spec.tv_sec * 1000L + spec.tv_nsec / 1000000 - start_time_msec;
    const int max_func_len = 13; static __thread char prefix[100]; 
    sprintf(prefix, "%d.%03d %*s():%-3d [tid=%ld]", delta_msec / 1000, delta_msec % 1000, max_func_len, func, line, syscall(__NR_gettid));
    return prefix;
}
#define log_printf_impl(fmt, ...) { dprintf(2, "%s: " fmt "%s", log_prefix(__FUNCTION__, __LINE__), __VA_ARGS__); }
// Format: <time_since_start> <func_name>:<line> : <custom_message>
#define log_printf(...) log_printf_impl(__VA_ARGS__, "")


#define fail_with_strerror(code, msg) do { char err_buf[1024]; strerror_r(code, err_buf, sizeof(err_buf));\
    log_printf(msg " (From err code: %s)\n", err_buf);  exit(EXIT_FAILURE);} while (0)

// thread-aware assert
#define ta_verify(stmt) do { if (stmt) break; fail_with_strerror(errno, "'" #stmt "' failed."); } while (0)

// verify pthread call
#define pt_verify(pthread_call) do { int code = (pthread_call); if (code == 0) break; \
    fail_with_strerror(code, "'" #pthread_call "' failed."); } while (0)

//=============== Начало примера ======================

typedef enum {
    VALID_STATE = 0,
    INVALID_STATE = 1
} state_t;


typedef struct {
    pthread_mutex_t mutex; 
    state_t current_state; // protected by mutex
} shared_state_t;

void process_safe_func(shared_state_t* state) {
    // all function is critical section, protected by mutex
    pthread_mutex_lock(&state->mutex); // try comment lock&unlock out and look at result
    ta_verify(state->current_state == VALID_STATE);
    state->current_state = INVALID_STATE; // do some work with state. 
    sched_yield();
    state->current_state = VALID_STATE;
    pthread_mutex_unlock(&state->mutex);
}
 
shared_state_t* load_state(const char* shm_name, bool do_create) {
    // открываем / создаем объект разделяемой памяти
    // по сути это просто open, только для виртуального файла (без сохранения данных на диск + ортогональное пространство имен)
    int fd = shm_open(shm_name, O_RDWR | (do_create ? O_CREAT : 0), 0644);
    ta_verify(fd >= 0);
    ta_verify(ftruncate(fd, sizeof(shared_state_t)) == 0);
    shared_state_t* state = mmap(
        /* desired addr, addr = */ NULL, 
        /* length = */ sizeof(shared_state_t), 
        /* access attributes, prot = */ PROT_READ | PROT_WRITE, 
        /* flags = */ MAP_SHARED,
        /* fd = */ fd,
        /* offset in file, offset = */ 0
    );
    close(fd);
    
    ta_verify(state != MAP_FAILED);
    if (!do_create) {
        return state;
    }
    // create interprocess mutex
    pthread_mutexattr_t mutex_attrs;
    pt_verify(pthread_mutexattr_init(&mutex_attrs));
    // Важно!
    pt_verify(pthread_mutexattr_setpshared(&mutex_attrs, PTHREAD_PROCESS_SHARED));
    pt_verify(pthread_mutex_init(&state->mutex, &mutex_attrs));
    pt_verify(pthread_mutexattr_destroy(&mutex_attrs));
    
    state->current_state = VALID_STATE;
    return state;
}

void unload_state(shared_state_t* state) {
    ta_verify(munmap(state, sizeof(shared_state_t)) == 0);
}

int main(int argc, char** argv)
{
    ta_verify(argc >= 2);
    if (strcmp("create_shm", argv[1]) == 0) {
        log_printf("  Creating state: %s\n", argv[2]);
        unload_state(load_state(argv[2], /*do_create=*/ 1));
        log_printf("  State created\n");
    } else if (strcmp("remove_shm", argv[1]) == 0) {
        log_printf("  Removing state: %s\n", argv[2]);
        // Файлы shm существуют пока не будет вызвана unlink.
        if (shm_unlink(argv[2]) == 0) {
            log_printf("  State was removed\n");   
        } else {
            log_printf("  State was NOT removed\n");   
        }
    } else if (strcmp("work", argv[1]) == 0) {
        ta_verify(argc == 4);
        int worker = strtol(argv[2], 0, 10);
        log_printf("  Worker %d started\n", worker);
        shared_state_t* state = load_state(argv[3], /*do_create=*/ 0);
       
        for (int j = 0; j < 10000; ++j) {
            process_safe_func(state);
        }

        unload_state(state);
        log_printf("  Worker %d finished\n", worker);
    } else {
        ta_verify(0 && "unknown command");
    }
    return 0;
}
```


Run: `gcc -Wall -fsanitize=thread shm.c -lrt -lpthread -o s.exe`



Run: `./s.exe remove_shm /my_shm  # на случай, если прошлый запуск закончился плохо и shm не удалилась`


    0.000          main():118 [tid=79569]:   Removing state: /my_shm
    0.002          main():123 [tid=79569]:   State was NOT removed



Run: `./s.exe create_shm /my_shm`


    0.000          main():114 [tid=79571]:   Creating state: /my_shm
    0.001          main():116 [tid=79571]:   State created



Run: `./s.exe work 1 /my_shm & PID=$! ; ./s.exe work 2 /my_shm ; wait $PID`


    0.000          main():128 [tid=79574]:   Worker 2 started
    0.000          main():128 [tid=79573]:   Worker 1 started
    0.159          main():136 [tid=79574]:   Worker 2 finished
    0.247          main():136 [tid=79573]:   Worker 1 finished



Run: `./s.exe remove_shm /my_shm`


    0.000          main():118 [tid=79576]:   Removing state: /my_shm
    0.000          main():121 [tid=79576]:   State was removed


Проблема: как решить, кто из независимых процессов будет создавать участок?

Способ разрешить конфликт создания участка разделяемой памяти:
  * Все процессы создают файлы с флагом O_EXCL | O_CREAT
  * Из-за О_EXCL выкинет ошибку для всех процессов кроме одного
  * Этот один процесс создаст файл, выделит память, создаст спинлок на инициализацию и начнёт инициализировать
  * Другие, которые получили ошибку попытаются открыть файл ещё раз, без этих флагов уже.
  * Далее они будут ждать (регулярно проверять) пока не изменится размер файла, потом откроют его, и дальше будут ждать инициализации на спинлоке.

# <a name="sem_anon"></a> Анонимные семафоры

Игровое сравнение: семафор это ящик с шариками.

<pre>
|   |
|   |
|   |
|_*_|</pre>
    ^
    |
Это семафор со значением 1 (ящик с одним шариком)

У семафора такая семантика:
* Операция post() кладет шарик в ящик. Работает мгновенно.
* Операция wait() извлекает шарик из ящика. Если шариков нет, то блокируется пока не появится шарик и затем его извлекает.
* Еще есть try_wait(), timed_wait() - они соответствуют названиям.

Шарики можно так же рассматривать как свободные ресурсы.

Семафор с одним шариком можно использовать как мьютекс. В данном случае шарик - это ресурс на право входить в критическую секцию. Соответственно lock - это wait. А unlock это post.


### Пример использования с многими шариками:    Построение очереди.

Создаём 2 семафора, semFree и semElementsInside.

При добавлении берём ресурс (~шарик) из semFree, под lock добавляем элемент, кладём ресурс в semElementsInside

При удалении берём ресурс из semElementsInside, под локом удаляем элемент, кладём ресурс в semFree<br>

### Пример с семафором в общей памяти


```cpp
%%cpp sem_anon.c
%run gcc -Wall -fsanitize=thread -lrt sem_anon.c -o sem_anon.exe
%run ./sem_anon.exe

#define _GNU_SOURCE 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdatomic.h>
#include <semaphore.h>

// log_printf - макрос для отладочного вывода, добавляющий время со старта программы, имя функции и номер строки
uint64_t start_time_msec; void  __attribute__ ((constructor)) start_time_setter() { struct timespec spec; clock_gettime(CLOCK_MONOTONIC, &spec); start_time_msec = spec.tv_sec * 1000L + spec.tv_nsec / 1000000; }
const char* log_prefix(const char* func, int line) {
    struct timespec spec; clock_gettime(CLOCK_MONOTONIC, &spec); int delta_msec = spec.tv_sec * 1000L + spec.tv_nsec / 1000000 - start_time_msec;
    const int max_func_len = 13; static __thread char prefix[100]; 
    sprintf(prefix, "%d.%03d %*s():%-3d [tid=%ld]", delta_msec / 1000, delta_msec % 1000, max_func_len, func, line, syscall(__NR_gettid));
    return prefix;
}
#define log_printf_impl(fmt, ...) { dprintf(2, "%s: " fmt "%s", log_prefix(__FUNCTION__, __LINE__), __VA_ARGS__); }
// Format: <time_since_start> <func_name>:<line> : <custom_message>
#define log_printf(...) log_printf_impl(__VA_ARGS__, "")


#define fail_with_strerror(code, msg) do { char err_buf[1024]; strerror_r(code, err_buf, sizeof(err_buf));\
    log_printf(msg " (From err code: %s)\n", err_buf);  exit(EXIT_FAILURE);} while (0)

// thread-aware assert
#define ta_verify(stmt) do { if (stmt) break; fail_with_strerror(errno, "'" #stmt "' failed."); } while (0)

// verify pthread call
#define pt_verify(pthread_call) do { int code = (pthread_call); if (code == 0) break; \
    fail_with_strerror(code, "'" #pthread_call "' failed."); } while (0)

//=============== Начало примера ======================

typedef enum {
    VALID_STATE = 0,
    INVALID_STATE = 1
} state_t;


typedef struct {
    sem_t semaphore; 
    state_t current_state; // protected by semaphore
} shared_state_t;

shared_state_t* state; // interprocess state

void process_safe_func() {
    // all function is critical section, protected by mutex
    sem_wait(&state->semaphore); // ~ lock
    ta_verify(state->current_state == VALID_STATE);
    state->current_state = INVALID_STATE; // do some work with state. 
    sched_yield();
    state->current_state = VALID_STATE;
    sem_post(&state->semaphore); // ~ unlock
}

void process_func(int process_num) 
{
    log_printf("  Process %d started\n", process_num);
    for (int j = 0; j < 10000; ++j) {
        process_safe_func();
    }
    log_printf("  Process %d finished\n", process_num);
}

 
shared_state_t* create_state() {
    shared_state_t* state = mmap(
        /* desired addr, addr = */ NULL, 
        /* length = */ sizeof(shared_state_t), 
        /* access attributes, prot = */ PROT_READ | PROT_WRITE, 
        /* flags = */ MAP_SHARED | MAP_ANONYMOUS,
        /* fd = */ -1,
        /* offset in file, offset = */ 0
    );
    ta_verify(state != MAP_FAILED);
    
    // create interprocess semaphore
    ta_verify(sem_init(
        &state->semaphore,
        1, // interprocess? (0 if will be used in one process)
        1  // initial value
    ) == 0);
    
    state->current_state = VALID_STATE;
    return state;
}

void delete_state(shared_state_t* state) {
    ta_verify(sem_destroy(&state->semaphore) == 0);
    ta_verify(munmap(state, sizeof(shared_state_t)) == 0);
}

int main()
{
    log_printf("Main process started\n");
    state = create_state();
    const int process_count = 2;
    pid_t processes[process_count];
    for (int i = 0; i < process_count; ++i) {
        log_printf("Creating process %d\n", i);
        ta_verify((processes[i] = fork()) >= 0);
        if (processes[i] == 0) {
            process_func(i);
            exit(0);
        }
    }
    for (int i = 0; i < process_count; ++i) {
        int status;
        ta_verify(waitpid(processes[i], &status, 0) != -1);
        ta_verify(WIFEXITED(status) && WEXITSTATUS(status) == 0);
        log_printf("Process %d 'joined'\n", i);
    }
    delete_state(state);
    log_printf("Main process finished\n");
    return 0;
}
```


Run: `gcc -Wall -fsanitize=thread -lrt sem_anon.c -o sem_anon.exe`



Run: `./sem_anon.exe`


    0.000          main():110 [tid=78601]: Main process started
    0.000          main():115 [tid=78601]: Creating process 0
    0.001          main():115 [tid=78601]: Creating process 1
    0.001  process_func():73  [tid=78602]:   Process 0 started
    0.002  process_func():73  [tid=78603]:   Process 1 started
    0.031  process_func():77  [tid=78603]:   Process 1 finished
    0.033  process_func():77  [tid=78602]:   Process 0 finished
    0.034          main():126 [tid=78601]: Process 0 'joined'
    0.034          main():126 [tid=78601]: Process 1 'joined'
    0.034          main():129 [tid=78601]: Main process finished


# <a name="sem_named"></a> Именнованные семафоры

Сначала простой пример


```cpp
%%cpp sem_named_simple.c
%# Обратите внимание: -lrt. Здесь нужна новая разделяемая библиотека
%run gcc -Wall -fsanitize=thread sem_named_simple.c -lrt -lpthread -o s.exe
%run ./s.exe 

#define _GNU_SOURCE 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdatomic.h>
#include <semaphore.h>
#include <fcntl.h>

// log_printf - макрос для отладочного вывода, добавляющий время со старта программы, имя функции и номер строки
uint64_t start_time_msec; void  __attribute__ ((constructor)) start_time_setter() { struct timespec spec; clock_gettime(CLOCK_MONOTONIC, &spec); start_time_msec = spec.tv_sec * 1000L + spec.tv_nsec / 1000000; }
const char* log_prefix(const char* func, int line) {
    struct timespec spec; clock_gettime(CLOCK_MONOTONIC, &spec); int delta_msec = spec.tv_sec * 1000L + spec.tv_nsec / 1000000 - start_time_msec;
    const int max_func_len = 13; static __thread char prefix[100]; 
    sprintf(prefix, "%d.%03d %*s():%-3d [tid=%ld]", delta_msec / 1000, delta_msec % 1000, max_func_len, func, line, syscall(__NR_gettid));
    return prefix;
}
#define log_printf_impl(fmt, ...) { dprintf(2, "%s: " fmt "%s", log_prefix(__FUNCTION__, __LINE__), __VA_ARGS__); }
// Format: <time_since_start> <func_name>:<line> : <custom_message>
#define log_printf(...) log_printf_impl(__VA_ARGS__, "")


#define fail_with_strerror(code, msg) do { char err_buf[1024]; strerror_r(code, err_buf, sizeof(err_buf));\
    log_printf(msg " (From err code: %s)\n", err_buf);  exit(EXIT_FAILURE);} while (0)

// thread-aware assert
#define ta_verify(stmt) do { if (stmt) break; fail_with_strerror(errno, "'" #stmt "' failed."); } while (0)
#define ta_warn_if_not(stmt) do { if (!(stmt)) { log_printf("WARNING: '" #stmt "' failed\n"); } } while (0)

// verify pthread call
#define pt_verify(pthread_call) do { int code = (pthread_call); if (code == 0) break; \
    fail_with_strerror(code, "'" #pthread_call "' failed."); } while (0)


//=============== Начало примера ======================


int main(int argc, char** argv)
{
    // создаем семафор (открываем и сразу закрываем)
    {
        sem_t* sem = sem_open("/s42", O_CREAT | O_EXCL, 0644, 1); 
        ta_verify(sem);
        ta_verify(sem_close(sem) == 0);
    }
    
    // синхронизация через семафор разных процессов
    pid_t pids[3];
    for (int i = 0; i < sizeof(pids) / sizeof(pid_t); ++i) {
        ta_verify((pids[i] = fork()) >= 0);
        if (pids[i] == 0) {
            sem_t* sem = sem_open("/s42", 0); 
            ta_verify(sem);
            sem_wait(sem);
            printf("Hello from %d\n", i); // типа межпроцессная критическая секция
            fflush(stdout);
            sem_post(sem);
            ta_verify(sem_close(sem) == 0);
            return 0;
        }
    }
    for (int i = 0; i < sizeof(pids) / sizeof(pid_t); ++i) {
        int status;
        ta_verify(waitpid(pids[i], &status, 0) != -1);
    }
    
    // удаляем семафорр
    ta_warn_if_not(sem_unlink("/s42") == 0);
    return 0;
}
```


Run: `gcc -Wall -fsanitize=thread sem_named_simple.c -lrt -lpthread -o s.exe`



Run: `./s.exe`


    Hello from 0
    Hello from 1
    Hello from 2


В примере про именованные объекты разделяемой памяти мы явно запускали процесс для инициализации состояния до процессов-воркеров, чтобы избежать гонки инициализации состояния.

В этом примере предлагается способ избежать гонки используя именованный семафор.

В примере используется одно и то же имя для объекта разделяемой памяти и семафора. Это безопасно, так как имя семафора автоматически расширяется префиксом или суффиксом `sem`. То есть в результате имена разные.


```cpp
%%cpp sem_named.c
%# Обратите внимание: -lrt. Здесь нужна новая разделяемая библиотека
%run gcc -Wall -fsanitize=thread sem_named.c -lrt -lpthread -o s.exe
%run ./s.exe work 1 /s42 & PID=$! ; ./s.exe work 2 /s42 ; wait $PID
%run ./s.exe cleanup /s42 # необязательная команда. Будет работать и без нее

#define _GNU_SOURCE 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdatomic.h>
#include <semaphore.h>
#include <fcntl.h>

// log_printf - макрос для отладочного вывода, добавляющий время со старта программы, имя функции и номер строки
uint64_t start_time_msec; void  __attribute__ ((constructor)) start_time_setter() { struct timespec spec; clock_gettime(CLOCK_MONOTONIC, &spec); start_time_msec = spec.tv_sec * 1000L + spec.tv_nsec / 1000000; }
const char* log_prefix(const char* func, int line) {
    struct timespec spec; clock_gettime(CLOCK_MONOTONIC, &spec); int delta_msec = spec.tv_sec * 1000L + spec.tv_nsec / 1000000 - start_time_msec;
    const int max_func_len = 13; static __thread char prefix[100]; 
    sprintf(prefix, "%d.%03d %*s():%-3d [tid=%ld]", delta_msec / 1000, delta_msec % 1000, max_func_len, func, line, syscall(__NR_gettid));
    return prefix;
}
#define log_printf_impl(fmt, ...) { dprintf(2, "%s: " fmt "%s", log_prefix(__FUNCTION__, __LINE__), __VA_ARGS__); }
// Format: <time_since_start> <func_name>:<line> : <custom_message>
#define log_printf(...) log_printf_impl(__VA_ARGS__, "")


#define fail_with_strerror(code, msg) do { char err_buf[1024]; strerror_r(code, err_buf, sizeof(err_buf));\
    log_printf(msg " (From err code: %s)\n", err_buf);  exit(EXIT_FAILURE);} while (0)

// thread-aware assert
#define ta_verify(stmt) do { if (stmt) break; fail_with_strerror(errno, "'" #stmt "' failed."); } while (0)
#define ta_warn_if_not(stmt) do { if (!(stmt)) { log_printf("WARNING: '" #stmt "' failed\n"); } } while (0)

// verify pthread call
#define pt_verify(pthread_call) do { int code = (pthread_call); if (code == 0) break; \
    fail_with_strerror(code, "'" #pthread_call "' failed."); } while (0)


//=============== Начало примера ======================

typedef enum {
    VALID_STATE = 0,
    INVALID_STATE = 1
} state_t;


typedef struct {
    pthread_mutex_t mutex; 
    state_t current_state; // protected by mutex
} shared_state_t;

void process_safe_func(shared_state_t* state) {
    // all function is critical section, protected by mutex
    pthread_mutex_lock(&state->mutex); // try comment lock&unlock out and look at result
    ta_verify(state->current_state == VALID_STATE);
    state->current_state = INVALID_STATE; // do some work with state. 
    sched_yield();
    state->current_state = VALID_STATE;
    pthread_mutex_unlock(&state->mutex);
}
 
shared_state_t* load_state(const char* shm_name, bool do_create) {
    // открываем / создаем объект разделяемой памяти
    int fd = shm_open(shm_name, O_RDWR | (do_create ? O_CREAT : 0), 0644);
    if (do_create) {
        ta_verify(ftruncate(fd, sizeof(shared_state_t)) == 0);
    }
    shared_state_t* state = mmap(
        /* desired addr, addr = */ NULL, 
        /* length = */ sizeof(shared_state_t), 
        /* access attributes, prot = */ PROT_READ | PROT_WRITE, 
        /* flags = */ MAP_SHARED,
        /* fd = */ fd,
        /* offset in file, offset = */ 0
    );
    ta_verify(state != MAP_FAILED);
    if (do_create) {
        // create interprocess mutex
        pthread_mutexattr_t mutex_attrs;
        pt_verify(pthread_mutexattr_init(&mutex_attrs));
        // Важно!
        pt_verify(pthread_mutexattr_setpshared(&mutex_attrs, PTHREAD_PROCESS_SHARED));
        pt_verify(pthread_mutex_init(&state->mutex, &mutex_attrs));
        pt_verify(pthread_mutexattr_destroy(&mutex_attrs));

        state->current_state = VALID_STATE;
    }
    return state;
}

void unload_state(shared_state_t* state) {
    ta_verify(munmap(state, sizeof(shared_state_t)) == 0);
}

shared_state_t* process_safe_init_and_load(const char* name) {
    // succeeded only for first process. This process will initalize state
    sem_t* init_semaphore = sem_open(
        name, O_CREAT | O_EXCL, 0644, 0); // Создаем семафор с изначальным значением 0. Если семафор уже есть, то команда пофейлится
    if (init_semaphore != SEM_FAILED) { // Если смогли сделать семафор, то мы - главный процесс, ответственный за инициализацию
        // initializing branch for initializing process
        shared_state_t* state = load_state(name, /*do_create=*/ 1);
        sem_post(init_semaphore); // Кладем в "ящик" весточку, что стейт проинициализирован
        sem_close(init_semaphore);
        return state;
    } else { // Если мы не главные процесс, то подождем инициализацию
        // branch for processes waiting initialisation
        init_semaphore = sem_open(name, 0);
        ta_verify(init_semaphore != SEM_FAILED);
        sem_wait(init_semaphore); // ждем весточку, что стейт готов
        sem_post(init_semaphore); // возвращаем весточку на место, чтобы другим процессам тоже досталось
        sem_close(init_semaphore);
        return load_state(name, /*do_create=*/ 0);
    }
}

int main(int argc, char** argv)
{
    ta_verify(argc >= 2);
    if (strcmp("cleanup", argv[1]) == 0) {
        log_printf("  Cleanup sem and shm: %s\n", argv[2]);
        ta_warn_if_not(shm_unlink(argv[2]) == 0);
        ta_warn_if_not(sem_unlink(argv[2]) == 0);
    } else if (strcmp("work", argv[1]) == 0) {
        ta_verify(argc == 4);
        int worker = strtol(argv[2], 0, 10);
        log_printf("  Worker %d started\n", worker);
        
        shared_state_t* state = process_safe_init_and_load(argv[3]);
       
        for (int j = 0; j < 10000; ++j) {
            process_safe_func(state);
        }
     
        unload_state(state);
        log_printf("  Worker %d finished\n", worker);
    } else {
        ta_verify(0 && "unknown command");
    }
    return 0;
}
```


Run: `gcc -Wall -fsanitize=thread sem_named.c -lrt -lpthread -o s.exe`



Run: `./s.exe work 1 /s42 & PID=$! ; ./s.exe work 2 /s42 ; wait $PID`


    0.001          main():138 [tid=78833]:   Worker 1 started
    0.000          main():138 [tid=78834]:   Worker 2 started
    0.027          main():147 [tid=78834]:   Worker 2 finished
    0.029          main():147 [tid=78833]:   Worker 1 finished



Run: `./s.exe cleanup /s42 # необязательная команда. Будет работать и без нее`


    0.000          main():132 [tid=78836]:   Cleanup sem and shm: /s42


### Важное замечание про именованные и неименованные семафоры

Для открытия/закрытия именованных семафоров используются `sem_open` и `sem_close`.

А для неименованных `sem_init` и `sem_destroy`. 

Смешивать эти операции определенно не стоит, если конечно, вы где-нибудь не найдете документацию, подтверждающую обратное. Делать `sem_open`, а затем `sem_destroy`, это как создавать объект конструктором одного класса, а уничтожать деструктором другого (для родственных классов, без виртуального деструктора).


```python

```

# <a name="sem_signal"></a> Сочетаемость семафоров и сигналов

Нет этой сочетаемости.

```
process 1
> send signal to process 2
> sem_post

process 2
> sem_wait
> check variable that is set in signal handler
```

Не гарантируется, что сигнал будет доставлен и обработан до того, как отработает sem_wait.


```cpp
%%cpp sem_and_signal.c
%run gcc -Wall -fsanitize=thread -lrt sem_and_signal.c -o sem_and_signal.exe
%run ./sem_and_signal.exe

#define _GNU_SOURCE 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <pthread.h>
#include <semaphore.h>

const char* log_prefix(const char* file, int line) {
    struct timeval tp; gettimeofday(&tp, NULL); struct tm ltime; localtime_r(&tp.tv_sec, &ltime);
    static __thread char prefix[100]; 
    size_t time_len = strftime(prefix, sizeof(prefix), "%H:%M:%S", &ltime);
    sprintf(prefix + time_len, ".%03ld %s:%3d [pid=%d]", tp.tv_usec / 1000, file, line, getpid());
    return prefix;
}

#define log_printf_impl(fmt, ...) { dprintf(2, "%s: " fmt "%s", log_prefix(__FILE__, __LINE__), __VA_ARGS__); }
#define log_printf(...) log_printf_impl(__VA_ARGS__, "")

// process-aware assert
#define pa_assert(stmt) if (stmt) {} else { log_printf("'" #stmt "' failed\n"); exit(EXIT_FAILURE); }

volatile sig_atomic_t signal_count = 0;

static void handler(int signum) {
    signal_count += 1;
}

typedef struct {
    sem_t semaphore_1;
    sem_t semaphore_2;
} shared_state_t;

shared_state_t* state;

 
shared_state_t* create_state() {
    shared_state_t* state = mmap(
        /* desired addr, addr = */ NULL, 
        /* length = */ sizeof(shared_state_t), 
        /* access attributes, prot = */ PROT_READ | PROT_WRITE, 
        /* flags = */ MAP_SHARED | MAP_ANONYMOUS,
        /* fd = */ -1,
        /* offset in file, offset = */ 0
    );
    pa_assert(state != MAP_FAILED);
    
    pa_assert(sem_init(&state->semaphore_1, 1, 0) == 0);
    pa_assert(sem_init(&state->semaphore_2, 1, 0) == 0);
    
    return state;
}

void delete_state(shared_state_t* state) {
    pa_assert(sem_destroy(&state->semaphore_1) == 0);
    pa_assert(sem_destroy(&state->semaphore_2) == 0);
    pa_assert(munmap(state, sizeof(shared_state_t)) == 0);
}

int main()
{
    log_printf("Main process started\n");
    state = create_state();
    pid_t process = fork();
    if (process == 0) {
        sigaction(SIGUSR1, &(struct sigaction){.sa_handler = handler, .sa_flags = SA_RESTART}, NULL);
        sleep(1); // imitate synchronous start
        for (int i = 0; ; ++i) {
            sem_wait(&state->semaphore_1); 
            int cnt = signal_count;
            if (cnt != i + 1) {
                fprintf(stderr, "Signals and semaphors are not ordered... i = %d, signals_count = %d\n", i, cnt);
                exit(-1);
            }
            if (i % 100000 == 0) {
                fprintf(stderr, "i = %d\n", i);
            }
            sem_post(&state->semaphore_2); 
        }
    } else {
        sleep(1); // imitate synchronous start
        int status;
        int ret;
        while ((ret = waitpid(process, &status, WNOHANG)) == 0) {
            kill(process, SIGUSR1);
            sem_post(&state->semaphore_1);
        
            while (sem_timedwait(&state->semaphore_2, &(struct timespec){.tv_nsec = 500000000}) == -1 
                   && (ret = waitpid(process, &status, WNOHANG)) == 0) {
            }
        }
        pa_assert(ret != -1)
        pa_assert(WIFEXITED(status) && WEXITSTATUS(status) == 0);
    }
   
    delete_state(state);
    log_printf("Main process finished\n");
    return 0;
}
```


Run: `gcc -Wall -fsanitize=thread -lrt sem_and_signal.c -o sem_and_signal.exe`



Run: `./sem_and_signal.exe`


    01:54:02.933 sem_and_signal.c: 69 [pid=28908]: Main process started
    i = 0
    i = 100000
    i = 200000
    i = 300000
    i = 400000
    i = 500000
    i = 600000
    i = 700000
    i = 800000
    i = 900000
    Signals and semaphors are not ordered... i = 951373, signals_count = 951373
    01:54:59.070 sem_and_signal.c: 99 [pid=28908]: 'ret != -1' failed


Ломается, долго, но ломается. Так, что нельзя рассчитывать, что сигнал отрпавленный раньше, будет обработан до того, как дойдет событие через sem_post/sem_wait отправленное позже.

Что неудивительно, так как семафоры работают напрямую через разделяемую память, а в обработке сигналов принимает участие еще и планировщик задач.

Если система многоядерная и два процесса выполняются одновременно, то между отправкой сигнала и получением события через семафор может не случиться переключений процессов планировщиком - тогда сигнал будет доставлен позже.

А на одноядерной системе представленная схема скорее всего будет работать. Так как между sem_post в одном процессе и завершением sem_wait в другом должно случиться переключение на второй процесс. В ходе которого вызовутся обработчики.


```python

```


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
