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
	pthread_cond_t cond;
	kb_mutex_t mutex;
	event_handler handler;
} event_queue_thread;

static DECALRE_DISPOSE(event_thread_queue_deinit, event_queue_thread *) {
	event_queue_thread *obj = *context;
	*context = NULL;
	pthread_cond_destroy(&obj->cond);
	lock_deinit(&obj->mutex);
	queue_destroy(obj->queue, free);
}
DECALRE_DISPOSE_END()

void event_queue_thread_main(void *_ctx, KBCTX scope, const bool *const quit) {
	event_queue_thread *context = _ctx;
	while (!*quit) {
		lock(context->mutex);
		while (queue_size(context->queue) == 0 && !*quit)
			pthread_cond_wait(&context->cond, raw_lock(context->mutex));

		void *data = queue_shift(context->queue);
		if (data == NULL) {
			debug_print(KBURN_LOG_ERROR, "strange: queue shift got null");
			continue;
		}

		context->handler(scope, data);

		unlock(context->mutex);
	}
}

void event_thread_deinit(KBCTX scope, event_queue_thread **queue_thread) {
	if ((*queue_thread)->thread)
		thread_destroy(scope, (*queue_thread)->thread);
	event_thread_queue_deinit(scope->disposables, queue_thread);
	*queue_thread = NULL;
}

static DECALRE_DISPOSE(graceful_quit, event_queue_thread) {
	thread_tell_quit(context->thread);
	autolock(context->mutex);
	pthread_cond_signal(&context->cond);
}
DECALRE_DISPOSE_END()

kburn_err_t event_thread_init(KBCTX scope, const char *title, event_handler handler, event_queue_thread **out) {
	DeferEnabled;

	event_queue_thread *ret = DeferFree(MyAlloc(event_queue_thread));
	register_dispose_pointer(scope->disposables, ret);
	*out = ret;
	DeferDispose(scope->disposables, out, event_thread_queue_deinit);

	ret->handler = handler;
	ret->mutex = CheckNull(lock_init());

	int r = pthread_cond_init(&ret->cond, NULL);
	if (r != 0) {
		debug_print(KBURN_LOG_ERROR, "pthread_cond_init failed with %d: %s", r, strerror(r));
		return make_error_code(KBURN_ERROR_KIND_SYSCALL, r);
	}
	IfErrorReturn(queue_create(&ret->queue));

	IfErrorReturn(thread_create(title, event_queue_thread_main, ret, scope, &ret->thread));
	DeferDispose(scope->threads, ret, graceful_quit);

	DeferAbort;
	return KBurnNoErr;
}

kburn_err_t event_thread_queue(event_queue_thread *thread, void *event) {
	if (!thread) {
		debug_print(KBURN_LOG_WARN, "push to thread after queue deinited.");
		return make_error_code(KBURN_ERROR_KIND_COMMON, KBurnObjectDestroy);
	}

	autolock(thread->mutex);

	kburn_err_t r = queue_push(thread->queue, event);

	pthread_cond_signal(&thread->cond);

	return r;
}
