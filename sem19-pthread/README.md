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


<a href="#hw" style="color:#856024">Комментарии к ДЗ</a>

[Ридинг Яковлева](https://github.com/victor-yacovlev/mipt-diht-caos/tree/master/practice/pthread)


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

static void *
thread_func(void *arg)
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


    22:39:49.794 [tid=28423]: Main func started
    22:39:49.794 [tid=28423]: Thread creating
    22:39:49.794 [tid=28424]:   Thread func started
    22:39:49.794 [tid=28424]:   Thread func finished
    22:39:49.794 [tid=28423]: Thread joined
    22:39:49.794 [tid=28423]: Main func finished



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
    thread_task_result_t* result = (thread_task_result_t*)malloc(sizeof(thread_task_result_t));
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


    22:39:50.472 [tid=28432]: Main func started
    22:39:50.473 [tid=28432]: Thread creating, args are: a=35 b=7
    22:39:50.473 [tid=28433]:   Thread func started
    22:39:50.473 [tid=28433]:   Thread func finished
    22:39:50.473 [tid=28432]: Thread joined. Result: c=42
    22:39:50.473 [tid=28432]: Main func finished


# <a name="pthread_cancel"></a> Прерывание потока

Пусть это возмоно сделать, но с этим нужно быть очень осторожным, особенно если поток, который вы прерываете владеет какими-либо ресурсами


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

static void *
thread_func(void *arg)
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


    22:39:51.171 [tid=28441]: Main func started
    22:39:51.175 [tid=28441]: Thread creating
    22:39:51.175 [tid=28442]:   Thread func started
    22:39:52.176 [tid=28441]: Thread canceling
    22:39:52.177 [tid=28441]: Thread joined
    22:39:52.177 [tid=28441]: Main func finished


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
    ta_assert(pthread_join(thread, NULL) == 0);
    log_printf("Thread joined\n");
    log_printf("Main func finished\n");
    return 0;
}
```


Run: `gcc pthread_cancel_fail.cpp -lpthread -o pthread_cancel_fail.exe`



Run: `timeout 3 ./pthread_cancel_fail.exe  # will fail (cancelation at cancelation points)`


    22:39:52.975 [tid=28451]: Main func started
    22:39:52.979 [tid=28451]: Thread creating
    22:39:52.980 [tid=28452]:   Thread func started
    22:39:53.980 [tid=28451]: Thread canceling



Run: `gcc -DASYNC_CANCEL pthread_cancel_fail.cpp -lpthread -o pthread_cancel_fail.exe`



Run: `timeout 3 ./pthread_cancel_fail.exe  # ok, async cancelation`


    22:39:56.643 [tid=28461]: Main func started
    22:39:56.644 [tid=28461]: Thread creating
    22:39:56.644 [tid=28462]:   Thread func started
    22:39:57.645 [tid=28461]: Thread canceling
    22:39:57.646 [tid=28461]: Thread joined
    22:39:57.647 [tid=28461]: Main func finished



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


    22:40:31.143 [tid=28470]: Main func started. Initial RSS = 800kb, initial VM usage = 72kb
    22:40:31.144 [tid=28470]: Thread creating
    22:40:31.144 [tid=28474]:   Thread func started
    22:40:32.144 [tid=28470]: Thread working. RSS = 800kb, delta RSS = 0kb
    22:40:32.182 [tid=28470]: Thread working. VM size = 8528kb, VM delta size = 8456kb (!)
    22:40:33.144 [tid=28474]:   Thread func finished
    22:40:33.144 [tid=28470]: Thread joined
    22:40:33.144 [tid=28470]: Main func finished



Run: `gcc -DMY_STACK_SIZE=16384 pthread_stack_size.cpp -lpthread -o pthread_stack_size.exe`



Run: `./pthread_stack_size.exe`


    22:40:33.961 [tid=28488]: Main func started. Initial RSS = 816kb, initial VM usage = 72kb
    22:40:33.961 [tid=28488]: Thread creating
    22:40:33.961 [tid=28492]:   Thread func started
    22:40:34.962 [tid=28488]: Thread working. RSS = 816kb, delta RSS = 0kb
    22:40:35.007 [tid=28488]: Thread working. VM size = 348kb, VM delta size = 276kb (!)
    22:40:35.967 [tid=28492]:   Thread func finished
    22:40:35.967 [tid=28488]: Thread joined
    22:40:35.967 [tid=28488]: Main func finished



```python

```


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


```python

```


```python

```
