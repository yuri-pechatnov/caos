```python
# look at tools/set_up_magics.ipynb
get_ipython().run_cell('# one_liner_str <too much code> \n')
None
```


    <IPython.core.display.Javascript object>


# Потоки и их использование

<br>
<div style="text-align: right"> Спасибо <a href="https://github.com/Disadvantaged">Голяр Димитрису</a> за участие в написании текста </div>
<br>


Сегодня в программе:
* <a href="#ptread_create" style="color:#856024">Создание и join потоков</a>
* <a href="#pthread_result" style="color:#856024">Аргументы и возвращаемое значение потока</a>
* <a href="#pthread_cancel" style="color:#856024">Прерывание/отмена/cancel потока</a>
* <a href="#pthread_attr" style="color:#856024">Атрибуты потока</a>
* <a href="#coro" style="color:#856024">Корутины</a>


<a href="#hw" style="color:#856024">Комментарии к ДЗ</a>

[Ридинг Яковлева](https://github.com/victor-yacovlev/mipt-diht-caos/tree/master/practice/pthread)


Атрибуты процесса (полнота списков не гарантируется):
* Виртуальное адресное пространство и данные в этой витруальной памяти
* Файловые дескрипторы, блокировки файлов
* PID
* argc, argv
* ulimit

Атрибуты потока:
* Маски сигналов (Маска сигналов наследует маску потока-родителя, изменения будут сохраняться только внутри потока)
* Состояние процесса R, S, T, Z
* Состояние регистров (какая ф-я сейчас выполняется) (состояние стека скорее входит в состояние вииртуального адресного пространства)
* TID




```python

```

# <a name="pthread_create"></a> Создание и join потока


```cpp
%%cpp pthread_create.c
%run gcc -fsanitize=thread pthread_create.c -lpthread -o pthread_create.exe
%run ./pthread_create.exe

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <pthread.h>

const char* log_prefix(const char* file, int line) {
    struct timeval tp; gettimeofday(&tp, NULL); struct tm ltime; localtime_r(&tp.tv_sec, &ltime);
    static __thread char prefix[100]; 
    size_t time_len = strftime(prefix, sizeof(prefix), "%H:%M:%S", &ltime);
    sprintf(prefix + time_len, ".%03ld %s:%d [tid=%ld]", tp.tv_usec / 1000, file, line, syscall(__NR_gettid));
    return prefix;
}

#define log_printf_impl(fmt, ...) { time_t t = time(0); dprintf(2, "%s: " fmt "%s", log_prefix(__FILE__, __LINE__), __VA_ARGS__); }
#define log_printf(...) log_printf_impl(__VA_ARGS__, "")

// thread-aware assert
#define ta_assert(stmt) if (stmt) {} else { log_printf("'" #stmt "' failed"); exit(EXIT_FAILURE); }

// Возвращаемое значение потока (~код возврата процесса) -- любое машинное слово.
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
    ta_assert(pthread_create(&thread, NULL, thread_func, 0) == 0); // В какой-то момент будет создан поток и в нем вызвана функция
    // Начиная отсюда неизвестно в каком порядке выполняются инструкции основного и дочернего потока
    ta_assert(pthread_join(thread, NULL) == 0); // -- аналог waitpid. Второй аргумент -- указатель в который запишется возвращаемое значение
    log_printf("Thread joined\n");
    log_printf("Main func finished\n");
    return 0;
}
```


Run: `gcc -fsanitize=thread pthread_create.c -lpthread -o pthread_create.exe`



Run: `./pthread_create.exe`


    23:00:02.131 pthread_create.c:37 [tid=7019]: Main func started
    23:00:02.158 pthread_create.c:39 [tid=7019]: Thread creating
    23:00:02.163 pthread_create.c:30 [tid=7021]:   Thread func started
    23:00:02.164 pthread_create.c:31 [tid=7021]:   Thread func finished
    23:00:02.164 pthread_create.c:43 [tid=7019]: Thread joined
    23:00:02.164 pthread_create.c:44 [tid=7019]: Main func finished



```python

```

# <a name="pthread_result"></a> Смотрим на возвращаемое потоком значение.


```cpp
%%cpp pthread_create.c
%run gcc -fsanitize=thread pthread_create.c -lpthread -o pthread_create.exe
%run ./pthread_create.exe

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <pthread.h>

const char* log_prefix(const char* file, int line) {
    struct timeval tp; gettimeofday(&tp, NULL); struct tm ltime; localtime_r(&tp.tv_sec, &ltime);
    static __thread char prefix[100]; 
    size_t time_len = strftime(prefix, sizeof(prefix), "%H:%M:%S", &ltime);
    sprintf(prefix + time_len, ".%03ld %s:%d [tid=%ld]", tp.tv_usec / 1000, file, line, syscall(__NR_gettid));
    return prefix;
}

#define log_printf_impl(fmt, ...) { time_t t = time(0); dprintf(2, "%s: " fmt "%s", log_prefix(__FILE__, __LINE__), __VA_ARGS__); }
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


Run: `gcc -fsanitize=thread pthread_create.c -lpthread -o pthread_create.exe`



Run: `./pthread_create.exe`


    22:59:49.724 pthread_create.c:50 [tid=7009]: Main func started
    22:59:49.738 pthread_create.c:54 [tid=7009]: Thread creating, args are: a=35 b=7
    22:59:49.766 pthread_create.c:40 [tid=7011]:   Thread func started
    22:59:49.766 pthread_create.c:44 [tid=7011]:   Thread func finished
    22:59:49.767 pthread_create.c:63 [tid=7009]: Thread joined. Result: c=42
    22:59:49.767 pthread_create.c:66 [tid=7009]: Main func finished


# <a name="pthread_cancel"></a> Прерывание потока

Пусть это возможно сделать, но с этим нужно быть очень осторожным, особенно если поток, который вы прерываете владеет какими-либо ресурсами


```cpp
%%cpp pthread_cancel.c
%run gcc -fsanitize=thread pthread_cancel.c -lpthread -o pthread_cancel.exe
%run ./pthread_cancel.exe

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <pthread.h>

const char* log_prefix(const char* file, int line) {
    struct timeval tp; gettimeofday(&tp, NULL); struct tm ltime; localtime_r(&tp.tv_sec, &ltime);
    static __thread char prefix[100]; 
    size_t time_len = strftime(prefix, sizeof(prefix), "%H:%M:%S", &ltime);
    sprintf(prefix + time_len, ".%03ld %s:%d [tid=%ld]", tp.tv_usec / 1000, file, line, syscall(__NR_gettid));
    return prefix;
}

#define log_printf_impl(fmt, ...) { time_t t = time(0); dprintf(2, "%s: " fmt "%s", log_prefix(__FILE__, __LINE__), __VA_ARGS__); }
#define log_printf(...) log_printf_impl(__VA_ARGS__, "")

// thread-aware assert
#define ta_assert(stmt) if (stmt) {} else { log_printf("'" #stmt "' failed"); exit(EXIT_FAILURE); }

static void* thread_func(void* arg)
{
    log_printf("  Thread func started\n");
    // В системных функциях разбросаны Cancellation points, в которых может быть прерван поток.
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
    ta_assert(pthread_cancel(thread) == 0); // принимает id потока и прерывает его.
    ta_assert(pthread_join(thread, NULL) == 0); // Если не сделать join, то останется зомби-поток.
    log_printf("Thread joined\n");
    log_printf("Main func finished\n");
    return 0;
}
```


Run: `gcc -fsanitize=thread pthread_cancel.c -lpthread -o pthread_cancel.exe`



Run: `./pthread_cancel.exe`


    22:59:33.672 pthread_cancel.c:38 [tid=6996]: Main func started
    22:59:33.682 pthread_cancel.c:40 [tid=6996]: Thread creating
    22:59:33.716 pthread_cancel.c:29 [tid=6998]:   Thread func started
    22:59:34.716 pthread_cancel.c:43 [tid=6996]: Thread canceling
    22:59:34.718 pthread_cancel.c:46 [tid=6996]: Thread joined
    22:59:34.718 pthread_cancel.c:47 [tid=6996]: Main func finished


По умолчанию pthread_cancel может прерывать поток, только в cancelation points (то есть в функциях, в реализациях которых есть проверка на это). 

Поэтому, если эти функции не вызывать, то поток не сможет быть прерван.

Но можно воспользоваться `pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);`. Тогда поток может быть прерван на уровне планировщика. (То есть поток скорее всего доработает текущий выделенный квант времени, но на следующий квант уже не запустится)


```cpp
%%cpp pthread_cancel_fail.c
%run gcc -fsanitize=thread pthread_cancel_fail.c -lpthread -o pthread_cancel_fail.exe
%run timeout 3 ./pthread_cancel_fail.exe  # will fail (cancelation at cancelation points)
%run gcc -fsanitize=thread  -DASYNC_CANCEL pthread_cancel_fail.c -lpthread -o pthread_cancel_fail.exe
%run timeout 3 ./pthread_cancel_fail.exe  # ok, async cancelation

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <pthread.h>

