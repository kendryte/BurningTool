#include "disposable.h"
#include "debug/print.h"
#include "global.h"
#include <stdlib.h>

typedef struct disposable_list_element {
	struct disposable_list_element *prev;
	void *object;
	disposable_list_t *parent;
	dispose_function callback;
	struct disposable_list_element *next;
	disposable_debug __debug;
} disposable_list_element_t;

typedef struct disposable_list {
	const char *comment;
	disposable_list_element_t *head;
	disposable_list_element_t *tail;
	uint32_t size;
	kb_mutex_t mutex;
	bool disposed;
} disposable_list_t;

#ifndef NDEBUG
disposable __toDisposable(dispose_function callback, void *userData, const char *debug_title, const char *func, const char *file, int line) {
	return (disposable){
		.object = userData,
		.callback = callback,
		.list = NULL,
		._dbg.title = debug_title,
		._dbg.func = func,
		._dbg.file = file,
		._dbg.line = line,
		._dbg.buff2 = " @ ",
	};
}
#else
disposable _toDisposable(dispose_function callback, void *userData, const char *debug_title) {
	return (disposable){.object = userData, .callback = callback, .list = NULL, ._dbg = debug_title};
}
#endif
disposable_list_t *disposable_list_init(const char *comment) {
	disposable_list_t *ret = calloc(1, sizeof(disposable_list_t));
	if (ret == NULL)
		return NULL;
	ret->mutex = lock_init();
	ret->comment = comment;
	return ret;
}

void disposable_list_deinit(disposable_list_t *list) {
	lock_deinit(&list->mutex);
	free(list);
}

disposable dispose_list_add(disposable_list_t *r, disposable e) {
	m_assert_ptr(r, "dispose: got null ptr");
	m_assert(e.list == NULL, "dispose: already add to other list");
	m_assert_ptr(e.object, "dispose: missing object (or NULL)");
	m_assert_ptr(e.callback, "dispose: missing callback (or NULL)");
	m_assert(!r->disposed, "dispose: list is disposed");

	if ((void *)r == (void *)e.object)
		m_assert(r->size == 0, "free self must at first element");

	lock(r->mutex);

	debug_trace_function("<%s>[%d], %s%s", NULLSTR(r->comment), r->size, DEBUG_OBJ_TITLE(e._dbg), DEBUG_OBJ_PATH(e._dbg));
	disposable_list_element_t *ele = calloc(1, sizeof(disposable_list_element_t));
	ele->callback = e.callback;
	ele->object = e.object;
	ele->next = NULL;
	ele->__debug = e._dbg;
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

static void debug_list_content(const disposable_list_t *r) {
	debug_trace_function();
	int index = 0;
	for (disposable_list_element_t *curs = r->head; curs != NULL; curs = curs->next) {
		debug_print_dbg(KBURN_LOG_TRACE, curs->__debug, COLOR_FMT(" [%02d] %s {%p}"),
						COLOR_ARG(GREY, index, DEBUG_OBJ_TITLE(curs->__debug), (void *)curs->object));
		index++;
	}
}

static void do_delete(disposable_list_t *r, disposable_list_element_t *target) {
	if (target->prev)
		target->prev->next = target->next;

	if (target->next)
		target->next->prev = target->prev;

	if (target == r->head)
		r->head = target->next;

	if (target == r->tail)
		r->tail = target->prev;

	r->size--;

	free(target);
}

void dispose_list_cancel(disposable_list_t *r, disposable e) {
	if (r == NULL)
		r = e.list;

	m_assert_ptr(r, "dispose: no list information");
	m_assert_ptr(r->mutex, "dispose: not init");

	debug_trace_function("<%s>[%d], %s%s", NULLSTR(r->comment), r->size, DEBUG_OBJ_TITLE(e._dbg), DEBUG_OBJ_PATH(e._dbg));

	if (!r->disposed)
		lock(r->mutex);

	bool found = false;
	int index = 0;
	for (disposable_list_element_t *curs = r->head; curs != NULL; curs = curs->next) {
		if (curs->callback == e.callback && curs->object == e.object) {
			debug_print_dbg(KBURN_LOG_TRACE, curs->__debug, " [%02d] " COLOR_FMT("<%s>[%d]") ": %s", index,
							COLOR_ARG(GREY, NULLSTR(r->comment), r->size), DEBUG_OBJ_TITLE(curs->__debug));
			do_delete(r, curs);
			found = true;
			break;
		}
		index++;
	}

	if (!found) {
		debug_list_content(r);
		m_abort("dispose not found object");
	}

	if (!r->disposed)
		unlock(r->mutex);
}

void dispose_all(disposable_list_t *r) {
	m_assert_ptr(r, "dispose: got null ptr");
	m_assert(!r->disposed, "dispose twice");

	r->disposed = true;
	lock(r->mutex);

	bool selfDisposing = r->size > 0 && r->head->callback == free_pointer && r->head->object == r;
	debug_trace_function("%s [size=%d, selfDisposing=%d]", NULLSTR(r->comment), r->size, selfDisposing);
	debug_list_content(r);

	bool selfLast = false;

	while (r->tail) {
		if (selfDisposing && r->size == 1)
			selfLast = true;

		disposable_list_element_t *current = r->tail;
		current->callback(r, current->object);

		if (selfLast) {
			debug_print(KBURN_LOG_TRACE, "  * dispose callback return, self memory free");
			break;
		} else {
			if (current == r->tail) {
				debug_list_content(r);
				debug_print_dbg(KBURN_LOG_ERROR, current->__debug, COLOR_FMT("dispose") "%s", COLOR_ARG(RED), DEBUG_OBJ_TITLE(current->__debug));
				m_abort("disposed function not call to dispose_list_delete()");
			}
		}
	}

	unlock(r->mutex);
}

void dispose(disposable target) {
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
	m_assert(plength - 1 == r->size, "not call to dispose_list_delete(): %s at %s:%d", current->__debug.title, current->__debug.file,
			 current->__debug.line);
}

DECALRE_DISPOSE(free_pointer, void) { free(context); }
DECALRE_DISPOSE_END()
DECALRE_DISPOSE(free_and_unset_pointer, void *) {
	free(*context);
	*context = NULL;
}
DECALRE_DISPOSE_END()
DECALRE_DISPOSE(dispose_child, void) { dispose_all(context); }
DECALRE_DISPOSE_END()
DECALRE_DISPOSE(dispose_all_and_deinit, disposable_list_t) {
	dispose_all(context);
	disposable_list_deinit(context);
}
DECALRE_DISPOSE_END()
