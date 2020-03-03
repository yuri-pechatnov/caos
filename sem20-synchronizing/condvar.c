// %%cpp condvar.c
// %run gcc -fsanitize=thread condvar.c -lpthread -o condvar.exe
// %run (for i in $(seq 0 100000); do echo -n "$i " ; done) | ./condvar.exe > out.txt
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

