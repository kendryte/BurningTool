#pragma once

#include "context.h"
#include "basic/disposable.h"
#include "thread-condition.h"
#include "canaan-burn/exported/debug.h"
#include <stdbool.h>

typedef void (*thread_function)(void *context, KBCTX scope, const bool *const quit);
typedef struct thread_passing_object *kbthread;
kburn_err_t thread_create(const char *debug_title, thread_function start_routine, void *context, KBCTX scope, kbthread *out_thread);
void thread_destroy(KBCTX scope, kbthread thread);
void thread_tell_quit(kbthread thread);
void thread_resume(kbthread thread);

thread_condition_t *thread_get_condition(kbthread thread);
thread_condition_t *get_current_thread_condition();
#define current_thread_wait_event(test, handle, context) thread_wait_event(get_current_thread_condition(), test, handle, context)
