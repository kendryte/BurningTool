#define _GNU_SOURCE
#include <pthread.h>
#include "global.h"
#include "thread.h"

typedef struct thread_passing_object
{
	KBCTX scope;
	bool running;
	bool quit;
	pthread_t thread;
	thread_function main;
	const char *debug_title;
} thread_passing_object;

static void thread_tell_quit(thread_passing_object *thread)
{
	thread->quit = true;
}

DECALRE_DISPOSE(destroy_thread, thread_passing_object)
{
	debug_print(YELLO("[thread]") " wait quit: %s", context->debug_title);
	thread_tell_quit(context);
	pthread_join(context->thread, NULL);
	debug_print(GREEN("[thread]") " %s quit success", context->debug_title);
	free(context);
}
DECALRE_DISPOSE_END()

static void *start_routine_wrapper(void *_ctx)
{
	thread_passing_object *context = _ctx;
	context->running = true;

	debug_print("[thread] \"%s\" start", context->debug_title);
	do_sleep(1000);

#ifdef APPLE
	pthread_setname_np(context->debug_title);
#else
	pthread_setname_np(context->thread, context->debug_title);
#endif

	context->quit = false;
	context->main(context->scope, &context->quit);
	debug_print("[thread] \"%s\" finished", context->debug_title);

	context->running = false;

	pthread_exit(NULL);
	return NULL;
}

static DECALRE_DISPOSE(set_null, thread_passing_object *)
{
	*context = NULL;
}
DECALRE_DISPOSE_END()

kburn_err_t thread_create(const char *debug_title, thread_function start_routine, KBCTX scope, thread_passing_object **out_thread)
{
	thread_passing_object *thread = MyAlloc(thread_passing_object);
	*out_thread = thread;

	if (debug_title)
		thread->debug_title = debug_title;
	else
		thread->debug_title = "<NULL>";

	thread->scope = scope;
	thread->main = start_routine;

	int thread_ret = pthread_create(&thread->thread, NULL, start_routine_wrapper, thread);
	if (thread_ret != 0)
	{
		debug_print("[thread] failed create: %d", thread_ret);
		return make_error_code(KBURN_ERROR_KIND_SYSCALL, thread_ret);
	}

	dispose_list_add(scope->threads, toDisposable(destroy_thread, thread));
	dispose_list_add(scope->threads, toDisposable(set_null, out_thread));

	return KBurnNoErr;
}
