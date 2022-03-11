#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct disposable_registry
{
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
} element;

typedef struct dispose_callback
{
	void *userData;
	dispose_function callback;
} dispose_callback;

void dispose_add(disposable_registry *source, dispose_callback element);
void dispose_delete(disposable_registry *source, dispose_callback element);
void dispose(disposable_registry *target);

static inline dispose_callback disposable(dispose_function callback, void *userData)
{
	dispose_callback ret = {
		userData,
		callback,
	};
	return ret;
}

#define disposable_foreach_start(list, ele)                         \
	lock(&(list)->lock);                                            \
	for (element *ele = (list)->head; ele != NULL; ele = ele->next) \
	{

#define disposable_foreach_end(list) \
	}                                \
	unlock(&(list)->lock)

#define DECALRE_DISPOSE(function_name, context_type)           \
	void function_name(disposable_registry *this, void *_ctx)  \
	{                                                          \
		dispose_delete(this, disposable(function_name, _ctx)); \
		context_type *context = _ctx;                          \
		if (1)

#define DECALRE_DISPOSE_END() \
	}
