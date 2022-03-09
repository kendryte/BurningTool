#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef void (*dispose_function)(void *userData);

typedef struct disposable_registry
{
	struct element *head;
	struct element *tail;
	uint32_t size;
	volatile int lock;
} disposable_registry;

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
