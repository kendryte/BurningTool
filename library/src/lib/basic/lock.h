#pragma once

#include "context.h"
#include <pthread.h>

#ifndef NDEBUG
#include <stdbool.h>
kb_mutex_t _kb__init_lock();
void _kb__deinit_lock(kb_mutex_t *mutex);
void _kb__lock(kb_mutex_t mutex, const char *varname, const char *file, int line);
void _kb__unlock(kb_mutex_t mutex);
pthread_mutex_t *_kb__raw_lock(kb_mutex_t mutex);
#define lock(mutex) _kb__lock(mutex, #mutex, __FILE__, __LINE__)
#define unlock(mutex) _kb__unlock(mutex)
#define lock_init() _kb__init_lock()
#define lock_deinit(mutex) _kb__deinit_lock(mutex)
#define raw_lock(mutex) _kb__raw_lock(mutex)
#else
void _kb_lock(pthread_mutex_t *mutex);
void _kb_unlock(pthread_mutex_t *mutex);
pthread_mutex_t *_kb_init_lock();
void _kb_deinit_lock(pthread_mutex_t **mutex);
#define lock(mutex) _kb_lock(mutex)
#define unlock(mutex) _kb_unlock(mutex)
#define lock_init() _kb_init_lock()
#define lock_deinit(mutex) _kb_deinit_lock(mutex)
#define raw_lock(mutex) (mutex)
#endif

static inline void _kb__unlock_ptr(kb_mutex_t *pmutex) {
	unlock(*pmutex);
}
#define autolock(mutex)                                                             \
	__extension__({                                                                 \
		kb_mutex_t __attribute__((cleanup(_kb__unlock_ptr))) __lock_holder = mutex; \
		lock(__lock_holder);                                                        \
	})
