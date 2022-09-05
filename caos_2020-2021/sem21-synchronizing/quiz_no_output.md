

# Как проверять асинхронщину?

1. `--sanitize=thread` - это стоит сделать сразу и не задумываясь. Максимально быстро даст результат. К сожалению бывает false nagative и в экзотических случаях false positive.

2. Головой. Вот об этом дальше.

В общем случае и с потоками и сигналами, у вас есть несколько потоков команд, которые выполняются параллельно.

* В случае сигналов и одного потока ситуация такая: выполняется основной поток команд, но в любой момент, когда сигналы не заблокированы, он может быть прерван обработчиком сигнала (и обработчик полностью завершится, прежде чем продолжит выполняться основной поток). **При анализе кода** нужно рассматривать каждый такой момент: "а не сломается ли, если придет сейчас?". 
<br>Так же нужно учитывать, что реальный порядок команд может отличаться от того, что вы пишете в коде из-за работы оптимизатора.

Картинка-пример выполнения команд с течением времени:
```
Signals blocked:         _BBBB___________...
Main thread commands:    1[34]6______7__9...
Can be interrupted:      Y____YYYYYYYYYYY...
Signal handler commands: ______123456____...
_______________________________^ consider that signal handler 
_______________________________  may interrupt after every command
B - blocked
Y - yes
1, 2, 3 -commands
```

* В случае межпоточного взаимодействия так же нужно понимать, что потоки могут переключаться в любые моменты. И нет никаких гарантий по порядку исполнения команд из разных потоков. Некоторые команды могут исполняться одновременно. В одном потоке может выполниться 100000 команд, пока в другом не выполнится ни одна... Так же нужно учитывать, что реальный порядок команд может отличаться от того, что вы пишете в коде из-за работы оптимизатора.

  Картинка-пример выполнения команд с течением времени:
```
Thread 1:      12_34______5
Thread 2:      1_2_3__4__56
```

  В отличие от сигналов здесь есть: 
  
  * Критические секции
    <br>Тут у вас есть гарантия, что оптимизатор не будет делать опасных перемещений команд сквозь границы критической секции.
    <br>Так же два потока не могут исполняться одновременно в одной критической секции.
    <br>То есть в ней можно безопасно работать с незащищенными данными.
  * Condvar
    <br>Можно рассматривать эту сущность как lockfree список потоков ожидающих события.
    <br>Во время wait на condvar отпускаетя мьютекс. То есть внутри него заканчивается критическая секция и начинается новая. То есть после выполнения wait данные, защищаемые мьютексом, могут измениться.
    <br>Функцию signal стоит рассматривать как группу атомарных операций. Ну и в принципе не стоит забывать, что у некоторых функций внутри атомарные операции.
  * Честное параллельное выполнение.
    <br>Это значит, что вне критических секций всё взаимодействие по данным должно осуществляться только через атомики (не то же самое, что volatile). Иначе один поток может увидеть изменения сделанные другим потоком в любом виде. (То есть полное UB)
 
  Если взаимодействуем через атомики, то:
  
  * Атомарные операции над одной переменной из двух потоков в любом случае выполнятся последовательно. То есть можно забить на то, что процессоры бывают многоядерные и представлять, что все шедулится на одном ядре.
  * **При анализе программы**, рассматриваем возможность переключения потоков в любом порядке: смотрим на команду в потоке 1 и думаем, что за ней может выполниться следующая команда потока 1, а может и следующая команда потока 2. И в обоих случаях ничего сломаться не должно.
  * Атомарные операции подразумевают барьеры памяти (абстрактные штуки, которые не дают оптимизатору переставлять операции с обычной памятью через атомарные)
    
  


# <a name="tasks"></a> Задачки на разную асинхронщину и не только. 

