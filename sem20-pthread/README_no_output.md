

# Потоки и их использование

<br>
<div style="text-align: right"> Спасибо <a href="https://github.com/Disadvantaged">Голяр Димитрису</a> за участие в написании текста </div>
<br>

<p><a href="https://www.youtube.com/watch?v=pP91ORe1YMk&list=PLjzMm8llUm4AmU6i_hPU0NobgA4VsBowc&index=21" target="_blank">
    <h3>Видеозапись семинара</h3> 
</a></p>


[Ридинг Яковлева](https://github.com/victor-yacovlev/mipt-diht-caos/tree/master/practice/pthread)


Сегодня в программе:
* <a href="#ptread_create" style="color:#856024">Создание и join потоков</a>
* <a href="#pthread_result" style="color:#856024">Аргументы и возвращаемое значение потока</a>
* <a href="#pthread_cancel" style="color:#856024">Прерывание/отмена/cancel потока</a>
* <a href="#pthread_attr" style="color:#856024">Атрибуты потока</a>
* <a href="#coro" style="color:#856024">Корутины</a>


<a href="#hw" style="color:#856024">Комментарии к ДЗ</a>



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

https://unix.stackexchange.com/questions/47595/linux-max-threads-count - про максимальное количество процессов и потоков в системе.

https://stackoverflow.com/questions/11679568/signal-handling-with-multiple-threads-in-linux - про потоки и сигналы. TLDR: обработчик вызывается в произвольном потоке, из тех, где сигнал не заблокирован. То есть для лучшей предсказуемости сигналы стоит изначально заблокировать еще до создания дополнительных потоков, а затем создать отдельный поток в котором их получать через sigsuspend/signalfd/sigwaitinfo (и только в нем они будут разблокированы).


```python

```

# <a name="pthread_create"></a> Создание и join потока


```cpp
%%cpp pthread_create.c
%MD **Обратите внимание на санитайзер - он ваш друг на все домашки с многопоточностью :)**
%run gcc -fsanitize=thread pthread_create.c -lpthread -o pthread_create.exe
%run ./pthread_create.exe

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>

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
    pt_verify(pthread_create(&thread, NULL, thread_func, 0)); // В какой-то момент будет создан поток и в нем вызвана функция
    // Начиная отсюда неизвестно в каком порядке выполняются инструкции основного и дочернего потока
    pt_verify(pthread_join(thread, NULL)); // -- аналог waitpid. Второй аргумент -- указатель в который запишется возвращаемое значение
    log_printf("Thread joined\n");
    log_printf("Main func finished\n");
    return 0;
}
```


```python

```

# <a name="pthread_result"></a> Смотрим на возвращаемое потоком значение.


```cpp
%%cpp pthread_create.c
%run clang -fsanitize=memory pthread_create.c -lpthread -o pthread_create.exe
%run ./pthread_create.exe

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>

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

#define fail_with_strerror(code, msg) do { char err_buf[1024]; strerror_r(code, err_buf, sizeof(err_buf)); log_printf(msg " (From err code: %s)\n", err_buf);  exit(EXIT_FAILURE);} while (0)

// thread-aware assert
#define ta_verify(stmt) do { if (stmt) break; fail_with_strerror(errno, "'" #stmt "' failed."); } while (0)

// verify pthread call
#define pt_verify(pthread_call) do { int code = (pthread_call); if (code == 0) break; fail_with_strerror(code, "'" #pthread_call "' failed."); } while (0)


typedef struct {
    int a;
    int b;
} thread_task_args_t;

// На самом деле проще записать результат в структуру аргументов
typedef struct {
    int c;
} thread_task_result_t;

static thread_task_result_t* thread_func(const thread_task_args_t* arg)
{
    log_printf("  Thread func started\n");
    thread_task_result_t* result = 
        (thread_task_result_t*)malloc(sizeof(thread_task_result_t));
    ta_verify(result != NULL);
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
    pt_verify(pthread_create(
        &thread, NULL, 
        (void* (*)(void*))thread_func, // Важно понимать, что тут происходит
        (void*)&args
    ));
    
    thread_task_result_t* result;
    pt_verify(pthread_join(thread, (void**)&result));
    log_printf("Thread joined. Result: c=%d\n", result->c);
    free(result);
    
    log_printf("Main func finished\n");
    return 0;
}
```

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
#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>

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
    pt_verify(pthread_create(&thread, NULL, thread_func, 0));
    sleep(1);
    log_printf("Thread canceling\n");
    pt_verify(pthread_cancel(thread)); // принимает id потока и прерывает его.
    pt_verify(pthread_join(thread, NULL)); // Если не сделать join, то останется зомби-поток.
    log_printf("Thread joined\n");
    log_printf("Main func finished\n");
    return 0;
}
```

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
#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>

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

static void *
thread_func(void *arg)
{
    log_printf("  Thread func started\n");
    #ifdef ASYNC_CANCEL
    pt_verify(pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL)); // Включаем более жесткий способ остановки потока
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
    pt_verify(pthread_create(&thread, NULL, thread_func, 0));
    sleep(1);
    log_printf("Thread canceling\n");
    pt_verify(pthread_cancel(thread));
    log_printf("Thread joining\n");
    pt_verify(pthread_join(thread, NULL));
    log_printf("Thread joined\n");
    log_printf("Main func finished\n");
    return 0;
}
```


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
#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>

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

