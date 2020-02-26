```python
# look at tools/set_up_magics.ipynb
get_ipython().run_cell('# one_liner_str <too much code> \n')
None
```


    <IPython.core.display.Javascript object>


# Потоки и их использование

<br>
<div style="text-align: right"> Спасибо ??? за участие в написании текста </div>
<br>


Сегодня в программе:
* <a href="#ptread_create" style="color:#856024">Создание и join потоков</a>
* <a href="#pthread_result" style="color:#856024">Аргументы и возвращаемое значение потока</a>
* <a href="#pthread_cancel" style="color:#856024">Прерывание/отмена/cancel потока</a>
* <a href="#pthread_attr" style="color:#856024">Атрибуты потока</a>
* <a href="#coro" style="color:#856024">Корутины</a>


<a href="#hw" style="color:#856024">Комментарии к ДЗ</a>

[Ридинг Яковлева](https://github.com/victor-yacovlev/mipt-diht-caos/tree/master/practice/pthread)


Атрибуты процесса
* Виртуальное адресное пространство и данные в этой витруальной памяти
* Файловые дескрипторы, блокировки файлов
* PID
* argc, argv
* ulimit

Для каждого потока свои 
* Маски сигналов
* Состояние процесса R, S, T, Z
* Состояние регистров (какая ф-я сейчас выполняется) (состояние стека скорее входит в состояние вииртуального адресного пространства)
* TID




```python

```

# <a name="pthread_create"></a> Создание и join потока


```cpp
%%cpp pthread_create.cpp
%run gcc pthread_create.cpp -lpthread -o pthread_create.exe
%run ./pthread_create.exe

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <pthread.h>

const char* log_prefix() {
    struct timeval tp; gettimeofday(&tp, NULL);
    static __thread char prefix[100];
    size_t time_len = strftime(prefix, sizeof(prefix), "%H:%M:%S", localtime(&tp.tv_sec));
    sprintf(prefix + time_len, ".%03ld [tid=%ld]", tp.tv_usec / 1000, syscall(__NR_gettid));
    return prefix;
}

#define log_printf_impl(fmt, ...) { time_t t = time(0); dprintf(2, "%s: " fmt "%s", log_prefix(), __VA_ARGS__); }
#define log_printf(...) log_printf_impl(__VA_ARGS__, "")

// thread-aware assert
#define ta_assert(stmt) if (stmt) {} else { log_printf("'" #stmt "' failed"); exit(EXIT_FAILURE); }

static void* thread_func(void* arg)
{
    log_printf("  Thread func started\n");
    log_printf("  Thread func finished\n");
    return NULL;
}

int main()
{
    log_printf("Main func started\n");
    pthread_t thread;
    log_printf("Thread creating\n");
    ta_assert(pthread_create(&thread, NULL, thread_func, 0) == 0);
    ta_assert(pthread_join(thread, NULL) == 0);
    log_printf("Thread joined\n");
    log_printf("Main func finished\n");
    return 0;
}
```


Run: `gcc pthread_create.cpp -lpthread -o pthread_create.exe`



Run: `./pthread_create.exe`


    11:12:39.164 [tid=5954]: Main func started
    11:12:39.169 [tid=5954]: Thread creating
    11:12:39.170 [tid=5955]:   Thread func started
    11:12:39.170 [tid=5955]:   Thread func finished
    11:12:39.170 [tid=5954]: Thread joined
    11:12:39.170 [tid=5954]: Main func finished



```python

```

# <a name="pthread_result"></a> Смотрим на возвращаемое потоком значение.


```cpp
%%cpp pthread_create.cpp
%run gcc pthread_create.cpp -lpthread -o pthread_create.exe
%run ./pthread_create.exe

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <pthread.h>

const char* log_prefix() {
    struct timeval tp; gettimeofday(&tp, NULL);
    static __thread char prefix[100];
    size_t time_len = strftime(prefix, sizeof(prefix), "%H:%M:%S", localtime(&tp.tv_sec));
    sprintf(prefix + time_len, ".%03ld [tid=%ld]", tp.tv_usec / 1000, syscall(__NR_gettid));
    return prefix;
}

#define log_printf_impl(fmt, ...) { time_t t = time(0); dprintf(2, "%s: " fmt "%s", log_prefix(), __VA_ARGS__); }
#define log_printf(...) log_printf_impl(__VA_ARGS__, "")

// thread-aware assert
#define ta_assert(stmt) if (stmt) {} else { log_printf("'" #stmt "' failed"); exit(EXIT_FAILURE); }


typedef struct {
    int a;
    int b;
} thread_task_args_t;

// На самом деле проще записать результат в структуру аргументов
typedef struct {
    int c;
} thread_task_result_t;

static thread_task_result_t* thread_func(const thread_task_args_t *arg)
{
    log_printf("  Thread func started\n");
    thread_task_result_t* result = 
        (thread_task_result_t*)malloc(sizeof(thread_task_result_t));
    result->c = arg->a + arg->b;
    log_printf("  Thread func finished\n");
    return result;
}

int main()
{
    log_printf("Main func started\n");
    pthread_t thread;
    
    thread_task_args_t args = {.a = 35, .b = 7};
    log_printf("Thread creating, args are: a=%d b=%d\n", args.a, args.b);
    ta_assert(pthread_create(
        &thread, NULL, 
        (void* (*)(void*))thread_func, // Важно понимать, что тут происходит
        (void*)&args
    ) == 0);
    
    thread_task_result_t* result;
    ta_assert(pthread_join(thread, (void**)&result) == 0);
    log_printf("Thread joined. Result: c=%d\n", result->c);
    free(result);
    
    log_printf("Main func finished\n");
    return 0;
}
```


Run: `gcc pthread_create.cpp -lpthread -o pthread_create.exe`



Run: `./pthread_create.exe`


    11:02:29.641 [tid=5846]: Main func started
    11:02:29.645 [tid=5846]: Thread creating, args are: a=35 b=7
    11:02:29.645 [tid=5847]:   Thread func started
    11:02:29.646 [tid=5847]:   Thread func finished
    11:02:29.646 [tid=5846]: Thread joined. Result: c=42
    11:02:29.646 [tid=5846]: Main func finished


# <a name="pthread_cancel"></a> Прерывание потока

Пусть это возможно сделать, но с этим нужно быть очень осторожным, особенно если поток, который вы прерываете владеет какими-либо ресурсами


```cpp
%%cpp pthread_cancel.cpp
%run gcc pthread_cancel.cpp -lpthread -o pthread_cancel.exe
%run ./pthread_cancel.exe

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <pthread.h>

const char* log_prefix() {
    struct timeval tp; gettimeofday(&tp, NULL);
    static __thread char prefix[100];
    size_t time_len = strftime(prefix, sizeof(prefix), "%H:%M:%S", localtime(&tp.tv_sec));
    sprintf(prefix + time_len, ".%03ld [tid=%ld]", tp.tv_usec / 1000, syscall(__NR_gettid));
    return prefix;
}

#define log_printf_impl(fmt, ...) { time_t t = time(0); dprintf(2, "%s: " fmt "%s", log_prefix(), __VA_ARGS__); }
#define log_printf(...) log_printf_impl(__VA_ARGS__, "")

// thread-aware assert
#define ta_assert(stmt) if (stmt) {} else { log_printf("'" #stmt "' failed"); exit(EXIT_FAILURE); }

static void* thread_func(void* arg)
{
    log_printf("  Thread func started\n");
    sleep(2);
    log_printf("  Thread func finished\n"); // not printed because thread canceled
    return NULL;
}

int main()
{
    log_printf("Main func started\n");
    pthread_t thread;
    log_printf("Thread creating\n");
    ta_assert(pthread_create(&thread, NULL, thread_func, 0) == 0);
    sleep(1);
    log_printf("Thread canceling\n");
    ta_assert(pthread_cancel(thread) == 0);
    ta_assert(pthread_join(thread, NULL) == 0);
    log_printf("Thread joined\n");
    log_printf("Main func finished\n");
    return 0;
}
```


Run: `gcc pthread_cancel.cpp -lpthread -o pthread_cancel.exe`



Run: `./pthread_cancel.exe`


    11:04:27.448 [tid=5859]: Main func started
    11:04:27.453 [tid=5859]: Thread creating
    11:04:27.453 [tid=5860]:   Thread func started
    11:04:28.454 [tid=5859]: Thread canceling
    11:04:28.454 [tid=5859]: Thread joined
    11:04:28.454 [tid=5859]: Main func finished


По умолчанию pthread_cancel может прерывать поток, только в cancelation points (то есть в функциях, в реализациях которых есть проверка на это). 

Поэтому, если эти функции не вызывать, то поток не сможет быть прерван.

Но можно воспользоваться `pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);`. Тогда поток может быть прерван на уровне планировщика. (То есть поток скорее всего доработает текущий выделенный квант времени, но на следующий квант уже не запустится)


```cpp
%%cpp pthread_cancel_fail.cpp
%run gcc pthread_cancel_fail.cpp -lpthread -o pthread_cancel_fail.exe
%run timeout 3 ./pthread_cancel_fail.exe  # will fail (cancelation at cancelation points)
%run gcc -DASYNC_CANCEL pthread_cancel_fail.cpp -lpthread -o pthread_cancel_fail.exe
%run timeout 3 ./pthread_cancel_fail.exe  # ok, async cancelation

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <pthread.h>

const char* log_prefix() {
    struct timeval tp; gettimeofday(&tp, NULL);
    static __thread char prefix[100];
    size_t time_len = strftime(prefix, sizeof(prefix), "%H:%M:%S", localtime(&tp.tv_sec));
    sprintf(prefix + time_len, ".%03ld [tid=%ld]", tp.tv_usec / 1000, syscall(__NR_gettid));
    return prefix;
}

#define log_printf_impl(fmt, ...) { time_t t = time(0); dprintf(2, "%s: " fmt "%s", log_prefix(), __VA_ARGS__); }
#define log_printf(...) log_printf_impl(__VA_ARGS__, "")

// thread-aware assert
#define ta_assert(stmt) if (stmt) {} else { log_printf("'" #stmt "' failed"); exit(EXIT_FAILURE); }

static void *
thread_func(void *arg)
{
    log_printf("  Thread func started\n");
    #ifdef ASYNC_CANCEL
    ta_assert(pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL) == 0);
    #endif
    while (1); // зависаем тут. В процессе явно не будет cancelation points
    log_printf("  Thread func finished\n"); 
    return NULL;
}

int main()
{
    log_printf("Main func started\n");
    pthread_t thread;
    log_printf("Thread creating\n");
    ta_assert(pthread_create(&thread, NULL, thread_func, 0) == 0);
    sleep(1);
    log_printf("Thread canceling\n");
    ta_assert(pthread_cancel(thread) == 0);
    log_printf("Thread joining\n");
    ta_assert(pthread_join(thread, NULL) == 0);
    log_printf("Thread joined\n");
    log_printf("Main func finished\n");
    return 0;
}
```


Run: `gcc pthread_cancel_fail.cpp -lpthread -o pthread_cancel_fail.exe`



Run: `timeout 3 ./pthread_cancel_fail.exe  # will fail (cancelation at cancelation points)`


    11:11:15.118 [tid=5925]: Main func started
    11:11:15.125 [tid=5925]: Thread creating
    11:11:15.125 [tid=5926]:   Thread func started
    11:11:16.125 [tid=5925]: Thread canceling
    11:11:16.129 [tid=5925]: Thread joining



Run: `gcc -DASYNC_CANCEL pthread_cancel_fail.cpp -lpthread -o pthread_cancel_fail.exe`



Run: `timeout 3 ./pthread_cancel_fail.exe  # ok, async cancelation`


    11:11:18.785 [tid=5935]: Main func started
    11:11:18.786 [tid=5935]: Thread creating
    11:11:18.786 [tid=5936]:   Thread func started
    11:11:19.789 [tid=5935]: Thread canceling
    11:11:19.789 [tid=5935]: Thread joining
    11:11:19.789 [tid=5935]: Thread joined
    11:11:19.789 [tid=5935]: Main func finished



```python

```

## А можно ли приджойнить основной поток?


```cpp
%%cpp join_main_thread.cpp
%run gcc join_main_thread.cpp -lpthread -o join_main_thread.exe
%run timeout 3 ./join_main_thread.exe ; echo "Exit code: $?"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <pthread.h>

const char* log_prefix() {
    struct timeval tp; gettimeofday(&tp, NULL);
    static __thread char prefix[100];
    size_t time_len = strftime(prefix, sizeof(prefix), "%H:%M:%S", localtime(&tp.tv_sec));
    sprintf(prefix + time_len, ".%03ld [tid=%ld]", tp.tv_usec / 1000, syscall(__NR_gettid));
    return prefix;
}

#define log_printf_impl(fmt, ...) { time_t t = time(0); dprintf(2, "%s: " fmt "%s", log_prefix(), __VA_ARGS__); }
#define log_printf(...) log_printf_impl(__VA_ARGS__, "")

// thread-aware assert
#define ta_assert(stmt) if (stmt) {} else { log_printf("'" #stmt "' failed"); exit(EXIT_FAILURE); }

pthread_t main_thread;

static void* thread_func(void* arg)
{
    log_printf("  Thread func started\n");
  
    log_printf("  Main thread joining\n");
    ta_assert(pthread_join(main_thread, NULL) == 0);
    log_printf("  Main thread joined\n");

    log_printf("  Thread func finished\n");

    _exit(42);
}

int main()
{
    log_printf("Main func started\n");
    main_thread = pthread_self();
    
    pthread_t thread;
    log_printf("Thread creating\n");
    ta_assert(pthread_create(&thread, NULL, thread_func, 0) == 0);
    
    pthread_exit(NULL);
}
```


Run: `gcc join_main_thread.cpp -lpthread -o join_main_thread.exe`



Run: `timeout 3 ./join_main_thread.exe ; echo "Exit code: $?"`


    11:17:58.189 [tid=6084]: Main func started
    11:17:58.197 [tid=6084]: Thread creating
    11:17:58.198 [tid=6085]:   Thread func started
    11:17:58.198 [tid=6085]:   Main thread joining
    11:17:58.198 [tid=6085]:   Main thread joined
    11:17:58.198 [tid=6085]:   Thread func finished
    Exit code: 42



```python

```


```python

```

# <a name="pthread_attr"></a> Атрибуты потока

* Размер стека
* Местоположение стека
* Размер защитной области после стека. Вот тут можно прокомментировать: это область ниже стека, которая является дырой в виртуальном адресном пространстве программы. То есть при попытке обращения к этой области произойдет segfault. Для чего необходима защитная область? Чтобы при переполнении стека получать segfault, а не неотлавливаемый проезд по памяти.


В следующем примере создадим поток двумя способами. С параметрами по умолчанию и указав минимальный размер стека. И посмотрим на потребления памяти. 

(Да, потреблениЯ. Там все не так просто, как кажется на первый взгляд :) )


