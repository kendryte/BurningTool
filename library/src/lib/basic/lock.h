#pragma once

#include "context.h"
#include <pthread.h>

#ifndef NDEBUG
#include <stdbool.h>
kb_mutex_t __init_lock();
void __deinit_lock(kb_mutex_t *mutex);
void __lock(kb_mutex_t mutex, const char *varname, const char *file, int line);
void __unlock(kb_mutex_t mutex);
pthread_mutex_t *__raw_lock(kb_mutex_t mutex);
#define lock(mutex) __lock(mutex, #mutex, __FILE__, __LINE__)
#define unlock(mutex) __unlock(mutex)
#define lock_init() __init_lock()
#define lock_deinit(mutex) __deinit_lock(mutex)
#define raw_lock(mutex) __raw_lock(mutex)
#else
void _lock(pthread_mutex_t *mutex);
void _unlock(pthread_mutex_t *mutex);
pthread_mutex_t *_init_lock();
void _deinit_lock(pthread_mutex_t **mutex);
#define lock(mutex) _lock(mutex)
#define unlock(mutex) _unlock(mutex)
#define lock_init() _init_lock()
#define lock_deinit(mutex) _deinit_lock(mutex)
#define raw_lock(mutex) (mutex)
#endif

static inline void __unlock_ptr(kb_mutex_t *pmutex) { unlock(*pmutex); }
#define autolock(mutex)                                                                                                                              \
	__extension__({                                                                                                                                  \
		kb_mutex_t __attribute__((cleanup(__unlock_ptr))) __lock_holder = mutex;                                                                     \
		lock(__lock_holder);                                                                                                                         \
	})