pthread_t main_thread;

static void* thread_func(void* arg)
{
    log_printf("  Thread func started\n");
  
    log_printf("  Main thread joining\n");
    pt_verify(pthread_join(main_thread, NULL));
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
    pt_verify(pthread_create(&thread, NULL, thread_func, 0));
    
    pthread_exit(NULL);
}
```

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
%MD ### Обычный стек 8мб, активно его не используем
%run gcc -fsanitize=thread pthread_stack_size.c -lpthread -o pthread_stack_size.exe
%run ./pthread_stack_size.exe 
%MD ### Маленький стек 16кб, активно его не используем
%run gcc -fsanitize=thread -DMY_STACK_SIZE=16384 pthread_stack_size.c -lpthread -o pthread_stack_size.exe
%run ./pthread_stack_size.exe 
%MD Во второй раз (VM delta size) не 16кб потому что имеются накладные расходы. Но оно меньше примерно на 8 MB
%MD ### Обычный стек 8мб, активно его используем
%run gcc -fsanitize=thread -DUSE_STACK=7000000 pthread_stack_size.c -lpthread -o pthread_stack_size.exe
%run ./pthread_stack_size.exe 

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>

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



long int get_maxrss() {
    struct rusage usage;
    ta_verify(getrusage(RUSAGE_SELF, &usage) == 0);
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
        ta_verify(0 && "unreachable");
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
    #ifdef USE_STACK
    char a[USE_STACK];
    for (int i = 2; i < sizeof(a); ++i) {
        a[i] = a[i - 1] ^ a[i - 2] * 19;
    }   
    log_printf("  Thread func started. Trash=%d\n", a[sizeof(a) - 1]); // Предотвращаем оптимизацию
    #else
    log_printf("  Thread func started\n");
    #endif
    sleep(2);
    log_printf("  Thread func finished\n"); 
    return NULL;
}

int main()
{
    double initial_rss = (double)get_maxrss() / 1000;
    double initial_vm_size = (double)get_vm_usage() / 1000;
    log_printf("Main func started. Initial RSS = %0.1lf MB, initial VM usage = %0.1lf MB\n", 
               initial_rss, initial_vm_size);
    pthread_t thread;
    pthread_attr_t thread_attr; 
    pt_verify(pthread_attr_init(&thread_attr)); // Атрибуты нужно инициализировать
    #ifdef MY_STACK_SIZE
    pt_verify(pthread_attr_setstacksize(&thread_attr, MY_STACK_SIZE)); // В структуру сохраняем размер стека
    #endif
    pt_verify(pthread_create(&thread, &thread_attr, thread_func, 0));
    pt_verify(pthread_attr_destroy(&thread_attr)); // И уничтожить
    sleep(1);
    double current_rss = (double)get_maxrss() / 1000;
    double current_vm_size = (double)get_vm_usage() / 1000;
    
    log_printf("Thread working. RSS = %0.1lf MB, delta RSS = %0.1lf MB\n", 
               current_rss, current_rss - initial_rss);
    log_printf("Thread working. VM size = %0.1lf MB, VM delta size = %0.1lf MB (!)\n", 
               current_vm_size, current_vm_size - initial_vm_size); 
    
    pt_verify(pthread_join(thread, NULL));
    log_printf("Main func finished\n");
    return 0;
}
```


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
#include <string.h>
#include <errno.h>
#include <task.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>

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


const int STACK_SIZE = 32768;

typedef struct {
    int sleep_time;
} task_args_t;

Channel *c;

void delaytask(task_args_t *args)
{
    taskdelay(args->sleep_time);
    log_printf("Task %dms is launched\n", args->sleep_time);
    chansendul(c, 0);
}

void taskmain(int argc, char **argv)
{    
    task_args_t args[argc];
    
    c = chancreate(sizeof(unsigned long), 0);

    for(int i = 1; i < argc; i++){
        args[i].sleep_time = atoi(argv[i]);
        log_printf("Schedule %dms task\n", args[i].sleep_time);
        taskcreate((void (*)(void*))delaytask, (void*)&args[i], STACK_SIZE);
    }

    for(int i = 1; i < argc; i++){
        chanrecvul(c);
        log_printf("Some task is finished\n");
    }
    taskexitall(0);
}
```


```python

```

# <a name="hw"></a> Комментарии к ДЗ

* posix/threads/parallel-sum: 
<br>scanf/printf и многие другие функции стандартной библиотеки потокобезопасны (но каждый раз лучше смотреть в man). 
<br>В задаче требуется "минимизировать объем памяти", уточню: сделать для потоков стеки минимального размера.


```python




```


```python

```


```python

```
