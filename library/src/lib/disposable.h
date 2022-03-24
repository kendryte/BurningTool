#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

typedef struct disposable_registry
{
	const char *comment;
	struct element *head;
	struct element *tail;
	uint32_t size;
	volatile int lock;
} disposable_registry;
extern disposable_registry lib_global_scope;

typedef void (*dispose_function)(disposable_registry *this, void *userData);

typedef struct element
{
	struct element *prev;
	void *userData;
	dispose_function callback;
	struct element *next;
	const char *debug_title;
} element;

typedef struct dispose_callback
{
	void *userData;
	dispose_function callback;
	const char *debug_title;
} dispose_callback;

kburn_err_t dispose_add(disposable_registry *source, dispose_callback element);
void dispose_delete(disposable_registry *source, dispose_callback element);
void dispose(disposable_registry *target);

#ifndef NDEBUG
#define STRINGIZE(x) STRINGIZE2(x)
#define STRINGIZE2(x) #x
dispose_callback __disposable(dispose_function callback, void *userData, const char *debug_title);
#define disposable(callback, userData) __disposable(callback, userData, \
													"" #callback "(" #userData ")\n                                                      at " __FILE__ ":" STRINGIZE(__LINE__))
#else
dispose_callback disposable(dispose_function callback, void *userData);
#endif

#define disposable_foreach_start(list, ele)                         \
	lock(&(list)->lock);                                            \
	for (element *ele = (list)->head; ele != NULL; ele = ele->next) \
	{

#define disposable_foreach_end(list) \
	}                                \
	unlock(&(list)->lock)

#define DECALRE_DISPOSE(function_name, context_type)                                                   \
	DECALRE_DISPOSE_HEADER(function_name, context_type)                                                \
	{                                                                                                  \
		debug_print("[dispose] \x1B[38;5;11m" #function_name "\x1B[0m(" #context_type " [%p])", _ctx); \
		dispose_delete(this, disposable(function_name, _ctx));                                         \
		context_type *context = _ctx;                                                                  \
		if (1)

#define DECALRE_DISPOSE_HEADER(function_name, context_type) \
	void function_name(disposable_registry *this, void *_ctx)

#define DECALRE_DISPOSE_END() \
	}

DECALRE_DISPOSE_HEADER(free_pointer, void);
DECALRE_DISPOSE_HEADER(free_pointer_null, void *);
DECALRE_DISPOSE_HEADER(dispose_child, void);

#define register_dispose_pointer(registry, _pointer) __extension__({      \
	void *pointer = _pointer;                                             \
	if ((void *)registry == (void *)pointer)                              \
		assert(registry->size == 0 && "free self must at first element"); \
	dispose_add(registry, disposable(free_pointer, pointer));             \
	pointer;                                                              \
})

#define register_dispose_pointer_null(registry, pointer) __extension__({ \
	dispose_add(registry, disposable(free_pointer_null, &pointer));      \
	pointer;                                                             \
})

#define dispose_chain(registry, child_registry) \
	dispose_add(registry, disposable(dispose_child, child_registry));

#define CALLOC_AUTO_FREE(disposable_registry, type, ...) (type *)register_dispose_pointer(disposable_registry, KBALLOC(type __VA_OPT__(, ) __VA_ARGS__))
