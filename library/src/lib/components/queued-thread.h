#pragma once

#include "context.h"
#include "basic/event-queue.h"
#include "thread.h"
#include <pthread.h>

typedef struct event_queue_thread *event_queue_thread_t;
typedef void (*event_handler)(KBCTX scope, void *event);

kburn_err_t event_thread_init(KBCTX scope, const char *title, event_handler handler, event_queue_thread_t *out);
kburn_err_t event_thread_queue(event_queue_thread_t thread, void *event, bool auto_fire);
void event_thread_fire(event_queue_thread_t thread);
void event_thread_deinit(KBCTX scope, event_queue_thread_t *thread);
