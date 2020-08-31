// %%cpp sem_anon.c
// %run gcc -Wall -fsanitize=thread -lrt sem_anon.c -o sem_anon.exe
// %run ./sem_anon.exe

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

