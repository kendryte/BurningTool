#define _GNU_SOURCE
#include "global.h"
#include <pthread.h>

typedef struct thread_passing_object
{
	KBCTX scope;
	bool running;
	bool quit;
	pthread_t thread;
	thread_function main;
	const char *debug_title;
} thread_passing_object;

DECALRE_DISPOSE(destroy_thread, thread_passing_object)
{
	context->quit = true;
	if (context->running)
		pthread_cancel(context->thread);

	free(context);
}
DECALRE_DISPOSE_END()

static void *start_routine_wrapper(void *_ctx)
{
	thread_passing_object *context = _ctx;
	context->running = true;

	debug_print("[thread] \"%s\" start", context->debug_title);

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

kburn_err_t thread_create(const char *debug_title, thread_function start_routine, KBCTX scope, thread_passing_object **out_thread)
{
	thread_passing_object *thread = KBALLOC(thread_passing_object);
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
		return KBURN_ERROR_KIND_SYSCALL | thread_ret;
	}

	dispose_add(scope->disposables, disposable(destroy_thread, thread));

	return KBurnNoErr;
}

void thread_tell_quit(thread_passing_object *thread)
{
	thread->quit = true;
}

void thread_wait_quit(thread_passing_object *thread)
{
	debug_print("[thread] wait quit: %s", thread->debug_title);
	thread_tell_quit(thread);
	pthread_join(thread->thread, NULL);
}