```cpp
%%cpp pthread_stack_size.cpp
%run gcc pthread_stack_size.cpp -lpthread -o pthread_stack_size.exe
%run ./pthread_stack_size.exe 
%run gcc -DMY_STACK_SIZE=16384 pthread_stack_size.cpp -lpthread -o pthread_stack_size.exe
%run ./pthread_stack_size.exe 

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <pthread.h>

const char* log_prefix() {
    struct timeval tp; gettimeofday(&tp, NULL);
    static __thread char prefix[100];
    size_t time_len = strftime(prefix, sizeof(prefix), "%H:%M:%S", localtime(&tp.tv_sec));
    sprintf(prefix + time_len, ".%03ld [tid=%ld]", tp.tv_usec / 1000, syscall(__NR_gettid));
    return prefix;
}

#define log_printf_impl(fmt, ...) { time_t t = time(0); dprintf(2, "%s: " fmt "%s", log_prefix(), __VA_ARGS__); }
#define log_printf(...) log_printf_impl(__VA_ARGS__, "")

// thread-aware assert
#define ta_assert(stmt) if (stmt) {} else { log_printf("'" #stmt "' failed"); exit(EXIT_FAILURE); }

long int get_maxrss() {
    struct rusage usage;
    ta_assert(getrusage(RUSAGE_SELF, &usage) == 0);
    return usage.ru_maxrss;
}

const char* run_and_get_output(const char* bash_cmd) {
    int fds[2];
    pipe(fds);
    int pid = fork();
    if (pid == 0) {
        dup2(fds[1], 1);
        close(fds[0]); close(fds[1]);
        execlp("bash", "bash", "-c", bash_cmd, NULL);
        ta_assert(0 && "unreachable");
    }
    close(fds[1]);
    static __thread char buffer[100];
    int size = 0, rd = 0;
    while ((rd = read(fds[0], buffer, sizeof(buffer) - size)) != 0) {
        if (rd > 0) {
            size += rd;
        }
    }
    buffer[size] = 0;
    return buffer;
}

long int get_vm_usage() {  
    char cmd1[10000];
    sprintf(cmd1, "cat /proc/%d/status | grep VmData", getpid());
    const char* vm_usage_s = run_and_get_output(cmd1);
    long int vm_usage;
    sscanf(vm_usage_s, "VmData: %ld kB", &vm_usage);
    return vm_usage;
}

static void *
thread_func(void *arg)
{
//     int a[800000];
//     for (int i = 0; i < sizeof(a) / sizeof(int); ++i) {
//         a[i] = i;
//     }   
    log_printf("  Thread func started\n");
    sleep(2);
    log_printf("  Thread func finished\n"); 
    return NULL;
}

int main()
{
    long int initial_rss = get_maxrss();
    long int initial_vm_size = get_vm_usage();
    log_printf("Main func started. Initial RSS = %ldkb, initial VM usage = %ldkb\n", 
               initial_rss, initial_vm_size);
    pthread_t thread;
    pthread_attr_t thread_attr; 
    ta_assert(pthread_attr_init(&thread_attr) == 0);
    #ifdef MY_STACK_SIZE
    ta_assert(pthread_attr_setstacksize(&thread_attr, MY_STACK_SIZE) == 0);
    #endif
    log_printf("Thread creating\n");
    ta_assert(pthread_create(&thread, &thread_attr, thread_func, 0) == 0);
    ta_assert(pthread_attr_destroy(&thread_attr) == 0);
    sleep(1);
    
    log_printf("Thread working. RSS = %ldkb, delta RSS = %ldkb\n", 
               get_maxrss(), get_maxrss() - initial_rss);
    log_printf("Thread working. VM size = %ldkb, VM delta size = %ldkb (!)\n", 
               get_vm_usage(), get_vm_usage() - initial_vm_size);
    
    ta_assert(pthread_join(thread, NULL) == 0);
    log_printf("Thread joined\n");
    log_printf("Main func finished\n");
    return 0;
}
```


