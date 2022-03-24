#include "global.h"
#include <stdlib.h>
#include <assert.h>

#ifndef NDEBUG
dispose_callback __disposable(dispose_function callback, void *userData, const char *debug_title)
{
	dispose_callback ret = {
		userData,
		callback,
		debug_title,
	};
	return ret;
}
#else
dispose_callback disposable(dispose_function callback, void *userData)
{
	dispose_callback ret = {
		.userData = userData,
		.callback = callback,
	};
	return ret;
}
#endif

kburn_err_t dispose_add(disposable_registry *source, dispose_callback e)
{
	debug_print("dispose_add(<%s>[%d]): %s", NULLSTR(source->comment), source->size, NULLSTR(e.debug_title));
	element *ele = KBALLOC(element);
	ele->callback = e.callback;
	ele->userData = e.userData;
	ele->next = NULL;
	ele->debug_title = e.debug_title;

	lock(&source->lock);

	if (!source->head)
		source->head = ele;

	if (source->tail)
		source->tail->next = ele;

	ele->prev = source->tail;

	source->tail = ele;
	source->size++;

	unlock(&source->lock);
	return KBurnNoErr;
}

static void do_delete(disposable_registry *source, element *target)
{
	assert((source->size > 0) && "delete element from empty list");

	if (target->prev)
		target->prev->next = target->next;

	if (target->next)
		target->next->prev = target->prev;

	if (target == source->head)
		source->head = source->head->next;

	if (target == source->tail)
		source->tail = source->tail->prev;

	source->size--;

	free(target);
}

void dispose_delete(disposable_registry *source, dispose_callback e)
{
	bool found = false;
	disposable_foreach_start(source, curs);
	if (curs->callback == e.callback && curs->userData == e.userData)
	{
		debug_print("dispose_delete(<%s>[%d]): %s", NULLSTR(source->comment), source->size, NULLSTR(curs->debug_title));
		do_delete(source, curs);
		found = true;
		break;
	}
	disposable_foreach_end(source);

	if (!found)
	{
		debug_print("\x1B[38;5;9mdispose_delete\x1B[0m(<%s>[%d]): %s", NULLSTR(source->comment), source->size, NULLSTR(e.debug_title));
		debug_print("  - not found");
	}
}

void dispose(disposable_registry *target)
{
	bool selfDisposing = target->size > 0 && target->head->callback == free_pointer && target->head->userData == target;
	debug_print("dispose(%s) [size=%d, selfDisposing=%d]", NULLSTR(target->comment), target->size, selfDisposing);
	bool selfLast = false;

	while (target->tail)
	{
		if (selfDisposing && target->size == 1)
			selfLast = true;

		element *current = target->tail;
		current->callback(target, current->userData);

		if (selfLast)
		{
			debug_print("  * dispose callback return, self memory free");
			return;
		}
		else
		{
			// debug_print("  * dispose callback return, size=%d", target->size);
			assert((current != target->tail) && "disposed function not call to dispose_delete()");
		}
	}
}

DECALRE_DISPOSE(free_pointer, void)
{
	free(context);
}
DECALRE_DISPOSE_END()
DECALRE_DISPOSE(free_pointer_null, void *)
{
	free(*context);
	*context = NULL;
}
DECALRE_DISPOSE_END()
DECALRE_DISPOSE(dispose_child, void)
{
	dispose(context);
}
DECALRE_DISPOSE_END()
