#include "platform_thread.h"
#include <stdlib.h>

#ifdef _WIN32

typedef struct {
    os_thread_func func;
    void *arg;
} thread_start_t;

static DWORD WINAPI thread_entry(LPVOID data) {
    thread_start_t *start = (thread_start_t *)data;
    os_thread_func func = start->func;
    void *arg = start->arg;
    free(start);
    func(arg);
    return 0;
}

int os_thread_create(os_thread_t *thread, os_thread_func func, void *arg) {
    thread_start_t *start = (thread_start_t *)malloc(sizeof(thread_start_t));
    if (!start) return -1;
    start->func = func;
    start->arg = arg;
    thread->handle = CreateThread(NULL, 0, thread_entry, start, 0, NULL);
    if (!thread->handle) {
        free(start);
        return -1;
    }
    return 0;
}

void os_thread_join(os_thread_t *thread) {
    WaitForSingleObject(thread->handle, INFINITE);
    CloseHandle(thread->handle);
}

void os_mutex_init(os_mutex_t *mutex) { InitializeCriticalSection(mutex); }
void os_mutex_lock(os_mutex_t *mutex) { EnterCriticalSection(mutex); }
void os_mutex_unlock(os_mutex_t *mutex) { LeaveCriticalSection(mutex); }
void os_mutex_destroy(os_mutex_t *mutex) { DeleteCriticalSection(mutex); }
void os_sem_init(os_sem_t *sem, int value) { *sem = CreateSemaphore(NULL, value, 32767, NULL); }
void os_sem_wait(os_sem_t *sem) { WaitForSingleObject(*sem, INFINITE); }
void os_sem_post(os_sem_t *sem) { ReleaseSemaphore(*sem, 1, NULL); }
void os_sem_destroy(os_sem_t *sem) { CloseHandle(*sem); }
void os_sleep_ms(int ms) { Sleep((DWORD)ms); }
unsigned long long os_time_ms(void) {
    LARGE_INTEGER freq, counter;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&counter);
    return (unsigned long long)(counter.QuadPart * 1000ULL / freq.QuadPart);
}

#else

#include <sys/time.h>
#include <unistd.h>

int os_thread_create(os_thread_t *thread, os_thread_func func, void *arg) {
    return pthread_create(&thread->handle, NULL, func, arg);
}

void os_thread_join(os_thread_t *thread) { pthread_join(thread->handle, NULL); }
void os_mutex_init(os_mutex_t *mutex) { pthread_mutex_init(mutex, NULL); }
void os_mutex_lock(os_mutex_t *mutex) { pthread_mutex_lock(mutex); }
void os_mutex_unlock(os_mutex_t *mutex) { pthread_mutex_unlock(mutex); }
void os_mutex_destroy(os_mutex_t *mutex) { pthread_mutex_destroy(mutex); }
void os_sem_init(os_sem_t *sem, int value) { sem_init(sem, 0, value); }
void os_sem_wait(os_sem_t *sem) { sem_wait(sem); }
void os_sem_post(os_sem_t *sem) { sem_post(sem); }
void os_sem_destroy(os_sem_t *sem) { sem_destroy(sem); }
void os_sleep_ms(int ms) { usleep((useconds_t)ms * 1000); }
unsigned long long os_time_ms(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (unsigned long long)tv.tv_sec * 1000ULL + (unsigned long long)tv.tv_usec / 1000ULL;
}

#endif
