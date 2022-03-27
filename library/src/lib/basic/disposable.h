#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "lock.h"

typedef struct disposable_list_element disposable_list_element_t;
typedef struct disposable_list disposable_list_t;
typedef void (*dispose_function)(disposable_list_t *this, void *userData);

struct disposable_debug
{
	const char *title;
	const char *file;
	int line;
};

typedef struct disposable
{
	void *object;
	disposable_list_t *list;
	dispose_function callback;
	struct disposable_debug _dbg;
} disposable;

#ifndef NDEBUG
disposable __toDisposable(dispose_function callback, void *userData, const char *debug_title, const char *file, int line);
#define toDisposable(callback, userData) __extension__({                                                                      \
	if (0)                                                                                                                    \
		callback(NULL, userData);                                                                                             \
	__toDisposable((dispose_function)(callback), (void *)(userData), "" #callback "(" #userData ") at ", __FILENAME__, __LINE__); \
})
#else
disposable toDisposable(dispose_function callback, void *userData);
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

#define DECALRE_DISPOSE(function_name, context_type)                                                   \
	DECALRE_DISPOSE_HEADER(function_name, context_type)                                                \
	{                                                                                                  \
		debug_print("[dispose] \x1B[38;5;11m" #function_name "\x1B[0m(" #context_type " [%p])", _ctx); \
		dispose_list_cancel(this, toDisposable(function_name, _ctx));                                  \
		context_type *context = _ctx;                                                                  \
		if (1)

#define DECALRE_DISPOSE_HEADER(function_name, context_type) \
	void function_name(disposable_list_t *this, void *_ctx)

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

#define dispose_chain(registry, child_registry) \
	dispose_list_add(registry, toDisposable(dispose_child, child_registry));