Про обработчики сигналов. Эти задачки про то, чтобы научиться видеть ошибки в асинхронном коде. Все примеры компилируются и при **каких-то обстоятельствах** точно работают. Вам нужно проверить, работают ли корректно при любых обстоятельствах. Если работают, то подумать, нет ли в коде других проблем.
* <a href="#sig_pause" style="color:#856024"> `while(!stopped) pause();` </a>
* <a href="#sig_handler_fin" style="color:#856024"> Финализация в хендлере </a>
* <a href="#sig_while_true_1" style="color:#856024"> `while(!stopped); #1` </a>
* <a href="#sig_while_true_2" style="color:#856024"> `while(!stopped); #2` </a>
* <a href="#sig_check" style="color:#856024"> `sigtimedwait` </a>
* <a href="#sig_check_regular" style="color:#856024"> `sigtimedwait` 2 </a>

Разное:
* <a href="#read_piece" style="color:#856024"> Retryable read </a>


Про межпоточную синхронизацию.
* <a href="#sig_mutex" style="color:#856024"> Signal handler synchronization </a>
* <a href="#condvar_1" style="color:#856024"> Condvar 1 </a>
* <a href="#condvar_2" style="color:#856024"> Condvar 2 </a>
* <a href="#condvar_3" style="color:#856024"> Condvar 3 </a>
* <a href="#mutex_char" style="color:#856024"> Mutex </a>
* <a href="#atomic" style="color:#856024"> Atomic </a>
* <a href="#philosophical_lock" style="color:#856024"> Philosophical lock </a> от [Андрея Баженова](https://github.com/TheRealBazhen)
* <a href="#threads" style="color:#856024"> Ниточки Ариадны </a>

Большинство задачек нацелены на то, чтобы показать как делать не надо. Как можно делать - в материалах семинаров.

**Постарайтесь решить задачки самостоятельно, не заглядывая в ответы сразу. Удачи!**


# Полезные ссылки

https://deadlockempire.github.io/ - интекрактивная игра, в которой нужно играть роль злого шедулера и заставлять крешиться программы. Мне кажется, очень полезно попроходить.

# <a name="sig_pause"></a> `while(!stopped) pause();`

Корректна ли программа с точки зрения асинхронной безопасности обработчиков сигналов?
То есть будет ли она завершаться по приходу сигнала?

Хотим завершаться как только придет сигнал.


```cpp
%%cpp sig_pause.c
%run gcc -g -O0 sig_pause.c -lpthread -o sig_pause.exe
%run # ./sig_pause.exe

#include <unistd.h>
#include <signal.h>

volatile sig_atomic_t stop = 0;

static void handler(int signum) {
    stop = 1;
}

int main(int argc, char* argv[]) {
    int signals[] = {SIGINT, SIGTERM, 0};
    for (int* signal = signals; *signal; ++signal) {
        sigaction(*signal, &(struct sigaction){.sa_handler=handler}, NULL);
    }
    while (!stop) {
        int a = 1;
        pause();
    }
    return 0;
}
```

<details>
<summary><b>Ответ</b></summary>
<p>
    
Как все из вас с недырявой памятью помнят, так делать нельзя, если сигнал придет между проверкой и `pause`, то мы на него не среагируем.
    
    
Воспроизвести ситуацию можно так:
    
* Запустим программу под gdb, чтобы заморозить ее в неудачный момент (на строке `int a = 1;`): `gdb -ex="b 21" -ex="handle SIGTERM nostop" -ex="handle SIGCHLD nostop" -ex=r --args ./sig_pause.exe`
* Пошлем сигнал, который должен приводить к немедленному завершению: `killall -SIGTERM sig_pause.exe`
* Жмем `c` в gdb. 
* Вот и все, мы не среагировали на сигнал.    
    
   
A еще, если тут не будет volatile, то while(!stop) может соптимизироваться до while(true)
    
</p>
</details>







```python

```

# <a name="sig_handler_fin"></a> Финализация в хендлере

Корректна ли программа с точки зрения асинхронной безопасности обработчиков сигналов? То есть будет ли она завершаться по приходу сигнала? Будут ли освобождены ресурсы?


```cpp
%%cpp sig_handler_fin.c
%run gcc -g -O0 sig_handler_fin.c -lpthread -o sig_handler_fin.exe
%run # ./sig_handler_fin.exe

#include <unistd.h>
#include <stdio.h>
#include <signal.h>

volatile sig_atomic_t resource = -1;

int resource_acquire() { // здесь захватывается какой-то ресурс, который обязательно нужно освободить
    static int res = 100; ++res;
    dprintf(2, "Resource %d acquired\n", res);
    return res;
}
void resource_release(int res) { // здесь освобождается
    dprintf(2, "Resource %d released\n", res);
}

static void handler(int signum) {
    if (signum == SIGUSR1) return;
    if (resource != -1) resource_release(resource);
    _exit(0);
}

int main(int argc, char* argv[]) {
    int signals[] = {SIGINT, SIGTERM, SIGUSR1, 0};
    for (int* signal = signals; *signal; ++signal) {
        sigaction(*signal, &(struct sigaction){.sa_handler=handler}, NULL);
    }
    while (1) {
        resource = resource_acquire(); // ~ accept
        sleep(1);
        resource_release(resource); // ~ shutdown & close
    }
    return 0;
}
```

<details>
<summary><b>Ответ</b></summary>
<p>
    
Сигнал может прийти и обработаться после acquire и до сохранения в атомик. То есть завершаться будет, но ресурсы могут не быть освобождены.

Воспроизвести можно так: 
* вставляем `pause();` перед `return res;`. Таким образом, можно руками воспроизвести ситуацию, когда обработчик выполнится во время исполнения определенной строчки основного потока.
* `killall -SIGUSR1 sig_handler_fin.exe` - так можно пропустить столько первых вхождений в pause, сколько мы хотим.
* `killall -SIGTERM sig_handler_fin.exe` - завершаем программу. При этом обработчик вызовется в "неудачный" момент. И тогда последний выделенный ресурс не освободится. А освободится второй раз ресурс выделенный в прошлый раз, что тоже может как-то выстрелить.
</p>
</details>






# <a name="sig_while_true_1"></a> `while(!stopped); #1`

Корректна ли программа с точки зрения асинхронной безопасности обработчиков сигналов? 


```cpp
%%cpp sig_while_true.c
%run gcc -g -O3 sig_while_true.c -lpthread -o sig_while_true.exe
%run # timeout -s SIGTERM 1 ./sig_while_true.exe

#include <stdio.h>
#include <unistd.h>
#include <signal.h>

sig_atomic_t stop = 0;

static void handler(int signum) {
    stop = 1;
}

int main(int argc, char* argv[]) {
    int signals[] = {SIGINT, SIGTERM, 0};
    for (int* signal = signals; *signal; ++signal) {
        sigaction(*signal, &(struct sigaction){.sa_handler=handler}, NULL);
    }
    while (!stop);
    printf("Stopped\n");
    return 0;
}
```

<details>
<summary><b>Ответ</b></summary>
<p>

Полный треш. Без volatile цикл оптимизируется и программа зависнет навсегда.

https://stackoverflow.com/questions/24931456/how-does-sig-atomic-t-actually-work

</p>
</details>






# <a name="sig_while_true_2"></a> `while(!stopped); #2`

Корректна ли программа с точки зрения асинхронной безопасности обработчиков сигналов? 


```cpp
%%cpp sig_while_true_2.c
%run gcc -g -O2 sig_while_true_2.c -lpthread -o sig_while_true_2.exe
%run # timeout -s SIGTERM 1 ./sig_while_true_2.exe

#include <stdio.h>
#include <unistd.h>
#include <signal.h>

volatile sig_atomic_t stop = 0;

static void handler(int signum) {
    stop = 1;
}

int main(int argc, char* argv[]) {
    int signals[] = {SIGINT, SIGTERM, 0};
    for (int* signal = signals; *signal; ++signal) {
        sigaction(*signal, &(struct sigaction){.sa_handler=handler}, NULL);
    }
    while (!stop);
    printf("Stopped\n");
    return 0;
}
```

<details>
<summary><b>Ответ</b></summary>
<p>

Программа верна с точки зрения безопасности.

Но она отвратительна с точки зрения производительности, такой код выдет выедать одно ядро процессора, что в значительной степени просадит всю производительность машинки.

За такой код в продакшне можно отрывать руки.

</p>
</details>






# <a name="sig_check"></a> `sigtimedwait`

Корректна ли программа с точки зрения асинхронной безопасности обработчиков сигналов?

То есть будет ли она завершаться по приходу сигнала? Будут ли освобождены ресурсы?


```cpp
%%cpp sig_check.c
%run gcc -g sig_check.c -o sig_check.exe
%run # ./sig_check.exe 

#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>

int resource_acquire() {
    char c;
    read(0, &c, 1); // think, that here is accept-like logic (waiting external clients)
    dprintf(2, "Resource %d acquired\n", (int)c);
    return (int)c;
}
void resource_release(int res) {
    dprintf(2, "Resource %d released\n", res);
}

int main() {
    sigset_t full_mask;
    sigfillset(&full_mask);
    sigprocmask(SIG_BLOCK, &full_mask, NULL); 
    siginfo_t info;
    struct timespec timeout = {0};
    // В данном случае sigtimedwait проверяет, а не пришел ли сигнал. Работает без ожиданий
    while (sigtimedwait(&full_mask, &info, &timeout) < 0) {
        int resource = resource_acquire(); // ~ accept
        sleep(1);
        resource_release(resource); // ~ shutdown & close
    }   
    return 0;
}
```

<details>
<summary><b>Ответ</b></summary>
<p>

С точки зрения асинхронной безопасности может и отчасти корректна, но в целом некорректна, так как не решает задачу.
Так как если не будет клиентов, мы навсегда зависнем в acquire и не будем реагировать на сигналы.

То есть завершаться не будет. Но ресурсы будет освобождать хорошо.

</p>
</details>






# <a name="sig_check_regular"></a> `sigtimedwait 2`

Корректна ли программа с точки зрения асинхронной безопасности обработчиков сигналов?

То есть будет ли она завершаться по приходу сигнала? Будут ли освобождены ресурсы?


```cpp
%%cpp sig_check_2.c
%run gcc -g sig_check_2.c -o sig_check_2.exe
%run # echo "0123" | ./sig_check_2.exe 

#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

sigset_t full_mask;
const struct timespec timeout = {.tv_sec = 0, .tv_nsec = 10000000}; // 10ms
siginfo_t info;

int resource_acquire() {
    char c; int read_res;
    while ((read_res = read(0, &c, 1)) < 0) {
        if (sigtimedwait(&full_mask, &info, &timeout) >= 0) {
            return -1;
        }
    }
    assert(read_res != 0);
    dprintf(2, "Resource %d acquired\n", (int)c);
    return (int)c;
}
void resource_release(int res) {
    dprintf(2, "Resource %d released\n", res);
}

int main() {
    fcntl(0, F_SETFL, fcntl(0, F_GETFL) | O_NONBLOCK);
    
    sigfillset(&full_mask);
    sigprocmask(SIG_BLOCK, &full_mask, NULL); 

    // В данном случае sigtimedwait проверяет, а не пришел ли сигнал. Работает без ожиданий
    while (sigtimedwait(&full_mask, &info, &timeout) < 0) {
        int resource = resource_acquire(); // ~ accept
        if (resource < 0) { break; }
        sleep(1); // имитация полезной работы 
        resource_release(resource); // ~ shutdown & close
    }   
    return 0;
}
```

<details>
<summary><b>Ответ</b></summary>
<p>

С точки зрения асинхронной безопасности корректна. Будет завершаться и освобождать ресурсы.

Но! Это скользкая дорожка. Поскольку здесь мы проверяем ситуацию раз в некоторое время. Из-за этого реагируем на события с опозданием. Если будем уменьшать таймаут, то начнем зря тратить все большее количество CPU. Получается порочный tradeoff.

Не надо так делать, если есть возможность этого избежать.

</p>
</details>


# <a name="read_piece"></a> Retryable read

Будет ли корректно читать все, что доступно для чтения в socket_fd?

Задача уже не про сигналы, можно о них не думать.


```cpp
%%cpp read_piece.c

union message {
    int i;
    char arr[4];
};
union message value;
  
//...

int readed_count = 0;
while(readed_count < 4) {
    r = read(socket_fd, &value + readed_count, sizeof(value) - readed_count);
    if (r > 0) {
        readed_count += r;
    } else if (r < 0) {
        assert(errno == EAGAIN);
    } else if (r == 0) {
        assert(0 && "can't read value");
    }
}
```

<details>
<summary><b>Ответ</b></summary>
<p>

Нет, здесь баг в арифметике указателей.

`&value + readed_count` при `readed_count == 1` это указатель находящийся уже за пределами value. Исправляется так: `(char*)&value + readed_count` 

</p>
</details>



```python

```

# <a name="sig_mutex"></a> Signal handler synchronization

Есть ли тут асинхронная безопасность?


```cpp
%%cpp sig_mutex.c
%run gcc -g -O0 sig_mutex.c -lpthread -o sig_mutex.exe
%run # через 1 секунду пошлется SIGINT, через 2 SIGKILL
%run # timeout -s SIGKILL 2 timeout -s SIGINT 1 ./sig_mutex.exe

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
long long int value = 0;

static void handler(int signum) {
    pthread_mutex_lock(&mutex);
    printf("Value = %lld\n", value);
    pthread_mutex_unlock(&mutex);
    exit(0);
}

int main(int argc, char* argv[]) {
    sigaction(SIGINT, &(struct sigaction){.sa_handler=handler}, NULL);
    while (1) {
        pthread_mutex_lock(&mutex);
        ++value;
        pthread_mutex_unlock(&mutex);
    }
    printf("Stopped\n");
    return 0;
}
```

<details>
<summary><b>Ответ</b></summary>
<p>

Нет, тут скрестили ужа с ежом. 
Обработчики сигнала выполняются в том потоке, куда пришел сигнал. (То есть выполнение в этом потоке прерывается, работает обработчик, потом выполнение возобновляется.)
Если в то время когда пришел сигнал, мьютекс захвачен, то хендлер навсегда зависнет на попытке его захватить.

Как воспроизвести некорректность? Вставьте `sched_yield();` после `++value;` и запустите.

</p>
</details>



```python

```

# <a name="condvar_1"></a> Condvar 1 

Есть ли тут асинхронная безопасность?


```cpp
%%cpp condvar.c
%run gcc -fsanitize=thread condvar.c -lpthread -o condvar.exe
%run # ./condvar.exe

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <pthread.h>
#include <stdatomic.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; 
pthread_cond_t condvar = PTHREAD_COND_INITIALIZER;
_Atomic int value = -1;

static void* producer_func(void* arg) 
{
    atomic_store(&value, 42);
    pthread_cond_signal(&condvar);
}

static void* consumer_func(void* arg) 
{
    pthread_mutex_lock(&mutex);
    while (atomic_load(&value) == -1) {
        pthread_cond_wait(&condvar, &mutex); 
    }
    printf("Value = %d\n", atomic_load(&value));
    pthread_mutex_unlock(&mutex);
    return NULL;
}

int main()
{
    pthread_t producer_thread;
    pthread_create(&producer_thread, NULL, producer_func, NULL);
    
    pthread_t consumer_thread;
    pthread_create(&consumer_thread, NULL, consumer_func, NULL);

    pthread_join(producer_thread, NULL); 
    pthread_join(consumer_thread, NULL); 
    return 0;
}
```

<details>
<summary><b>Ответ</b></summary>
<p>

Нет, так как присвоение в атомик и сигнализация может произойти в одном потоке между `while (atomic_load(&value) == -1)` и `pthread_cond_wait(&condvar, &mutex);` в другом.

Воспроизведение: вставьте `sleep(2);` сразу после `while (value == -1) {`. И `sleep(1);` до `atomic_store(&value, 42);`.

</p>
</details>


# <a name="condvar_2"></a> Condvar 2

Есть ли тут асинхронная безопасность?


```cpp
%%cpp condvar_2.c
%run gcc -fsanitize=thread condvar_2.c -lpthread -o condvar_2.exe
%run # ./condvar_2.exe

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <pthread.h>
#include <stdatomic.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; 
pthread_cond_t condvar = PTHREAD_COND_INITIALIZER;
_Atomic int value = -1;

static void* producer_func(void* arg) 
{
    pthread_mutex_lock(&mutex);
    atomic_store(&value, 42);
    pthread_cond_signal(&condvar);
    pthread_mutex_unlock(&mutex);
}

static void* consumer_func(void* arg) 
{ 
    pthread_mutex_lock(&mutex);
    while (atomic_load(&value) == -1) {
        pthread_cond_wait(&condvar, &mutex); 
    }
    printf("Value = %d\n", atomic_load(&value));
    pthread_mutex_unlock(&mutex);
    return NULL;
}

int main()
{
    pthread_t producer_thread;
    pthread_create(&producer_thread, NULL, producer_func, NULL);
    
    pthread_t consumer_thread;
    pthread_create(&consumer_thread, NULL, consumer_func, NULL);

    pthread_join(producer_thread, NULL); 
    pthread_join(consumer_thread, NULL); 
    return 0;
}
```

<details>
<summary><b>Ответ</b></summary>
<p>

Тут есть.

</p>
</details>


# <a name="condvar_3"></a> Condvar 3

Есть ли тут асинхронная безопасность?


```cpp
%%cpp condvar_3.c
%run gcc -fsanitize=thread condvar_3.c -lpthread -o condvar_3.exe
%run #./condvar_3.exe

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <pthread.h>
#include <stdatomic.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; 
pthread_cond_t condvar = PTHREAD_COND_INITIALIZER;
_Atomic int value = -1;

static void* producer_func(void* arg) 
{
    atomic_store(&value, 42);
    pthread_mutex_lock(&mutex);
    pthread_mutex_unlock(&mutex);
    pthread_cond_signal(&condvar);
}

static void* consumer_func(void* arg) 
{
    pthread_mutex_lock(&mutex);
    while (atomic_load(&value) == -1) {
        pthread_cond_wait(&condvar, &mutex); 
    }
    printf("Value = %d\n", atomic_load(&value));
    pthread_mutex_unlock(&mutex);
    return NULL;
}

int main()
{
    pthread_t producer_thread;
    pthread_create(&producer_thread, NULL, producer_func, NULL);
    
    pthread_t consumer_thread;
    pthread_create(&consumer_thread, NULL, consumer_func, NULL);

    pthread_join(producer_thread, NULL); 
    pthread_join(consumer_thread, NULL); 
    return 0;
}
```

<details>
<summary><b>Ответ</b></summary>
<p>

Как-ни странно, но добавление пустой критической секции тоже спасает ситуацию. Пустая критическая секция позволяет подождать, пока другой поток войдет в wait.

</p>
</details>


# <a name="mutex_char"></a> Mutex

Есть ли тут асинхронная безопасность?


```cpp
%%cpp mutex.c
%run gcc -O3 -fsanitize=thread mutex.c -lpthread -o mutex.exe
%run # ./mutex.exe

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <pthread.h>
#include <assert.h>

pthread_mutex_t mutex[2] = {PTHREAD_MUTEX_INITIALIZER, PTHREAD_MUTEX_INITIALIZER};

struct {
    char state_0: 8; // protected by mutex[0]
    char state_1: 8; // protected by mutex[1]
} states = {.state_0 = 0, .state_1 = 0};

static void* tread_safe_func_0(void* arg) 
{
    for (int iter = 0; iter < 1000; ++iter) {
        pthread_mutex_lock(&mutex[0]);
        assert(states.state_0 == (iter & 1));
        states.state_0 ^= 1;
        pthread_mutex_unlock(&mutex[0]);
    }
}

static void* tread_safe_func_1(void* arg) 
{
    for (int iter = 0; iter < 1000; ++iter) {
        pthread_mutex_lock(&mutex[1]);
        assert(states.state_1 == (iter & 1));
        states.state_1 ^= 1;
        pthread_mutex_unlock(&mutex[1]);
    }
}

int main()
{
    pthread_t threads[2];
    pthread_create(&threads[0], NULL, tread_safe_func_0, NULL);
    pthread_create(&threads[1], NULL, tread_safe_func_1, NULL);
    for (int t = 0; t < 2; ++t) {
        pthread_join(threads[t], NULL); 
    }
    return 0;
}
```

<details>
<summary><b>Ответ</b></summary>
<p>

Операции с битовыми полями выполняются через битовые операции. Соответственно из памяти загружается как минимум байт и пишется как минимум байт. То есть читаются и пишутся не только те биты, которые мы меняем. Из-за этого происходит гонка данных. То есть асинхронной безопасности тут нет.

Казалось бы, битовые поля по 8 бит - так же как и "по умолчанию". В char же 8 бит!
<br> Но нет, из памяти грузится по два байта, как подсказывает выхлоп тред-санитайзера.

Для лучшего понимания, можно думать, что
```cpp
states.state_0 ^= 1;
```

Может работать скорее как 
```cpp
auto states_copy = states;
states_cop.state_0 ^= 1;
states = states_copy;
```            
То есть непреднамеренно затронет `.state_1`

</p>
</details>


# <a name="atomic"></a> Atomic

Есть ли тут асинхронная безопасность?


```cpp
%%cpp atomic.c
%run gcc -fsanitize=thread atomic.c -lpthread -o atomic.exe
%run # ./atomic.exe

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <pthread.h>
#include <stdatomic.h>

#define size 3
_Atomic int* data = NULL;

static void* consumer_func(void* arg) {
    int* local_data;
    while ((local_data = (int*)atomic_load(&data)) == NULL) {
        sched_yield();
    }
    for (int i = 0; i < size; ++i) {
        printf("%d ", local_data[i]);
    }
    return NULL;
}

int main()
{
    pthread_t consumer_thread;
    pthread_create(&consumer_thread, NULL, consumer_func, NULL);
    sched_yield();
    int created_data[size] = {10, 20, 30};
    atomic_store(&data, (void*)created_data);
    pthread_join(consumer_thread, NULL); 
    return 0;
}
```

<details>
<summary><b>Ответ</b></summary>
<p>

Вероятно именно этот пример будет работать всегда. Но тут есть мощная идейная ошибка, которая может выстрелить в более сложном случае. Ну или просто привести в мусору в коде, который тоже может стрелять, пусть и окольными путями.

Ошибка в `_Atomic int* data = NULL;`.

Если вы подумали, что тут атомарный указатель на `int`, то вы неправы.

Тут неатомарный указатель на атомарный `int`. В общем `_Atomic` работает с тем же приоритетом, что и `const`.

Вы же помните, что `const int*` это неконстантный указатель на константный `int`? :)

