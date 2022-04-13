#pragma once

#include "base.h"
#include "basic/lock.h"

typedef struct thread_condition *thread_condition_t;
typedef bool (*test_condition)(void *state_object);
typedef void (*on_condition_is_true)(void *state_object);

thread_condition_t thread_condition_init();
void thread_condition_deinit(thread_condition_t *ppctx);
void _thread_wait_event(thread_condition_t *ppctx, test_condition test, on_condition_is_true handle, void *context);
#define thread_wait_event(ppctx, test, handle, context) \
	if (0) {                                            \
		if (test(context)) {                            \
			handle(context);                            \
		}                                               \
	}                                                   \
	_thread_wait_event(ppctx, (test_condition)test, (on_condition_is_true)handle, context)
void thread_fire_event(thread_condition_t *ppctx);
kb_mutex_t thread_get_event_lock(thread_condition_t *ppctx);
pthread_cond_t *thread_get_event_condition(thread_condition_t *ppctx);
#define thread_event_autolock(ppctx)                                        \
	__extension__({                                                         \
		thread_condition_t *_th_event_ppctx = ppctx;                        \
		kb_mutex_t _th_event_lock = thread_get_event_lock(_th_event_ppctx); \
		bool r = false;                                                     \
		if (_th_event_lock) {                                               \
			r = autolock(_th_event_lock);                                   \
		}                                                                   \
		r;                                                                  \
	})
