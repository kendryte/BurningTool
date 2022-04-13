#include "thread.h"
#include "context.h"
#include "basic/errors.h"
#include "basic/sleep.h"
#include "thread-condition.h"
#include <pthread.h>
#include "debug/print.h"

enum ThreadStage
{
	THREAD_INIT = 0,
	THREAD_PAUSE,
	THREAD_RUNNING,
	THREAD_QUITTING,
	THREAD_COMPLETE,
};

static const char *stage_name(enum ThreadStage stage) {
	switch (stage) {
	case THREAD_INIT:
		return "INIT";
	case THREAD_PAUSE:
		return "PAUSE";
	case THREAD_RUNNING:
		return "RUNNING";
	case THREAD_QUITTING:
		return "QUITTING";
	case THREAD_COMPLETE:
		return "COMPLETE";
	}
	return "???";
}

typedef struct thread_passing_object {
	KBCTX scope;
	pthread_t thread;
	thread_function main;
	void *user_data;
	const char *debug_title;

	enum ThreadStage stage;

	thread_condition_t condition;

	bool quit_signal;
} thread_passing_object;

void thread_tell_quit(thread_passing_object *thread) {
	autolock(thread_get_event_lock(&thread->condition));

	thread->quit_signal = true;
	if (thread->stage != THREAD_COMPLETE) {
		thread->stage = THREAD_QUITTING;
		thread_condition_deinit(&thread->condition);
	}
}

void thread_resume(thread_passing_object *thread) {
	autolock(thread_get_event_lock(&thread->condition));
	if (thread->stage == THREAD_INIT || thread->stage == THREAD_PAUSE) {
		thread->stage = THREAD_RUNNING;
		thread_fire_event(&thread->condition);
	}
}

static DECALRE_DISPOSE(_destroy_thread, thread_passing_object) {
	debug_print(KBURN_LOG_DEBUG, COLOR_FMT("[thread]") " %s stage: %s", COLOR_ARG(YELLOW), context->debug_title, stage_name(context->stage));

	switch (context->stage) {
	case THREAD_INIT:
	case THREAD_PAUSE:
	case THREAD_RUNNING:
		thread_tell_quit(context);
		pthread_join(context->thread, NULL);
		break;
	case THREAD_QUITTING:
		pthread_join(context->thread, NULL);
		break;
	case THREAD_COMPLETE:
		break;
	default:
		m_abort("invalid thread state");
	}

	free(context);
}
DECALRE_DISPOSE_END()

void thread_destroy(KBCTX scope, thread_passing_object *thread) {
	if (!thread) {
		return;
	}
	_destroy_thread(scope->threads, thread);
}

__thread thread_passing_object *__current_thread_object = NULL;
thread_condition_t *thread_get_condition(kbthread thread) {
	m_assert(thread->stage == THREAD_RUNNING, "thread not start, call thread_resume first.");
	return &thread->condition;
}
thread_condition_t *get_current_thread_condition() {
	m_assert_ptr(__current_thread_object, "not call from thread");
	m_assert(__current_thread_object->stage == THREAD_RUNNING, "thread not even start?!");
	return thread_get_condition(__current_thread_object);
}

static void *start_routine_wrapper(void *_ctx) {
	thread_passing_object *context = _ctx;
	__current_thread_object = context;

	kb_mutex_t rawlock = thread_get_event_lock(&context->condition);
	pthread_cond_t *rawcond = thread_get_event_condition(&context->condition);

	if (context->stage == THREAD_QUITTING) {
		debug_print(KBURN_LOG_WARN, "[thread] \"%s\" quit before start.", context->debug_title);
		pthread_exit(NULL);
	}

	lock(rawlock);
	if (context->stage == THREAD_INIT) {
		context->stage = THREAD_PAUSE;
	}

#ifdef APPLE
	pthread_setname_np(context->debug_title);
#else
	pthread_setname_np(context->thread, context->debug_title);
#endif

	while (context->stage == THREAD_PAUSE) {
		debug_print(KBURN_LOG_INFO, "[thread] \"%s\" wait start", context->debug_title);
		pthread_cond_wait(rawcond, raw_lock(rawlock));
	}

	unlock(rawlock);

	debug_print(KBURN_LOG_INFO, "[thread] \"%s\" start", context->debug_title);
	context->main(context->user_data, context->scope, &context->quit_signal);
	debug_print(KBURN_LOG_INFO, "[thread] \"%s\" finished", context->debug_title);

	if (context->stage == THREAD_QUITTING) {
		context->stage = THREAD_COMPLETE;
	} else {
		lock(rawlock);
		context->stage = THREAD_COMPLETE;
		unlock(rawlock);
		thread_condition_deinit(&context->condition);
	}

	pthread_exit(NULL);
	return NULL;
}

kburn_err_t thread_create(const char *debug_title, thread_function start_routine, void *context, KBCTX scope, thread_passing_object **out_thread) {
	thread_passing_object *thread = MyAlloc(thread_passing_object);

	if (debug_title) {
		thread->debug_title = debug_title;
	} else {
		thread->debug_title = "<NULL>";
	}

	thread->scope = scope;
	thread->main = start_routine;
	thread->user_data = context;
	if (out_thread == NULL) {
		thread->stage = THREAD_RUNNING;
	}

	thread->condition = thread_condition_init();

	int thread_ret = pthread_create(&thread->thread, NULL, start_routine_wrapper, thread);
	if (thread_ret != 0) {
		debug_print(KBURN_LOG_ERROR, "[thread] failed create: %d", thread_ret);
		return make_error_code(KBURN_ERROR_KIND_SYSCALL, thread_ret);
	}

	dispose_list_add(scope->threads, toDisposable(_destroy_thread, thread));

	if (out_thread != NULL) {
		*out_thread = thread;
		dispose_list_add(scope->threads, toDisposable(unset_pointer, (void **)out_thread));
	}

	return KBurnNoErr;
}