const char* log_prefix(const char* file, int line) {
    struct timeval tp; gettimeofday(&tp, NULL); struct tm ltime; localtime_r(&tp.tv_sec, &ltime);
    static __thread char prefix[100]; 
    size_t time_len = strftime(prefix, sizeof(prefix), "%H:%M:%S", &ltime);
    sprintf(prefix + time_len, ".%03ld %s:%d [tid=%ld]", tp.tv_usec / 1000, file, line, syscall(__NR_gettid));
    return prefix;
}

#define log_printf_impl(fmt, ...) { time_t t = time(0); dprintf(2, "%s: " fmt "%s", log_prefix(__FILE__, __LINE__), __VA_ARGS__); }
#define log_printf(...) log_printf_impl(__VA_ARGS__, "")

// thread-aware assert
#define ta_assert(stmt) if (stmt) {} else { log_printf("'" #stmt "' failed"); exit(EXIT_FAILURE); }

static void *
thread_func(void *arg)
{
    log_printf("  Thread func started\n");
    #ifdef ASYNC_CANCEL
    ta_assert(pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL) == 0); // Включаем более жесткий способ остановки потока
    #endif
    // Без опции ASYNC_CANCEL поток не может быть остановлен во время своей работы.
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


Run: `gcc -fsanitize=thread pthread_cancel_fail.c -lpthread -o pthread_cancel_fail.exe`



