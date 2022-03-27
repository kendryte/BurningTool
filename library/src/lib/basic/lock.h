#pragma once

#ifndef NDEBUG
#include <stdbool.h>
typedef struct kb_mutex *kb_mutex_t;
kb_mutex_t __init_lock();
void __deinit_lock(kb_mutex_t *mutex);
void __lock(kb_mutex_t mutex, const char *varname, const char *file, int line);
void __unlock(kb_mutex_t mutex);
#define lock(mutex) __lock(mutex, #mutex, __FILE__, __LINE__)
#define unlock(mutex) __unlock(mutex)
#define lock_init() __init_lock()
#define lock_deinit(mutex) __deinit_lock(mutex)
#else
#define kb_mutex_t *pthread_mutex_t
void _lock(pthread_mutex_t *mutex);
void _unlock(pthread_mutex_t *mutex);
pthread_mutex_t *_init_lock();
void _deinit_lock(pthread_mutex_t **mutex);
#define lock(mutex) _lock(mutex)
#define unlock(mutex) _unlock(mutex)
#define lock_init() _init_lock()
#define lock_deinit(mutex) _deinit_lock(mutex)
#endif
