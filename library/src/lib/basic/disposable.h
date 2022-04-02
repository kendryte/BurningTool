#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "lock.h"
#include "debug/print.h"

typedef struct disposable_list_element disposable_list_element_t;
typedef struct disposable_list disposable_list_t;
typedef void (*dispose_function)(disposable_list_t *this, void *userData);

typedef struct debug_bundle disposable_debug;

typedef struct disposable
{
	void *object;
	disposable_list_t *list;
	dispose_function callback;
	disposable_debug _dbg;
} disposable;

#ifndef NDEBUG
disposable __toDisposable(dispose_function callback, void *userData, const char *debug_title, const char *func, const char *file, int line);
#define toDisposable(callback, userData) __extension__({                                                    \
	if (0)                                                                                                  \
		callback((disposable_list_t *)NULL, userData);                                                      \
	__toDisposable((dispose_function)(callback), (void *)(userData), #callback "(" #userData ")", __func__, \
				   __FILE__, __LINE__);                                                                     \
})
#else
disposable _toDisposable(dispose_function callback, void *userData);
#define toDisposable(callback, userData) __extension__({                                          \
	if (0)                                                                                        \
		callback((disposable_list_t *)NULL, userData);                                            \
	_toDisposable((dispose_function)(callback), (void *)(userData), #callback "(" #userData ")"); \
})
#endif

static inline disposable bindToList(disposable_list_t *list, disposable d)
{
	d.list = list;
	return d;
}

disposable_list_t *disposable_list_init(const char *comment);
void disposable_list_deinit(disposable_list_t *list);
disposable dispose_list_add(disposable_list_t *source, disposable element);
void dispose_list_cancel(disposable_list_t *source, disposable element);
void dispose_all(disposable_list_t *target);
void dispose(disposable target);

#define DECALRE_DISPOSE(function_name, context_type)                                                          \
	DECALRE_DISPOSE_HEADER(function_name, context_type)                                                       \
	{                                                                                                         \
		debug_print(KBURN_LOG_DEBUG, "[dispose] - " #function_name "(" #context_type " [%p])", (void *)_ctx); \
		dispose_list_cancel(this, toDisposable(function_name, _ctx));                                         \
		context_type *context = _ctx;                                                                         \
		if (1)

#define DECALRE_DISPOSE_HEADER(function_name, context_type) \
	void function_name(disposable_list_t *this, context_type *_ctx)

#define DECALRE_DISPOSE_END() \
	}

DECALRE_DISPOSE_HEADER(free_pointer, void);
DECALRE_DISPOSE_HEADER(free_and_unset_pointer, void *);
DECALRE_DISPOSE_HEADER(dispose_child, void);
DECALRE_DISPOSE_HEADER(dispose_all_and_deinit, disposable_list_t);

#define register_dispose_pointer(registry, _pointer) __extension__({ \
	void *pointer = _pointer;                                        \
	dispose_list_add(registry, toDisposable(free_pointer, pointer)); \
	pointer;                                                         \
})

#define register_dispose_pointer_and_unset(registry, pointer) __extension__({   \
	dispose_list_add(registry, toDisposable(free_and_unset_pointer, &pointer)); \
	pointer;                                                                    \
})
