#include "global.h"
#include <stdlib.h>
#include <string.h>

static disposable_registry global_disposable = {NULL, NULL, 0, 0};

void kburnDispose()
{
	debug_print("kburnDispose()");
	dispose(&global_disposable);
}

void global_resource_register(dispose_function callback, void *userData)
{
	dispose_add(&global_disposable, disposable(callback, userData));
}

void global_resource_unregister(dispose_function callback, void *userData)
{
	dispose_delete(&global_disposable, disposable(callback, userData));
}

uint32_t kburnGetResourceCount()
{
	return global_disposable.size;
}
