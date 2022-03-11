#include "global.h"
#include <stdlib.h>
#include <assert.h>

void dispose_add(disposable_registry *source, dispose_callback e)
{
	debug_print("dispose_add(%p[%s], %p) [size=%d]", (void *)source, source->comment, (void *)e.userData, source->size);
	element *ele = malloc(sizeof(element));
	ele->callback = e.callback;
	ele->userData = e.userData;
	ele->next = NULL;

	lock(&source->lock);

	if (!source->head)
		source->head = ele;

	if (source->tail)
		source->tail->next = ele;

	ele->prev = source->tail;

	source->tail = ele;
	source->size++;

	unlock(&source->lock);
}

static void do_delete(disposable_registry *source, element *target)
{
	debug_print("\tdispose::do_delete(%p, %p) [size=%d]", (void *)source, (void *)target->userData, source->size);

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
	debug_print("dispose_delete(%p[%s], %p) [size=%d]", (void *)source, source->comment, (void *)e.userData, source->size);

	disposable_foreach_start(source, curs);
	if (curs->callback == e.callback && curs->userData == e.userData)
	{
		do_delete(source, curs);
		unlock(&source->lock);
		return;
	}
	disposable_foreach_end(source);

	debug_print("  - not found");
}

void dispose(disposable_registry *target)
{
	bool selfDisposing = target->size > 0 && target->head->callback == free_pointer && target->head->userData == target;
	debug_print("dispose(%p[%s]) [size=%d, selfDisposing=%d]", (void *)target, target->comment, target->size, selfDisposing);
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
			debug_print("  * dispose callback return, size=%d", target->size);
			assert((current != target->tail) && "disposed function not call to dispose_delete()");
		}
	}
}

DECALRE_DISPOSE(free_pointer, void)
{
	free(context);
}
DECALRE_DISPOSE_END()

void *register_free_pointer_pass(disposable_registry *reg, void *ptr)
{
	if (reg == ptr)
	{
		assert(reg->size == 0 && "free self must at first element");
	}
	dispose_add(reg, disposable(free_pointer, ptr));
	return ptr;
}

DECALRE_DISPOSE(dispose_child, void)
{
	dispose(context);
}
DECALRE_DISPOSE_END()

void dispose_chain(disposable_registry *parent, disposable_registry *child)
{
	dispose_add(parent, disposable(dispose_child, child));
}