Run: `timeout 3 ./pthread_cancel_fail.exe  # will fail (cancelation at cancelation points)`


    22:59:09.845 pthread_cancel_fail.c:44 [tid=6975]: Main func started
    22:59:09.866 pthread_cancel_fail.c:46 [tid=6975]: Thread creating
    22:59:10.300 pthread_cancel_fail.c:32 [tid=6977]:   Thread func started
    22:59:11.316 pthread_cancel_fail.c:49 [tid=6975]: Thread canceling
    22:59:11.317 pthread_cancel_fail.c:51 [tid=6975]: Thread joining



Run: `gcc -fsanitize=thread  -DASYNC_CANCEL pthread_cancel_fail.c -lpthread -o pthread_cancel_fail.exe`



Run: `timeout 3 ./pthread_cancel_fail.exe  # ok, async cancelation`


    22:59:14.397 pthread_cancel_fail.c:44 [tid=6986]: Main func started
    22:59:14.400 pthread_cancel_fail.c:46 [tid=6986]: Thread creating
    22:59:14.406 pthread_cancel_fail.c:32 [tid=6988]:   Thread func started
    22:59:15.417 pthread_cancel_fail.c:49 [tid=6986]: Thread canceling
    22:59:15.418 pthread_cancel_fail.c:51 [tid=6986]: Thread joining
    22:59:15.418 pthread_cancel_fail.c:53 [tid=6986]: Thread joined
    22:59:15.421 pthread_cancel_fail.c:54 [tid=6986]: Main func finished



```python

```

## А можно ли приджойнить основной поток?


