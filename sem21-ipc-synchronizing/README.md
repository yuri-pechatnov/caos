


# Inter Process Communications (IPC)

<br>
<div style="text-align: right"> Спасибо <a href="https://github.com/SyrnikRebirth">Сове Глебу</a> и <a href="https://github.com/Disadvantaged">Голяр Димитрису</a> за участие в написании текста </div>
<br>


Сегодня в программе:
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

[Ридинг Яковлева](https://github.com/victor-yacovlev/mipt-diht-caos/tree/master/practice/posix_ipc)

# <a name="mmap"></a> `mmap`


Разделяемая память - это когда два региона виртуальной памяти (один в одном процессе, другой в другом) 
ссылаются на одну и ту же физическую память. То есть могут обмениваться информацией через нее.

Межпроцессное взаимодействие через разделяемую память нужно, 
когда у нас есть две различные программы (могут быть написаны на разных языках)
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
#include <pthread.h>
#include <stdatomic.h>

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
    pa_assert(state->current_state == VALID_STATE);
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
    pa_assert(state != MAP_FAILED);
    
    // create and initialize interprocess mutex
    pthread_mutexattr_t mutex_attrs; 
    pa_assert(pthread_mutexattr_init(&mutex_attrs) == 0);
    // Важно! Без этого атрибута один из процессов навсегда зависнет в lock мьютекса
    // Вероятно этот атрибут влияет на отсутствие флага FUTEX_PRIVATE_FLAG в операциях с futex
    // Если он стоит, то ядро может делать некоторые оптимизации в предположении, что futex используется одним процессом
    pa_assert(pthread_mutexattr_setpshared(&mutex_attrs, PTHREAD_PROCESS_SHARED) == 0);
    pa_assert(pthread_mutex_init(&state->mutex, &mutex_attrs) == 0);
    pa_assert(pthread_mutexattr_destroy(&mutex_attrs) == 0);
    
    state->current_state = VALID_STATE; // Инициализирем защищаемое состояние
    return state;
}

void delete_state(shared_state_t* state) {
    pa_assert(pthread_mutex_destroy(&state->mutex) == 0);
    pa_assert(munmap(state, sizeof(shared_state_t)) == 0);
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
        pa_assert((processes[i] = fork()) >= 0); 
        if (processes[i] == 0) {
            process_func(i); // Имитируем работу из разных процессов
            exit(0);
        }
    }
    for (int i = 0; i < process_count; ++i) {
        int status;
        pa_assert(waitpid(processes[i], &status, 0) != -1)
        pa_assert(WIFEXITED(status) && WEXITSTATUS(status) == 0);
        log_printf("Process %d 'joined'\n", i);
    }
    delete_state(state);
    log_printf("Main process finished\n");
    return 0;
}
```


Run: `gcc -Wall -fsanitize=thread mmap.c -lpthread -o mmap.exe`



Run: `./mmap.exe`


    23:50:46.929 mmap.c: 95 [pid=18821]: Main process started
    23:50:46.936 mmap.c:100 [pid=18821]: Creating process 0
    23:50:46.940 mmap.c:100 [pid=18821]: Creating process 1
    23:50:46.945 mmap.c: 56 [pid=18823]:   Process 0 started
    23:50:46.948 mmap.c: 56 [pid=18824]:   Process 1 started
    23:50:47.381 mmap.c: 60 [pid=18823]:   Process 0 finished
    23:50:47.383 mmap.c:111 [pid=18821]: Process 0 'joined'
    23:50:47.393 mmap.c: 60 [pid=18824]:   Process 1 finished
    23:50:47.396 mmap.c:111 [pid=18821]: Process 1 'joined'
    23:50:47.396 mmap.c:114 [pid=18821]: Main process finished


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
#include <pthread.h>
#include <stdatomic.h>

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
    pa_assert(state->current_state == VALID_STATE);
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
    pa_assert(state != MAP_FAILED);
    
    state->lock = 0;
    state->current_state = VALID_STATE;
    return state;
}

void delete_state(shared_state_t* state) {
    pa_assert(munmap(state, sizeof(shared_state_t)) == 0);
}

int main()
{
    log_printf("Main process started\n");
    state = create_state();
    const int process_count = 2;
    pid_t processes[process_count];
    for (int i = 0; i < process_count; ++i) {
        log_printf("Creating process %d\n", i);
        pa_assert((processes[i] = fork()) >= 0);
        if (processes[i] == 0) {
            process_func(i);
            exit(0);
        }
    }
    for (int i = 0; i < process_count; ++i) {
        int status;
        pa_assert(waitpid(processes[i], &status, 0) != -1)
        pa_assert(WIFEXITED(status) && WEXITSTATUS(status) == 0);
        log_printf("Process %d 'joined'\n", i);
    }
    delete_state(state);
    log_printf("Main process finished\n");
    return 0;
}
```