Run: `gcc pthread_stack_size.cpp -lpthread -o pthread_stack_size.exe`



Run: `./pthread_stack_size.exe`


    11:30:53.776 [tid=6189]: Main func started. Initial RSS = 760kb, initial VM usage = 72kb
    11:30:53.776 [tid=6189]: Thread creating
    11:30:53.777 [tid=6193]:   Thread func started
    11:30:54.779 [tid=6189]: Thread working. RSS = 760kb, delta RSS = 0kb
    11:30:54.819 [tid=6189]: Thread working. VM size = 8528kb, VM delta size = 8456kb (!)
    11:30:55.778 [tid=6193]:   Thread func finished
    11:30:55.778 [tid=6189]: Thread joined
    11:30:55.778 [tid=6189]: Main func finished



Run: `gcc -DMY_STACK_SIZE=16384 pthread_stack_size.cpp -lpthread -o pthread_stack_size.exe`



Run: `./pthread_stack_size.exe`


    11:30:56.501 [tid=6207]: Main func started. Initial RSS = 720kb, initial VM usage = 72kb
    11:30:56.502 [tid=6207]: Thread creating
    11:30:56.502 [tid=6211]:   Thread func started
    11:30:57.505 [tid=6207]: Thread working. RSS = 720kb, delta RSS = 0kb
    11:30:57.548 [tid=6207]: Thread working. VM size = 348kb, VM delta size = 276kb (!)
    11:30:58.503 [tid=6211]:   Thread func finished
    11:30:58.503 [tid=6207]: Thread joined
    11:30:58.503 [tid=6207]: Main func finished



