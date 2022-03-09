#include "global.h"
#include <stdlib.h>
#include <assert.h>

typedef struct element
{
	struct element *prev;
	void *userData;
	dispose_function callback;
	struct element *next;
} element;

void dispose_add(disposable_registry *source, dispose_callback e)
{
	debug_print("dispose_add(%p, %p) [size=%d]", (void *)source, (void *)e.userData, source->size);
	element *ele = malloc(sizeof(element));
	ele->callback = e.callback;
	ele->userData = e.userData;
	ele->next = NULL;

	lock(&source->lock);

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

	lock(&source->lock);
	for (element *curs = source->head; curs != NULL; curs = curs->next)
	{
		if (curs->callback == e.callback && curs->userData == e.userData)
		{
			do_delete(source, curs);
			unlock(&source->lock);
			return;
		}
	}
	unlock(&source->lock);

	debug_print("  - not found");
}

void dispose(disposable_registry *target)
{
	debug_print("dispose(%p) [size=%d]", (void *)target, target->size);

	lock(&target->lock);
	while (target->head)
	{
		element *current = target->head;
		current->callback(current->userData);

		assert((current != target->head) && "disposed function not call to dispose_delete()");
	}
	unlock(&target->lock);
}