Run: `gcc -Wall -fsanitize=thread mmap.c -lpthread -o mmap.exe`



Run: `./mmap.exe`


    10:57:00.877 mmap.c: 97 [pid=24117]: Main process started
    10:57:00.886 mmap.c:102 [pid=24117]: Creating process 0
    10:57:00.887 mmap.c:102 [pid=24117]: Creating process 1
    10:57:00.890 mmap.c: 67 [pid=24119]:   Process 0 started
    10:57:00.899 mmap.c: 67 [pid=24120]:   Process 1 started
    10:57:00.990 mmap.c: 71 [pid=24119]:   Process 0 finished
    10:57:00.991 mmap.c:113 [pid=24117]: Process 0 'joined'
    10:57:01.029 mmap.c: 71 [pid=24120]:   Process 1 finished
    10:57:01.030 mmap.c:113 [pid=24117]: Process 1 'joined'
    10:57:01.030 mmap.c:116 [pid=24117]: Main process finished


# <a name="shm"></a> `shm_open`

Сделаем то же самое, что и в предыдущем примере, но на этот раз не из родственных процессов. Воспользуемся именноваными объектами разделяемой памяти.


```cpp
%%cpp shm.c
%# Обратите внимание: -lrt. Здесь нужна новая разделяемая библиотека
%run gcc -Wall -fsanitize=thread shm.c -lrt -lpthread -o s.exe
%run ./s.exe create_shm /my_shm
%run ./s.exe work 1 /my_shm & PID=$! ; ./s.exe work 2 /my_shm ; wait $PID
%run ./s.exe remove_shm /my_shm

#define _GNU_SOURCE 
#include <stdio.h> 
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <pthread.h>
#include <string.h>

#include <sys/stat.h>
#include <fcntl.h>

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
    pa_assert(state->current_state == VALID_STATE);
    state->current_state = INVALID_STATE; // do some work with state. 
    sched_yield();
    state->current_state = VALID_STATE;
    pthread_mutex_unlock(&state->mutex);
}
 
shared_state_t* load_state(const char* shm_name, bool do_create) {
    // открываем / создаем объект разделяемой памяти
    // по сути это просто open, только для виртуального файла (без сохранения данных на диск + ортогональное пространство имен)
    int fd = shm_open(shm_name, O_RDWR | (do_create ? O_CREAT : 0), 0644);
    pa_assert(fd >= 0);
    pa_assert(ftruncate(fd, sizeof(shared_state_t)) == 0);
    shared_state_t* state = mmap(
        /* desired addr, addr = */ NULL, 
        /* length = */ sizeof(shared_state_t), 
        /* access attributes, prot = */ PROT_READ | PROT_WRITE, 
        /* flags = */ MAP_SHARED,
        /* fd = */ fd,
        /* offset in file, offset = */ 0
    );
    pa_assert(state != MAP_FAILED);
    if (!do_create) {
        return state;
    }
    // create interprocess mutex
    pthread_mutexattr_t mutex_attrs;
    pa_assert(pthread_mutexattr_init(&mutex_attrs) == 0);
    // Важно!
    pa_assert(pthread_mutexattr_setpshared(&mutex_attrs, PTHREAD_PROCESS_SHARED) == 0);
    pa_assert(pthread_mutex_init(&state->mutex, &mutex_attrs) == 0);
    pa_assert(pthread_mutexattr_destroy(&mutex_attrs) == 0);
    
    state->current_state = VALID_STATE;
    return state;
}

void unload_state(shared_state_t* state) {
    pa_assert(munmap(state, sizeof(shared_state_t)) == 0);
}

int main(int argc, char** argv)
{
    pa_assert(argc >= 2);
    if (strcmp("create_shm", argv[1]) == 0) {
        log_printf("  Creating state: %s\n", argv[2]);
        unload_state(load_state(argv[2], /*do_create=*/ 1));
        log_printf("  State created\n");
    } else if (strcmp("remove_shm", argv[1]) == 0) {
        log_printf("  Removing state: %s\n", argv[2]);
        // Файлы shm существуют пока не будет вызвана unlink.
        pa_assert(shm_unlink(argv[2]) == 0)
        log_printf("  State removed\n");   
    } else if (strcmp("work", argv[1]) == 0) {
        pa_assert(argc >= 3);
        int worker = strtol(argv[2], 0, 10);
        log_printf("  Worker %d started\n", worker);
        shared_state_t* state = load_state(argv[3], /*do_create=*/ 0);
       
        for (int j = 0; j < 10000; ++j) {
            process_safe_func(state);
        }

        unload_state(state);
        log_printf("  Worker %d finished\n", worker);
    } else {
        pa_assert(0 && "unknown command")
    }
    return 0;
}
```