```cpp
%%cpp join_main_thread.c
%run gcc join_main_thread.c -lpthread -o join_main_thread.exe
%run timeout 3 ./join_main_thread.exe ; echo "Exit code: $?"
%run gcc -fsanitize=thread join_main_thread.c -lpthread -o join_main_thread.exe
%run timeout 3 ./join_main_thread.exe ; echo "Exit code: $?"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <pthread.h>

const char* log_prefix(const char* file, int line) {
    struct timeval tp; gettimeofday(&tp, NULL); struct tm ltime; localtime_r(&tp.tv_sec, &ltime);
    static __thread char prefix[100]; 
    size_t time_len = strftime(prefix, sizeof(prefix), "%H:%M:%S", &ltime);
    sprintf(prefix + time_len, ".%03ld %s:%d [tid=%ld]", tp.tv_usec / 1000, file, line, syscall(__NR_gettid));
    return prefix;
}

#define log_printf_impl(fmt, ...) { time_t t = time(0); dprintf(2, "%s: " fmt "%s", log_prefix(__FILE__, __LINE__), __VA_ARGS__); }
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


Run: `gcc join_main_thread.c -lpthread -o join_main_thread.exe`



Run: `timeout 3 ./join_main_thread.exe ; echo "Exit code: $?"`


    22:58:43.802 join_main_thread.c:46 [tid=6952]: Main func started
    22:58:43.806 join_main_thread.c:50 [tid=6952]: Thread creating
    22:58:43.806 join_main_thread.c:33 [tid=6953]:   Thread func started
    22:58:43.806 join_main_thread.c:35 [tid=6953]:   Main thread joining
    22:58:43.806 join_main_thread.c:37 [tid=6953]:   Main thread joined
    22:58:43.806 join_main_thread.c:39 [tid=6953]:   Thread func finished
    Exit code: 42



Run: `gcc -fsanitize=thread join_main_thread.c -lpthread -o join_main_thread.exe`



Run: `timeout 3 ./join_main_thread.exe ; echo "Exit code: $?"`


    22:58:45.154 join_main_thread.c:46 [tid=6962]: Main func started
    22:58:45.154 join_main_thread.c:50 [tid=6962]: Thread creating
    22:58:45.179 join_main_thread.c:33 [tid=6964]:   Thread func started
    22:58:45.179 join_main_thread.c:35 [tid=6964]:   Main thread joining
    FATAL: ThreadSanitizer CHECK failed: ../../../../src/libsanitizer/tsan/tsan_rtl_thread.cc:302 "((tid)) < ((kMaxTid))" (0xffffffffffffffff, 0x1fc0)
        #0 <null> <null> (libtsan.so.0+0x838bf)
        #1 <null> <null> (libtsan.so.0+0x9f539)
        #2 <null> <null> (libtsan.so.0+0x87acd)
        #3 <null> <null> (libtsan.so.0+0x4cc06)
        #4 <null> <null> (join_main_thread.exe+0x400e34)
        #5 <null> <null> (libtsan.so.0+0x2970d)
        #6 <null> <null> (libpthread.so.0+0x76b9)
        #7 <null> <null> (libc.so.6+0x10741c)
    
    Exit code: 66


Без санитайзера можно, с санитайзером - нет. Не знаю есть ли тут какое-то принципиальное нарушение, но не надо так делать)


```python

```

# <a name="pthread_attr"></a> Атрибуты потока

* Размер стека
* Местоположение стека
* Размер защитной области после стека. Вот тут можно прокомментировать: это область ниже стека, которая является дырой в виртуальном адресном пространстве программы. То есть при попытке обращения к этой области произойдет segfault. Для чего необходима защитная область? Чтобы при переполнении стека получать segfault, а не неотлавливаемый проезд по памяти.


В следующем примере создадим поток двумя способами. С параметрами по умолчанию и указав минимальный размер стека. И посмотрим на потребления памяти. 

(Да, потреблениЯ. Там все не так просто, как кажется на первый взгляд :). Загляните в `/proc/<pid>/status`)


```cpp
%%cpp pthread_stack_size.c
%run gcc -fsanitize=thread pthread_stack_size.c -lpthread -o pthread_stack_size.exe
%run ./pthread_stack_size.exe 
%run gcc -fsanitize=thread -DMY_STACK_SIZE=16384 pthread_stack_size.c -lpthread -o pthread_stack_size.exe
%run ./pthread_stack_size.exe 
%run # Во второй раз (VM delta size) не 16кб потому что имеются накладные расходы.

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <pthread.h>