```python

```


```python

```


```python

```

# <a name="coro"></a> Coroutines


```python
!rm -rf ./libtask
!git clone git@github.com:0intro/libtask.git
!cd libtask && make 
```

    Cloning into 'libtask'...
    remote: Enumerating objects: 143, done.[K
    remote: Total 143 (delta 0), reused 0 (delta 0), pack-reused 143[K
    Receiving objects: 100% (143/143), 43.33 KiB | 0 bytes/s, done.
    Resolving deltas: 100% (90/90), done.
    Checking connectivity... done.
    gcc -c asm.S
    gcc -Wall -Wextra -c -I. -ggdb channel.c
    gcc -Wall -Wextra -c -I. -ggdb context.c
    gcc -Wall -Wextra -c -I. -ggdb fd.c
    gcc -Wall -Wextra -c -I. -ggdb net.c
    gcc -Wall -Wextra -c -I. -ggdb print.c
    gcc -Wall -Wextra -c -I. -ggdb qlock.c
    gcc -Wall -Wextra -c -I. -ggdb rendez.c
    gcc -Wall -Wextra -c -I. -ggdb task.c
    gcc -Wall -Wextra -c -I. -ggdb ip.c
    ar rvc libtask.a asm.o channel.o context.o fd.o net.o print.o qlock.o rendez.o task.o ip.o 
    a - asm.o
    a - channel.o
    a - context.o
    a - fd.o
    a - net.o
    a - print.o
    a - qlock.o
    a - rendez.o
    a - task.o
    a - ip.o
    gcc -Wall -Wextra -c -I. -ggdb echo.c
    gcc -o echo echo.o libtask.a
    gcc -Wall -Wextra -c -I. -ggdb httpload.c
    gcc -o httpload httpload.o libtask.a
    gcc -Wall -Wextra -c -I. -ggdb primes.c
    gcc -o primes primes.o libtask.a
    gcc -Wall -Wextra -c -I. -ggdb tcpload.c
    gcc -o tcpload tcpload.o libtask.a
    gcc -Wall -Wextra -c -I. -ggdb tcpproxy.c
    gcc -o tcpproxy tcpproxy.o libtask.a 
    gcc -Wall -Wextra -c -I. -ggdb testdelay.c
    gcc -o testdelay testdelay.o libtask.a



```cpp
%%cpp coro.cpp
%run gcc -I ./libtask coro.cpp ./libtask/libtask.a -lpthread -o coro.exe
%run ./coro.exe 300 100 200 1000

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <task.h>


const char* log_prefix() {
    struct timeval tp; gettimeofday(&tp, NULL);
    static __thread char prefix[100];
    size_t time_len = strftime(prefix, sizeof(prefix), "%H:%M:%S", localtime(&tp.tv_sec));
    sprintf(prefix + time_len, ".%03ld [tid=%ld]", tp.tv_usec / 1000, syscall(__NR_gettid));
    return prefix;
}

#define log_printf_impl(fmt, ...) { time_t t = time(0); dprintf(2, "%s: " fmt "%s", log_prefix(), __VA_ARGS__); }
#define log_printf(...) log_printf_impl(__VA_ARGS__, "")

// thread-aware assert
#define ta_assert(stmt) if (stmt) {} else { log_printf("'" #stmt "' failed"); exit(EXIT_FAILURE); }


const int STACK_SIZE = 32768;

Channel *c;

void delaytask(void *v)
{
    int ms = *(int*)(void*)&v;
    taskdelay(ms);
    log_printf("Task %dms is launched\n", ms);
    chansendul(c, 0);
}

void taskmain(int argc, char **argv)
{    
    c = chancreate(sizeof(unsigned long), 0);

    for(int i = 1; i < argc; i++){
        int ms = atoi(argv[i]);
        log_printf("Schedule %dms task\n", ms);
        taskcreate(delaytask, *(void**)&ms, STACK_SIZE);
    }
    
    int a = 1;
    // a == 1

    for(int i = 1; i < argc; i++){
        log_printf("Some task is finished\n");
        chanrecvul(c);
    }
    taskexitall(0);
}
```


Run: `gcc -I ./libtask coro.cpp ./libtask/libtask.a -lpthread -o coro.exe`



Run: `./coro.exe 300 100 200 1000`


    00:32:26.848 [tid=3565]: Schedule 300ms task
    00:32:26.849 [tid=3565]: Schedule 100ms task
    00:32:26.849 [tid=3565]: Schedule 200ms task
    00:32:26.849 [tid=3565]: Schedule 1000ms task
    00:32:26.849 [tid=3565]: Some task is finished
    00:32:26.950 [tid=3565]: Task 100ms is launched
    00:32:26.950 [tid=3565]: Some task is finished
    00:32:27.049 [tid=3565]: Task 200ms is launched
    00:32:27.050 [tid=3565]: Some task is finished
    00:32:27.149 [tid=3565]: Task 300ms is launched
    00:32:27.150 [tid=3565]: Some task is finished
    00:32:27.849 [tid=3565]: Task 1000ms is launched



```python

```

# <a name="hw"></a> Комментарии к ДЗ

* 


```python

```


```python

```


```python

```


```python

```