Исправляется код очень просто: `_Atomic (int*) data = NULL;`.

Касательно ответа на исходный вопрос: теоретически асинхронной безопасности нет. Но практически - скорее всего безопасность есть.

</p>
</details>


# <a name="philosophical_lock"></a> Philosophical lock

by [Андрей Баженов](https://github.com/TheRealBazhen)

Вдохновлено курсом concurrency. Найдите как можно больше проблем в этом коде (минимум две принципиально разные серьезные проблемы):


```cpp
%%cpp philosophical_lock.c
%run gcc -fsanitize=thread philosophical_lock.c -lpthread -o philosophical_lock.exe
%run # ./philosophical_lock.exe 

#include <assert.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

struct philosopher_data {
    pthread_mutex_t* left_fork;
    pthread_mutex_t* right_fork;
    int number; // protected by left_fork mutex
};

void philosopher_func(struct philosopher_data* data) {
    pthread_mutex_lock(data->left_fork);
    pthread_mutex_lock(data->right_fork);

    printf("Philosopher %d has eaten\n", data->number); fflush(stdout);

    pthread_mutex_unlock(data->left_fork);
    pthread_mutex_unlock(data->right_fork);
}

int main() {
    const int num_threads = 2;
    pthread_mutex_t mutexes[num_threads];
    pthread_t threads[num_threads];
    struct philosopher_data data[num_threads];

    for (int i = 0; i < num_threads; ++i) {
        assert(pthread_mutex_init(&mutexes[i], NULL) == 0);
        data[i] = (struct philosopher_data){.number=4, .left_fork=&mutexes[i], 
            .right_fork=&mutexes[(i + 1) % num_threads]};
        assert(pthread_create(&threads[i], NULL, (void* (*)(void*))philosopher_func, (void*)&data[i]) == 0);
    }

    for (int i = 0; i < num_threads; ++i) {
        assert(pthread_join(threads[i], NULL) == 0);
    }
}
```

<details>
<summary><b>Ответ</b></summary>
<p>
    
<details>
<summary>Проблема #0</summary>
Да, вы получили потенциальный deadlock. Как от него избавиться - вопрос не этого курса:)
Однако, именно с эти кодом у меня воспроизвести не получилось. Если же расставить sleep-ов, то успех обеспечен.
</details>

