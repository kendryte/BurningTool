#pragma once

#include "basic/disposable.h"
#include "context.h"
#include "errors.h"
#include <stdbool.h>

typedef void (*thread_function)(void *context, KBCTX scope, const bool *const quit);
typedef struct thread_passing_object *kbthread;
kburn_err_t thread_create(const char *debug_title, thread_function start_routine, void *context, KBCTX scope, kbthread *out_thread);
DECALRE_DISPOSE_HEADER(destroy_thread, struct thread_passing_object);
void thread_tell_quit(kbthread thread);
