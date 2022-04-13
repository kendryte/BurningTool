#include "queued-thread.h"
#include "context.h"
#include "basic/errors.h"
#include "basic/event-queue.h"
#include "basic/lock.h"
#include "basic/resource-tracker.h"
#include "thread.h"
#include <pthread.h>

typedef struct event_queue_thread {
	kbthread thread;
	queue_t queue;
	event_handler handler;
	KBCTX scope;
} event_queue_thread;

static DECALRE_DISPOSE(event_thread_queue_deinit, event_queue_thread *) {
	event_queue_thread *obj = *context;
	*context = NULL;
	queue_destroy(obj->queue, free);
}
DECALRE_DISPOSE_END()

static inline bool queue_is_not_empty(event_queue_thread *context) {
	return queue_size(context->queue) > 0;
}
static inline void work_in_queue(event_queue_thread *context) {
	void *data = queue_shift(context->queue);
	if (data == NULL) {
		debug_print(KBURN_LOG_ERROR, "strange: queue shift got null");
	} else {
		context->handler(context->scope, data);
	}
}

void event_queue_thread_main(void *_ctx, KBCTX scope, const bool *const quit) {
	event_queue_thread *context = _ctx;

	context->scope = scope;
	while (!*quit) {
		current_thread_wait_event(queue_is_not_empty, work_in_queue, context);
	}
}

void event_thread_deinit(KBCTX scope, event_queue_thread **queue_thread) {
	if ((*queue_thread)->thread) {
		thread_destroy(scope, (*queue_thread)->thread);
	}
	event_thread_queue_deinit(scope->disposables, queue_thread);
	*queue_thread = NULL;
}

kburn_err_t event_thread_init(KBCTX scope, const char *title, event_handler handler, event_queue_thread **out) {
	DeferEnabled;

	event_queue_thread *ret = DeferFree(MyAlloc(event_queue_thread));
	register_dispose_pointer(scope->disposables, ret);
	*out = ret;
	DeferDispose(scope->disposables, out, event_thread_queue_deinit);

	ret->handler = handler;

	IfErrorReturn(queue_create(&ret->queue));

	IfErrorReturn(thread_create(title, event_queue_thread_main, ret, scope, &ret->thread));
	thread_resume(ret->thread);

	DeferAbort;
	return KBurnNoErr;
}

kburn_err_t event_thread_queue(event_queue_thread *et, void *event) {
	if (!et) {
		debug_print(KBURN_LOG_WARN, "push to thread after queue deinited.");
		return make_error_code(KBURN_ERROR_KIND_COMMON, KBurnObjectDestroy);
	}

	thread_event_autolock(thread_get_condition(et->thread));

	kburn_err_t r = queue_push(et->queue, event);

	return r;
}
