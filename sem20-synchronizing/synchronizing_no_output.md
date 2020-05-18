

# Синхронизация потоков

<br>
<div style="text-align: right"> Спасибо <a href="https://github.com/SyrnikRebirth">Сове Глебу</a>, <a href="https://github.com/Disadvantaged">Голяр Димитрису</a> и <a href="https://github.com/nikvas2000">Николаю Васильеву</a> за участие в написании текста </div>
<br>


Сегодня в программе:
* <a href="#mutex" style="color:#856024">Мьютексы</a>
  <br> MUTEX ~ MUTual EXclusion
* <a href="#spinlock" style="color:#856024">Spinlock'и и атомики</a>
  <br> [Атомики в С на cppreference](https://ru.cppreference.com/w/c/atomic)
  <br> <a href="#c_atomic_life" style="color:#856024">Atomic в C и как с этим жить </a> (раздел от <a href="https://github.com/nikvas2000">Николая Васильева</a>)
  <br> <details> <summary>Про compare_exchange_weak vs compare_exchange_strong</summary> <p>
https://stackoverflow.com/questions/4944771/stdatomic-compare-exchange-weak-vs-compare-exchange-strong
<br>The weak compare-and-exchange operations may fail spuriously, that is, return false while leaving the contents of memory pointed to by expected before the operation is the same that same as that of the object and the same as that of expected after the operation. [ Note: This spurious failure enables implementation of compare-and-exchange on a broader class of machines, e.g., loadlocked store-conditional machines. A consequence of spurious failure is that nearly all uses of weak compare-and-exchange will be in a loop. 
</p>
</details>
  
* <a href="#condvar" style="color:#856024">Condition variable (aka условные переменные)</a>
* <a href="#condvar_queue" style="color:#856024">Пример thread-safe очереди</a>
  


<a href="#hw" style="color:#856024">Комментарии к ДЗ</a>

[Ридинг Яковлева](https://github.com/victor-yacovlev/mipt-diht-caos/tree/master/practice/mutex-condvar-atomic)

# <a name="mutex"></a> Mutex


```cpp
%%cpp mutex.c
%# Санитайзер отслеживает небезопасный доступ 
%# к одному и тому же участку в памяти из разных потоков
%# (а так же другие небезопасные вещи). 
%# В таких задачах советую всегда использовать
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

// Инициализируем мьютекс
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // protects: state 
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
#include <stdatomic.h> //! Этот заголовочный файл плохо гуглится

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

# <a name="condvar"></a> Condition variable


```cpp
%%cpp condvar.c
%run gcc -fsanitize=thread condvar.c -lpthread -o condvar.exe
%run ./condvar.exe > out.txt
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
    promise->value = value;
    pthread_mutex_unlock(&promise->mutex);
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
    ta_assert(pthread_create(&thread_A_id, NULL, thread_A_func, NULL) == 0);
    
    pthread_t thread_B_id;
    log_printf("Creating thread B\n");
    ta_assert(pthread_create(&thread_B_id, NULL, thread_B_func, NULL) == 0);
    
    ta_assert(pthread_join(thread_A_id, NULL) == 0); 
    log_printf("Thread A joined\n");
    
    ta_assert(pthread_join(thread_B_id, NULL) == 0); 
    log_printf("Thread B joined\n");
    
    log_printf("Main func finished\n");
    return 0;
}
```

Способ достичь успеха без боли: все изменения данных делаем под mutex. Операции с condvar тоже делаем только под заблокированным mutex.

# <a name="condvar_queue"></a> Пример thread-safe очереди


```cpp
%%cpp condvar_queue.c
%run gcc -fsanitize=thread condvar_queue.c -lpthread -o condvar_queue.exe
%run (for i in $(seq 0 100000); do echo -n "$i " ; done) | ./condvar_queue.exe > out.txt
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
        // Не важно где внутри мьютекса посылать сигнал, так как другой поток не сможет зайти в критическую секцию, пока не завершится текущая
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


```python

```

# <a name="c_atomic_life"></a> Atomic в C и как с этим жить


В C++ атомарные переменные реализованы через `std::atomic` в силу объектной ориентированности языка. 
В C же к объявлению переменной приписывается _Atomic или _Atomic(). Лучше использовать второй вариант (почему, будет ниже). Ситуация усложняется отсуствием документации. Про атомарные функции с переменными можно посмотреть в ридинге Яковлева.

## Пример с _Atomic


```cpp
%%cpp atomic_example1.c
%run gcc -fsanitize=thread atomic_example1.c -lpthread -o atomic_example1.exe
%run ./atomic_example1.exe > out.txt
%run cat out.txt

#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>

// _Atomic навешивается на `int`
_Atomic int x;

int main(int argc, char* argv[]) {
    atomic_store(&x, 1);
    printf("%d\n", atomic_load(&x));
    
    int i = 2;
    // изменение не пройдет, так как x = 1, а i = 2, i станет равным x
    atomic_compare_exchange_strong(&x, &i, 3);
    printf("%d\n", atomic_load(&x));

    // тут пройдет
    atomic_compare_exchange_strong(&x, &i, 3);
    printf("%d\n", atomic_load(&x));
    return 0;
}
```

Казалось бы все хорошо, но давайте попробуем с указателями


```cpp
%%cpp atomic_example2.c
%run gcc -fsanitize=thread atomic_example2.c -lpthread -o atomic_example2.exe
%run ./atomic_example2.exe > out.txt
%run cat out.txt

#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// ПЛОХОЙ КОД!!!
_Atomic int* x;

int main(int argc, char* argv[]) {
    int data[3] = {10, 20, 30};
    int* one = data + 0;
    int* two = data + 1;
    int* three = data + 2;
    
    atomic_store(&x, one);

    printf("%d\n", *atomic_load(&x));
    
    int* i = two;
    // изменение не пройдет, так как x = 1, а i = 2, i станет равным x
    atomic_compare_exchange_strong(&x, &i, three);
    printf("%d\n", *atomic_load(&x));

    i = one;
    // тут пройдет
    atomic_compare_exchange_strong(&x, &i, three);
    printf("%d\n", *atomic_load(&x));
    return 0;
}
```

Получаем ад из warning/error от компилятора
(все в зависимости от компилятора и платформы: `gcc 7.4.0 Ubuntu 18.04.1` - warning, `clang 11.0.0 macOS` - error).

Может появиться желание написать костыль, явно прикастовав типы:


```cpp
%%cpp atomic_example3.c
%run gcc -fsanitize=thread atomic_example3.c -lpthread -o atomic_example3.exe
%run ./atomic_example3.exe > out.txt
%run cat out.txt

#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// ПЛОХОЙ КОД!!!
_Atomic int* x;

int main(int argc, char* argv[]) {
    int data[3] = {10, 20, 30};
    int* one = data + 0;
    int* two = data + 1;
    int* three = data + 2;
    
    atomic_store(&x, (_Atomic int*) one);

    printf("%d\n", *(int*)atomic_load(&x));

    int* i = two;
    // изменение не пройдет, так как x = 1, а i = 2, i станет равным x
    atomic_compare_exchange_strong(&x, (_Atomic int**) &i, (_Atomic int*) three);
    printf("%d\n", *(int*)atomic_load(&x));
   
    i = one;
    // тут пройдет
    atomic_compare_exchange_strong(&x, (_Atomic int**) &i, (_Atomic int*) three);
    i = (int*) atomic_load(&x);
    printf("%d\n", *(int*)atomic_load(&x));
    return 0;
}
```

Теперь gcc перестает кидать warnings (в clang до сих пор error). Но код может превратиться в ад из кастов. 

Но! Этот код идейно полностью некорректен.

Посмотрим на `_Atomic int* x;`
В данном случае это работает как `(_Atomic int)* x`, а не как `_Atomic (int*) x` что легко подумать!
<br>То есть получается неатомарный указатель на атомарный `int`. Хотя задумывалось как атомарный указатель на неатомарный `int`.

Поэтому лучше использовать `_Atomic (type)`.

При его использовании код становится вполне читаемым и что главное - корректным. Соответственно компилируется без проблем в gcc/clang.

## Как надо писать


```cpp
%%cpp atomic_example4.c
%run gcc -fsanitize=thread atomic_example4.c -lpthread -o atomic_example4.exe
%run ./atomic_example4.exe > out.txt
%run cat out.txt

#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// Теперь именно атомарный указатель. Как и должно было быть.
_Atomic (int*) x;

int main(int argc, char* argv[]) {
    int data[3] = {10, 20, 30};
    int* one = data + 0;
    int* two = data + 1;
    int* three = data + 2;
    
    atomic_store(&x, one);
    printf("%d\n", *atomic_load(&x));
    
    int* i = two;
    // изменение не пройдет, так как x = 1, а i = 2, i станет равным x
    atomic_compare_exchange_strong(&x, &i, three);
    printf("%d\n", *atomic_load(&x));

    i = one;
    // тут пройдет
    atomic_compare_exchange_strong(&x, &i, three);
    printf("%d\n", *atomic_load(&x));
    return 0;
}
```

## Более общая мысль

В общем-то тут нет ничего особого. Так же как и _Atomic ведет себя всем знакомый const.

Все же помнят, что `const int*` это неконстантный указатель на константный `int`? :)

Но натолкнувшись на ошибку компиляции в одном из вышеприведенных примеров, два человека очень долго тупили. Поэтому они написали этот текст :) 

Чтобы вы, читающие, не тупили как мы. Удачи!


```python

```

# <a name="hw"></a> Комментарии к ДЗ

* inf17-0: posix/threads/mutex
  <br>Много потоков, много мьютексов, циклический список.
  <br>Решения с одним мьютексом не принимаются (в таких решениях потоки не будут выполнять работу (сложение чисел) параллельно). На каждый элемент нужен отдельный мьютекс (на самом деле необязательно, разрешаю применять творческий подход).
  <br>При этом изменение трех чисел должно происходить атомарно с точки зрения гипотетического потока, который в любой момент, может взять лок на все мьютексы и прочитать состояние всех чисел.
* inf17-1: posix/threads/condvar
  <br>Здесь необязательно реализовывать очередь, а тем более ее копировать, но принципе тот же.
  <br>Вспоминаем про аргументы передаваемые потоку.
  <br>Нельзя использовать `pipe` и `socketpair`.
  <br>В задаче есть две "тяжелые" операции: поиск простого числа и его вывод. Они должны параллелиться по типу конвеера.
* inf17-2: posix/threads/atomic
  <br>Задачка на CAS


```python

```


```python

```