<details>
<summary>Проблема #1</summary>
Куда более серьезная проблема - использование неинициализированных ресурсов. Наши потоки смотрят как на свой mutex, так и на следующий, который может быть не инициализирован. Решается просто - выносом запуска потоков в отдельный цикл. Но локализуется ошибка непросто. Проявление бывает довольно интересное - у меня `mutex_lock` не блочил ничего и происходило одновременное обращение.
    
Хотя тред-санитайзер в одном запуске из 5 находит проблему. Вывод: если тест не очень хороший - запускайте в цикле на много итераций. 
</details>

</p>
</details>


```python

```


```python

```


```python

```

# <a name="threads"></a> Ниточки Ариадны

Есть ли асинхронная безопасность?


```cpp
%%cpp tmp.c
%run gcc -fsanitize=address tmp.c -lpthread -o tmp.exe
%run # ./tmp.exe


#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>

uint64_t N = 20;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t func_sleep = PTHREAD_COND_INITIALIZER;
pthread_cond_t main_sleep = PTHREAD_COND_INITIALIZER;
uint64_t number = -1;

void thread_func() {
    for (uint64_t i = 0; i < N; ++i) {
        pthread_mutex_lock(&mutex);
        number = i;       
        while (number != -1) {
            pthread_cond_wait(&func_sleep, &mutex);
        }
        pthread_mutex_unlock(&mutex);
        pthread_cond_signal(&main_sleep);
    }
}

int main() {
    pthread_t thread;
    pthread_create(&thread, NULL, (void* (*)(void*))thread_func, NULL);
    for (uint32_t i = 0; i < N; ++i) {
        pthread_mutex_lock(&mutex);
        while (number == -1) {
            pthread_cond_wait(&main_sleep, &mutex);
        }
        uint64_t number_copy = number;
        number = -1;
        pthread_mutex_unlock(&mutex);
        pthread_cond_signal(&func_sleep);
        printf("%ld\n", number_copy);
    }
    pthread_join(thread, NULL);
    return 0;
}
```

<details>
<summary><b>Ответ</b></summary>
<p>

Оба потока зависнут на cond_wait. Хотя несколько первых итераций может и выполниьтся.
Проблема в том, что `pthread_cond_signal(&main_sleep);` делается не в нужный момент.
Мы записали то, что хотим передать в number, и не просигналили, то того, как начать ждать, пока наше число извлечет основной поток. А как он извлечет число, если между передачей и ожиданием получения, мы не просигналили?

</p>
</details>



```python

```
