#include "global.h"
#include <stdlib.h>

typedef struct disposable_list_element
{
	struct disposable_list_element *prev;
	void *object;
	disposable_list_t *parent;
	dispose_function callback;
	struct disposable_list_element *next;
	struct disposable_debug __debug;
} disposable_list_element_t;

typedef struct disposable_list
{
	const char *comment;
	disposable_list_element_t *head;
	disposable_list_element_t *tail;
	uint32_t size;
	kb_mutex_t mutex;
	bool disposed;
} disposable_list_t;
// FILE_LINE_FORMAT FILE_LINE_VALUE
#ifndef NDEBUG
disposable __toDisposable(dispose_function callback, void *userData, const char *debug_title, const char *file, int line)
{
	return (disposable){
		.object = userData,
		.callback = callback,
		.list = NULL,
		._dbg = (struct disposable_debug){
			.title = debug_title,
			.file = file,
			.line = line,
		},
	};
}
#else
disposable toDisposable(dispose_function callback, void *userData)
{
	return (disposable){.object = userData, .callback = callback, .list = NULL};
}
#endif

disposable_list_t *disposable_list_init(const char *comment)
{
	disposable_list_t *ret = calloc(1, sizeof(disposable_list_t));
	if (ret == NULL)
		return NULL;
	ret->mutex = lock_init();
	ret->comment = comment;
	return ret;
}

void disposable_list_deinit(disposable_list_t *list)
{
	lock_deinit(&list->mutex);
	free(list);
}

disposable dispose_list_add(disposable_list_t *r, disposable e)
{
	m_assert_ptr(r, "dispose: got null ptr");
	m_assert(e.list == NULL, "dispose: already add to other list");
	m_assert(!r->disposed, "dispose: is disposed");

	if ((void *)r == (void *)e.object)
		m_assert(r->size == 0, "free self must at first element");

	lock(r->mutex);

	debug_print("dispose_list_add(<%s>[%d]): %s @ " FILE_LINE_FORMAT, NULLSTR(r->comment), r->size, NULLSTR(e._dbg.title), FILE_LINE_VALUE(e._dbg.file, e._dbg.line));
	disposable_list_element_t *ele = calloc(1, sizeof(disposable_list_element_t));
	ele->callback = e.callback;
	ele->object = e.object;
	ele->next = NULL;
#ifndef NDEBUG
	ele->__debug = e._dbg;
#endif
	ele->parent = r;
	e.list = r;

	if (!r->head)
		r->head = ele;

	if (r->tail)
		r->tail->next = ele;

	ele->prev = r->tail;

	r->tail = ele;
	r->size++;

	unlock(r->mutex);
	return e;
}

static void do_delete(disposable_list_t *r, disposable_list_element_t *target)
{
	if (target->prev)
		target->prev->next = target->next;

	if (target->next)
		target->next->prev = target->prev;

	if (target == r->head)
		r->head = r->head->next;

	if (target == r->tail)
		r->tail = r->tail->prev;

	r->size--;

	free(target);
}

void dispose_list_cancel(disposable_list_t *r, disposable e)
{
	if (r == NULL)
		r = e.list;
	m_assert_ptr(r, "dispose: no list information");

	m_assert_ptr(r->mutex, "dispose: not init");

	if (!r->disposed)
		lock(r->mutex);

	bool found = false;
	for (disposable_list_element_t *curs = r->head; curs != NULL; curs = curs->next)
	{
		if (curs->callback == e.callback && curs->object == e.object)
		{
			debug_print("\t" GREY("<%s>[%d]): %s @ " FILE_LINE_FORMAT), NULLSTR(r->comment), r->size, NULLSTR(curs->__debug.title), FILE_LINE_VALUE(curs->__debug.file, curs->__debug.line));
			do_delete(r, curs);
			found = true;
			break;
		}
	}

	if (!found)
	{
		debug_print(RED("dispose_list_delete") "(<%s>[%d]): %s", NULLSTR(r->comment), r->size, NULLSTR(r->comment));
		m_abort("dispose not found object");
	}

	if (!r->disposed)
		unlock(r->mutex);
}

void dispose_all(disposable_list_t *r)
{
	m_assert_ptr(r, "dispose: got null ptr");
	m_assert(!r->disposed, "dispose twice");

	r->disposed = true;
	lock(r->mutex);

	bool selfDisposing = r->size > 0 && r->head->callback == free_pointer && r->head->object == r;
	debug_print("dispose_all(%s) [size=%d, selfDisposing=%d]", NULLSTR(r->comment), r->size, selfDisposing);
	bool selfLast = false;

	while (r->tail)
	{
		if (selfDisposing && r->size == 1)
			selfLast = true;

		disposable_list_element_t *current = r->tail;
		current->callback(r, current->object);

		if (selfLast)
		{
			debug_print("  * dispose callback return, self memory free");
			break;
		}
		else
		{
			if (current == r->tail)
			{
				debug_print(RED("dispose") " %s @ " FILE_LINE_FORMAT, NULLSTR(current->__debug.title), FILE_LINE_VALUE(current->__debug.file, current->__debug.line));
				m_abort("disposed function not call to dispose_list_delete()");
			}
		}
	}

	unlock(r->mutex);
}

void dispose(disposable target)
{
	disposable_list_t *r = target.list;

	m_assert_ptr(r, "dispose object not in a list");
	m_assert(!r->disposed, "dispose twice");

	m_assert(!(target.callback == free_pointer && target.object == r), "can not dispose self");

	if (r->size == 0)
		return;

	disposable_list_element_t *current = r->tail;

	uint32_t plength = r->size;
	current->callback(r, current->object);

	// debug_print("  * dispose callback return, size=%d", target->size);
	m_assert(plength + 1 == r->size, "disposed function not call to dispose_list_delete()");
}

DECALRE_DISPOSE(free_pointer, void)
{
	free(context);
}
DECALRE_DISPOSE_END()
DECALRE_DISPOSE(free_and_unset_pointer, void *)
{
	free(*context);
	*context = NULL;
}
DECALRE_DISPOSE_END()
DECALRE_DISPOSE(dispose_child, void)
{
	dispose_all(context);
}
DECALRE_DISPOSE_END()
DECALRE_DISPOSE(dispose_all_and_deinit, disposable_list_t)
{
	dispose_all(context);
	disposable_list_deinit(context);
}
DECALRE_DISPOSE_END()
