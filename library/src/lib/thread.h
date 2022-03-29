#pragma once

#include <stdbool.h>
#include "context.h"
#include "errors.h"

typedef void (*thread_function)(KBCTX scope, const bool *const quit);
typedef struct thread_passing_object *kbthread;
kburn_err_t thread_create(const char *debug_title, thread_function start_routine, KBCTX scope, kbthread *out_thread);