Run: `gcc -Wall -fsanitize=thread shm.c -lrt -lpthread -o s.exe`



Run: `./s.exe create_shm /my_shm`


    18:26:22.229 shm.c: 95 [pid=453]:   Creating state: /my_shm
    18:26:22.272 shm.c: 97 [pid=453]:   State created



Run: `./s.exe work 1 /my_shm & PID=$! ; ./s.exe work 2 /my_shm ; wait $PID`


    18:26:22.783 shm.c:105 [pid=457]:   Worker 2 started
    18:26:22.803 shm.c:105 [pid=456]:   Worker 1 started
    18:26:24.322 shm.c:113 [pid=457]:   Worker 2 finished
    18:26:24.366 shm.c:113 [pid=456]:   Worker 1 finished



Run: `./s.exe remove_shm /my_shm`


    18:26:24.747 shm.c: 99 [pid=461]:   Removing state: /my_shm
    18:26:24.770 shm.c:101 [pid=461]:   State removed


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


```cpp
%%cpp sem_anon.c
%run gcc -Wall -fsanitize=thread -lrt sem_anon.c -o sem_anon.exe
%run ./sem_anon.exe

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
    pa_assert(state->current_state == VALID_STATE);
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
    pa_assert(state != MAP_FAILED);
    
    // create interprocess semaphore
    pa_assert(sem_init(
        &state->semaphore,
        1, // interprocess? (0 if will be used in one process)
        1  // initial value
    ) == 0);
    
    state->current_state = VALID_STATE;
    return state;
}

void delete_state(shared_state_t* state) {
    pa_assert(sem_destroy(&state->semaphore) == 0);
    pa_assert(munmap(state, sizeof(shared_state_t)) == 0);
}

int main()
{
    log_printf("Main process started\n");
    state = create_state();
    const int process_count = 2;
    pid_t processes[process_count];
    for (int i = 0; i < process_count; ++i) {
        log_printf("Creating process %d\n", i);
        pa_assert((processes[i] = fork()) >= 0);
        if (processes[i] == 0) {
            process_func(i);
            exit(0);
        }
    }
    for (int i = 0; i < process_count; ++i) {
        int status;
        pa_assert(waitpid(processes[i], &status, 0) != -1)
        pa_assert(WIFEXITED(status) && WEXITSTATUS(status) == 0);
        log_printf("Process %d 'joined'\n", i);
    }
    delete_state(state);
    log_printf("Main process finished\n");
    return 0;
}
```


Run: `gcc -Wall -fsanitize=thread sem_anon.c -o sem_anon.exe`



Run: `./sem_anon.exe`


    18:53:55.475 sem_anon.c: 92 [pid=745]: Main process started
    18:53:55.482 sem_anon.c: 97 [pid=745]: Creating process 0
    18:53:55.487 sem_anon.c: 97 [pid=745]: Creating process 1
    18:53:55.502 sem_anon.c: 55 [pid=747]:   Process 0 started
    18:53:55.530 sem_anon.c: 55 [pid=749]:   Process 1 started
    18:53:56.946 sem_anon.c: 59 [pid=749]:   Process 1 finished
    18:53:56.966 sem_anon.c: 59 [pid=747]:   Process 0 finished
    18:53:56.967 sem_anon.c:108 [pid=745]: Process 0 'joined'
    18:53:56.967 sem_anon.c:108 [pid=745]: Process 1 'joined'
    18:53:56.967 sem_anon.c:111 [pid=745]: Main process finished


