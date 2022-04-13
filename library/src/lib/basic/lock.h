#pragma once

#include "context.h"
#include <stdbool.h>
#include <pthread.h>
#include "debug/path.h"

typedef struct mlock *kb_mutex_t;

#ifndef NDEBUG
bool _kb__lock(kb_mutex_t kb_mutex_t, debug_bundle dbg);
#define lock(kb_mutex_t) _kb__lock(kb_mutex_t, DEBUG_SAVE(#kb_mutex_t))
#else
bool _kb__lock(kb_mutex_t kb_mutex_t);
#define lock(kb_mutex_t) _kb__lock(kb_mutex_t)
#endif

void unlock(kb_mutex_t kb_mutex_t);

kb_mutex_t lock_init();

typedef void (*reference_destroy)(void *ctx, void *object);
void _lock_bind_destruct(kb_mutex_t l, reference_destroy destruct, void *context, void *object);
#define lock_bind_destruct(l, destruct, context, object) \
	if (0)                                               \
		destruct(context, object);                       \
	_lock_bind_destruct(l, (reference_destroy)destruct, context, object)

void lock_deinit(kb_mutex_t l);

pthread_mutex_t *raw_lock(kb_mutex_t l);

static inline void _kb__unlock_ptr(kb_mutex_t *pl) {
	unlock(*pl);
}
#define autolock(pl)                                                                      \
	__extension__({                                                                       \
		bool _auto_lock_result = lock(pl);                                                \
		if (_auto_lock_result) {                                                          \
			kb_mutex_t __attribute__((cleanup(_kb__unlock_ptr))) __auto_lock_holder = pl; \
		} else {                                                                          \
			debugger;                                                                     \
			debug_print(KBURN_LOG_ERROR, "autolock failed");                              \
		}                                                                                 \
		_auto_lock_result;                                                                \
	})