const char* log_prefix(const char* file, int line) {
    struct timeval tp; gettimeofday(&tp, NULL); struct tm ltime; localtime_r(&tp.tv_sec, &ltime);
    static __thread char prefix[100]; 
    size_t time_len = strftime(prefix, sizeof(prefix), "%H:%M:%S", &ltime);
    sprintf(prefix + time_len, ".%03ld %s:%d [tid=%ld]", tp.tv_usec / 1000, file, line, syscall(__NR_gettid));
    return prefix;
}

#define log_printf_impl(fmt, ...) { time_t t = time(0); dprintf(2, "%s: " fmt "%s", log_prefix(__FILE__, __LINE__), __VA_ARGS__); }
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
    ta_assert(pthread_attr_init(&thread_attr) == 0); // Атрибуты нужно инициализировать
    #ifdef MY_STACK_SIZE
    ta_assert(pthread_attr_setstacksize(&thread_attr, MY_STACK_SIZE) == 0); // В структуру сохраняем размер стека
    #endif
    log_printf("Thread creating\n");
    ta_assert(pthread_create(&thread, &thread_attr, thread_func, 0) == 0);
    ta_assert(pthread_attr_destroy(&thread_attr) == 0); // И уничтожить
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


Run: `gcc -fsanitize=thread pthread_stack_size.c -lpthread -o pthread_stack_size.exe`



Run: `./pthread_stack_size.exe`


    22:56:49.532 pthread_stack_size.c:86 [tid=6855]: Main func started. Initial RSS = 12584kb, initial VM usage = 37580989492kb
    22:56:49.534 pthread_stack_size.c:93 [tid=6855]: Thread creating
    22:56:49.546 pthread_stack_size.c:75 [tid=6861]:   Thread func started
    22:56:50.547 pthread_stack_size.c:99 [tid=6855]: Thread working. RSS = 14272kb, delta RSS = 1688kb
    22:56:50.696 pthread_stack_size.c:101 [tid=6855]: Thread working. VM size = 37581000420kb, VM delta size = 10928kb (!)
    22:56:51.547 pthread_stack_size.c:77 [tid=6861]:   Thread func finished
    22:56:51.548 pthread_stack_size.c:104 [tid=6855]: Thread joined
    22:56:51.549 pthread_stack_size.c:105 [tid=6855]: Main func finished



Run: `gcc -fsanitize=thread -DMY_STACK_SIZE=16384 pthread_stack_size.c -lpthread -o pthread_stack_size.exe`



Run: `./pthread_stack_size.exe`


    22:56:54.562 pthread_stack_size.c:86 [tid=6875]: Main func started. Initial RSS = 12564kb, initial VM usage = 37580989492kb
    22:56:54.562 pthread_stack_size.c:93 [tid=6875]: Thread creating
    22:56:54.606 pthread_stack_size.c:75 [tid=6881]:   Thread func started
    22:56:55.606 pthread_stack_size.c:99 [tid=6875]: Thread working. RSS = 14192kb, delta RSS = 1628kb
    22:56:55.714 pthread_stack_size.c:101 [tid=6875]: Thread working. VM size = 37580992612kb, VM delta size = 3120kb (!)
    22:56:56.606 pthread_stack_size.c:77 [tid=6881]:   Thread func finished
    22:56:56.607 pthread_stack_size.c:104 [tid=6875]: Thread joined
    22:56:56.607 pthread_stack_size.c:105 [tid=6875]: Main func finished



Run: `# Во второй раз (VM delta size) не 16кб потому что имеются накладные расходы.`



```python

```


```python

```


```python

```

# <a name="coro"></a> Coroutines

Корутины -- это потоки внутри одного юзерспейса. То есть, это потоки внутри потока.
Для этого используется программный (реализованный в коде пользователя), а не системный scheduler.

Файберы (=корутины, =потоки в юзерспейсе):
<br>`+` Известно, когда может быть вызвано переключение контекста. Файберы работающие внутри одного потока могут не пользоваться межпоточной синхронизацией при общении друг с другом.
<br>`+` Низкие затраты на переключение контекста. Это очень эффективно, если есть много потоков перекладывающих друг другу данные.
<br>`+` ...
<br>`-` Привязанность к фреймворку. Нельзя использовать блокирующие вызовы не через этот фреймворк.
<br>`-` Нельзя подключиться к процессу с помощью gdb и посмотреть на все потоки. 
<br>`-` ...


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
