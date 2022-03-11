#include "global.h"
#include <stdlib.h>
#include <assert.h>

void dispose_add(disposable_registry *source, dispose_callback e)
{
	debug_print("dispose_add(%p, %p) [size=%d]", (void *)source, (void *)e.userData, source->size);
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
	debug_print("dispose_delete(%p, %p) [size=%d]", (void *)source, (void *)e.userData, source->size);

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
	debug_print("dispose(%p) [size=%d]", (void *)target, target->size);

	while (target->tail)
	{
		element *current = target->tail;
		current->callback(target, current->userData);

		debug_print("  * dispose callback return, size=%d", target->size);
		assert((current != target->tail) && "disposed function not call to dispose_delete()");
	}
}
