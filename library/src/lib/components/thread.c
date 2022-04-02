#include "thread.h"
#include "basic/errors.h"
#include "context.h"
#include "debug/print.h"
#include "global.h"
#include <pthread.h>

typedef struct thread_passing_object {
	KBCTX scope;
	bool running;
	bool quit;
	pthread_t thread;
	thread_function main;
	void *user_data;
	const char *debug_title;
} thread_passing_object;

void thread_tell_quit(thread_passing_object *thread) { thread->quit = true; }

DECALRE_DISPOSE(destroy_thread, thread_passing_object) {
	debug_print(KBURN_LOG_DEBUG, COLOR_FMT("[thread]") " wait quit: %s", COLOR_ARG(YELLOW, context->debug_title));
	thread_tell_quit(context);
	pthread_join(context->thread, NULL);
	debug_print(KBURN_LOG_INFO, COLOR_FMT("[thread]") " %s quit success", COLOR_ARG(GREEN, context->debug_title));
	free(context);
}
DECALRE_DISPOSE_END()

static void *start_routine_wrapper(void *_ctx) {
	thread_passing_object *context = _ctx;
	context->running = true;

	debug_print(KBURN_LOG_INFO, "[thread] \"%s\" start", context->debug_title);
	do_sleep(1000);

#ifdef APPLE
	pthread_setname_np(context->debug_title);
#else
	pthread_setname_np(context->thread, context->debug_title);
#endif

	context->quit = false;
	context->main(context->user_data, context->scope, &context->quit);
	debug_print(KBURN_LOG_INFO, "[thread] \"%s\" finished", context->debug_title);

	context->running = false;

	pthread_exit(NULL);
	return NULL;
}

static DECALRE_DISPOSE(set_null, thread_passing_object *) { *context = NULL; }
DECALRE_DISPOSE_END()

kburn_err_t thread_create(const char *debug_title, thread_function start_routine, void *context, KBCTX scope, thread_passing_object **out_thread) {
	thread_passing_object *thread = MyAlloc(thread_passing_object);
	*out_thread = thread;

	if (debug_title)
		thread->debug_title = debug_title;
	else
		thread->debug_title = "<NULL>";

	thread->scope = scope;
	thread->main = start_routine;
	thread->user_data = context;

	int thread_ret = pthread_create(&thread->thread, NULL, start_routine_wrapper, thread);
	if (thread_ret != 0) {
		debug_print(KBURN_LOG_ERROR, "[thread] failed create: %d", thread_ret);
		return make_error_code(KBURN_ERROR_KIND_SYSCALL, thread_ret);
	}

	dispose_list_add(scope->threads, toDisposable(destroy_thread, thread));
	dispose_list_add(scope->threads, toDisposable(set_null, out_thread));

	return KBurnNoErr;
}