# <a name="sem_named"></a> Именнованные семафоры

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
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <pthread.h>
#include <string.h>

#include <sys/stat.h>
#include <fcntl.h>
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
#define pa_warn_if_not(stmt) if (stmt) {} else { log_printf("WARNING: '" #stmt "' failed\n"); }
#define pa_assert(stmt) if (stmt) {} else { log_printf("'" #stmt "' failed\n"); exit(EXIT_FAILURE); }

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
    pa_assert(state->current_state == VALID_STATE);
    state->current_state = INVALID_STATE; // do some work with state. 
    sched_yield();
    state->current_state = VALID_STATE;
    pthread_mutex_unlock(&state->mutex);
}
 
shared_state_t* load_state(const char* shm_name, bool do_create) {
    // открываем / создаем объект разделяемой памяти
    int fd = shm_open(shm_name, O_RDWR | (do_create ? O_CREAT : 0), 0644);
    if (do_create) {
        pa_assert(ftruncate(fd, sizeof(shared_state_t)) == 0);
    }
    shared_state_t* state = mmap(
        /* desired addr, addr = */ NULL, 
        /* length = */ sizeof(shared_state_t), 
        /* access attributes, prot = */ PROT_READ | PROT_WRITE, 
        /* flags = */ MAP_SHARED,
        /* fd = */ fd,
        /* offset in file, offset = */ 0
    );
    pa_assert(state != MAP_FAILED);
    if (do_create) {
        // create interprocess mutex
        pthread_mutexattr_t mutex_attrs;
        pa_assert(pthread_mutexattr_init(&mutex_attrs) == 0);
        // Важно!
        pa_assert(pthread_mutexattr_setpshared(&mutex_attrs, PTHREAD_PROCESS_SHARED) == 0);
        pa_assert(pthread_mutex_init(&state->mutex, &mutex_attrs) == 0);
        pa_assert(pthread_mutexattr_destroy(&mutex_attrs) == 0);

        state->current_state = VALID_STATE;
    }
    return state;
}

void unload_state(shared_state_t* state) {
    pa_assert(munmap(state, sizeof(shared_state_t)) == 0);
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
        pa_assert(init_semaphore != SEM_FAILED);
        sem_wait(init_semaphore); // ждем весточку, что стейт готов
        sem_post(init_semaphore); // возвращаем весточку на место, чтобы другим процессам тоже досталось
        sem_close(init_semaphore);
        return load_state(name, /*do_create=*/ 0);
    }
}

int main(int argc, char** argv)
{
    pa_assert(argc >= 2);
    if (strcmp("cleanup", argv[1]) == 0) {
        log_printf("  Cleanup sem and shm: %s\n", argv[2]);
        pa_warn_if_not(shm_unlink(argv[2]) == 0);
        pa_warn_if_not(sem_unlink(argv[2]) == 0);
        log_printf("  State created\n");
    } else if (strcmp("work", argv[1]) == 0) {
        pa_assert(argc >= 3);
        int worker = strtol(argv[2], 0, 10);
        log_printf("  Worker %d started\n", worker);
        
        shared_state_t* state = process_safe_init_and_load(argv[3]);
       
        for (int j = 0; j < 10000; ++j) {
            process_safe_func(state);
        }
     
        unload_state(state);
        log_printf("  Worker %d finished\n", worker);
    } else {
        pa_assert(0 && "unknown command")
    }
    return 0;
}
```


Run: `gcc -Wall -fsanitize=thread sem_named.c -lrt -lpthread -o s.exe`



Run: `./s.exe work 1 /s42 & PID=$! ; ./s.exe work 2 /s42 ; wait $PID`


    17:14:22.861 sem_named.c:125 [pid=31019]:   Worker 1 started
    17:14:22.862 sem_named.c:125 [pid=31020]:   Worker 2 started
    17:14:22.887 sem_named.c:134 [pid=31020]:   Worker 2 finished
    17:14:22.892 sem_named.c:134 [pid=31019]:   Worker 1 finished



Run: `./s.exe cleanup /s42 # необязательная команда. Будет работать и без нее`


    17:14:23.016 sem_named.c:118 [pid=31024]:   Cleanup sem and shm: /s42
    17:14:23.016 sem_named.c:121 [pid=31024]:   State created


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
