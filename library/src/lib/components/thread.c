#include "thread.h"
#include "context.h"
#include "basic/errors.h"
#include "basic/sleep.h"
#include "thread-condition.h"
#include <pthread.h>
#include "debug/print.h"

typedef struct thread_passing_object {
	KBCTX scope;
	bool running;
	bool quit;
	pthread_t thread;
	thread_function main;
	void *user_data;
	const char *debug_title;

	bool started;
	bool break_init_wait;

	thread_condition_t condition;
} thread_passing_object;

void thread_tell_quit(thread_passing_object *thread) {
	thread->quit = true;
	thread_resume(thread);
}

static DECALRE_DISPOSE(_destroy_thread, thread_passing_object) {
	debug_print(KBURN_LOG_DEBUG, COLOR_FMT("[thread]") " wait quit: %s", COLOR_ARG(YELLOW), context->debug_title);
	thread_tell_quit(context);
	thread_condition_deinit(&context->condition);
	pthread_join(context->thread, NULL);
	debug_print(KBURN_LOG_INFO, COLOR_FMT("[thread]") " %s quit success", COLOR_ARG(GREEN), context->debug_title);
	free(context);
}
DECALRE_DISPOSE_END()

void thread_destroy(KBCTX scope, thread_passing_object *thread) {
	if (!thread) {
		return;
	}
	if (!thread->running) {
		return;
	}
	if (thread->quit) {
		pthread_join(thread->thread, NULL);
		return;
	}
	_destroy_thread(scope->threads, thread);
}

static inline bool is_thread_started(thread_passing_object *context) {
	return context->started;
}
static inline void set_started_to_true(thread_passing_object *context) {
	context->break_init_wait = true;
}

__thread thread_passing_object *__current_thread_object = NULL;
thread_condition_t *thread_get_condition(kbthread thread) {
	m_assert(thread->started, "thread not even start, call thread_resume first!");
	return &thread->condition;
}
thread_condition_t *get_current_thread_condition() {
	m_assert_ptr(__current_thread_object, "not call from thread");
	return thread_get_condition(__current_thread_object);
}

static void *start_routine_wrapper(void *_ctx) {
	thread_passing_object *context = _ctx;
	__current_thread_object = context;
	context->running = true;
	context->quit = false;

#ifdef APPLE
	pthread_setname_np(context->debug_title);
#else
	pthread_setname_np(context->thread, context->debug_title);
#endif

	while (!context->break_init_wait) {
		thread_wait_event(&context->condition, is_thread_started, set_started_to_true, context);
		if (context->quit) {
			debug_print(KBURN_LOG_WARN, "[thread] \"%s\" quit before start.", context->debug_title);
			return NULL;
		}
	}

	debug_print(KBURN_LOG_INFO, "[thread] \"%s\" start", context->debug_title);
	context->main(context->user_data, context->scope, &context->quit);
	debug_print(KBURN_LOG_INFO, "[thread] \"%s\" finished", context->debug_title);

	context->running = false;

	pthread_exit(NULL);
	return NULL;
}

void thread_resume(thread_passing_object *thread) {
	thread->started = true;
	thread_fire_event(&thread->condition);
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

	int thread_ret = pthread_create(&thread->thread, NULL, start_routine_wrapper, thread);
	if (thread_ret != 0) {
		debug_print(KBURN_LOG_ERROR, "[thread] failed create: %d", thread_ret);
		return make_error_code(KBURN_ERROR_KIND_SYSCALL, thread_ret);
	}

	thread->condition = thread_condition_init();

	dispose_list_add(scope->threads, toDisposable(_destroy_thread, thread));

	if (out_thread != NULL) {
		*out_thread = thread;
		dispose_list_add(scope->threads, toDisposable(unset_pointer, (void **)out_thread));
	} else {
		thread_resume(thread);
	}

	return KBurnNoErr;
}
