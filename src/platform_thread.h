#ifndef PLATFORM_THREAD_H
#define PLATFORM_THREAD_H

#ifdef __cplusplus
extern "C" {
#endif

typedef void *(*os_thread_func)(void *);

#ifdef _WIN32
#include <windows.h>
typedef struct { HANDLE handle; } os_thread_t;
typedef CRITICAL_SECTION os_mutex_t;
typedef HANDLE os_sem_t;
#else
#include <pthread.h>
#include <semaphore.h>
typedef struct { pthread_t handle; } os_thread_t;
typedef pthread_mutex_t os_mutex_t;
typedef sem_t os_sem_t;
#endif

int os_thread_create(os_thread_t *thread, os_thread_func func, void *arg);
void os_thread_join(os_thread_t *thread);
void os_mutex_init(os_mutex_t *mutex);
void os_mutex_lock(os_mutex_t *mutex);
void os_mutex_unlock(os_mutex_t *mutex);
void os_mutex_destroy(os_mutex_t *mutex);
void os_sem_init(os_sem_t *sem, int value);
void os_sem_wait(os_sem_t *sem);
void os_sem_post(os_sem_t *sem);
void os_sem_destroy(os_sem_t *sem);
void os_sleep_ms(int ms);

#ifdef __cplusplus
}
#endif

#endif
